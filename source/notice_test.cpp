#include <windows.h>
#include "farm_notice_vision.h"
#include <iostream>
#include <stdexcept>
int main(int argc,char**argv){try {
    if(argc!=3)throw std::runtime_error("Pass fixtures and images");
    int checks=0;auto check=[&](bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);};
    std::string fixtures=argv[1],images=argv[2];
    auto title=cv::imread(images+"/notice_title.png"),ok=cv::imread(images+"/notice_ok.png"),body=cv::imread(images+"/notice_not_ready_body.png");
    auto detect=[&](cv::Mat f){return FarmFindNotice(f,title,ok,body);};
    auto original=cv::imread(fixtures+"/notice-not-ready.png");
    auto notice=detect(original);
    std::cout<<"Actual notice kind="<<int(notice.kind)<<" OK="<<notice.ok<<" panel="<<notice.panel<<"\n";
    check(notice.kind==FarmNoticeKind::NotReady,"Recognize the actual not-ready crop notice");
    check(cv::norm(notice.ok-cv::Point(480,419))<20,"Click the observed OK rather than a fixed unrelated point");
    cv::Mat resized;cv::resize(original,resized,{1280,720});
    auto large=detect(resized);check(large.kind==notice.kind&&cv::norm(large.ok-cv::Point(640,559))<25,"Scale the OK target with LDPlayer resolution");
    check(FarmNoticeTextKind("daconhiemvumoihaikiemtra")==FarmNoticeKind::NewTasks,"Daily new-task notification is informational");
    check(FarmNoticeTextKind("nhiemvudad uoccapnhat")==FarmNoticeKind::NewTasks,"Task data refresh is informational");
    check(FarmNoticeTextKind("banmuonmuahatbangkimcuong")==FarmNoticeKind::None,"Purchase authorization is not an informational notice");
    check(FarmNoticeTextKind("banchacchanxacnhanban")==FarmNoticeKind::None,"Sell confirmation is not dismissed as an informational notice");
    check(FarmNoticeTextKind("thongbaokhongbiet")==FarmNoticeKind::None,"Unknown messages are not clicked blindly");
    auto noButton=original.clone();noButton(cv::Rect(375,375,225,90)).setTo(cv::Scalar(255,255,255));
    check(detect(noButton).kind==FarmNoticeKind::None,"A missing OK button cannot trigger a click");
    auto noTitle=original.clone();noTitle(cv::Rect(390,90,220,50)).setTo(cv::Scalar(255,255,255));
    check(detect(noTitle).kind==FarmNoticeKind::None,"The notification title is mandatory");
    auto doubleButton=original.clone();ok.copyTo(doubleButton(cv::Rect(580,384,ok.cols,ok.rows)));
    check(detect(doubleButton).kind==FarmNoticeKind::None,"A second action button blocks generic dismissal");
    check(detect(cv::Mat()).kind==FarmNoticeKind::None,"Empty/loading captures cannot trigger dismissal");
    cv::Mat plain(540,960,CV_8UC3,cv::Scalar(30,180,50));check(detect(plain).kind==FarmNoticeKind::None,"Gameplay has no notification target");
    auto tasks=cv::imread(fixtures+"/notice-new-tasks-simulated.png");
    check(detect(tasks).kind==FarmNoticeKind::NewTasks,"Read new-task text through OCR in a simulated single-OK notice");
    check(cv::norm(detect(tasks).ok-notice.ok)<5,"Daily-task dismissal keeps the same observed OK geometry");
    for(const auto& name:{"confirm","success","ready"}) {
        auto sale=cv::imread(fixtures+"/../sell-fixtures/"+name+".png");
        check(!sale.empty()&&detect(sale).kind==FarmNoticeKind::None,"Actual sell confirmation/result/screen is not treated as this notice");
    }
    std::cout<<checks<<" notification checks passed\n";return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<"\n";return 1;}}
