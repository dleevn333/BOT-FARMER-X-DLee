#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <algorithm>

constexpr int SO_HAT_TRONG = 34;
inline constexpr const char* ds_hat_trong[SO_HAT_TRONG] = {
    "Ca rot", "Cu cai", "Dau tay", "Viet quat", "Khoai tay", "Nam", "Bap", "Ca chua", "Sung", "Anh dao", "Khoai lang", "Xuong rong Saguaro", "Xuong rong gai vang", "Tao", "Hat de", "Nho", "Xuong rong Cholla", "Mang cau", "Xuong rong le gai", "Bi ngo", "Lua", "Dua hau", "Dua", "Xoai", "Du du", "Cay phong", "Cay dau", "Khe", "Tao duong",
    "Hoa chum sao (balo)", "Tieu hanh tinh (balo)", "Cay sao mai (balo)", "Tulip cam (balo)", "Rau chan vit (balo)"
};
inline constexpr const char* ds_anh_hat_trong[SO_HAT_TRONG] = {
    "seed_carot.png", "seed_cucai.png", "seed_dautay.png", "seed_vietquat.png", "seed_khoaitay.png", "seed_nam.png", "seed_bap.png", "seed_cachua.png", "seed_sung.png", "seed_anhdao.png", "seed_khoailang.png", "seed_saguaro.png", "seed_gaivang.png", "seed_tao.png", "seed_hatde.png", "seed_nho.png", "seed_cholla.png", "seed_mangcau.png", "seed_legai.png", "seed_bingo.png", "seed_lua.png", "seed_duahau.png", "seed_dua.png", "seed_xoai.png", "seed_dudu.png", "seed_cayphong.png", "seed_caydau.png", "seed_khe.png", "seed_taoduong.png",
    "plant_hoachumsao.png", "plant_tieuhanhtinh.png", "plant_caysaomai.png", "plant_tulipcam.png", "plant_rauchanvit.png"
};

struct PlantRing { cv::Point center{-1,-1}; float radius = 0; double score = 0; };

inline bool PlantBrown(const cv::Vec3b& hsv) {
    return hsv[0] >= 3 && hsv[0] <= 28 && hsv[1] >= 45 && hsv[1] <= 200
        && hsv[2] >= 20 && hsv[2] <= 205;
}

// The game itself is the placement oracle. Brown soil alone never permits a
// click: require the cyan annulus, a brown center, and a near-avatar location.
inline PlantRing PlantFindValidRing(const cv::Mat& frame, cv::Point avatar = {480,285}) {
    PlantRing best;
    if (frame.empty()) return best;
    cv::Mat hsv, cyan;
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv, cv::Scalar(82,65,65), cv::Scalar(108,235,255), cyan);
    cv::Rect roi(avatar.x-180,avatar.y-145,360,235);
    roi &= cv::Rect(0,0,frame.cols,frame.rows);
    if (roi.empty()) return best;
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(cyan(roi).clone(),contours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
    for (const auto& contour : contours) {
        auto box = cv::boundingRect(contour);
        if (box.width < 12 || box.height < 10 || box.width > 125 || box.height > 115) continue;
        double ratio = double(box.width)/box.height;
        double area = cv::contourArea(contour);
        if (ratio < .65 || ratio > 1.7 || area < 90) continue;
        double perimeter = cv::arcLength(contour,true);
        double circularity = 4*CV_PI*area/(perimeter*perimeter+1e-9);
        if (circularity < .55) continue;
        cv::Point center(roi.x+box.x+box.width/2,roi.y+box.y+box.height/2);
        if (cv::norm(center-avatar) > 155 || !PlantBrown(hsv.at<cv::Vec3b>(center))) continue;
        float radius = float(box.width+box.height)/4;
        int cyanOuter = 0, outer = 0, cyanInner = 0, inner = 0;
        for (int y = -int(radius); y <= int(radius); ++y) for (int x = -int(radius); x <= int(radius); ++x) {
            cv::Point p = center+cv::Point(x,y);
            if (p.x<0 || p.y<0 || p.x>=frame.cols || p.y>=frame.rows) continue;
            double d = std::sqrt(double(x*x+y*y))/radius;
            if (d >= .58 && d <= .84) { ++outer; cyanOuter += cyan.at<uchar>(p)>0; }
            if (d < .20) { ++inner; cyanInner += cyan.at<uchar>(p)>0; }
        }
        if (!outer || !inner || double(cyanOuter)/outer < .60 || double(cyanInner)/inner > .35) continue;
        double score = circularity + double(cyanOuter)/outer;
        if (score > best.score) best = {center,radius,score};
    }
    return best;
}

