#pragma once

// Planting controls use normalized game coordinates even when LDPlayer is
// resized. Every movement releases the joystick, including an early STOP.
inline cv::Point PlantClientPoint(HWND game, cv::Point point) {
    RECT rect{};
    if (!GetClientRect(game,&rect) || rect.right<=0 || rect.bottom<=0) return {-1,-1};
    return {cvRound(point.x*rect.right/960.),cvRound(point.y*rect.bottom/540.)};
}
inline cv::Mat PlantFrame(ThongTinTool* tool) {
    if (!tool->h_game) return {};
    auto raw=BoDieuKhien::ChupManHinh(tool->h_game);
    if (raw.empty())return {};
    if(std::abs(double(raw.cols)/raw.rows-960./540.)>.05) {
        tool->thongBaoStatus="Trong cay: khung game khong dung ti le 16:9";
        return {};
    }
    if(raw.size()!=cv::Size(960,540))cv::resize(raw,raw,{960,540},0,0,cv::INTER_AREA);
    return raw;
}
inline bool PlantWait(ThongTinTool* tool, int milliseconds) {
    for(int elapsed=0;elapsed<milliseconds;elapsed+=40) {
        if(!tool->dangChay)return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(std::min(40,milliseconds-elapsed)));
    }
    return tool->dangChay;
}
inline void PlantClick(ThongTinTool* tool, cv::Point point) {
    if(!tool->dangChay)return;
    auto actual=PlantClientPoint(tool->h_game,point);
    if(actual.x>=0)BoDieuKhien::BamChuot(tool->h_game,actual.x,actual.y);
}
inline void PlantMove(ThongTinTool* tool, cv::Point2d direction, int milliseconds) {
    double length=cv::norm(direction);
    if(length<.01 || !tool->dangChay)return;
    auto center=PlantClientPoint(tool->h_game,{152,403});
    auto destination=PlantClientPoint(tool->h_game,{cvRound(152+direction.x/length*57),cvRound(403+direction.y/length*57)});
    if(center.x<0 || destination.x<0)return;
    HWND game=tool->h_game;
    auto post=[&](UINT message,WPARAM flags,cv::Point p){PostMessageW(game,message,flags,MAKELPARAM(p.x,p.y));};
    post(WM_MOUSEMOVE,0,center);
    post(WM_LBUTTONDOWN,MK_LBUTTON,center);
    PlantWait(tool,60);
    for(int elapsed=0;elapsed<milliseconds && tool->dangChay;elapsed+=40) {
        post(WM_MOUSEMOVE,MK_LBUTTON,destination);
        PlantWait(tool,std::min(40,milliseconds-elapsed));
    }
    post(WM_LBUTTONUP,0,destination);
    post(WM_MOUSEMOVE,0,center);
    PlantWait(tool,300);
}
inline void PlantSwipe(ThongTinTool* tool,bool downward) {
    cv::Point begin(850,downward?440:205),end(850,downward?205:440);
    auto start=PlantClientPoint(tool->h_game,begin);
    if(start.x<0 || !tool->dangChay)return;
    PostMessageW(tool->h_game,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(start.x,start.y));
    for(int step=1;step<=16 && tool->dangChay;++step) {
        auto p=PlantClientPoint(tool->h_game,begin+(end-begin)*step/16);
        PostMessageW(tool->h_game,WM_MOUSEMOVE,MK_LBUTTON,MAKELPARAM(p.x,p.y));
        PlantWait(tool,22);
    }
    auto finish=PlantClientPoint(tool->h_game,end);
    PostMessageW(tool->h_game,WM_LBUTTONUP,0,MAKELPARAM(finish.x,finish.y));
    PlantWait(tool,350);
}
inline const std::vector<cv::Mat>& PlantGlyphs() {
    static const auto glyphs=PlantLoadGlyphs("images");
    return glyphs;
}
inline bool PlantIsBag(const cv::Mat& frame) {
    return FarmHasImage(frame,"plant_bag_header.png",{790,0,120,80},.85);
}
inline int PlantHeldSeed(const cv::Mat& frame) {
    if(frame.empty() || PlantIsBag(frame))return -1;
    return PlantIdentifySeed(frame,{730,292,65,54},PlantGlyphs(),.91,.05).index;
}
inline int PlantSeedCount(const cv::Mat& frame) {
    if(frame.empty())return -1;
    cv::Mat white, readable;
    cv::inRange(frame(cv::Rect(728,335,77,30)),cv::Scalar(195,195,195),cv::Scalar(255,255,255),white);
    cv::bitwise_not(white,white);
    cv::copyMakeBorder(white,white,12,12,12,12,cv::BORDER_CONSTANT,cv::Scalar(255));
    cv::cvtColor(white,readable,cv::COLOR_GRAY2BGR);
    auto words=FarmReadText(readable,{0,0,readable.cols,readable.rows});
    int count=-1;
    for(const auto& word:words) {
        int digits=0,value=0;
        for(wchar_t c:word.raw)if(c>=L'0' && c<=L'9'){++digits;value=value*10+(c-L'0');}
        if(digits>0 && digits<6)count=value;
    }
    return count;
}
inline void PlantCameraCloser(ThongTinTool* tool) {
    if(!tool->h_cha || !tool->dangChay)return;
    // LDPlayer's built-in F4 pinch avoids the game's right-click mouse lock.
    const LPARAM scan=LPARAM(MapVirtualKeyW(VK_F4,MAPVK_VK_TO_VSC))<<16;
    for(int step=0;step<50 && tool->dangChay;++step) {
        PostMessageW(tool->h_cha,WM_KEYDOWN,VK_F4,scan|1);
        PlantWait(tool,30);
        PostMessageW(tool->h_cha,WM_KEYUP,VK_F4,scan|LPARAM(0xc0000001));
        PlantWait(tool,70);
    }
    PlantWait(tool,400);
}
inline void PlantCloseBag(ThongTinTool* tool) {
    if(tool->dangChay&&PlantIsBag(PlantFrame(tool))){PlantClick(tool,{923,35});PlantWait(tool,300);}
}
inline double PlantBagDifference(const cv::Mat& a,const cv::Mat& b) {
    if(a.empty()||b.empty()||a.size()!=b.size())return 1e6;
    cv::Rect area(35,145,890,365);
    return cv::norm(a(area),b(area),cv::NORM_L1)/(area.area()*3.);
}
inline bool PlantWaitBagStable(ThongTinTool* tool) {
    // Packet images load after the panel opens; swipe bounce also moves cards.
    // Read only after consecutive settled frames, including a minimum pause.
    if(!PlantWait(tool,350))return false;
    auto before=PlantFrame(tool);int stable=0;
    for(int sample=0;sample<12&&tool->dangChay;++sample) {
        if(!PlantWait(tool,150))return false;
        auto after=PlantFrame(tool);
        if(!PlantIsBag(after))return false;
        stable=PlantBagDifference(before,after)<.15?stable+1:0;
        if(stable>=3)return true;
        before=after;
    }
    return false;
}
inline bool PlantOpenSeedBag(ThongTinTool* tool) {
    auto frame=PlantFrame(tool);
    if(frame.empty())return false;
    if(!PlantIsBag(frame)) {
        PlantClick(tool,{920,303});
        if(!PlantWait(tool,550))return false;
        frame=PlantFrame(tool);
    }
    if(!PlantIsBag(frame))return false;
    PlantClick(tool,{855,33});if(!PlantWait(tool,250))return false;
    PlantClick(tool,{180,105});if(!PlantWait(tool,350))return false;
    if(!PlantWaitBagStable(tool))return false;
    // Stop resetting at the top instead of blindly performing five swipes.
    for(int up=0;up<16&&tool->dangChay;++up) {
        auto before=PlantFrame(tool);
        PlantSwipe(tool,false);
        if(!PlantWaitBagStable(tool))return false;
        auto after=PlantFrame(tool);
        if(!PlantIsBag(after))return false;
        if(PlantBagDifference(before,after)<1.5)return true;
    }
    return false;
}
inline int PlantReadBagPage(const cv::Mat& frame,int page,PlantInventorySnapshot& inventory,bool replace=false) {
    int unread=0;
    for(auto card:PlantBagCards(frame)) {
        auto identity=PlantIdentifySeed(frame,{card.x+18,card.y+55,card.width-36,76},PlantGlyphs());
        unread+=identity.index<0;
        PlantInventoryRemember(inventory,page,card,identity,replace);
    }
    return unread;
}
inline bool PlantScanInventory(ThongTinTool* tool,PlantInventorySnapshot& result) {
    tool->thongBaoStatus="Quet balo mot luot: nho tat ca cac hat";
    if(!PlantOpenSeedBag(tool))return false;
    for(int page=0;page<16&&tool->dangChay;++page) {
        auto frame=PlantFrame(tool);
        if(!PlantIsBag(frame))break;
        if(PlantBagCards(frame).empty()) {
            if(page==0&&PlantWait(tool,300)) {
                auto empty=PlantFrame(tool);
                if(PlantIsBag(empty)&&PlantBagCards(empty).empty()&&PlantBagDifference(frame,empty)<1.5){result.complete=true;return true;}
            }
            break;
        }
        tool->thongBaoStatus="Quet balo mot luot: trang "+std::to_string(page+1);
        if(PlantReadBagPage(frame,page,result)>0) {
            // Retry unread packets on this page, without another full bag scan.
            if(!PlantWaitBagStable(tool))return false;
            frame=PlantFrame(tool);
            PlantReadBagPage(frame,page,result);
        }
        PlantSwipe(tool,true);
        if(!PlantWaitBagStable(tool))return false;
        auto after=PlantFrame(tool);
        if(!PlantIsBag(after))break;
        if(PlantBagDifference(frame,after)<1.5){result.complete=true;return true;}
    }
    PlantCloseBag(tool);
    tool->thongBaoStatus="Chua quet het balo; khong dung du lieu thieu";
    return false;
}
inline bool PlantEnsureInventory(ThongTinTool* tool) {
    return PlantInventoryEnsure(tool->hatTrongDaNho,reinterpret_cast<std::uintptr_t>(tool->h_game),
        [&](PlantInventorySnapshot& next){return PlantScanInventory(tool,next);});
}
inline bool PlantEquipSeed(ThongTinTool* tool,int index) {
    if(index<0||index>=SO_HAT_TRONG||!tool->dangChay)return false;
    if(!PlantEnsureInventory(tool))return false;
    // Missing seeds are remembered too: no bag-opening or per-seed search.
    if(!tool->hatTrongDaNho.Has(index))return false;
    auto frame=PlantFrame(tool);
    if(PlantHeldSeed(frame)==index)return true;
    for(int refresh=0;refresh<2&&tool->dangChay;++refresh) {
        auto slot=tool->hatTrongDaNho.snapshot.seeds[index];
        if(!slot.present)return false;
        if(!PlantOpenSeedBag(tool))return false;
        for(int page=0;page<slot.page&&tool->dangChay;++page)PlantSwipe(tool,true);
        if(!PlantWaitBagStable(tool))return false;
        frame=PlantFrame(tool);
        if(!PlantIsBag(frame))return false;
        auto identity=PlantIdentifySeed(frame,{slot.card.x+18,slot.card.y+55,slot.card.width-36,76},PlantGlyphs());
        if(identity.index!=index) {
            // Exhausted packets compact the grid. Repair the visible page once.
            PlantReadBagPage(frame,slot.page,tool->hatTrongDaNho.snapshot,true);
            auto moved=tool->hatTrongDaNho.snapshot.seeds[index];
            identity=PlantIdentifySeed(frame,{moved.card.x+18,moved.card.y+55,moved.card.width-36,76},PlantGlyphs());
        }
        if(identity.index==index) {
            PlantClick(tool,identity.point);
            for(int attempt=0;attempt<8&&tool->dangChay;++attempt) {
                if(!PlantWait(tool,200))return false;
                if(PlantHeldSeed(PlantFrame(tool))==index)return true;
            }
            PlantCloseBag(tool);
            tool->thongBaoStatus="Khong xac nhan duoc hat dang cam: "+std::string(ds_hat_trong[index]);
            return false;
        }
        // An externally changed bag or a page boundary invalidates the whole
        // snapshot once; it is then reused for every remaining selected seed.
        if(refresh==0) {
            tool->hatTrongDaNho.Invalidate();
            if(!PlantEnsureInventory(tool)||!tool->hatTrongDaNho.Has(index)){PlantCloseBag(tool);return false;}
        }
    }
    PlantCloseBag(tool);
    return false;
}

