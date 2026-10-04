#include <opencv2/opencv.hpp>
#include "farm_sell_vision.h"
#include <iostream>
#include <stdexcept>
int main(int argc,char** argv) {try {
    if(argc!=3)throw std::runtime_error("Pass fixture and image directories");
    int checks=0;auto check=[&](bool valid,const char* message){++checks;if(!valid)throw std::runtime_error(message);};
    auto load=[&](const char* name){return cv::imread(std::string(argv[2])+"/"+name);};
    auto success=cv::imread(std::string(argv[1])+"/success.png");
    auto confirm=cv::imread(std::string(argv[1])+"/confirm.png");
    auto title=load("tieu_de_ban_xong_moi.png"),ok=load("ok_ban_xong_moi.png"),header=load("tieu_de_ban_moi.png");
    check(!success.empty()&&!confirm.empty()&&!title.empty()&&!ok.empty()&&!header.empty(),"Fixtures and templates exist");
    check(FarmSaleImage(success,load("ok.png"),{300,310,380,180},.75).x<0,"Reproduce the old OK template miss");
    auto button=FarmSaleResultOk(success,title,ok);
    check(button.x>=440&&button.x<=520&&button.y>=380&&button.y<=430,"New completed-sale OK is inside the actual button");
    check(!FarmSaleReady(success,header),"The dimmed shop behind the result is not a completed dismissal");
    check(FarmSaleResultOk(confirm,title,ok).x<0,"Never dismiss the actual sell confirmation as a result");
    check(!FarmSaleReady(confirm,header),"The confirmation overlay cannot complete the sell loop");
    auto ready=cv::imread(std::string(argv[1])+"/ready.png");
    check(FarmSaleReady(ready,header)&&FarmSaleResultOk(ready,title,ok).x<0,"The actual shop after OK is ready and has no result button");
    cv::Mat shifted;cv::warpAffine(success,shifted,cv::Mat(cv::Matx23d(1,0,8,0,1,-5)),success.size());
    check(FarmSaleResultOk(shifted,title,ok)==button+cv::Point(8,-5),"Follow an offset result modal rather than hard-coded OK coordinates");
    auto noButton=success.clone();noButton(cv::Rect(390,370,180,80)).setTo(cv::Scalar(255,255,255));
    check(FarmSaleResultOk(noButton,title,ok).x<0,"A result title alone does not invent an OK click");
    auto noTitle=success.clone();noTitle(cv::Rect(330,90,300,50)).setTo(cv::Scalar(255,255,255));
    check(FarmSaleResultOk(noTitle,title,ok).x<0,"An OK without the completed-sale title is ignored");
    check(FarmSaleResultOk(cv::Mat(),title,ok).x<0,"Empty capture is harmless");
    check(FarmSaleResultOk(success,cv::Mat(),ok).x<0,"Missing title asset does not click");
    check(FarmSaleResultOk(success,title,cv::Mat()).x<0,"Missing OK asset does not click");
    check(!FarmSaleReady(cv::Mat(),header),"Empty capture is not a ready shop");
    check(FarmSaleResultOk(success(cv::Rect(0,0,40,40)),title,ok).x<0,"Small captures stay in bounds");
    std::cout<<checks<<" sell checks passed\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<"\n";return 1;}}
