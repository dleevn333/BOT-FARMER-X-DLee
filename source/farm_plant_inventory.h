#pragma once
#include "farm_plant_vision.h"
#include <array>
#include <cstdint>
#include <atomic>

struct PlantInventorySlot {
    bool present=false;
    int page=-1;
    cv::Rect card;
    cv::Point point{-1,-1};
    double score=0;
};
struct PlantInventorySnapshot {
    std::array<PlantInventorySlot,SO_HAT_TRONG> seeds{};
    bool complete=false;
};
struct PlantInventoryCache {
    PlantInventorySnapshot snapshot;
    std::uintptr_t gameKey=0;
    std::atomic<bool> valid{false};
    std::atomic<unsigned> scans{0};
    void Invalidate(){valid=false;}
    void Purchased(int bought){if(bought>0)Invalidate();}
    bool Has(int seed)const{return valid&&seed>=0&&seed<SO_HAT_TRONG&&snapshot.seeds[seed].present;}
    void Exhausted(int seed){if(seed>=0&&seed<SO_HAT_TRONG)snapshot.seeds[seed]=PlantInventorySlot{};}
};
inline void PlantInventoryRemember(PlantInventorySnapshot& result,int page,cv::Rect card,
    const PlantSeedIdentity& identity,bool replace=false) {
    if(identity.index<0||identity.index>=SO_HAT_TRONG||page<0)return;
    auto& slot=result.seeds[identity.index];
    if(replace||!slot.present||page<slot.page||(page==slot.page&&identity.score>slot.score))
        slot={true,page,card,identity.point,identity.score};
}
template<class Scanner>
inline bool PlantInventoryEnsure(PlantInventoryCache& cache,std::uintptr_t gameKey,Scanner&& scanner) {
    if(!gameKey)return false;
    if(cache.gameKey!=gameKey){cache.gameKey=gameKey;cache.Invalidate();}
    if(cache.valid)return true;
    PlantInventorySnapshot next;
    ++cache.scans;
    if(!scanner(next)||!next.complete)return false;
    cache.snapshot=std::move(next);cache.valid=true;
    return true;
}