inline bool PlantGoHome(ThongTinTool* tool) {
    auto frame=PlantFrame(tool);
    if(frame.empty())return false;
    if(PlantIsBag(frame)){PlantClick(tool,{923,35});PlantWait(tool,400);frame=PlantFrame(tool);}
    for(int close=0;close<8&&tool->dangChay;++close) {
        cv::Point target(-1,-1);
        if(FarmIsHarvest(frame))target={921,35};
        else if(FarmHasImage(frame,"shop_header.png",{80,25,230,70},.8))target={844,55};
        else if(FarmHasImage(frame,"tieu_de_ban_moi.png",{15,5,400,70},.8))target={915,38};
        else if(FarmHasImage(frame,"gio_hang_mua.png",{610,180,335,190},.8)
             ||FarmHasImage(frame,"npc_mua_cong_cu_moi.png",{610,180,335,190},.8)
             ||FarmHasImage(frame,"npc_ban_nongsan_moi.png",{610,180,335,190},.8))target={780,348};
        else if(FarmHasImage(frame,"npc_dialog.png",{195,390,65,100},.82))target={500,450};
        if(target.x<0)break;
        PlantClick(tool,target);PlantWait(tool,600);frame=PlantFrame(tool);
    }
    tool->thongBaoStatus="Trong cay: den nong trai cua minh";
    for(int attempt=0;attempt<8&&tool->dangChay;++attempt) {
        auto shortcut=BoDieuKhien::TimAnhTrongVung(frame,"plant_nha_ta.png",{595,35,145,90},.87);
        if(shortcut.x>=0) {
            PlantClick(tool,shortcut);
            PlantWait(tool,1200);
            for(int wait=0;wait<20&&tool->dangChay;++wait) {
                frame=PlantFrame(tool);
                if(FarmHasImage(frame,"plant_house_marker.png",{170,100,470,300},.82))return true;
                PlantWait(tool,400);
            }
            return false;
        }
        PlantClick(tool,{925,25});PlantWait(tool,450);frame=PlantFrame(tool);
    }
    return false;
}

