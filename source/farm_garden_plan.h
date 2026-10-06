#pragma once
#include <opencv2/opencv.hpp>
#include <array>
#include <vector>
#include <algorithm>
#include <cstdint>
#include "farm_ground_motion.h"

struct GardenPlot {
    std::array<cv::Point2f,4> corners; // top-left, top-right, bottom-right, bottom-left
    cv::Point2f Center()const{return (corners[0]+corners[1]+corners[2]+corners[3])*.25f;}
    float Width()const{return (cv::norm(corners[1]-corners[0])+cv::norm(corners[2]-corners[3]))*.5f;}
    float Height()const{return (cv::norm(corners[3]-corners[0])+cv::norm(corners[2]-corners[1]))*.5f;}
    cv::Point2f Point(float u,float v)const {
        return (corners[0]*(1-u)+corners[1]*u)*(1-v)+(corners[3]*(1-u)+corners[2]*u)*v;
    }
};
inline cv::Point2f GardenTransform(cv::Point2f p,const cv::Matx33d& t) {
    auto v=t*cv::Vec3d(p.x,p.y,1);return {float(v[0]/v[2]),float(v[1]/v[2])};
}
struct GardenDetectionDebug {std::array<cv::Rect,2> bounds;std::array<std::vector<float>,2> rows;int minimum=0;};
inline std::vector<GardenPlot> GardenDetectPlots(const cv::Mat& frame,GardenDetectionDebug* debug=nullptr) {
    std::vector<GardenPlot> plots;if(frame.empty()||frame.size()!=cv::Size(960,540))return plots;
    cv::Mat hsv,gold,brown;cv::cvtColor(frame,hsv,cv::COLOR_BGR2HSV);
    cv::inRange(hsv,cv::Scalar(8,20,55),cv::Scalar(32,210,255),gold);
    cv::inRange(hsv,cv::Scalar(3,40,20),cv::Scalar(28,210,255),brown);
    std::vector<cv::Mat> channels;cv::split(hsv,channels);cv::Mat thin,contrast;
    cv::morphologyEx(channels[2],thin,cv::MORPH_TOPHAT,cv::getStructuringElement(cv::MORPH_RECT,{19,19}));
    cv::threshold(thin,contrast,10,255,cv::THRESH_BINARY);cv::bitwise_and(gold,contrast,gold);
    gold.rowRange(0,5).setTo(0);gold.rowRange(510,540).setTo(0);
    gold.colRange(0,20).setTo(0);gold.colRange(935,960).setTo(0);gold.colRange(430,530).setTo(0);
    struct Edge {float mid=0,slope=0,intercept=0,top=0,bottom=0,weight=0,left=0,right=0;};
    std::vector<cv::Vec4i> lines;cv::HoughLinesP(gold,lines,1,CV_PI/180,40,145,75);
    std::vector<Edge> segments;
    for(auto line:lines) {
        int dy=line[3]-line[1],dx=line[2]-line[0];
        if(std::abs(dy)<145||std::abs(dx)>std::abs(dy)*.4)continue;
        float slope=float(dx)/dy,intercept=line[0]-slope*line[1];
        segments.push_back({slope*280+intercept,slope,intercept,float(std::min(line[1],line[3])),float(std::max(line[1],line[3])),float(std::abs(dy)),0,0});
    }
    std::sort(segments.begin(),segments.end(),[](auto a,auto b){return a.mid<b.mid;});
    std::vector<Edge> edges;
    for(auto edge:segments) {
        if(edges.empty()||edge.mid-edges.back().mid>18)edges.push_back(edge);
        else {
            auto& group=edges.back();float sum=group.weight+edge.weight;
            group.mid=(group.mid*group.weight+edge.mid*edge.weight)/sum;
            group.slope=(group.slope*group.weight+edge.slope*edge.weight)/sum;
            group.intercept=(group.intercept*group.weight+edge.intercept*edge.weight)/sum;
            group.top=std::min(group.top,edge.top);group.bottom=std::max(group.bottom,edge.bottom);group.weight=sum;
        }
    }
    for(auto& edge:edges) {
        int left=0,right=0,total=0;
        for(int y=cvRound(edge.top)+5;y<cvRound(edge.bottom)-5;y+=5) {
            int x=cvRound(edge.slope*y+edge.intercept);if(y<195&&x>595)continue;
            for(int offset:{12,17,22})if(x-offset>=0&&x+offset<960) {left+=brown.at<uchar>(y,x-offset)>0;right+=brown.at<uchar>(y,x+offset)>0;++total;}
        }
        if(total){edge.left=float(left)/total;edge.right=float(right)/total;}
    }
    struct Bank {Edge left,right;bool found=false;std::vector<float> rows;};std::array<Bank,2> banks;
    for(int side=0;side<2;++side) {
        float best=0;
        for(auto left:edges)if(((left.mid<480)==(side==0))&&left.right>.3&&left.right-left.left>.12)
        for(auto right:edges)if(((right.mid<480)==(side==0))&&right.left>.3&&right.left-right.right>.12) {
            float width=right.mid-left.mid,overlap=std::min(left.bottom,right.bottom)-std::max(left.top,right.top);
            if(width<130||width>460||overlap<140)continue;
            float score=overlap*(left.right-left.left+right.left-right.right);
            if(score>best){best=score;banks[side].left=left;banks[side].right=right;banks[side].found=true;}
        }
    }
    cv::Mat rowsMask=gold.clone();rowsMask(cv::Rect(600,0,335,195)).setTo(0);rowsMask(cv::Rect(550,0,385,45)).setTo(0);
    rowsMask(cv::Rect(875,195,85,95)).setTo(0);rowsMask(cv::Rect(850,280,110,85)).setTo(0);
    cv::HoughLinesP(rowsMask,lines,1,CV_PI/180,20,40,30);
    for(int side=0;side<2;++side) {
        auto& bank=banks[side];if(!bank.found)continue;
        float top=std::min(bank.left.top,bank.right.top),bottom=std::max(bank.left.bottom,bank.right.bottom);
        std::vector<float> levels;
        if(top>7)levels.push_back(top);if(bottom<505)levels.push_back(bottom);
        for(auto line:lines) {
            int x1=std::min(line[0],line[2]),x2=std::max(line[0],line[2]);float y=(line[1]+line[3])*.5f;
            if(std::abs(line[1]-line[3])>10||y<top-6||y>bottom+6)continue;
            float left=bank.left.slope*y+bank.left.intercept,right=bank.right.slope*y+bank.right.intercept;
            if(x1<left-30||x2>right+30)continue;
            if(x2-x1<(side?40:(right-left)*.4f))continue;
            if(std::abs(x1-left)>30&&std::abs(x2-right)>30)continue;
            levels.push_back(y);
        }
        std::sort(levels.begin(),levels.end());
        for(float y:levels)if(bank.rows.empty()||y-bank.rows.back()>13)bank.rows.push_back(y);else bank.rows.back()=(bank.rows.back()+y)*.5f;
        if(bank.rows.size()>=4) {
            std::vector<float> gaps;for(size_t i=1;i<bank.rows.size();++i)gaps.push_back(bank.rows[i]-bank.rows[i-1]);
            std::sort(gaps.begin(),gaps.end());float spacing=gaps[gaps.size()/2]*.55f;
            std::vector<float> coherent;
            for(float y:bank.rows) {
                if(coherent.empty()||y-coherent.back()>=spacing)coherent.push_back(y);
                else if(std::abs(y-bottom)<15)coherent.back()=y;
            }
            bank.rows=coherent;
        }
        if(debug)debug->bounds[side]=cv::Rect(cvRound(bank.left.mid),cvRound(top),cvRound(bank.right.mid-bank.left.mid),cvRound(bottom-top));
    }
    auto& left=banks[0].rows;auto& right=banks[1].rows;
    if(left.size()>=3&&right.size()>=3) {
        int support=0;float offset=0;
        for(float a:left)for(float b:right)if(std::abs(a-b)<40) {
            float candidate=b-a;int matched=0;
            for(float level:left)if(std::any_of(right.begin(),right.end(),[&](float other){return std::abs(other-level-candidate)<12;}))++matched;
            if(matched>support){support=matched;offset=candidate;}
        }
        if(support>=2) {
            std::vector<float> aligned;
            for(float level:left) {
                float expected=level+offset;
                auto nearestBoundary=std::min_element(right.begin(),right.end(),[&](float a,float b){return std::abs(a-expected)<std::abs(b-expected);});
                float y=std::abs(*nearestBoundary-expected)<18?*nearestBoundary:expected;
                if(y>=std::min(banks[1].left.top,banks[1].right.top)-15&&y<=std::max(banks[1].left.bottom,banks[1].right.bottom)+15)aligned.push_back(y);
            }
            right=aligned;
        }
    }
    for(int side=0;side<2;++side) {
        const auto& bank=banks[side];if(debug)debug->rows[side]=bank.rows;
        if(!bank.found)continue;
        for(size_t i=1;i<bank.rows.size();++i) {
            float top=bank.rows[i-1],bottom=bank.rows[i];if(bottom-top<45||bottom-top>220)continue;
            GardenPlot plot{{cv::Point2f(bank.left.slope*top+bank.left.intercept,top),cv::Point2f(bank.right.slope*top+bank.right.intercept,top),
                cv::Point2f(bank.right.slope*bottom+bank.right.intercept,bottom),cv::Point2f(bank.left.slope*bottom+bank.left.intercept,bottom)}};
            std::vector<cv::Point> polygon;for(auto p:plot.corners)polygon.push_back(cv::Point(cvRound(p.x),cvRound(p.y)));
            cv::Mat mask=cv::Mat::zeros(frame.size(),CV_8U);cv::fillConvexPoly(mask,polygon,255);cv::Mat soil;cv::bitwise_and(mask,brown,soil);
            if(cv::countNonZero(mask)>5000&&double(cv::countNonZero(soil))/cv::countNonZero(mask)>.20)plots.push_back(plot);
        }
    }
    std::sort(plots.begin(),plots.end(),[](auto a,auto b){return a.Center().y<b.Center().y;});return plots;
}
struct GardenNode {
    cv::Point2f point;int plot=0;bool planted=false;
    std::uint64_t attemptedSeeds=0;
};
struct GardenPlan {
    std::vector<GardenPlot> plots;
    std::vector<GardenNode> nodes;
    cv::Matx33d worldToScreen=cv::Matx33d::eye();
    std::uintptr_t gameKey=0;
    bool ready=false,localized=false;
    unsigned visited=0,confirmed=0;
    cv::Mat lastFrame;
    cv::Mat homeFrame;
    cv::Matx33d homePose=cv::Matx33d::eye();
    bool homeCalibrated=false;
    void Clear(){plots.clear();nodes.clear();lastFrame.release();homeFrame.release();homeCalibrated=false;ready=false;localized=false;visited=confirmed=0;worldToScreen=cv::Matx33d::eye();}
    bool Track(const cv::Mat& affine) {
        if(affine.empty()){localized=false;return false;}
        cv::Matx33d step=cv::Matx33d::eye();
        if((affine.rows!=2&&affine.rows!=3)||affine.cols!=3){localized=false;return false;}
        for(int y=0;y<affine.rows;++y)for(int x=0;x<3;++x)step(y,x)=affine.at<double>(y,x);
        worldToScreen=step*worldToScreen;localized=true;return true;
    }
    int Merge(const std::vector<GardenPlot>& visible) {
        if(!localized)return 0;auto inverse=worldToScreen.inv();int added=0;
        for(auto plot:visible) {
            for(auto& p:plot.corners)p=GardenTransform(p,inverse);
            bool exists=false;
            for(auto& old:plots) {
                std::vector<cv::Point2f> a(old.corners.begin(),old.corners.end()),b(plot.corners.begin(),plot.corners.end()),intersection;
                double overlap=cv::intersectConvexConvex(a,b,intersection);
                if(overlap/std::min(std::abs(cv::contourArea(a)),std::abs(cv::contourArea(b)))>.55||cv::norm(old.Center()-plot.Center())<std::min(old.Height(),plot.Height())*.55f) {
                    exists=true;if(plot.Height()>old.Height()*1.1)old=plot;break;
                }
            }
            if(!exists){plots.push_back(plot);++added;}
        }
        return added;
    }
    void Build() {
        nodes.clear();visited=confirmed=0;
        std::sort(plots.begin(),plots.end(),[](auto a,auto b){return a.Center().y>b.Center().y;});
        int row=0;
        for(size_t first=0;first<plots.size();++row) {
            size_t last=first+1;float y=plots[first].Center().y;
            while(last<plots.size()&&std::abs(plots[last].Center().y-y)<plots[first].Height()*.4f)++last;
            std::sort(plots.begin()+first,plots.begin()+last,[row](auto a,auto b){return row%2?a.Center().x>b.Center().x:a.Center().x<b.Center().x;});
            first=last;
        }
        for(int plot=0;plot<int(plots.size());++plot) {
            auto& bed=plots[plot];
            int cols=std::clamp(cvRound(bed.Width()/48.f),3,7),rows=std::clamp(cvRound(bed.Height()/38.f),2,5);
            for(int y=0;y<rows;++y)for(int step=0;step<cols;++step) {
                int x=y%2?cols-1-step:step;
                float u=.15f+.70f*x/std::max(1,cols-1),v=.80f-.60f*y/std::max(1,rows-1);
                nodes.push_back({bed.Point(u,v),plot,false,0});
            }
        }
        ready=!nodes.empty();
    }
    int Next(int seed)const {
        if(seed<0||seed>=64)return -1;
        for(int i=0;i<int(nodes.size());++i)if(!nodes[i].planted&&!(nodes[i].attemptedSeeds&(1ull<<seed)))return i;
        return -1;
    }
    void Visit(int index,int seed,bool planted) {
        if(index<0||index>=int(nodes.size())||seed<0||seed>=64)return;
        auto& n=nodes[index];
        if(!n.attemptedSeeds)++visited;
        n.attemptedSeeds|=1ull<<seed;
        if(planted&&!n.planted){n.planted=true;++confirmed;}
    }
};
