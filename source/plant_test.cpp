#include "farm_plant_vision.h"
#include "farm_plant_inventory.h"
#include <iostream>
#include <stdexcept>
static int checks=0;
static void Check(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
int main(int argc,char** argv){try {
    Check(argc==3,"Pass assets and image directories");
    std::string assets=argv[1],images=argv[2];
    std::vector<cv::Mat> glyphs;
    for(int i=0;i<SO_HAT_TRONG;++i)glyphs.push_back(PlantSeedGlyph(cv::imread(images+"/"+ds_anh_hat_trong[i]),i>=29));
    for(auto& glyph:glyphs)Check(!glyph.empty(),"Every selectable seed has a recognition asset");
    auto bag=cv::imread(assets+"/bag-all.png");
    Check(PlantBagCards(bag).size()==12,"Find all observed backpack cards");
    Check(PlantIdentifySeed(bag,{55,200,105,80},glyphs).index==-1,"Watering tool must never become a seed");
    const int expected[]={6,4,14,11,0};
    for(int col=1;col<6;++col)Check(PlantIdentifySeed(bag,{55+150*col,200,105,80},glyphs).index==expected[col-1],"Distinguish ordinary seeds");
    for(int col=0;col<5;++col)Check(PlantIdentifySeed(bag,{55+150*col,390,105,80},glyphs).index==29+col,"Identify bag-only event seeds");
    auto ref=cv::imread(assets+"/ring-reference.png");
    auto ring=PlantFindValidRing(ref,{190,65});
    Check(ring.center.x>=0&&cv::norm(ring.center-cv::Point(128,57))<8,"Recognize user's valid cyan planting ring");
    auto live=cv::imread(assets+"/live-blue-ring.jpg");
    auto smallRing=PlantFindValidRing(live);
    Check(smallRing.center.x>=0&&cv::norm(smallRing.center-cv::Point(453,305))<6,"Recognize actual small ring after moving off an occupied crop");
    auto noRing=cv::Mat(540,960,CV_8UC3,cv::Scalar(55,85,115));
    Check(PlantFindValidRing(noRing).center.x<0,"Bare brown soil is not permission to plant");
    cv::Mat hsv;cv::cvtColor(ref,hsv,cv::COLOR_BGR2HSV);
    for(int y=0;y<hsv.rows;++y)for(int x=0;x<hsv.cols;++x)if(hsv.at<cv::Vec3b>(y,x)[0]>82&&hsv.at<cv::Vec3b>(y,x)[0]<108)hsv.at<cv::Vec3b>(y,x)[0]=0;
    cv::Mat red;cv::cvtColor(hsv,red,cv::COLOR_HSV2BGR);
    Check(PlantFindValidRing(red,{190,65}).center.x<0,"Reject an invalid red marker");
    auto night=cv::imread(assets+"/world-home-night.png");
    if(night.rows==580)night=night(cv::Rect(0,40,960,540)).clone();
    Check(PlantNextSoilSpot(night,{}).x>=0,"Find soil in the actual night farm");
    cv::Mat texture(540,960,CV_8UC3);cv::RNG rng(31415);rng.fill(texture,cv::RNG::UNIFORM,30,220);
    cv::GaussianBlur(texture,texture,{5,5},0);
    cv::Mat shifted,affine=(cv::Mat_<double>(2,3)<<1,0,-18,0,1,24);
    cv::warpAffine(texture,shifted,affine,texture.size());cv::Mat transform;
    Check(PlantTrackCamera(texture,shifted,transform),"Track known camera movement");
    Check(std::abs(transform.at<double>(0,2)+18)<1&&std::abs(transform.at<double>(1,2)-24)<1,"Camera translation direction stays correct");
    std::vector<cv::Point2f> seen{{400,240}};PlantTransformSpots(seen,transform);
    Check(cv::norm(seen[0]-cv::Point2f(382,264))<1,"Keep visited soil attached to ground");
    Check(!PlantTrackCamera(noRing,noRing,transform),"Do not invent movement on textureless frames");
    auto count13=cv::imread(assets+"/count-thirteen.png");
    auto count11=cv::imread(assets+"/count-eleven.png");
    auto thirteen=PlantCountMask(count13),eleven=PlantCountMask(count11);
    Check(PlantCountChanged(thirteen,eleven),"Short x11 count is confirmed by its changed glyphs when OCR omits it");
    Check(!PlantCountChanged(thirteen,thirteen),"An unchanged quantity never confirms planting");
    Check(!PlantCountChanged(thirteen,cv::Mat::zeros(thirteen.size(),CV_8U)),"A faded or missing count never confirms planting");
    PlantInventorySnapshot observed;
    for(auto card:PlantBagCards(bag)) {
        auto identity=PlantIdentifySeed(bag,{card.x+18,card.y+55,card.width-36,76},glyphs);
        PlantInventoryRemember(observed,0,card,identity);
    }
    observed.complete=true;
    PlantInventoryCache cache;int scans=0;
    auto scan=[&](PlantInventorySnapshot& result){++scans;result=observed;return true;};
    for(int seed:{0,1,2,4,6,29,30,33}) {
        Check(PlantInventoryEnsure(cache,101,scan),"Multiple selections share a completed backpack scan");
        Check(cache.Has(seed)==observed.seeds[seed].present,"Remember both available and absent seeds");
    }
    Check(scans==1&&cache.scans==1,"Eight selected seed types open one full scan, including missing types");
    auto oldPoint=cache.snapshot.seeds[4].point;
    cache.Exhausted(4);
    Check(!cache.Has(4)&&cache.Has(6)&&cache.valid,"Consuming the last potato preserves the remembered corn and missing types");
    Check(PlantInventoryEnsure(cache,101,scan)&&scans==1,"The next planting round reuses inventory after exhaustion");
    cache.Purchased(0);
    Check(PlantInventoryEnsure(cache,101,scan)&&scans==1,"An empty shop does not trigger a needless inventory scan");
    cache.Purchased(1);
    Check(!cache.valid&&PlantInventoryEnsure(cache,101,scan)&&scans==2&&cache.Has(4),"A successful purchase refreshes all types once");
    for(int seed:{1,2,4,6,33})Check(PlantInventoryEnsure(cache,101,scan),"Remaining selections reuse the purchased inventory snapshot");
    Check(scans==2,"Purchase refresh does not become one scan per seed");
    PlantSeedIdentity moved{4,.98,.5,oldPoint+cv::Point(150,0)};
    PlantInventoryRemember(cache.snapshot,0,{oldPoint.x+132,oldPoint.y-55,138,175},moved,true);
    Check(cache.snapshot.seeds[4].point==moved.point&&cache.Has(6),"Repair a compacted page without losing other remembered seeds");
    Check(PlantInventoryEnsure(cache,102,scan)&&scans==3,"A changed LDPlayer render handle cannot reuse stale positions");
    PlantInventoryCache other;int otherScans=0;
    Check(PlantInventoryEnsure(other,202,[&](PlantInventorySnapshot& result){++otherScans;result.complete=true;return true;}),"A second LDPlayer keeps its own empty inventory");
    for(int seed:{0,1,2,4}) {
        Check(PlantInventoryEnsure(other,202,[&](PlantInventorySnapshot&){++otherScans;return false;}),"An empty backpack is a complete reusable snapshot");
        Check(!other.Has(seed),"Empty stock skips each missing selection without scanning");
    }
    Check(otherScans==1&&cache.Has(0),"Empty inventory does not overwrite another instance");
    cache.Invalidate();
    Check(!PlantInventoryEnsure(cache,102,[&](PlantInventorySnapshot& result){result=observed;result.complete=false;return true;}),"A page limit or interrupted scan cannot be committed as complete");
    Check(!cache.valid&&!cache.Has(0),"A partial scan never reports trustworthy presence or absence");
    Check(PlantInventoryEnsure(cache,102,scan)&&scans==4,"Retry a failed scan once, then reuse the complete result");
    Check(!PlantInventoryEnsure(cache,0,scan)&&scans==4,"Do not scan without a valid game handle");
    std::cout<<checks<<" planting vision checks passed\n";return 0;
}catch(const std::exception& e){std::cerr<<"Planting check failed: "<<e.what()<<"\n";return 1;}}
