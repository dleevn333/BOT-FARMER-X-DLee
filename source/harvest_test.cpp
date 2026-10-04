#include <opencv2/opencv.hpp>
#include <windows.h>
#include "farm_ocr.h"
#include <iostream>
#include <stdexcept>
static int checks=0;
static void Check(bool valid,const char* message){++checks;if(!valid)throw std::runtime_error(message);}
int main(int argc,char** argv){try {
    Check(argc==2,"Pass the isolated crop filter fixtures directory");
    for(int crop=0;crop<SO_NONG_SAN;++crop) {
        auto name=std::string(ds_hat_gionghai[crop]);
        for(const std::string& prefix:{std::string(),std::string("Hat "),std::string("Hat giong "),std::string("Qua ")})
            Check(FarmMatchCrop(prefix+name).id==crop,"All crop names and full-label prefixes resolve uniquely");
    }
    Check(FarmNormalize(L"Đậu tây")=="dautay","Normalize Vietnamese marks and crossed d");
    Check(FarmMatchCrop("daulay").id==2,"Recover one OCR letter error in Dau tay");
    Check(FarmMatchCrop("nhansan").id==30,"Recover one OCR letter error in Nhan sam");
    Check(FarmMatchCrop("trangkhuyel").id==29,"Recover a long multiline crop label");
    Check(FarmMatchCrop("xuongrongchoila").id==16,"Recover a long cactus label without losing its species");
    Check(FarmMatchCrop("duahau").id==21&&FarmMatchCrop("dua").id==22,"Dua is not a substring match for Dua hau");
    Check(FarmMatchCrop("taoduong").id==28&&FarmMatchCrop("tao").id==13,"Tao is not a substring match for Tao duong");
    Check(FarmMatchCrop("lua").id==20&&FarmMatchCrop("dua").id==22,"Short names use exact matching");
    for(const auto& text:{"", "nh", "dao", "ca", "xuongrong", "khuyet", "rau xa lach"})
        Check(FarmMatchCrop(text).id<0,"Reject incomplete, ambiguous or unsupported labels");
    auto current=cv::imread(std::string(argv[1])+"/filter-current.png");
    auto cells=FarmReadCropChoices(current);
    Check(cells.size()==4,"Detect the four actual compacted crop cards");
    for(int crop:{6,2,15}) {
        auto it=std::find_if(cells.begin(),cells.end(),[&](const auto& cell){return cell.id==crop;});
        Check(it!=cells.end(),"Read Bap, Dau tay and Nho from isolated labels");
        Check(it->tile.contains(FarmCropClick(it->tile))&&it->tile.contains(FarmCropCheckbox(it->tile)),"Clicks and checks stay inside the detected card");
    }
    Check(cells.back().text=="rauxalach"&&cells.back().id<0,"An unsupported crop must not become another selected crop");
    auto old=cv::imread(std::string(argv[1])+"/filter-special.png");
    auto special=FarmReadCropChoices(old);
    Check(special.size()==2,"Detect the old two-card filter without fixed column assumptions");
    for(int crop:{29,30})Check(std::any_of(special.begin(),special.end(),[&](const auto& cell){return cell.id==crop;}),"Read both lines of Trang khuyet and Nhan sam");
    cv::Mat empty(540,960,CV_8UC3,cv::Scalar(184,200,216));
    Check(FarmCropTiles(empty).empty(),"An empty filter does not invent clickable crops");
    cv::rectangle(empty,{47,139,165,44},cv::Scalar(245,245,245),cv::FILLED);
    cv::rectangle(empty,{232,196,165,44},cv::Scalar(245,245,245),cv::FILLED);
    auto shifted=FarmCropTiles(empty);
    Check(shifted.size()==2&&shifted[0].x==47&&shifted[1].y==196,"Follow shifted card coordinates instead of a guessed grid");
    std::cout<<checks<<" harvest checks passed\n";return 0;
}catch(const std::exception& error){std::cerr<<"Harvest check failed: "<<error.what()<<"\n";return 1;}}
