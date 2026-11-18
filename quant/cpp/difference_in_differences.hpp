#pragma once
#include "stats.hpp"
#include <array>
#include <cmath>
#include <vector>
namespace kline {
struct DifferenceInDifferences{
    double treatment_effect=0,standard_error=0;
    std::array<double,4> cell_means{};
};
inline DifferenceInDifferences difference_in_differences(
    const std::vector<double>& outcomes,const std::vector<int>& treated,
    const std::vector<int>& post){
    check_finite(outcomes);
    require(outcomes.size()==treated.size()&&
            outcomes.size()==post.size(),"Difference-in-differences shape");
    std::array<double,4> sum{},sum_squares{};
    std::array<int,4> count{};
    for(int i=0;i<int(outcomes.size());i++){
        require((treated[i]==0||treated[i]==1)&&
                (post[i]==0||post[i]==1),"Binary group and period indicators");
        int index=2*treated[i]+post[i];
        sum[index]+=outcomes[i];
        sum_squares[index]+=outcomes[i]*outcomes[i];
        count[index]++;
    }
    DifferenceInDifferences fit;
    double se2=0;
    for(int j=0;j<4;j++){
        require(count[j]>=2,"At least two observations per treatment-time cell");
        fit.cell_means[j]=sum[j]/count[j];
        double sample_variance=(sum_squares[j]-
             sum[j]*sum[j]/count[j])/(count[j]-1);
        se2+=std::max(0.0,sample_variance)/count[j];
    }
    fit.treatment_effect=(fit.cell_means[3]-fit.cell_means[2])-
                         (fit.cell_means[1]-fit.cell_means[0]);
    fit.standard_error=std::sqrt(se2);
    return fit;
}
} // namespace kline
