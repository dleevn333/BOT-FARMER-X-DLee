#include <windows.h>
#include "farm_bag_vision.h"
#include <iostream>
#include <stdexcept>
int main(int argc,char**argv){try{
    if(argc!=3)throw std::runtime_error("Pass fixtures and images");
    int checks=0;auto check=[&](bool valid,const char* why){++checks;if(!valid)throw std::runtime_error(why);};
    auto frame=cv::imread(std::string(argv[1])+"/backpack-current.png");
    auto glyphs=PlantLoadGlyphs(argv[2]);auto digits=PlantLoadPacketDigits(argv[2]);auto cards=PlantBagCards(frame);
    check(cards.size()==11,"Find all eleven complete packets in the actual seed filter");
    const int ids[]={21,4,14,11,0,29,30,31,32,33,19};
    const int quantities[]={6,2,1,1,15,16,23,15,5,1,2};
    for(size_t i=0;i<cards.size();++i) {
        auto name=PlantPacketName(frame,cards[i]);auto identity=PlantDecodePacketIdentity(frame,cards[i],name,glyphs);
        int quantity=PlantPacketQuantity(frame,cards[i],digits);
        std::cout<<i<<" name="<<name<<" seed="<<identity.index<<" quantity="<<quantity<<"\n";
        check(identity.index==ids[i],"Verify the actual packet identity, never by position alone");
        check(quantity==quantities[i],"Read the actual quantity of every seed packet");
    }
    check(PlantPacketNameId("hatduahau")==21&&PlantPacketNameId("hatdua")==22,"Full seed names distinguish Dua from Dua hau");
    check(PlantPacketNameId("hatnuoc")<0,"An unknown label cannot become a known seed");
    auto wrong=PlantDecodePacketIdentity(frame,cards[0],"hatcarot",glyphs);
    check(wrong.index==21,"Conflicting OCR is corrected using the actual packet image");
    auto current=cv::imread(std::string(argv[1])+"/backpack-seeds-12.png");
    auto all=cv::imread(std::string(argv[1])+"/backpack-all.png");
    check(PlantBagSeedFilterSelected(current),"Selected seed filter is a filled pill");
    check(!PlantBagSeedFilterSelected(all),"Bright unselected text cannot count as selected seed filter");
    auto occluded=cv::imread(std::string(argv[1])+"/backpack-occluded.png");
    check(PlantBagSeedFilterSelected(occluded),"Selected filter remains recognized while hovered");
    auto hiddenCards=PlantBagCards(occluded);
    check(PlantDecodePacketIdentity(occluded,hiddenCards[3],PlantPacketName(occluded,hiddenCards[3]),glyphs).index<0,"An occluded icon remains unknown rather than becoming another seed");
    auto latest=PlantBagCards(current);
    check(latest.size()==12,"All twelve current seed packets fit one page");
    const int latestIds[]={6,21,4,14,11,0,29,30,31,32,33,19};
    const int latestQty[]={4,6,2,1,1,15,16,23,15,5,1,2};
    for(size_t i=0;i<latest.size();++i) {
        auto identity=PlantDecodePacketIdentity(current,latest[i],PlantPacketName(current,latest[i]),glyphs);
        std::cout<<"current "<<i<<" seed="<<identity.index<<" qty="<<PlantPacketQuantity(current,latest[i],digits)<<"\n";
        check(identity.index==latestIds[i],"Current packet identity survives changed positions");
        check(PlantPacketQuantity(current,latest[i],digits)==latestQty[i],"Current packet quantities match visible labels");
    }
    std::cout<<checks<<" real backpack checks passed\n";return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<"\n";return 1;}}
