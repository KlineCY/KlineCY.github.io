#pragma once
#include "portfolio.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline Matrix categorical_transition_probabilities(
    const std::vector<int>& states,int categories,double pseudocount=0){
    require(states.size()>=2&&categories>=2&&categories<=1000&&
            std::isfinite(pseudocount)&&pseudocount>=0,
            "Invalid Markov state transition data");
    Matrix transitions(categories,std::vector<double>(categories,pseudocount));
    for(int i=1;i<int(states.size());i++){
        require(states[i]>=0&&states[i]<categories&&
                states[i-1]>=0&&states[i-1]<categories,"State out of range");
        transitions[states[i-1]][states[i]]++;
    }
    for(auto& row:transitions){
        double total=0;
        for(double v:row)total+=v;
        require(total>0,"Unvisited Markov state requires pseudocount");
        for(double& v:row)v/=total;
    }
    return transitions;
}
} // namespace kline
