#pragma once
#include "portfolio.hpp"
#include <cmath>
#include <vector>
namespace kline {
inline std::vector<double> credit_rating_distribution(
    const std::vector<double>& initial,const Matrix& transition,
    int years){
    check_finite(initial);
    require(!initial.empty()&&transition.size()==initial.size()&&years>=0,
            "Invalid rating transition dimensions");
    double sum=0;
    for(double p:initial){require(p>=0,"Negative initial rating mass");sum+=p;}
    require(std::fabs(sum-1)<1e-10,"Initial rating state mass must sum to one");
    int n=int(initial.size());
    for(const auto& row:transition){
        check_finite(row);require(int(row.size())==n,"Transition row dimensions");
        double s=0;for(double p:row){require(p>=0,"Negative migration probability");s+=p;}
        require(std::fabs(s-1)<1e-10,"Migration rows must sum to one");
    }
    std::vector<double> current=initial;
    for(int step=0;step<years;step++){
        std::vector<double> next(n);
        for(int i=0;i<n;i++)for(int j=0;j<n;j++)
            next[j]+=current[i]*transition[i][j];
        current=next;
    }
    return current;
}
inline double credit_migration_expected_loss(
    const std::vector<double>& ending_rating_distribution,
    const std::vector<double>& rating_loss_given_state){
    check_finite(ending_rating_distribution);
    check_finite(rating_loss_given_state);
    require(ending_rating_distribution.size()==rating_loss_given_state.size(),
            "Migration loss shape");
    double result=0,total=0;
    for(int i=0;i<int(ending_rating_distribution.size());i++){
        require(ending_rating_distribution[i]>=0&&
                rating_loss_given_state[i]>=0,"Migration loss input");
        result+=ending_rating_distribution[i]*rating_loss_given_state[i];
        total+=ending_rating_distribution[i];
    }
    require(std::fabs(total-1)<1e-9,"Rating distribution is not normalized");
    return result;
}
} // namespace kline
