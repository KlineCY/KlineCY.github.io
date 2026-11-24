#include "lab24_linear.hpp"
#include "lab24_optim.hpp"
#include "lab24_network.hpp"
#include "lab24_state.hpp"
#include "lab24_panel.hpp"
#include "lab24_learning.hpp"
#include "lab24_rates.hpp"
#include "lab24_derivative.hpp"
#include "lab24_credit.hpp"
#include "lab24_portfolio.hpp"
#include "lab24_execution.hpp"
#include "lab24_data.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
using namespace kline::lab24;
void compare(const Vec& a,const Vec& b,const char* name){
    if(a.size()!=b.size())throw std::runtime_error(std::string(name)+": output dimension");
    for(std::size_t i=0;i<a.size();++i){
        if(!std::isfinite(a[i])||std::abs(a[i]-b[i])>1e-7*std::max(1.0,std::abs(b[i]))){
            std::cerr<<name<<" ["<<i<<"] actual "<<a[i]<<" expected "<<b[i]<<"\n";
            throw std::runtime_error("Numerical identity failed");
        }
    }
}
template<class F>void reject(F f){
    bool ok=false;
    try{f();}catch(const std::invalid_argument&){ok=true;}
    if(!ok)throw std::runtime_error("Invalid data accepted");
}
int main(){
    const double nan=std::numeric_limits<double>::quiet_NaN();
    compare(qr_solve({{1,0,2},{0,1,3},{1,1,5}},{}),{2,3,0},"qr_solve");
    reject([&]{qr_solve({},{});});
    reject([&]{Mat a={{1,0,2},{0,1,3},{1,1,5}};a[0][0]=nan;qr_solve(a,{});});
    compare(spectral2({{2,1},{1,2}},{}),{3,1,0.7071067811865476,0.7071067811865476,2},"spectral2");
    reject([&]{spectral2({},{});});
    reject([&]{Mat a={{2,1},{1,2}};a[0][0]=nan;spectral2(a,{});});
    compare(ridge2({{-1,-1,-6},{-1,1,2},{1,-1,-2},{1,1,6}},{4}),{0,1,2,2},"ridge2");
    reject([&]{ridge2({},{4});});
    reject([&]{Mat a={{-1,-1,-6},{-1,1,2},{1,-1,-2},{1,1,6}};a[0][0]=nan;ridge2(a,{4});});
    compare(woodbury_rank1({{1,3},{1,1}},{1}),{1.6666666666666667,-0.3333333333333333},"woodbury_rank1");
    reject([&]{woodbury_rank1({},{1});});
    reject([&]{Mat a={{1,3},{1,1}};a[0][0]=nan;woodbury_rank1(a,{1});});
    compare(power_iteration({{4,0,0},{0,2,0},{0,0,1}},{40}),{4,0},"power_iteration");
    reject([&]{power_iteration({},{40});});
    reject([&]{Mat a={{4,0,0},{0,2,0},{0,0,1}};a[0][0]=nan;power_iteration(a,{40});});
    compare(simplex_projection({{0.2},{1.2},{-0.3}},{}),{0,1,0},"simplex_projection");
    reject([&]{simplex_projection({},{});});
    reject([&]{Mat a={{0.2},{1.2},{-0.3}};a[0][0]=nan;simplex_projection(a,{});});
    compare(l1_ball({{2},{-1},{0.5}},{2}),{1.5,-0.5,0},"l1_ball");
    reject([&]{l1_ball({},{2});});
    reject([&]{Mat a={{2},{-1},{0.5}};a[0][0]=nan;l1_ball(a,{2});});
    compare(box_budget({{0.8,0,0.7},{0.6,0,0.7},{0.2,0,0.7}},{1}),{0.6,0.4,0},"box_budget");
    reject([&]{box_budget({},{1});});
    reject([&]{Mat a={{0.8,0,0.7},{0.6,0,0.7},{0.2,0,0.7}};a[0][0]=nan;box_budget(a,{1});});
    compare(elastic_coordinate({{3,2},{-1,2},{0.2,2}},{1,1}),{0.6666666666666666,0,0},"elastic_coordinate");
    reject([&]{elastic_coordinate({},{1,1});});
    reject([&]{Mat a={{3,2},{-1,2},{0.2,2}};a[0][0]=nan;elastic_coordinate(a,{1,1});});
    compare(entropy_tilt({{0.5,0},{0.5,1}},{1.0986122886681098}),{0.75,0.13081203594113697},"entropy_tilt");
    reject([&]{entropy_tilt({},{1.0986122886681098});});
    reject([&]{Mat a={{0.5,0},{0.5,1}};a[0][0]=nan;entropy_tilt(a,{1.0986122886681098});});
    compare(laplacian_energy({{0,1,2,1,2},{1,2,1,2,4}},{}),{6},"laplacian_energy");
    reject([&]{laplacian_energy({},{});});
    reject([&]{Mat a={{0,1,2,1,2},{1,2,1,2,4}};a[0][0]=nan;laplacian_energy(a,{});});
    compare(pagerank({{0,1,1},{1,0,1}},{2,0.85,50}),{0.5,0.5},"pagerank");
    reject([&]{pagerank({},{2,0.85,50});});
    reject([&]{Mat a={{0,1,1},{1,0,1}};a[0][0]=nan;pagerank(a,{2,0.85,50});});
    compare(diffusion({{0,1,1}},{0.25,1,1,0}),{0.75,0.25},"diffusion");
    reject([&]{diffusion({},{0.25,1,1,0});});
    reject([&]{Mat a={{0,1,1}};a[0][0]=nan;diffusion(a,{0.25,1,1,0});});
    compare(minimax_path({{0,1,2},{1,2,3},{0,2,5}},{3,0,2}),{3},"minimax_path");
    reject([&]{minimax_path({},{3,0,2});});
    reject([&]{Mat a={{0,1,2},{1,2,3},{0,2,5}};a[0][0]=nan;minimax_path(a,{3,0,2});});
    compare(minimum_spanning_tree({{0,1,2},{1,2,3},{0,2,5}},{3}),{5,3},"minimum_spanning_tree");
    reject([&]{minimum_spanning_tree({},{3});});
    reject([&]{Mat a={{0,1,2},{1,2,3},{0,2,5}};a[0][0]=nan;minimum_spanning_tree(a,{3});});
    compare(scalar_kalman({{2,1}},{0,1,0}),{1,0.5,-2.2655121234846454},"scalar_kalman");
    reject([&]{scalar_kalman({},{0,1,0});});
    reject([&]{Mat a={{2,1}};a[0][0]=nan;scalar_kalman(a,{0,1,0});});
    compare(hmm_forward({{0.8,0.2}},{0.5,0.9,0.8}),{0.8,0.2,-0.6931471805599453},"hmm_forward");
    reject([&]{hmm_forward({},{0.5,0.9,0.8});});
    reject([&]{Mat a={{0.8,0.2}};a[0][0]=nan;hmm_forward(a,{0.5,0.9,0.8});});
    compare(viterbi({{0.9,0.1},{0.8,0.2}},{0.5,0.9,0.9}),{0,0},"viterbi");
    reject([&]{viterbi({},{0.5,0.9,0.9});});
    reject([&]{Mat a={{0.9,0.1},{0.8,0.2}};a[0][0]=nan;viterbi(a,{0.5,0.9,0.9});});
    compare(ou_transition_fit({{1},{0.5},{0.25},{0.125}},{1}),{0.6931471805599453,0,0},"ou_transition_fit");
    reject([&]{ou_transition_fit({},{1});});
    reject([&]{Mat a={{1},{0.5},{0.25},{0.125}};a[0][0]=nan;ou_transition_fit(a,{1});});
    compare(markov_hitting({{0.8}},{2}),{0.36,5},"markov_hitting");
    reject([&]{markov_hitting({},{2});});
    reject([&]{Mat a={{0.8}};a[0][0]=nan;markov_hitting(a,{2});});
    compare(within_panel({{0,0,1},{0,1,3},{1,0,10},{1,1,12}},{}),{2,1},"within_panel");
    reject([&]{within_panel({},{});});
    reject([&]{Mat a={{0,0,1},{0,1,3},{1,0,10},{1,1,12}};a[0][0]=nan;within_panel(a,{});});
    compare(cluster_ols({{0,0,1},{0,1,3},{1,0,5},{1,1,7}},{}),{3,2,0},"cluster_ols");
    reject([&]{cluster_ols({},{});});
    reject([&]{Mat a={{0,0,1},{0,1,3},{1,0,5},{1,1,7}};a[0][0]=nan;cluster_ols(a,{});});
    compare(paired_t({{0,1},{0,2},{0,3}},{}),{2,0.5773502691896258,3.4641016151377544},"paired_t");
    reject([&]{paired_t({},{});});
    reject([&]{Mat a={{0,1},{0,2},{0,3}};a[0][0]=nan;paired_t(a,{});});
    compare(randomization_exact({{1},{1}},{}),{1,0.5},"randomization_exact");
    reject([&]{randomization_exact({},{});});
    reject([&]{Mat a={{1},{1}};a[0][0]=nan;randomization_exact(a,{});});
    compare(iv_first_stage({{-1,-2,-4},{-1,0,0},{1,0,0},{1,2,4}},{}),{1,2,2},"iv_first_stage");
    reject([&]{iv_first_stage({},{});});
    reject([&]{Mat a={{-1,-2,-4},{-1,0,0},{1,0,0},{1,2,4}};a[0][0]=nan;iv_first_stage(a,{});});
    compare(online_logistic({{1,0,1}},{0.2}),{0.1,0,0.6931471805599453},"online_logistic");
    reject([&]{online_logistic({},{0.2});});
    reject([&]{Mat a={{1,0,1}};a[0][0]=nan;online_logistic(a,{0.2});});
    compare(adagrad_ridge({{1,0,2}},{0.1,1,0.001}),{0.09995002498750626,0,4},"adagrad_ridge");
    reject([&]{adagrad_ridge({},{0.1,1,0.001});});
    reject([&]{Mat a={{1,0,2}};a[0][0]=nan;adagrad_ridge(a,{0.1,1,0.001});});
    compare(brier_decomposition({{0.25,0},{0.75,1}},{}),{0.0625,0.0625,0.25,0.25},"brier_decomposition");
    reject([&]{brier_decomposition({},{});});
    reject([&]{Mat a={{0.25,0},{0.75,1}};a[0][0]=nan;brier_decomposition(a,{});});
    compare(isotonic_pav({{1,0},{2,1},{3,0},{4,1}},{}),{0,0.5,0.5,1},"isotonic_pav");
    reject([&]{isotonic_pav({},{});});
    reject([&]{Mat a={{1,0},{2,1},{3,0},{4,1}};a[0][0]=nan;isotonic_pav(a,{});});
    compare(stacking_two({{0,2,1},{2,0,1}},{}),{0.5,0.5,0},"stacking_two");
    reject([&]{stacking_two({},{});});
    reject([&]{Mat a={{0,2,1},{2,0,1}};a[0][0]=nan;stacking_two(a,{});});
    compare(discount_bootstrap({{0.05},{0.05}},{}),{0.9523809523809523,0.9070294784580498},"discount_bootstrap");
    reject([&]{discount_bootstrap({},{});});
    reject([&]{Mat a={{0.05},{0.05}};a[0][0]=nan;discount_bootstrap(a,{});});
    compare(forward_curve({{1,0.9801986733067553},{2,0.9417645335842487}},{}),{0.02,0.04},"forward_curve");
    reject([&]{forward_curve({},{});});
    reject([&]{Mat a={{1,0.9801986733067553},{2,0.9417645335842487}};a[0][0]=nan;forward_curve(a,{});});
    compare(curve_keyrate({{1,10},{2,100}},{1,2,0.02,0.04}),{102.11362137173113,0.0009801496650067136,0.01846048081811597},"curve_keyrate");
    reject([&]{curve_keyrate({},{1,2,0.02,0.04});});
    reject([&]{Mat a={{1,10},{2,100}};a[0][0]=nan;curve_keyrate(a,{1,2,0.02,0.04});});
    compare(duration_immunize({{1},{3}},{100,2,0}),{50,50,50,50},"duration_immunize");
    reject([&]{duration_immunize({},{100,2,0});});
    reject([&]{Mat a={{1},{3}};a[0][0]=nan;duration_immunize(a,{100,2,0});});
    compare(yield_newton({{1,110}},{100,-0.5,0.5}),{0.09531017980432493,0},"yield_newton");
    reject([&]{yield_newton({},{100,-0.5,0.5});});
    reject([&]{Mat a={{1,110}};a[0][0]=nan;yield_newton(a,{100,-0.5,0.5});});
    compare(crr_american({{1}},{100,100,0,0,0.6931471805599453,1,1,0}),{33.333333333333336,33.333333333333336},"crr_american");
    reject([&]{crr_american({},{100,100,0,0,0.6931471805599453,1,1,0});});
    reject([&]{Mat a={{1}};a[0][0]=nan;crr_american(a,{100,100,0,0,0.6931471805599453,1,1,0});});
    compare(asian_path({{110,110,110},{90,90,90}},{100,0,1,1}),{5,5},"asian_path");
    reject([&]{asian_path({},{100,0,1,1});});
    reject([&]{Mat a={{110,110,110},{90,90,90}};a[0][0]=nan;asian_path(a,{100,0,1,1});});
    compare(butterfly_density({{90,15},{100,8},{120,2}},{0,1}),{0.02666666666666667,0},"butterfly_density");
    reject([&]{butterfly_density({},{0,1});});
    reject([&]{Mat a={{90,15},{100,8},{120,2}};a[0][0]=nan;butterfly_density(a,{0,1});});
    compare(variance_strip({{90,1},{100,1},{110,1}},{100,100,0,1}),{0.006122028364452608,0.07824339182609998},"variance_strip");
    reject([&]{variance_strip({},{100,100,0,1});});
    reject([&]{Mat a={{90,1},{100,1},{110,1}};a[0][0]=nan;variance_strip(a,{100,100,0,1});});
    compare(discrete_delta_pnl({{0,100,0.5},{1,110,0.5}},{5,10,0,0}),{0,0},"discrete_delta_pnl");
    reject([&]{discrete_delta_pnl({},{5,10,0,0});});
    reject([&]{Mat a={{0,100,0.5},{1,110,0.5}};a[0][0]=nan;discrete_delta_pnl(a,{5,10,0,0});});
    compare(migration_two({{0.8,0.1,0.1},{0,0.8,0.2},{0,0,1}},{2,1,0,0}),{0.64,0.16,0.2},"migration_two");
    reject([&]{migration_two({},{2,1,0,0});});
    reject([&]{Mat a={{0.8,0.1,0.1},{0,0.8,0.2},{0,0,1}};a[0][0]=nan;migration_two(a,{2,1,0,0});});
    compare(poisson_binomial({{0.1},{0.2}},{}),{0.72,0.26,0.02},"poisson_binomial");
    reject([&]{poisson_binomial({},{});});
    reject([&]{Mat a={{0.1},{0.2}};a[0][0]=nan;poisson_binomial(a,{});});
    compare(loss_convolution({{0.1,1},{0.2,1}},{0.9}),{0.3,1,1.2},"loss_convolution");
    reject([&]{loss_convolution({},{0.9});});
    reject([&]{Mat a={{0.1,1},{0.2,1}};a[0][0]=nan;loss_convolution(a,{0.9});});
    compare(survival_hazard({{1,0.6931471805599453},{2,0.6931471805599453}},{}),{0.5,0.5,0.25,0.25},"survival_hazard");
    reject([&]{survival_hazard({},{});});
    reject([&]{Mat a={{1,0.6931471805599453},{2,0.6931471805599453}};a[0][0]=nan;survival_hazard(a,{});});
    compare(discrete_cds_spread({{1,0.9,1}},{0.4}),{0.06,0.9,0.06666666666666667},"discrete_cds_spread");
    reject([&]{discrete_cds_spread({},{0.4});});
    reject([&]{Mat a={{1,0.9,1}};a[0][0]=nan;discrete_cds_spread(a,{0.4});});
    compare(factor_neutral_two({{1,-1},{1,0},{1,1}},{0,0}),{0,0,0},"factor_neutral_two");
    reject([&]{factor_neutral_two({},{0,0});});
    reject([&]{Mat a={{1,-1},{1,0},{1,1}};a[0][0]=nan;factor_neutral_two(a,{0,0});});
    compare(risk_budget_diag({{1,0.5},{2,0.5}},{}),{0.6666666666666666,0.3333333333333333},"risk_budget_diag");
    reject([&]{risk_budget_diag({},{});});
    reject([&]{Mat a={{1,0.5},{2,0.5}};a[0][0]=nan;risk_budget_diag(a,{});});
    compare(factor_cov_risk({{0.5,1,1},{0.5,1,1}},{1}),{1,1,0.5,1.224744871391589},"factor_cov_risk");
    reject([&]{factor_cov_risk({},{1});});
    reject([&]{Mat a={{0.5,1,1},{0.5,1,1}};a[0][0]=nan;factor_cov_risk(a,{1});});
    compare(turnover_prox({{0.2,0.5,1,0.1},{0.4,0.35,1,0.1}},{}),{0.4,0.4},"turnover_prox");
    reject([&]{turnover_prox({},{});});
    reject([&]{Mat a={{0.2,0.5,1,0.1},{0.4,0.35,1,0.1}};a[0][0]=nan;turnover_prox(a,{});});
    compare(rebalance_drift({{0.5,0.2,0.5},{0.5,0,0.5}},{}),{0.1,0.045454545454545456,0.5454545454545454,0.45454545454545453},"rebalance_drift");
    reject([&]{rebalance_drift({},{});});
    reject([&]{Mat a={{0.5,0.2,0.5},{0.5,0,0.5}};a[0][0]=nan;rebalance_drift(a,{});});
    compare(ac_schedule({{100}},{4,1,0}),{25,25,25,25},"ac_schedule");
    reject([&]{ac_schedule({},{4,1,0});});
    reject([&]{Mat a={{100}};a[0][0]=nan;ac_schedule(a,{4,1,0});});
    compare(participation_schedule({{100},{200},{100}},{50,0.2}),{20,30,0,0},"participation_schedule");
    reject([&]{participation_schedule({},{50,0.2});});
    reject([&]{Mat a={{100},{200},{100}};a[0][0]=nan;participation_schedule(a,{50,0.2});});
    compare(queue_fifo({{2,5},{0,6}},{10,5}),{3,2,0},"queue_fifo");
    reject([&]{queue_fifo({},{10,5});});
    reject([&]{Mat a={{2,5},{0,6}};a[0][0]=nan;queue_fifo(a,{10,5});});
    compare(multi_venue_sweep({{10,2,0},{11,4,0}},{5}),{53,5,0,2,3},"multi_venue_sweep");
    reject([&]{multi_venue_sweep({},{5});});
    reject([&]{Mat a={{10,2,0},{11,4,0}};a[0][0]=nan;multi_venue_sweep(a,{5});});
    compare(impact_decay_fit({{0,4},{1,2},{2,1}},{}),{4,0.6931471805599453,1},"impact_decay_fit");
    reject([&]{impact_decay_fit({},{});});
    reject([&]{Mat a={{0,4},{1,2},{2,1}};a[0][0]=nan;impact_decay_fit(a,{});});
    compare(asof_age({{1,10,2},{3,30,3},{5,50,8}},{2}),{1,10,1,1,30,0,0,0,0},"asof_age");
    reject([&]{asof_age({},{2});});
    reject([&]{Mat a={{1,10,2},{3,30,3},{5,50,8}};a[0][0]=nan;asof_age(a,{2});});
    compare(purged_mask({{0,1},{1,3},{4,5},{6,7}},{2,3,1}),{1,0,0,1},"purged_mask");
    reject([&]{purged_mask({},{2,3,1});});
    reject([&]{Mat a={{0,1},{1,3},{4,5},{6,7}};a[0][0]=nan;purged_mask(a,{2,3,1});});
    compare(label_uniqueness({{0,1},{1,2}},{}),{0.75,0.75},"label_uniqueness");
    reject([&]{label_uniqueness({},{});});
    reject([&]{Mat a={{0,1},{1,2}};a[0][0]=nan;label_uniqueness(a,{});});
    compare(revision_latest({{1,2,10},{1,4,12},{2,3,20}},{3}),{1,2,10,2,3,20},"revision_latest");
    reject([&]{revision_latest({},{3});});
    reject([&]{Mat a={{1,2,10},{1,4,12},{2,3,20}};a[0][0]=nan;revision_latest(a,{3});});
    compare(event_time_bars({{10,2},{12,3},{11,1}},{3}),{10,12,10,12,3,10.666666666666666,12,12,11,11,3,11.666666666666666},"event_time_bars");
    reject([&]{event_time_bars({},{3});});
    reject([&]{Mat a={{10,2},{12,3},{11,1}};a[0][0]=nan;event_time_bars(a,{3});});
    // Additional algebraic invariants and economically relevant failure cases.
    auto w=box_budget({{.9,0,.6},{.4,0,.7},{.2,0,.7}},{1});
    compare({sum(w)},{1},"box budget invariant");
    auto q=entropy_tilt({{.3,1},{.7,2}},{0});
    compare(q,{1.7,0},"entropy at zero tilt");
    auto g=diffusion({{0,1,1},{1,2,2}},{.1,10,1,2,3});
    compare({sum(g)},{6},"graph conservation");
    auto v=poisson_binomial({{.1},{.2},{.3}},{});
    compare({sum(v),v[1]+2*v[2]+3*v[3]},{1,.6},"default distribution normalization");
    auto d=loss_convolution({{.1,2},{.2,3}},{.95});
    compare({d[0]},{.8},"heterogeneous loss expectation");
    auto h=hmm_forward({{1,1},{1,1}},{.4,.9,.8});
    compare(h,{.48,.52,0},"unobserved HMM prediction");
    auto s=ac_schedule({{100}},{5,1,2});
    compare({sum(s)},{100},"execution conservation");
    if(!(s.front()>s.back()))throw std::runtime_error("Inventory-risk schedule direction");
    auto b=event_time_bars({{10,7},{11,1}},{3});
    compare({b[4]+b[10]+b[16],b[5]*b[4]+b[11]*b[10]+b[17]*b[16]},{8,81},"volume and cash conservation");
    reject([]{qr_solve({{1,2,3},{2,4,6}},{});});
    reject([]{box_budget({{1,0,.2}},{1});});
    reject([]{diffusion({{0,1,1}},{2,1,1,0});});
    reject([]{purged_mask({{2,1}},{0,1,0});});
    reject([]{poisson_binomial({{1.1}},{});});
    reject([]{yield_newton({{1,100}},{100,1,2});});
    reject([]{revision_latest({{1,2,3},{1,2,4}},{2});});
    reject([]{queue_fifo({{3,0}},{2,1});});
    std::cout<<"PASS: 60 matrix and path algorithms, 120 invalid-input guards, 8 extra boundary tests and 8 structural invariants\n";
}
