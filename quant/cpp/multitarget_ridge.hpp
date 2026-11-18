#pragma once
#include "learning.hpp"
#include <vector>
namespace kline {
struct MultiTargetRidge{
    std::vector<RidgeModel> outputs;
};
inline MultiTargetRidge fit_multitarget_ridge(
    const Matrix& x,const Matrix& targets,double penalty){
    require(!targets.empty()&&x.size()==targets.size()&&
            !targets[0].empty(),"Invalid multitarget training targets");
    int outputs=int(targets[0].size());
    for(const auto& row:targets){
        check_finite(row);require(int(row.size())==outputs,
                                  "Multitarget ragged outputs");
    }
    MultiTargetRidge m;
    for(int j=0;j<outputs;j++){
        std::vector<double> y;
        for(const auto& row:targets)y.push_back(row[j]);
        m.outputs.push_back(fit_ridge(x,y,penalty));
    }
    return m;
}
inline std::vector<double> multitarget_predict(
    const MultiTargetRidge& model,const std::vector<double>& features){
    require(!model.outputs.empty(),"Empty multitarget model");
    std::vector<double> out;
    for(const auto& task:model.outputs)
        out.push_back(ridge_predict(task,features));
    return out;
}
} // namespace kline
