#include "farm_bag_vision.h"
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
    int count=-1;std::wstring raw;
    for(const auto& word:words) {
        for(wchar_t c:word.raw)if(!iswspace(c))raw+=c;
    }
    if(!raw.empty()&&(raw[0]==L'x'||raw[0]==L'X'||raw[0]==L'×'))raw.erase(raw.begin());
    bool numeric=!raw.empty()&&raw.size()<6;int value=0;
    for(auto c:raw)if(c>=L'0'&&c<=L'9')value=value*10+(c-L'0');else numeric=false;
    if(numeric)count=value;
    if(count<0) {
        // OCR omits "x1" (observed as letters). Recognize only the verified
        // one-seed glyph; other quantities still use OCR or the change check.
        static const auto one=cv::imread("images/plant_count_one.png",cv::IMREAD_GRAYSCALE);
        if(PlantCountIsOne(PlantCountMask(frame),one))count=1;
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
    if(!PlantWait(tool,250))return false;
    auto before=PlantFrame(tool);int stable=0;
    for(int sample=0;sample<12&&tool->dangChay;++sample) {
        if(!PlantWait(tool,150))return false;
        auto after=PlantFrame(tool);
        if(!PlantIsBag(after))return false;
        auto a=PlantBagCards(before),b=PlantBagCards(after);
        bool layout=a.size()==b.size();
        if(layout)for(size_t i=0;i<a.size();++i)if(cv::norm(a[i].tl()-b[i].tl())>2)layout=false;
        stable=layout&&PlantBagDifference(before,after)<2.?stable+1:0;
        if(stable>=2)return true;
        before=after;
    }
    return false;
}
inline bool PlantSeedFilterSelected(const cv::Mat& frame) {
    return PlantBagSeedFilterSelected(frame);
}
inline bool PlantIsBagPanel(const cv::Mat& frame) {
    if(frame.empty())return false;
    cv::Mat hsv,red;cv::cvtColor(frame(cv::Rect(250,5,400,35)),hsv,cv::COLOR_BGR2HSV);
    cv::inRange(hsv,cv::Scalar(0,100,150),cv::Scalar(16,255,255),red);
    return cv::countNonZero(red)>red.total()*.55;
}
inline std::string PlantPageKey(const cv::Mat& frame);
inline void PlantClosePhoneMenu(ThongTinTool* tool) {
    auto frame=PlantFrame(tool);if(frame.empty())return;
    cv::Mat hsv,cyan;cv::cvtColor(frame(cv::Rect(620,450,330,85)),hsv,cv::COLOR_BGR2HSV);
    cv::inRange(hsv,cv::Scalar(75,80,80),cv::Scalar(108,255,255),cyan);
    auto header=cv::mean(frame(cv::Rect(640,20,280,40)));
    if(cv::countNonZero(cyan)>cyan.total()*.45&&header[0]>160&&header[1]>160&&header[2]>160) {
        PlantClick(tool,{900,42});PlantWait(tool,400);
    }
}
inline bool PlantOpenSeedBag(ThongTinTool* tool,bool top=false) {
    PlantClosePhoneMenu(tool);
    auto frame=PlantFrame(tool);
    if(frame.empty())return false;
    if(!PlantIsBag(frame)) {
        PlantClick(tool,{920,303});if(!PlantWait(tool,550))return false;
        frame=PlantFrame(tool);
    }
    if(!PlantIsBag(frame)) {
        if(!PlantIsBagPanel(frame))return false;
        PlantClick(tool,{855,33});if(!PlantWait(tool,300))return false;
        frame=PlantFrame(tool);
    }
    if(!PlantIsBag(frame))return false;
    if(!PlantSeedFilterSelected(frame)) {
        PlantClick(tool,{180,105});if(!PlantWait(tool,350))return false;
        tool->conTroBalo.localized=false;
        if(!PlantSeedFilterSelected(PlantFrame(tool))){tool->thongBaoStatus="Balo: chua mo dung bo loc Hat giong";return false;}
    }
    if(!PlantWaitBagStable(tool))return false;
    if(!top&&tool->conTroBalo.localized) {
        auto key=PlantPageKey(PlantFrame(tool));
        const auto& keys=tool->hatTrongDaNho.snapshot.pageKeys;
        auto found=std::find(keys.begin(),keys.end(),key);
        if(found!=keys.end()){tool->conTroBalo.page=int(found-keys.begin());return true;}
        auto visible=PlantFrame(tool);
        for(const auto& slot:tool->hatTrongDaNho.snapshot.seeds)if(slot.present&&slot.page==tool->conTroBalo.page) {
            auto identity=PlantIdentifySeed(visible,{slot.card.x+18,slot.card.y+55,slot.card.width-36,76},PlantGlyphs());
            if(identity.index>=0&&tool->hatTrongDaNho.snapshot.seeds[identity.index].card==slot.card)return true;
        }
        tool->conTroBalo.localized=false;
    }
    for(int up=0;up<24&&tool->dangChay;++up) {
        auto before=PlantFrame(tool);PlantSwipe(tool,false);
        if(!PlantWaitBagStable(tool))return false;
        auto after=PlantFrame(tool);
        if(PlantBagDifference(before,after)<1.5) {
            tool->conTroBalo={0,true};return true;
        }
    }
    return false;
}
inline PlantSeedIdentity PlantPacketIdentity(const cv::Mat& frame,cv::Rect card,const std::string& name) {return PlantDecodePacketIdentity(frame,card,name,PlantGlyphs());}
inline std::string PlantPageKey(const cv::Mat& frame) {
    std::string key;
    for(auto card:PlantBagCards(frame)) {
        key+=PlantPacketName(frame,card)+"@"+std::to_string(card.y/5)+";";
    }
    return key;
}
inline int PlantReadBagPage(const cv::Mat& frame,int page,PlantInventorySnapshot& inventory,bool replace=false) {
    int unread=0;
    for(auto card:PlantBagCards(frame)) {
        auto name=PlantPacketName(frame,card);
        auto identity=PlantPacketIdentity(frame,card,name);
        unread+=identity.index<0;
        PlantInventoryRemember(inventory,page,card,identity,replace,PlantPacketQuantity(frame,card));
    }
    if(replace&&page>=0&&page<int(inventory.pageKeys.size()))inventory.pageKeys[page]=PlantPageKey(frame);
    return unread;
}
inline void PlantReportInventory(ThongTinTool* tool) {
    int found=0;std::string summary;
    for(int seed=0;seed<SO_HAT_TRONG;++seed)if(tool->hatTrongDaNho.Has(seed)) {
        ++found;if(!summary.empty())summary+="; ";summary+=ds_hat_trong[seed];
        auto count=tool->hatTrongDaNho.snapshot.seeds[seed].quantity;
        summary+="="+(count>=0?std::to_string(count):std::string("?"));
    }
    summary=std::to_string(found)+" loai: "+summary;
    if(tool->hatTrongDaNho.snapshot.unreadCards>0)summary+="; co goi chua doc duoc";
    std::lock_guard<std::mutex> lock(tool->trongThongTinMutex);tool->ketQuaBalo=summary;
}
inline bool PlantScanInventory(ThongTinTool* tool,PlantInventorySnapshot& result) {
    tool->thongBaoStatus="Balo: quet mot luot ten, so luong va vi tri hat";
    if(!PlantOpenSeedBag(tool,true))return false;
    for(int page=0;page<24&&tool->dangChay;++page) {
        auto frame=PlantFrame(tool);if(!PlantIsBag(frame))return false;
        auto key=PlantPageKey(frame);
        if(PlantBagCards(frame).empty()) {result.complete=page==0;return result.complete;}
        tool->conTroBalo={page,true};
        tool->thongBaoStatus="Balo: doc trang "+std::to_string(page+1);
        int unread=PlantReadBagPage(frame,page,result);
        if(unread>0&&PlantWaitBagStable(tool))unread=PlantReadBagPage(PlantFrame(tool),page,result,true);
        result.unreadCards+=unread;result.pageKeys.push_back(key);
        PlantSwipe(tool,true);if(!PlantWaitBagStable(tool))return false;
        auto after=PlantFrame(tool);if(!PlantIsBag(after))return false;
        if(PlantPageKey(after)==key||PlantBagDifference(frame,after)<.75) {result.complete=true;tool->conTroBalo={page,true};return true;}
    }
    tool->thongBaoStatus="Balo: chua doc het danh sach; khong ket luan hat thieu";return false;
}
inline bool PlantEnsureInventory(ThongTinTool* tool) {
    auto frame=PlantFrame(tool);
    if(FarmHasImage(frame,"plant_edit_guard.png",{780,0,180,180},.85)) {
        tool->thongBaoStatus="Hay thoat che do chinh sua vuon truoc khi chay bot";return false;
    }
    bool ready=PlantInventoryEnsure(tool->hatTrongDaNho,reinterpret_cast<std::uintptr_t>(tool->h_game),
        [&](PlantInventorySnapshot& next){return PlantScanInventory(tool,next);});
    if(ready)PlantReportInventory(tool);return ready;
}
inline bool PlantBagGoPage(ThongTinTool* tool,int page) {
    if(page<0||!PlantOpenSeedBag(tool))return false;
    for(int move=0;move<24&&tool->dangChay&&tool->conTroBalo.page!=page;++move) {
        bool down=tool->conTroBalo.page<page;
        auto before=PlantFrame(tool);auto old=PlantPageKey(before);
        PlantSwipe(tool,down);if(!PlantWaitBagStable(tool))return false;
        auto after=PlantFrame(tool);if(PlantPageKey(after)==old)return false;
        tool->conTroBalo.page+=down?1:-1;
    }
    return tool->conTroBalo.page==page;
}
inline bool PlantEquipSeed(ThongTinTool* tool,int index) {
    if(index<0||index>=SO_HAT_TRONG||!tool->dangChay||!PlantEnsureInventory(tool)||!tool->hatTrongDaNho.Has(index))return false;
    if(PlantHeldSeed(PlantFrame(tool))==index)return true;
    for(int refresh=0;refresh<2&&tool->dangChay;++refresh) {
        auto slot=tool->hatTrongDaNho.snapshot.seeds[index];
        if(!slot.present)return false;
        if(PlantBagGoPage(tool,slot.page)) {
            auto frame=PlantFrame(tool);
            PlantReadBagPage(frame,tool->conTroBalo.page,tool->hatTrongDaNho.snapshot,true);
            slot=tool->hatTrongDaNho.snapshot.seeds[index];
            auto identity=PlantPacketIdentity(frame,slot.card,PlantPacketName(frame,slot.card));
            if(identity.index==index) {
                PlantClick(tool,identity.point);
                for(int wait=0;wait<12&&tool->dangChay;++wait) {
                    if(!PlantWait(tool,200))return false;
                    if(PlantHeldSeed(PlantFrame(tool))==index){PlantReportInventory(tool);return true;}
                }
                PlantCloseBag(tool);tool->thongBaoStatus="Balo: chua xac nhan cam dung "+std::string(ds_hat_trong[index]);return false;
            }
        }
        if(refresh==0) {
            tool->hatTrongDaNho.Invalidate();tool->conTroBalo.localized=false;
            if(!PlantEnsureInventory(tool))return false;
        }
    }
    PlantCloseBag(tool);return false;
}

