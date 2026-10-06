#pragma once
#include "farm_ocr.h"
#include "farm_sell_vision.h"

enum class FarmNoticeKind {None,NotReady,NewTasks};
struct FarmNotice {FarmNoticeKind kind=FarmNoticeKind::None;cv::Point ok{-1,-1};cv::Rect panel;};
inline bool FarmNoticeUnsafeText(const std::string& text) {
    for(const auto& forbidden:{"muahat","muavatpham","kimcuong","thanhtoan","xacnhanban","chacchan","xoavatpham"})
        if(text.find(forbidden)!=std::string::npos)return true;
    return false;
}
inline FarmNoticeKind FarmNoticeTextKind(const std::string& text) {
    if(FarmNoticeUnsafeText(text))return FarmNoticeKind::None;
    if(text.find("thuhoach")!=std::string::npos&&(text.find("lonlen")!=std::string::npos||text.find("chualon")!=std::string::npos||text.find("chuadu")!=std::string::npos))return FarmNoticeKind::NotReady;
    if(text.find("nhiemvu")!=std::string::npos&&(text.find("moi")!=std::string::npos||text.find("capnhat")!=std::string::npos))return FarmNoticeKind::NewTasks;
    return FarmNoticeKind::None;
}
inline cv::Point FarmNoticeImage(const cv::Mat& frame,const cv::Mat& image,cv::Rect roi,double score=.83) {
    for(int scale:{100,95,105,90,110}) {
        if(image.empty())break;cv::Mat sized;cv::resize(image,sized,{},scale/100.,scale/100.);
        auto found=FarmSaleImage(frame,sized,roi,score);if(found.x>=0)return found;
    }
    return {-1,-1};
}
inline FarmNotice FarmFindNotice(const cv::Mat& input,const cv::Mat& title,const cv::Mat& ok,const cv::Mat& notReady) {
    FarmNotice result;if(input.empty()||input.cols<400||input.rows<240)return result;
    cv::Mat frame;if(input.size()!=cv::Size(960,540))cv::resize(input,frame,{960,540});else frame=input;
    cv::Mat paper;cv::inRange(frame,cv::Scalar(230,230,230),cv::Scalar(255,255,255),paper);
    std::vector<std::vector<cv::Point>> contours;cv::findContours(paper,contours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
    for(const auto& contour:contours) {
        auto box=cv::boundingRect(contour);
        if(box.width<350||box.width>750||box.height<220||box.height>460||std::abs(box.x+box.width/2-480)>65||box.y<35||box.br().y>515)continue;
        if(cv::contourArea(contour)<box.area()*.72)continue;
        auto heading=cv::Rect(box.x+80,box.y+8,box.width-160,75);
        if(FarmNoticeImage(frame,title,heading,.84).x<0)continue;
        auto footer=cv::Rect(box.x+20,box.y+box.height/2,box.width-40,box.height/2-8);
        auto button=FarmNoticeImage(frame,ok,footer,.83);if(button.x<0)continue;
        cv::Mat hsv,blue;cv::cvtColor(frame(footer),hsv,cv::COLOR_BGR2HSV);
        cv::inRange(hsv,cv::Scalar(80,95,160),cv::Scalar(115,255,255),blue);
        std::vector<std::vector<cv::Point>> buttons;cv::findContours(blue,buttons,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
        int count=0;for(auto shape:buttons){auto b=cv::boundingRect(shape);if(b.width>80&&b.height>28&&cv::contourArea(shape)>1800)++count;}
        if(count!=1)continue;
        auto body=cv::Rect(box.x+30,box.y+90,box.width-60,box.height-175);
        std::string text;for(const auto& word:FarmReadText(frame,body))text+=word.text;
        if(FarmNoticeUnsafeText(text))continue;
        auto kind=FarmNoticeTextKind(text);
        if(kind==FarmNoticeKind::None&&FarmNoticeImage(frame,notReady,body,.87).x>=0)kind=FarmNoticeKind::NotReady;
        if(kind==FarmNoticeKind::None)continue;
        result.kind=kind;result.panel=box;
        result.ok={cvRound(button.x*input.cols/960.),cvRound(button.y*input.rows/540.)};return result;
    }
    return result;
}