inline bool PlantClickConfirmed(ThongTinTool* tool,const cv::Mat& before,PlantRing ring,int seed) {
    int oldCount=PlantSeedCount(before);
    auto oldMask=PlantCountMask(before);
    cv::Mat previousMask;
    PlantClick(tool,{763,325});
    cv::Mat after;
    // During the planting animation the action button and its white count fade.
    // Wait for the count to become legible again before declaring failure.
    for(int attempt=0;attempt<10&&tool->dangChay;++attempt) {
        if(!PlantWait(tool,300))return false;
        after=PlantFrame(tool);
        if(after.empty()||PlantIsBag(after))return false;
        int newCount=PlantSeedCount(after);
        if(oldCount>0&&newCount>=0&&newCount<oldCount)return true;
        if(oldCount==1&&PlantHeldSeed(after)!=seed)return true;
        auto newMask=PlantCountMask(after);
        // Windows OCR can omit short counts such as x11. Compare only the
        // stationary white count glyphs after the button's fade has finished.
        if(attempt>=5&&!previousMask.empty()&&!newMask.empty()
           &&cv::norm(previousMask,newMask,cv::NORM_L1)<8*255
           &&PlantHeldSeed(after)==seed&&PlantCountChanged(oldMask,newMask))return true;
        previousMask=newMask;
    }
    if(after.empty())return false;
    auto nextRing=PlantFindValidRing(after);
    cv::Rect area(ring.center.x-22,ring.center.y-22,44,44);
    area&=cv::Rect(0,0,after.cols,after.rows);
    if(area.empty())return false;
    cv::Mat difference,oldHsv,newHsv,oldGreen,newGreen;
    cv::absdiff(before(area),after(area),difference);
    cv::cvtColor(before(area),oldHsv,cv::COLOR_BGR2HSV);
    cv::cvtColor(after(area),newHsv,cv::COLOR_BGR2HSV);
    cv::inRange(oldHsv,cv::Scalar(28,65,50),cv::Scalar(82,255,255),oldGreen);
    cv::inRange(newHsv,cv::Scalar(28,65,50),cv::Scalar(82,255,255),newGreen);
    return nextRing.center.x<0&&cv::mean(difference)[1]>10
        &&cv::countNonZero(newGreen)>cv::countNonZero(oldGreen)+12;
}