inline std::vector<cv::Rect> PlantBagCards(const cv::Mat& frame) {
    std::vector<cv::Rect> result;
    if (frame.empty()) return result;
    cv::Rect region(22,140,915,390);
    region &= cv::Rect(0,0,frame.cols,frame.rows);
    if (region.empty()) return result;
    cv::Mat white;
    cv::inRange(frame(region),cv::Scalar(185,185,185),cv::Scalar(255,255,255),white);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(white,contours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
    for (const auto& contour : contours) {
        cv::Rect box = cv::boundingRect(contour);
        if (box.width<110 || box.width>155 || box.height<130 || box.height>195) continue;
        if (cv::contourArea(contour)<7000) continue;
        box.x += region.x; box.y += region.y;
        result.push_back(box);
    }
    std::sort(result.begin(),result.end(),[](auto a,auto b){return a.y/30==b.y/30?a.x<b.x:a.y<b.y;});
    return result;
}

inline cv::Mat PlantSeedGlyph(const cv::Mat& original, bool alreadyGlyph = false) {
    if (original.empty() || alreadyGlyph) return original;
    int x1=int(original.cols*.22), y1=int(original.rows*.22);
    int x2=int(original.cols*.82), y2=int(original.rows*.82);
    return original(cv::Rect(x1,y1,x2-x1,y2-y1)).clone();
}

struct PlantSeedIdentity { int index=-1; double score=0, runnerUp=0; cv::Point point{-1,-1}; };
inline cv::Mat PlantCountMask(const cv::Mat& frame) {
    if(frame.empty()||frame.cols<805||frame.rows<364)return {};
    cv::Mat white,labels,stats,centers,clean;
    cv::inRange(frame(cv::Rect(728,340,77,24)),cv::Scalar(195,195,195),cv::Scalar(255,255,255),white);
    int n=cv::connectedComponentsWithStats(white,labels,stats,centers);
    clean=cv::Mat::zeros(white.size(),CV_8U);
    for(int i=1;i<n;++i)if(stats.at<int>(i,cv::CC_STAT_AREA)>=6
       &&stats.at<int>(i,cv::CC_STAT_HEIGHT)>=8&&stats.at<int>(i,cv::CC_STAT_WIDTH)<25)clean.setTo(255,labels==i);
    return clean;
}
inline bool PlantCountChanged(const cv::Mat& before,const cv::Mat& after) {
    if(before.empty()||after.empty()||before.size()!=after.size())return false;
    int a=cv::countNonZero(before),b=cv::countNonZero(after);
    if(a<40||b<40||b<a*.35||b>a*2.)return false;
    cv::Mat difference;cv::bitwise_xor(before,after,difference);
    return cv::countNonZero(difference)>=18;
}
inline PlantSeedIdentity PlantIdentifySeed(const cv::Mat& frame, cv::Rect search,
    const std::vector<cv::Mat>& glyphs, double threshold=.94, double margin=.05) {
    PlantSeedIdentity result;
    search &= cv::Rect(0,0,frame.cols,frame.rows);
    if (frame.empty() || search.empty()) return result;
    for (int i=0; i<int(glyphs.size()); ++i) {
        if (glyphs[i].empty()) continue;
        double score = 0;
        cv::Point point;
        for (int percent=80;percent<=180;percent+=5) {
            cv::Mat resized, correlation;
            cv::resize(glyphs[i],resized,{},percent/100.,percent/100.,cv::INTER_LINEAR);
            if (resized.cols>search.width || resized.rows>search.height || resized.cols<8 || resized.rows<8) continue;
            cv::matchTemplate(frame(search),resized,correlation,cv::TM_CCOEFF_NORMED);
            double current; cv::Point at;
            cv::minMaxLoc(correlation,nullptr,&current,nullptr,&at);
            if (current>score) {score=current;point=search.tl()+at+cv::Point(resized.cols/2,resized.rows/2);}
        }
        if (score>result.score) {result.runnerUp=result.score;result.index=i;result.score=score;result.point=point;}
        else result.runnerUp=std::max(result.runnerUp,score);
    }
    if (result.score<threshold || result.score-result.runnerUp<margin) result.index=-1;
    return result;
}

inline cv::Point PlantNearestSoil(const cv::Mat& frame, cv::Point avatar={480,285}) {
    if(frame.empty())return {-1,-1};
    cv::Mat hsv,soil;
    cv::cvtColor(frame,hsv,cv::COLOR_BGR2HSV);
    cv::inRange(hsv,cv::Scalar(3,45,20),cv::Scalar(26,195,185),soil);
    // Never target HUD, joystick, action buttons, or the player's own sprite.
    soil.rowRange(0,std::min(95,soil.rows)).setTo(0);
    if(soil.rows>390)soil.rowRange(390,soil.rows).setTo(0);
    soil.colRange(0,std::min(100,soil.cols)).setTo(0);
    if(soil.cols>740)soil.colRange(740,soil.cols).setTo(0);
    cv::erode(soil,soil,cv::getStructuringElement(cv::MORPH_ELLIPSE,{23,23}));
    cv::circle(soil,avatar,28,cv::Scalar(0),-1);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(soil.clone(),contours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
    cv::Mat valid=cv::Mat::zeros(soil.size(),CV_8U);
    for (size_t i=0;i<contours.size();++i) if(cv::contourArea(contours[i])>1000)
        cv::drawContours(valid,contours,int(i),cv::Scalar(255),-1);
    cv::Point nearest(-1,-1); double minimum=1e20;
    for(int y=95;y<std::min(390,valid.rows);y+=4) for(int x=100;x<std::min(740,valid.cols);x+=4) {
        if(!valid.at<uchar>(y,x))continue;
        double distance=cv::norm(cv::Point(x,y)-avatar);
        if(distance<minimum){minimum=distance;nearest={x,y};}
    }
    return nearest;
}

inline cv::Point PlantNextSoilSpot(const cv::Mat& frame,const std::vector<cv::Point2f>& visited,
    cv::Point avatar={480,285}) {
    if(frame.empty())return {-1,-1};
    cv::Mat hsv,soil;
    cv::cvtColor(frame,hsv,cv::COLOR_BGR2HSV);
    cv::inRange(hsv,cv::Scalar(3,45,20),cv::Scalar(26,195,185),soil);
    cv::erode(soil,soil,cv::getStructuringElement(cv::MORPH_ELLIPSE,{17,17}));
    cv::Point nearest(-1,-1);double minimum=1e20;
    for(int y=110;y<std::min(370,frame.rows);y+=18)for(int x=120;x<std::min(730,frame.cols);x+=18) {
        if(!soil.at<uchar>(y,x))continue;
        if((x<315&&y<220)||(x>595&&y<195)||(x<255&&y>325))continue;
        bool tried=false;
        for(auto p:visited)if(cv::norm(p-cv::Point2f(float(x),float(y)))<30){tried=true;break;}
        if(tried)continue;
        double distance=cv::norm(cv::Point(x,y)-avatar);
        if(distance<minimum){minimum=distance;nearest={x,y};}
    }
    return nearest;
}

// Track the ground between bounded joystick steps. HUD and avatar features
// are excluded so screen coordinates of attempted/occupied spots move with
// the camera instead of sending the character back to the same planting spot.
inline bool PlantTrackCamera(const cv::Mat& before,const cv::Mat& after,cv::Mat& transform) {
    if(before.empty()||after.empty()||before.size()!=after.size())return false;
    cv::Mat a,b,mask=cv::Mat::zeros(before.size(),CV_8U);
    cv::cvtColor(before,a,cv::COLOR_BGR2GRAY);
    cv::cvtColor(after,b,cv::COLOR_BGR2GRAY);
    mask(cv::Rect(90,100,660,290)&cv::Rect(0,0,a.cols,a.rows)).setTo(255);
    for(auto area:{cv::Rect(0,0,315,220),cv::Rect(595,0,365,195),cv::Rect(0,325,255,215),cv::Rect(405,220,150,130)})
        mask(area&cv::Rect(0,0,a.cols,a.rows)).setTo(0);
    std::vector<cv::Point2f> points,next,backward,source,target;
    cv::goodFeaturesToTrack(a,points,260,.01,6,mask);
    if(points.size()<8)return false;
    std::vector<uchar> status,reverse;std::vector<float> error,reverseError;
    cv::calcOpticalFlowPyrLK(a,b,points,next,status,error,{31,31},3);
    cv::calcOpticalFlowPyrLK(b,a,next,backward,reverse,reverseError,{31,31},3);
    for(size_t i=0;i<points.size();++i) {
        if(!status[i]||!reverse[i]||error[i]>35||cv::norm(points[i]-backward[i])>2.5)continue;
        source.push_back(points[i]);target.push_back(next[i]);
    }
    if(source.size()<8)return false;
    cv::Mat inliers;
    transform=cv::estimateAffinePartial2D(source,target,inliers,cv::RANSAC,3.0);
    if(transform.empty()||cv::countNonZero(inliers)<8)return false;
    double scale=std::hypot(transform.at<double>(0,0),transform.at<double>(1,0));
    return scale>.9&&scale<1.1&&std::abs(transform.at<double>(0,2))<220&&std::abs(transform.at<double>(1,2))<220;
}

inline void PlantTransformSpots(std::vector<cv::Point2f>& spots,const cv::Mat& transform) {
    if(transform.empty())return;
    for(auto& p:spots){double x=p.x,y=p.y;p.x=float(transform.at<double>(0,0)*x+transform.at<double>(0,1)*y+transform.at<double>(0,2));
        p.y=float(transform.at<double>(1,0)*x+transform.at<double>(1,1)*y+transform.at<double>(1,2));}
}
