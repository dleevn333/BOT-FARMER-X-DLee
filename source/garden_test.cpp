#include "farm_garden_plan.h"
#include "farm_plant_inventory.h"
#include <iostream>
#include <stdexcept>
int main(int argc,char**argv){try{
    if(argc!=2&&argc!=3)throw std::runtime_error("Pass garden fixtures directory");
    int checks=0;auto check=[&](bool valid,const char* why){++checks;if(!valid)throw std::runtime_error(why);};
    auto frame=cv::imread(argc==3?argv[2]:std::string(argv[1])+"/garden-reference.png");
    GardenDetectionDebug debug;auto detected=GardenDetectPlots(frame,&debug);
    std::cout<<"min="<<debug.minimum<<"\n";
    for(int bank=0;bank<2;++bank){std::cout<<"bank "<<bank<<" "<<debug.bounds[bank]<<" rows:";for(auto y:debug.rows[bank])std::cout<<" "<<y;std::cout<<"\n";}
    std::cout<<"Detected "<<detected.size()<<" plots\n";
    for(auto p:detected)std::cout<<p.Center()<<" w="<<p.Width()<<" h="<<p.Height()<<"\n";
    if(argc==3)return 0;
    check(detected.size()==8,"Detect all eight actual beds, including partly covered edges");
    cv::Mat night;frame.convertTo(night,-1,.55);
    check(GardenDetectPlots(night).size()==8,"Keep all eight bed boundaries when daylight intensity falls to night");
    GardenPlan plan;plan.localized=true;check(plan.Merge(detected)==8,"Survey records every bed once");
    check(plan.Merge(detected)==0,"Repeated observations do not duplicate beds");
    plan.Build();check(plan.ready&&plan.nodes.size()>=64,"Build coverage waypoints for the complete large garden");
    std::vector<int> bedNodes(8);for(auto node:plan.nodes){check(node.plot>=0&&node.plot<8,"Every waypoint belongs to a known bed");++bedNodes[node.plot];}
    for(int count:bedNodes)check(count>=8,"Every bed receives multiple rows of planting probes");
    auto node=plan.Next(6);plan.Visit(node,6,false);check(plan.Next(6)!=node,"Do not revisit a failed point with the same seed");
    check(plan.Next(21)==node,"A different seed can test a point that did not fit the first type");
    plan.Visit(node,21,true);check(plan.Next(0)!=node&&plan.confirmed==1,"A planted point is excluded from all seed types");
    auto original=plan.nodes[0].point;cv::Mat move=(cv::Mat_<double>(2,3)<<1,0,-80,0,1,45);
    check(plan.Track(move),"Track camera movement without discarding visited points");
    auto now=GardenTransform(original,plan.worldToScreen);check(cv::norm(now-(original+cv::Point2f(-80,45)))<.1,"Map coordinates follow the camera correctly");
    check(!plan.Track(cv::Mat())&&!plan.localized&&plan.confirmed==1,"Lost localization pauses navigation while preserving completed work");
    cv::Mat texture(540,960,CV_8U);cv::RNG random(4517);random.fill(texture,cv::RNG::UNIFORM,40,210);cv::GaussianBlur(texture,texture,{3,3},0);
    cv::Mat ground;cv::cvtColor(texture,ground,cv::COLOR_GRAY2BGR);
    cv::Mat known=(cv::Mat_<double>(3,3)<<1.01,.003,-22,.002,.99,19,.00002,-.00007,1),warped;
    cv::warpPerspective(ground,warped,known,ground.size());cv::Mat measured;
    check(GardenGroundMotion(ground,warped,measured),"Track planar ground through perspective camera changes");
    std::vector<cv::Point2f> anchors{{300,200},{650,350}},expected,observed;
    cv::perspectiveTransform(anchors,expected,known);cv::perspectiveTransform(anchors,observed,measured);
    check(cv::norm(expected[0]-observed[0])<2&&cv::norm(expected[1]-observed[1])<2,"World positions remain accurate after perspective motion");
    check(plan.Track(measured),"Map accepts a full planar camera homography");
    auto rainyBefore=cv::imread(std::string(argv[1])+"/rain-before.png");
    auto rainyAfter=cv::imread(std::string(argv[1])+"/rain-after.png");
    check(GardenGroundMotion(rainyBefore,rainyAfter,measured),"Keep ground localization during a real rain and brightness transition");
    anchors={{300,300}};cv::perspectiveTransform(anchors,observed,measured);
    check(observed[0].y>330&&observed[0].y<352&&std::abs(observed[0].x-300)<15,"Rain motion follows actual bed displacement rather than fixed HUD text");
    check(GardenDetectPlots(cv::Mat()).empty(),"An empty capture is not a garden");
    cv::Mat empty(540,960,CV_8UC3,cv::Scalar(20,150,40));check(GardenDetectPlots(empty).empty(),"Grass alone cannot create planting beds");
    PlantInventoryCache stock;stock.valid=true;stock.snapshot.complete=true;
    PlantInventoryRemember(stock.snapshot,1,{310,145,138,175},{6,.98,.4,{380,230}},false,6);
    PlantInventoryRemember(stock.snapshot,0,{160,335,138,175},{4,.98,.4,{230,420}},false,2);
    PlantInventoryRemember(stock.snapshot,0,{10,145,138,175},{21,.98,.4,{80,230}},false,6);
    bool selected[SO_HAT_TRONG]={};for(int seed:{6,4,21,15})selected[seed]=true;
    auto order=PlantInventoryOrder(stock,selected,6);check(order==std::vector<int>({6,21,4}),"Use the selected held seed first, then nearby backpack pages; skip missing types");
    stock.Consumed(6);check(stock.snapshot.seeds[6].quantity==5&&stock.Has(6),"Confirmed planting decrements just that seed quantity");
    stock.Consumed(4,0);check(!stock.Has(4)&&stock.Has(21)&&stock.valid,"The last seed is exhausted without forgetting other packets");
    std::cout<<checks<<" garden and inventory checks passed\n";return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<"\n";return 1;}}
