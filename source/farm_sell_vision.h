#pragma once
#include <opencv2/opencv.hpp>

inline cv::Point FarmSaleImage(const cv::Mat& frame,const cv::Mat& image,cv::Rect roi,double score=.84) {
    if(frame.empty()||image.empty())return {-1,-1};
    roi &= cv::Rect(0,0,frame.cols,frame.rows);
    if(roi.width<image.cols||roi.height<image.rows)return {-1,-1};
    cv::Mat result;cv::matchTemplate(frame(roi),image,result,cv::TM_CCOEFF_NORMED);
    double best;cv::Point location;cv::minMaxLoc(result,nullptr,&best,nullptr,&location);
    if(best<score)return {-1,-1};
    return {roi.x+location.x+image.cols/2,roi.y+location.y+image.rows/2};
}
inline cv::Point FarmSaleResultOk(const cv::Mat& frame,const cv::Mat& title,const cv::Mat& ok) {
    // Only dismiss the completed-sale modal, never a purchase or sell confirmation.
    if(FarmSaleImage(frame,title,{280,65,400,115}).x<0)return {-1,-1};
    return FarmSaleImage(frame,ok,{300,330,380,145});
}
inline bool FarmSaleReady(const cv::Mat& frame,const cv::Mat& header) {
    if(FarmSaleImage(frame,header,{15,5,400,70},.82).x<0)return false;
    cv::Rect white(260,20,100,20);
    if((white & cv::Rect(0,0,frame.cols,frame.rows))!=white)return false;
    auto color=cv::mean(frame(white));
    return color[0]>200&&color[1]>200&&color[2]>200;
}
