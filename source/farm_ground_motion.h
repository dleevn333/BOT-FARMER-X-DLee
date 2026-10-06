#pragma once
#include <opencv2/opencv.hpp>

inline cv::Mat GardenGroundMask(const cv::Mat& frame) {
    cv::Mat hsv,gray,soil,gold,path;cv::cvtColor(frame,hsv,cv::COLOR_BGR2HSV);
    cv::inRange(hsv,cv::Scalar(0,0,25),cv::Scalar(179,40,235),gray);
    cv::inRange(hsv,cv::Scalar(3,55,20),cv::Scalar(28,210,210),soil);
    cv::erode(soil,soil,cv::getStructuringElement(cv::MORPH_ELLIPSE,{7,7}));
    cv::inRange(hsv,cv::Scalar(8,20,55),cv::Scalar(32,210,255),gold);
    cv::inRange(hsv,cv::Scalar(10,10,135),cv::Scalar(38,115,255),path);
    cv::Mat mask=gray|soil|gold|path;
    mask.rowRange(0,90).setTo(0);mask.rowRange(440,mask.rows).setTo(0);
    mask.colRange(0,80).setTo(0);mask.colRange(920,mask.cols).setTo(0);
    mask(cv::Rect(0,0,315,220)).setTo(0);mask(cv::Rect(595,0,365,195)).setTo(0);
    mask(cv::Rect(405,220,150,130)).setTo(0);mask(cv::Rect(0,325,255,215)).setTo(0);
    mask(cv::Rect(730,260,230,280)).setTo(0);return mask;
}
inline bool GardenGroundMotion(const cv::Mat& before,const cv::Mat& after,cv::Mat& homography) {
    if(before.empty()||after.empty()||before.size()!=cv::Size(960,540)||before.size()!=after.size())return false;
    cv::Mat a,b;cv::cvtColor(before,a,cv::COLOR_BGR2GRAY);cv::cvtColor(after,b,cv::COLOR_BGR2GRAY);
    cv::equalizeHist(a,a);cv::equalizeHist(b,b);
    std::vector<cv::Point2f> points,next,back,source,target;
    cv::goodFeaturesToTrack(a,points,400,.01,5,GardenGroundMask(before));
    if(points.size()<12)return false;
    std::vector<uchar> forward,reverse;std::vector<float> errors,reversedErrors;
    cv::calcOpticalFlowPyrLK(a,b,points,next,forward,errors,{31,31},3);
    cv::calcOpticalFlowPyrLK(b,a,next,back,reverse,reversedErrors,{31,31},3);
    for(size_t i=0;i<points.size();++i)if(forward[i]&&reverse[i]&&errors[i]<35&&cv::norm(points[i]-back[i])<2.5){source.push_back(points[i]);target.push_back(next[i]);}
    if(source.size()<12)return false;
    cv::Mat inliers;homography=cv::findHomography(source,target,cv::RANSAC,3.,inliers);
    if(homography.empty()||cv::countNonZero(inliers)<12) {
        auto affine=cv::estimateAffinePartial2D(source,target,inliers,cv::RANSAC,2.5);
        if(affine.empty()||cv::countNonZero(inliers)<12)return false;
        homography=cv::Mat::eye(3,3,CV_64F);affine.copyTo(homography(cv::Rect(0,0,3,2)));
    }
    if(homography.empty()||cv::countNonZero(inliers)<12||cv::countNonZero(inliers)<source.size()*.25)return false;
    std::vector<cv::Point2f> supported;
    for(int i=0;i<int(source.size());++i)if(inliers.at<uchar>(i))supported.push_back(source[i]);
    auto spread=cv::boundingRect(supported);if(spread.width<100||spread.height<60)return false;
    homography/=homography.at<double>(2,2);
    std::vector<cv::Point2f> corners{{100,100},{740,100},{740,390},{100,390}},projected;
    cv::perspectiveTransform(corners,projected,homography);
    for(size_t i=0;i<corners.size();++i)if(!std::isfinite(projected[i].x)||!std::isfinite(projected[i].y)||cv::norm(projected[i]-corners[i])>250)return false;
    double ratio=std::abs(cv::contourArea(projected))/cv::contourArea(corners);
    return ratio>.65&&ratio<1.5;
}
