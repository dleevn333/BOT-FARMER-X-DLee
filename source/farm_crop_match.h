#pragma once
#include <opencv2/opencv.hpp>
#include <array>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

constexpr int SO_NONG_SAN=31;
inline constexpr const char* ds_hat_gionghai[SO_NONG_SAN]={
    "Ca rot", "Cu cai", "Dau tay", "Viet quat", "Khoai tay", "Nam", "Bap", "Ca chua", "Sung", "Anh dao", "Khoai lang", "Xuong rong Saguaro", "Xuong rong gai vang", "Tao", "Hat de", "Nho", "Xuong rong Cholla", "Mang cau", "Xuong rong le gai", "Bi ngo", "Lua", "Dua hau", "Dua", "Xoai", "Du du", "Cay phong", "Cay dau", "Khe", "Tao duong", "Trang khuyet", "Nhan sam"
};
inline std::string FarmCropKey(const std::string& text) {
    std::string key;
    for(unsigned char c:text)if(std::isalpha(c)&&c<128)key+=char(std::tolower(c));
    return key;
}
inline int FarmCropDistance(const std::string& a,const std::string& b) {
    std::vector<int> previous(b.size()+1),next(b.size()+1);
    for(size_t j=0;j<=b.size();++j)previous[j]=int(j);
    for(size_t i=0;i<a.size();++i) {
        next[0]=int(i+1);
        for(size_t j=0;j<b.size();++j)next[j+1]=std::min({next[j]+1,previous[j+1]+1,previous[j]+(a[i]!=b[j])});
        previous.swap(next);
    }
    return previous.back();
}
struct FarmCropMatch {int id=-1;int edits=1000;};
inline FarmCropMatch FarmMatchCrop(const std::string& text) {
    auto key=FarmCropKey(text);
    if(key.empty())return {};
    FarmCropMatch best;int second=1000;
    for(int crop=0;crop<SO_NONG_SAN;++crop) {
        auto name=FarmCropKey(ds_hat_gionghai[crop]);
        int distance=1000;
        for(const std::string& prefix:{std::string(),std::string("hat"),std::string("hatgiong"),std::string("qua")}) {
            auto target=prefix+name;
            // Short names (Dua, Lua, Tao...) must match exactly. Longer labels
            // permit a small OCR error, never a substring or a missing word.
            int allowance=name.size()<5?0:name.size()<10?1:2;
            int edits=FarmCropDistance(key,target);
            if(edits<=allowance&&double(edits)/target.size()<=.20)distance=std::min(distance,edits);
        }
        if(distance<best.edits){second=best.edits;best={crop,distance};}
        else second=std::min(second,distance);
    }
    if(best.edits>=1000||second<=best.edits)return {};
    return best;
}
inline std::vector<cv::Rect> FarmCropTiles(const cv::Mat& frame) {
    std::vector<cv::Rect> tiles;
    if(frame.empty())return tiles;
    cv::Rect area=cv::Rect(24,112,913,343)&cv::Rect(0,0,frame.cols,frame.rows);
    if(area.empty())return tiles;
    cv::Mat white;cv::inRange(frame(area),cv::Scalar(224,224,224),cv::Scalar(255,255,255),white);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(white,contours,cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
    for(const auto& contour:contours) {
        auto tile=cv::boundingRect(contour);
        if(tile.width<145||tile.width>190||tile.height<36||tile.height>60||cv::contourArea(contour)<4000)continue;
        tile.x+=area.x;tile.y+=area.y;tiles.push_back(tile);
    }
    std::sort(tiles.begin(),tiles.end(),[](auto a,auto b){return std::abs(a.y-b.y)<15?a.x<b.x:a.y<b.y;});
    return tiles;
}
inline cv::Point FarmCropClick(cv::Rect tile){return {tile.x+tile.width/2,tile.y+tile.height/2};}
inline cv::Point FarmCropCheckbox(cv::Rect tile){return {tile.x+tile.width-19,tile.y+tile.height/2};}