inline bool PlantGoHome(ThongTinTool* tool) {
    PlantClosePhoneMenu(tool);
    auto frame=PlantFrame(tool);
    if(frame.empty())return false;
    if(PlantIsBag(frame)){PlantClick(tool,{923,35});PlantWait(tool,400);frame=PlantFrame(tool);}
    FarmCloseHarvest(tool);FarmExitStore(tool);frame=PlantFrame(tool);
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
                static const auto marker=cv::imread("images/plant_house_marker.png");
                for(int scale=25;scale<=140;scale+=5) {
                    cv::Mat markerScaled;cv::resize(marker,markerScaled,{},scale/100.,scale/100.);
                    if(FarmSaleImage(frame,markerScaled,{80,70,780,390},.84).x>=0)return true;
                }
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
    bool retried=false;
    for(int attempt=0;attempt<16&&tool->dangChay;++attempt) {
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
        // Retry a lost click once only when the same seed/count and a stable
        // valid ring prove that the previous attempt consumed nothing.
        if(attempt>=5&&!retried&&!oldMask.empty()&&!newMask.empty()
           &&cv::norm(oldMask,newMask,cv::NORM_L1)<=3*255
           &&PlantHeldSeed(after)==seed) {
            auto valid=PlantFindValidRing(after);
            if(valid.center.x>=0&&cv::norm(valid.center-ring.center)<=5) {
                if(!PlantWait(tool,250))return false;
                auto settled=PlantFrame(tool);auto settledRing=PlantFindValidRing(settled);
                auto settledMask=PlantCountMask(settled);
                if(settledRing.center.x>=0&&cv::norm(settledRing.center-valid.center)<=5
                   &&!settledMask.empty()&&cv::norm(oldMask,settledMask,cv::NORM_L1)<=3*255) {
                    PlantClick(tool,{763,325});retried=true;
                }
            }
        }
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

#include "farm_garden_runtime.h"

inline void FarmPlantSelected(ThongTinTool* tool,int maxPlants=240) {
    struct Report {ThongTinTool* tool;~Report(){std::lock_guard<std::mutex> lock(tool->trongThongTinMutex);tool->ketQuaTrong=tool->dangChay?tool->thongBaoStatus:"Da dung luot trong; da xac nhan "+std::to_string(tool->soCayVuaTrong)+" cay";}} report{tool};
    tool->soCayVuaTrong=0;tool->henTrongCay=GetTickCount64()+60000;
    bool any=false;for(bool selected:tool->cacHatCanTrong)any|=selected;
    if(!any){tool->thongBaoStatus="Trong cay: chua chon hat";return;}
    if(!PlantEnsureInventory(tool)){tool->thongBaoStatus="Trong cay: chua doc duoc balo";PlantCloseBag(tool);return;}
    auto order=PlantInventoryOrder(tool->hatTrongDaNho,tool->cacHatCanTrong,PlantHeldSeed(PlantFrame(tool)));
    int missing=0;std::string names;
    for(int seed=0;seed<SO_HAT_TRONG;++seed)if(tool->cacHatCanTrong[seed]&&!tool->hatTrongDaNho.Has(seed)) {
        ++missing;if(!names.empty())names+=", ";names+=ds_hat_trong[seed];
    }
    PlantCloseBag(tool);
    if(order.empty()){tool->thongBaoStatus="Balo: khong co/chua doc duoc hat da chon ("+names+")";return;}
    if(!GardenSurvey(tool,true))return;
    auto deadline=std::chrono::steady_clock::now()+std::chrono::minutes(10);
    double speed=80;int relocalized=0;bool complete=true;
    for(int seed:order) {
        if(!tool->dangChay)break;
        if(!PlantEquipSeed(tool,seed)){++missing;continue;}
        while(tool->dangChay&&tool->hatTrongDaNho.Has(seed)&&tool->soCayVuaTrong<maxPlants) {
            if(std::chrono::steady_clock::now()>=deadline){complete=false;break;}
            int index=tool->soDoVuon.Next(seed);if(index<0)break;
            int result=GardenVisitNode(tool,index,seed,speed);
            if(result==-2) {
                tool->hatTrongDaNho.Invalidate();tool->conTroBalo.localized=false;
                tool->thongBaoStatus="Trong cay: chua xac minh hat/thao tac; dung luot, da xac nhan "+std::to_string(tool->soCayVuaTrong)+" cay";return;
            }
            if(result<0) {
                if(relocalized++<1&&GardenSurvey(tool,true))continue;
                tool->thongBaoStatus="Trong cay: mat vi tri tren so do; da xac nhan "+std::to_string(tool->soCayVuaTrong)+" cay; can quet vuon lai";return;
            }
            tool->soDoVuon.Visit(index,seed,result==1);
            GardenPublishState(tool);
            if(result==1)++tool->soCayVuaTrong;
            if(!tool->hatTrongDaNho.Has(seed))break;
        }
        if(tool->soCayVuaTrong>=maxPlants||!complete)break;
    }
    PlantReportInventory(tool);
    bool stockLeft=false,untried=false;
    for(int seed:order)if(tool->hatTrongDaNho.Has(seed)){stockLeft=true;untried|=tool->soDoVuon.Next(seed)>=0;}
    if((stockLeft&&!untried)||tool->soDoVuon.confirmed==tool->soDoVuon.nodes.size())tool->soDoVuon.ready=false;
    tool->thongBaoStatus="Trong cay: "+std::to_string(tool->soCayVuaTrong)+" cay; vuon "+std::to_string(tool->soDoVuon.plots.size())+" luong; da kiem tra "+std::to_string(tool->soDoVuon.visited)+"/"+std::to_string(tool->soDoVuon.nodes.size())+" diem";
    if(missing)tool->thongBaoStatus+="; hat khong co/chua lay duoc: "+std::to_string(missing)+" ("+names+")";
    if(!complete)tool->thongBaoStatus+="; giu tien do de tiep tuc";
}
