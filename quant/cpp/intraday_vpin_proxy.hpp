#pragma once
#include "stats.hpp"
#include <algorithm>
#include <cmath>
#include <vector>
namespace kline {
inline double volume_bucket_imbalance_proxy(
    const std::vector<double>& buy_volume,
    const std::vector<double>& sell_volume){
    check_finite(buy_volume);check_finite(sell_volume);
    require(buy_volume.size()==sell_volume.size(),
            "VPIN bucket shape mismatch");
    double total=0,imbalance=0;
    for(int i=0;i<int(buy_volume.size());i++){
        require(buy_volume[i]>=0&&sell_volume[i]>=0,
                "Negative classified trade volume");
        total+=buy_volume[i]+sell_volume[i];
        imbalance+=std::fabs(buy_volume[i]-sell_volume[i]);
    }
    require(total>0,"VPIN proxy needs positive trading volume");
    return imbalance/total;
}
} // namespace kline
