#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
namespace kline {
struct VolumeBar{double volume=0,vwap=0;};
inline std::vector<VolumeBar> fixed_volume_bars(
    const std::vector<double>& trade_prices,
    const std::vector<double>& trade_sizes,double target_volume){
    check_finite(trade_prices);check_finite(trade_sizes);
    require(trade_prices.size()==trade_sizes.size()&&
            std::isfinite(target_volume)&&target_volume>0,
            "Invalid volume bar target");
    std::vector<VolumeBar> bars;
    double volume=0,notional=0;
    for(int i=0;i<int(trade_prices.size());i++){
        require(trade_prices[i]>0&&trade_sizes[i]>=0,
                "Invalid trade for volume bar");
        double remain=trade_sizes[i];
        while(remain>1e-12){
            double part=std::min(remain,target_volume-volume);
            volume+=part;notional+=trade_prices[i]*part;remain-=part;
            if(volume>=target_volume-1e-12){
                bars.push_back({target_volume,notional/volume});
                volume=0;notional=0;
            }
        }
    }
    if(volume>1e-12)bars.push_back({volume,notional/volume});
    return bars;
}
} // namespace kline
