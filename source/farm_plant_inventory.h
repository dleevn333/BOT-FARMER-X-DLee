#pragma once
#include "farm_plant_vision.h"
#include <array>
#include <cstdint>
#include <atomic>
#include <string>

struct PlantInventorySlot {
    bool present=false;
    int page=-1;
    cv::Rect card;
    cv::Point point{-1,-1};
    double score=0;
    int quantity=-1;
};
struct PlantInventorySnapshot {
    std::array<PlantInventorySlot,SO_HAT_TRONG> seeds{};
    bool complete=false;
    int unreadCards=0;
    std::vector<std::string> pageKeys;
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
    void Consumed(int seed,int remaining=-1){
        if(!Has(seed))return;
        auto& slot=snapshot.seeds[seed];
        if(remaining>=0)slot.quantity=remaining;
        else if(slot.quantity>1)--slot.quantity;
        if(remaining==0)Exhausted(seed);
    }
};
inline void PlantInventoryRemember(PlantInventorySnapshot& result,int page,cv::Rect card,
    const PlantSeedIdentity& identity,bool replace=false,int quantity=-1) {
    if(identity.index<0||identity.index>=SO_HAT_TRONG||page<0)return;
    auto& slot=result.seeds[identity.index];
    if(replace||!slot.present||page<slot.page||(page==slot.page&&identity.score>slot.score))
        slot={quantity!=0,page,card,identity.point,identity.score,quantity};
}
struct PlantBagCursor {int page=0;bool localized=false;};
inline std::vector<int> PlantInventoryOrder(const PlantInventoryCache& cache,const bool* selected,int held=-1) {
    std::vector<int> order;
    for(int seed=0;seed<SO_HAT_TRONG;++seed)if(selected[seed]&&cache.Has(seed))order.push_back(seed);
    std::stable_sort(order.begin(),order.end(),[&](int a,int b){
        if(a==b)return false;
        if(a==held||b==held)return a==held;
        const auto& x=cache.snapshot.seeds[a];const auto& y=cache.snapshot.seeds[b];
        if(x.page!=y.page)return x.page<y.page;
        if(std::abs(x.card.y-y.card.y)>20)return x.card.y<y.card.y;
        return x.card.x<y.card.x;
    });
    return order;
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