inline void FarmPlantSelected(ThongTinTool* tool,int maxPlants=40) {
    bool any=false;for(bool chosen:tool->cacHatCanTrong)any|=chosen;
    if(!any){tool->thongBaoStatus="Trong cay: chua chon hat de trong";return;}
    if(tool->hatTrongDaNho.valid&&tool->hatTrongDaNho.gameKey==reinterpret_cast<std::uintptr_t>(tool->h_game)) {
        bool available=false;int missing=0;
        for(int seed=0;seed<SO_HAT_TRONG;++seed)if(tool->cacHatCanTrong[seed]){available|=tool->hatTrongDaNho.Has(seed);missing+=!tool->hatTrongDaNho.Has(seed);}
        if(!available){tool->thongBaoStatus="Balo da nho: bo qua "+std::to_string(missing)+" loai hat khong co";tool->henTrongCay=GetTickCount64()+60000;return;}
    }
    if(!tool->dangChay||!PlantGoHome(tool)){if(tool->dangChay)tool->thongBaoStatus="Trong cay: chua xac nhan den nong trai";return;}
    if(!PlantEnsureInventory(tool)) {
        if(tool->dangChay){tool->thongBaoStatus="Trong cay: chua quet duoc balo";tool->henTrongCay=GetTickCount64()+60000;}
        return;
    }
    PlantCloseBag(tool);
    PlantCameraCloser(tool);
    tool->soCayVuaTrong=0;
    int skipped=0,moves=0,entrySteps=0;
    std::vector<cv::Point2f> visited;
    auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(200);
    double speed=55;
    bool noSoil=false;
    for(int seed=0;seed<SO_HAT_TRONG&&tool->dangChay&&tool->soCayVuaTrong<maxPlants;++seed) {
        if(!tool->cacHatCanTrong[seed])continue;
        if(std::chrono::steady_clock::now()>deadline)break;
        tool->thongBaoStatus="Lay hat da nho: "+std::string(ds_hat_trong[seed]);
        if(!PlantEquipSeed(tool,seed)){++skipped;continue;}
        int stalled=0;
        while(tool->dangChay&&tool->soCayVuaTrong<maxPlants&&moves<140&&std::chrono::steady_clock::now()<deadline) {
            auto before=PlantFrame(tool);
            if(before.empty()||PlantHeldSeed(before)!=seed)break;
            auto ring=PlantFindValidRing(before);
            if(ring.center.x>=0) {
                PlantWait(tool,180);
                auto stable=PlantFrame(tool);auto again=PlantFindValidRing(stable);
                if(again.center.x<0||cv::norm(again.center-ring.center)>5)continue;
                tool->thongBaoStatus="Trong "+std::string(ds_hat_trong[seed])+": vong xanh hop le";
                bool confirmed=PlantClickConfirmed(tool,stable,again,seed);
                visited.push_back(cv::Point2f(again.center));
                if(confirmed)++tool->soCayVuaTrong;
                else if(tool->dangChay){tool->thongBaoStatus="Khong xac nhan duoc thao tac trong; dung luot de tranh bam lap";tool->henTrongCay=GetTickCount64()+60000;return;}
                if(PlantHeldSeed(PlantFrame(tool))!=seed){tool->hatTrongDaNho.Exhausted(seed);break;}
                before=PlantFrame(tool);
            }
            auto destination=PlantNextSoilSpot(before,visited);
            if(destination.x<0){
                // Home arrives on the pavement below the plots. Walk up the
                // observed center lane in short steps until the soil is visible.
                if(tool->soCayVuaTrong==0 && entrySteps<3) {
                    tool->thongBaoStatus="Trong cay: di tu cong vao luong dat";
                    PlantMove(tool,{0,-1},900);++entrySteps;++moves;continue;
                }
                noSoil=true;break;
            }
            cv::Point2d delta=destination-cv::Point(480,285);
            double distance=cv::norm(delta);
            if(distance<20){visited.push_back(cv::Point2f(destination));continue;}
            int duration=std::clamp(cvRound(distance/speed*1000),250,1100);
            tool->thongBaoStatus="Tim cho trong: di den o dat; da trong "+std::to_string(tool->soCayVuaTrong)+" cay";
            PlantMove(tool,delta,duration);++moves;
            auto after=PlantFrame(tool);cv::Mat transform;
            if(PlantTrackCamera(before,after,transform)) {
                PlantTransformSpots(visited,transform);
                double motion=std::hypot(transform.at<double>(0,2),transform.at<double>(1,2));
                if(motion>1.5){speed=.6*speed+.4*std::clamp(motion/(duration/1000.),15.,200.);stalled=0;}
                else {visited.push_back(cv::Point2f(destination));++stalled;}
            } else {
                // Bare soil can have too few corners for optical flow. A changed
                // ground image still proves movement, without guessing a transform.
                cv::Rect ground(315,100,270,290);
                double change=after.empty()?0:cv::norm(before(ground),after(ground),cv::NORM_L1)/(ground.area()*3.);
                if(change>2){visited.clear();stalled=0;}
                else ++stalled;
            }
            if(stalled>=4){tool->thongBaoStatus="Trong cay: khong thay di chuyen on dinh; hay kiem tra duong di";tool->henTrongCay=GetTickCount64()+60000;return;}
        }
        if(noSoil)break;
    }
    tool->henTrongCay=GetTickCount64()+60000;
    tool->thongBaoStatus="Trong cay: "+std::to_string(tool->soCayVuaTrong)+" cay; bo qua "+std::to_string(skipped)+" loai hat khong co/khong nhan dien duoc";
    if(noSoil)tool->thongBaoStatus+="; khong con diem dat trong tam nhin";
}
