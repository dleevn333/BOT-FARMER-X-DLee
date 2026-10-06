#pragma once

inline bool GardenTrackGround(const cv::Mat& before,const cv::Mat& after,cv::Mat& transform) {
    bool valid=GardenGroundMotion(before,after,transform);
    return valid;
}
inline bool GardenSyncFrame(ThongTinTool* tool,const cv::Mat& frame) {
    auto& plan=tool->soDoVuon;
    if(frame.empty())return false;
    if(!plan.lastFrame.empty()) {
        cv::Mat change;
        if(!GardenTrackGround(plan.lastFrame,frame,change)){plan.localized=false;return false;}
        plan.Track(change);
    }
    plan.lastFrame=frame;return true;
}
inline bool GardenZoom(ThongTinTool* tool,int key,int steps,bool track=false) {
    auto& plan=tool->soDoVuon;
    HWND previous=GetForegroundWindow();DWORD previousPid=0;GetWindowThreadProcessId(previous,&previousPid);
    if(previous!=tool->h_cha&&previousPid!=GetCurrentProcessId()) {
        tool->thongBaoStatus="Camera: bam Quet so do khi cua so bot/LDPlayer dang duoc chon";return false;
    }
    struct RestoreFocus {HWND previous,game;~RestoreFocus(){if(previous!=game&&GetForegroundWindow()==game&&IsWindow(previous))SetForegroundWindow(previous);}} restore{previous,tool->h_cha};
    SetForegroundWindow(tool->h_cha);if(!PlantWait(tool,150)||GetForegroundWindow()!=tool->h_cha)return false;
    auto pivot=PlantClientPoint(tool->h_game,{480,230});PostMessageW(tool->h_game,WM_MOUSEMOVE,0,MAKELPARAM(pivot.x,pivot.y));
    auto sendKey=[&](bool up){INPUT input{};input.type=INPUT_KEYBOARD;input.ki.wVk=WORD(key);input.ki.dwFlags=up?KEYEVENTF_KEYUP:0;return SendInput(1,&input,sizeof(input))==1;};
    int chunk=track?2:8;
    for(int batch=0;batch<steps&&tool->dangChay;batch+=chunk) {
        if(GetForegroundWindow()!=tool->h_cha)return false;
        auto before=track?PlantFrame(tool):cv::Mat();
        if(track&&!GardenSyncFrame(tool,before))return false;
        if(!sendKey(false))return false;
        for(int step=0;step<std::min(chunk,steps-batch)&&tool->dangChay;++step) {
            if(step&&!sendKey(false))break;
            if(!PlantWait(tool,100))break;
        }
        sendKey(true);
        if(!tool->dangChay)return false;
        if(!PlantWait(tool,160))return false;
        if(track) {
            auto after=PlantFrame(tool);cv::Mat transform;
            if(!GardenTrackGround(before,after,transform)){plan.localized=false;return false;}
            plan.Track(transform);
            plan.lastFrame=after;
        }
    }
    return tool->dangChay;
}
inline void GardenPublishState(ThongTinTool* tool) {
    tool->vuonSoLuong=int(tool->soDoVuon.plots.size());tool->vuonSoDiem=int(tool->soDoVuon.nodes.size());
    tool->vuonDaKiemTra=tool->soDoVuon.visited;tool->vuonDaTrong=tool->soDoVuon.confirmed;
}
inline bool GardenSurvey(ThongTinTool* tool,bool preserve=true) {
    if(FarmHasImage(PlantFrame(tool),"plant_edit_guard.png",{780,0,180,180},.85)){tool->thongBaoStatus="Hay thoat che do chinh sua vuon truoc khi quet";return false;}
    auto old=tool->soDoVuon;
    PlantCloseBag(tool);
    if(!PlantGoHome(tool))return false;
    if(preserve&&old.ready&&old.homeCalibrated&&old.gameKey==reinterpret_cast<std::uintptr_t>(tool->h_game)) {
        auto home=PlantFrame(tool);cv::Mat change;
        if(GardenTrackGround(old.homeFrame,home,change)) {
            auto& reused=tool->soDoVuon;reused=old;reused.worldToScreen=old.homePose;
            reused.Track(change);reused.lastFrame=home;
            reused.homePose=reused.worldToScreen;reused.homeFrame=home;
            GardenPublishState(tool);tool->thongBaoStatus="Da dinh vi lai tren so do cu; giu tien do vuon";return true;
        }
    }
    tool->thongBaoStatus="So do vuon: camera rong va khao sat cac luong";
    if(!GardenZoom(tool,VK_F5,48))return false;
    auto overviewHome=PlantFrame(tool);
    auto& plan=tool->soDoVuon;plan.Clear();plan.gameKey=reinterpret_cast<std::uintptr_t>(tool->h_game);plan.localized=true;
    int unchanged=0;
    for(int survey=0;survey<16&&tool->dangChay;++survey) {
        auto before=PlantFrame(tool);if(!GardenSyncFrame(tool,before))return false;
        GardenDetectionDebug debug;auto visible=GardenDetectPlots(before,&debug);
        int added=plan.Merge(visible);unchanged=added?0:unchanged+1;
        GardenPublishState(tool);
        tool->thongBaoStatus="So do vuon: da ghi "+std::to_string(plan.plots.size())+" luong";
        float top=1e6,bottom=0;for(const auto& bed:visible){top=std::min(top,bed.corners[0].y);bottom=std::max(bottom,bed.corners[2].y);}
        cv::Mat hsv,brown;cv::cvtColor(before,hsv,cv::COLOR_BGR2HSV);
        cv::inRange(hsv,cv::Scalar(3,40,20),cv::Scalar(28,210,205),brown);
        int soilAbove=cv::countNonZero(brown(cv::Rect(100,10,730,8)));
        bool entire=debug.bounds[0].height>0&&debug.bounds[1].height>0&&debug.bounds[0].y>7&&debug.bounds[1].y>7
            &&debug.bounds[0].br().y<505&&debug.bounds[1].br().y<505;
        if(entire&&visible.size()>=4&&soilAbove<80) {
            plan.plots.clear();plan.Merge(visible);plan.lastFrame=before;break;
        }
        if(unchanged>=2&&plan.plots.size()>=4&&debug.bounds[0].y>40&&debug.bounds[1].y>40&&soilAbove<80)break;
        PlantMove(tool,{0,-1},visible.size()>=6?150:400);
        auto after=PlantFrame(tool);cv::Mat transform;
        if(!GardenTrackGround(before,after,transform)){plan.localized=false;tool->thongBaoStatus="So do vuon: mat dau camera khi khao sat; chua dung duong di";return false;}
        plan.Track(transform);
        plan.lastFrame=after;
    }
    if(!tool->dangChay)return false;
    if(plan.plots.empty()){tool->thongBaoStatus="So do vuon: chua nhan ra ranh gioi luong";return false;}
    plan.Build();
    if(preserve&&old.gameKey==plan.gameKey&&old.ready&&old.plots.size()==plan.plots.size()&&old.nodes.size()==plan.nodes.size()) {
        for(size_t i=0;i<plan.nodes.size();++i){plan.nodes[i].planted=old.nodes[i].planted;plan.nodes[i].attemptedSeeds=old.nodes[i].attemptedSeeds;}
        plan.visited=old.visited;plan.confirmed=old.confirmed;
    }
    // Keep the surveyed camera scale: changing it adds another localization
    // jump and is unnecessary for the visible placement ring.
    if(!PlantGoHome(tool))return false;
    plan.homeFrame=PlantFrame(tool);cv::Mat homeChange;
    if(!GardenTrackGround(overviewHome,plan.homeFrame,homeChange)){plan.localized=false;tool->thongBaoStatus="So do vuon: chua dinh vi lai duoc o cong vuon";return false;}
    plan.worldToScreen=cv::Matx33d::eye();plan.Track(homeChange);plan.homePose=plan.worldToScreen;
    plan.lastFrame=plan.homeFrame;plan.localized=true;plan.homeCalibrated=true;
    GardenPublishState(tool);
    tool->thongBaoStatus="So do vuon: "+std::to_string(plan.plots.size())+" luong, "+std::to_string(plan.nodes.size())+" diem; da san sang";
    return true;
}
inline bool GardenMoveTracked(ThongTinTool* tool,cv::Point2d direction,int duration,double& speed) {
    auto before=PlantFrame(tool);if(!GardenSyncFrame(tool,before))return false;
    PlantMove(tool,direction,duration);
    if(!tool->dangChay)return false;
    auto after=PlantFrame(tool);cv::Mat transform;
    if(!GardenTrackGround(before,after,transform)){tool->soDoVuon.localized=false;return false;}
    tool->soDoVuon.Track(transform);
    tool->soDoVuon.lastFrame=after;
    std::vector<cv::Point2f> avatar{{480,285}},shifted;cv::perspectiveTransform(avatar,shifted,transform);
    double motion=cv::norm(shifted[0]-avatar[0]);
    if(motion>2)speed=.65*speed+.35*std::clamp(motion/(duration/1000.),20.,220.);
    return motion>1.5;
}
inline int GardenSettledQuantity(ThongTinTool* tool,int seed) {
    int same=0,empty=0;
    for(int wait=0;wait<12&&tool->dangChay;++wait) {
        if(!PlantWait(tool,200))return -1;
        auto frame=PlantFrame(tool);if(frame.empty()||PlantIsBag(frame))return -1;
        int held=PlantHeldSeed(frame);
        if(held==seed) {
            empty=0;
            if(++same>=2){int count=PlantSeedCount(frame);return count>0?count:-1;}
        } else {
            same=0;
            auto mask=PlantCountMask(frame);
            if(held<0&&!mask.empty()&&(cv::countNonZero(mask)==0||PlantSeedCount(frame)==0))++empty;else empty=0;
            if(wait>=4&&empty>=3)return 0;
        }
    }
    return -1; // Unknown quantities are kept, never guessed as depleted.
}
inline void GardenTestRoute(ThongTinTool* tool) {
    if(!GardenSurvey(tool,false))return;
    double speed=80;int reached=0;
    for(size_t plot=0;plot<tool->soDoVuon.plots.size()&&tool->dangChay;++plot) {
        for(int step=0;step<64&&tool->dangChay;++step) {
            auto target=GardenTransform(tool->soDoVuon.plots[plot].Center(),tool->soDoVuon.worldToScreen);
            auto delta=target-cv::Point2f(480,285);
            if(cv::norm(delta)<28){++reached;break;}
            tool->thongBaoStatus="Thu duong di, khong trong: luong "+std::to_string(plot+1)+"/"+std::to_string(tool->soDoVuon.plots.size());
            int duration=std::clamp(cvRound(cv::norm(delta)/speed*1000),120,220);
            if(!GardenMoveTracked(tool,delta,duration,speed)&&!tool->soDoVuon.localized) {
                tool->thongBaoStatus="Thu duong di: mat dau camera; da den "+std::to_string(reached)+" luong";return;
            }
        }
    }
    tool->thongBaoStatus="Thu duong di: da den "+std::to_string(reached)+"/"+std::to_string(tool->soDoVuon.plots.size())+" luong; khong trong hay mua hat";
}
// 1 confirmed; 0 unsuitable; -1 lost localization; -2 unverified input/result.
inline int GardenVisitNode(ThongTinTool* tool,int index,int seed,double& speed) {
    auto& plan=tool->soDoVuon;int stalled=0;
    for(int move=0;move<32&&tool->dangChay;++move) {
        if(!plan.localized)return -1;
        auto frame=PlantFrame(tool);if(frame.empty()||PlantHeldSeed(frame)!=seed)return -2;
        if(!GardenSyncFrame(tool,frame))return -1;
        auto target=GardenTransform(plan.nodes[index].point,plan.worldToScreen);
        if(!std::isfinite(target.x)||!std::isfinite(target.y)||std::abs(target.x)>2500||std::abs(target.y)>2500){plan.localized=false;return -1;}
        auto ring=PlantFindValidRing(frame);
        auto position=cv::Point2f(480,300);
        auto delta=target-position;double distance=cv::norm(delta);
        if(distance<35) {
            if(ring.center.x>=0) {
                if(!PlantWait(tool,220))return 0;
                auto stable=PlantFrame(tool);auto valid=PlantFindValidRing(stable);
                auto worldRing=valid.center.x>=0?GardenTransform(cv::Point2f(valid.center),plan.worldToScreen.inv()):cv::Point2f(-1,-1);
                const auto& bed=plan.plots[plan.nodes[index].plot];std::vector<cv::Point2f> polygon(bed.corners.begin(),bed.corners.end());
                if(valid.center.x>=0&&cv::norm(valid.center-ring.center)<=5&&cv::pointPolygonTest(polygon,worldRing,false)>=0) {
                    tool->thongBaoStatus="Trong "+std::string(ds_hat_trong[seed])+": luong "+std::to_string(plan.nodes[index].plot+1)+"/"+std::to_string(plan.plots.size());
                    if(PlantClickConfirmed(tool,stable,valid,seed)) {
                        int remaining=GardenSettledQuantity(tool,seed);
                        if(remaining==0&&tool->hatTrongDaNho.snapshot.seeds[seed].quantity>1) {
                            tool->hatTrongDaNho.Invalidate();tool->conTroBalo.localized=false;
                        }
                        tool->hatTrongDaNho.Consumed(seed,remaining);
                        plan.nodes[index].point=worldRing;
                        return 1;
                    }
                    tool->thongBaoStatus="Trong cay: chua xac nhan tieu thu hat; bo qua diem nay";
                    return -2;
                }
            }
            // Bounded probes around this waypoint; red markers never trigger planting.
            static const cv::Point2d probes[]={{1,0},{0,1},{-1,0},{0,-1}};
            if(move>=4)return 0;
            if(!GardenMoveTracked(tool,probes[move],180,speed)&&!plan.localized)return -1;
        } else {
            int duration=std::clamp(cvRound(distance/speed*1000),120,220);
            tool->thongBaoStatus="Di theo so do: luong "+std::to_string(plan.nodes[index].plot+1)+"/"+std::to_string(plan.plots.size())+", diem "+std::to_string(index+1)+"/"+std::to_string(plan.nodes.size());
            if(!GardenMoveTracked(tool,delta,duration,speed)) {
                if(!plan.localized)return -1;
                if(++stalled>=3)return 0;
            } else stalled=0;
        }
    }
    return 0;
}
