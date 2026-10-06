#pragma once
#include "farm_ocr.h"
#include "farm_plant_vision.h"
#include <filesystem>

inline bool PlantBagSeedFilterSelected(const cv::Mat& frame) {
    if(frame.empty()||frame.size()!=cv::Size(960,540))return false;
    cv::Mat pale;cv::inRange(frame(cv::Rect(128,92,95,25)),cv::Scalar(90,110,170),cv::Scalar(220,240,255),pale);
    return cv::countNonZero(pale)>pale.total()*.60;
}

inline std::string PlantPacketName(const cv::Mat& frame,cv::Rect card) {
    std::string name;for(const auto& word:FarmReadText(frame,{card.x+4,card.y+3,card.width-8,40}))name+=word.text;
    return name;
}
inline int PlantPacketNameId(const std::string& name) {
    for(int seed=0;seed<SO_HAT_TRONG;++seed) {
        std::string title=ds_hat_trong[seed];auto suffix=title.find(" (");if(suffix!=std::string::npos)title.resize(suffix);
        auto key=FarmCropKey(title);
        for(const auto& prefix:{std::string(),std::string("hat"),std::string("hatgiong")})if(name==prefix+key)return seed;
    }
    auto crop=FarmMatchCrop(name);if(crop.id>=0&&crop.id<29)return crop.id;
    return -1;
}
struct PlantPacketDigit {int value;cv::Mat mask;};
inline std::vector<PlantPacketDigit> PlantLoadPacketDigits(const std::string& directory) {
    std::vector<PlantPacketDigit> samples;const std::string prefix="plant_packet_digit_";
    if(!std::filesystem::exists(directory))return samples;
    for(const auto& entry:std::filesystem::directory_iterator(directory)) {
        auto name=entry.path().filename().string();
        if(name.rfind(prefix,0)!=0||name.size()<=prefix.size()||name[prefix.size()]<'0'||name[prefix.size()]>'9')continue;
        auto mask=cv::imread(entry.path().string(),cv::IMREAD_GRAYSCALE);
        if(!mask.empty())samples.push_back({name[prefix.size()]-'0',mask});
    }
    return samples;
}
inline cv::Mat PlantNormalizePacketDigit(const cv::Mat& face) {
    cv::Mat canvas=cv::Mat::zeros(36,28,CV_8U),sized;
    double scale=std::min(24./face.cols,30./face.rows);
    cv::resize(face,sized,{std::max(1,cvRound(face.cols*scale)),std::max(1,cvRound(face.rows*scale))},0,0,cv::INTER_NEAREST);
    sized.copyTo(canvas(cv::Rect((28-sized.cols)/2,(36-sized.rows)/2,sized.cols,sized.rows)));return canvas;
}
inline int PlantPacketQuantity(const cv::Mat& frame,cv::Rect card,const std::vector<PlantPacketDigit>& samples) {
    auto roi=cv::Rect(card.x+25,card.y+108,card.width-50,36)&cv::Rect(0,0,frame.cols,frame.rows);
    if(roi.empty())return -1;
    cv::Mat white,labels,stats,centers;cv::inRange(frame(roi),cv::Scalar(195,195,195),cv::Scalar(255,255,255),white);
    int n=cv::connectedComponentsWithStats(white,labels,stats,centers);
    std::vector<std::pair<cv::Rect,int>> parts;
    for(int i=1;i<n;++i) {
        cv::Rect b(stats.at<int>(i,cv::CC_STAT_LEFT),stats.at<int>(i,cv::CC_STAT_TOP),stats.at<int>(i,cv::CC_STAT_WIDTH),stats.at<int>(i,cv::CC_STAT_HEIGHT));
        if(stats.at<int>(i,cv::CC_STAT_AREA)>=10&&b.height>=8&&b.height<=27&&b.width>=2&&b.width<=26
           &&centers.at<double>(i,0)>roi.width*.18&&centers.at<double>(i,0)<roi.width*.82)parts.push_back({b,i});
    }
    std::sort(parts.begin(),parts.end(),[](auto a,auto b){return a.first.x<b.first.x;});
    if(parts.empty()||parts.size()>4)return -1;
    cv::Mat clean=cv::Mat::zeros(white.size(),CV_8U);
    for(auto part:parts)clean.setTo(255,labels==part.second);
    auto bounds=cv::boundingRect(clean);cv::Mat readable;
    cv::bitwise_not(clean(bounds),readable);cv::copyMakeBorder(readable,readable,14,14,14,14,cv::BORDER_CONSTANT,cv::Scalar(255));
    cv::cvtColor(readable,readable,cv::COLOR_GRAY2BGR);
    for(const auto& word:FarmReadText(readable,{0,0,readable.cols,readable.rows})) {
        int digits=0,value=0;bool numeric=true;
        for(auto c:word.raw)if(c>=L'0'&&c<=L'9'){++digits;value=value*10+(c-L'0');}else if(!iswspace(c))numeric=false;
        if(numeric&&digits==int(parts.size()))return value;
    }
    int quantity=0;
    for(auto part:parts) {
        cv::Mat face=labels(part.first)==part.second;
        auto normalized=PlantNormalizePacketDigit(face);
        std::array<double,10> scores{};
        for(const auto& sample:samples) {
            if(sample.mask.size()!=normalized.size())continue;
            cv::Mat both,either;cv::bitwise_and(normalized,sample.mask,both);cv::bitwise_or(normalized,sample.mask,either);
            double score=double(cv::countNonZero(both))/std::max(1,cv::countNonZero(either));
            scores[sample.value]=std::max(scores[sample.value],score);
        }
        int best=int(std::max_element(scores.begin(),scores.end())-scores.begin());double second=0;
        for(int digit=0;digit<10;++digit)if(digit!=best)second=std::max(second,scores[digit]);
        if(scores[best]<.82||scores[best]-second<.10)return -1;
        quantity=quantity*10+best;
    }
    return quantity;
}
inline int PlantPacketQuantity(const cv::Mat& frame,cv::Rect card) {
    static const auto samples=PlantLoadPacketDigits("images");return PlantPacketQuantity(frame,card,samples);
}
inline PlantSeedIdentity PlantDecodePacketIdentity(const cv::Mat& frame,cv::Rect card,const std::string& name,const std::vector<cv::Mat>& glyphs) {
    auto region=cv::Rect(card.x+18,card.y+55,card.width-36,76);
    int named=PlantPacketNameId(name);
    if(named>=0) {
        auto only=glyphs;
        for(int i=0;i<int(only.size());++i) {
            int seed=i<SO_HAT_TRONG?i:i==SO_HAT_TRONG?30:31;
            if(seed!=named)only[i]=cv::Mat();
        }
        auto match=PlantIdentifySeed(frame,region,only);
        if(match.index==named)return match;
    }
    return PlantIdentifySeed(frame,region,glyphs);
}
