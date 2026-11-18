#pragma once
#include "mean_cvar_allocation.hpp"
#include "risk_budget_allocation.hpp"
#include "black_litterman_complete.hpp"
#include "turnover_penalized_allocation.hpp"
#include "factor_risk_covariance.hpp"
#include "online_covariance.hpp"
#include "tax_lot_fifo.hpp"
#include "integer_portfolio.hpp"
#include "portfolio_drawdown_control.hpp"
#include "stress_factor_mapping.hpp"
#include "par_curve_bootstrap.hpp"
#include "credit_migration.hpp"
#include "credit_spread_risk.hpp"
#include "american_option_sensitivities.hpp"
#include "barrier_option_mc.hpp"
#include "variance_swap_accounting.hpp"
#include "continuous_futures_series.hpp"
#include "fx_forward_parity.hpp"
#include "market_order_fills.hpp"
#include "vwap_execution_schedule.hpp"
#include "short_borrow_ledger.hpp"
#include "liquidity_capacity.hpp"
#include "trade_fill_reconciliation.hpp"
#include "implementation_shortfall.hpp"
#include "lagged_strategy_backtest.hpp"
#include "execution_intensity.hpp"
#include "io.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
namespace kline {
inline void finance_research_vector(std::ostream& out,
              const std::string& name,const std::vector<double>& values){
    out<<"Index,"<<name<<"\n";
    for(int i=0;i<int(values.size());i++)
        out<<i<<","<<values[i]<<"\n";
}
inline Matrix finance_research_scenarios(
         const CsvTable& t,const std::string& a,const std::string& b){
    auto first=t.numbers(a),second=t.numbers(b);
    Matrix scenarios;
    for(int i=0;i<int(first.size());i++)
        scenarios.push_back({first[i],second[i]});
    return scenarios;
}
inline bool run_research_finance_cli(const std::string& cmd,int argc,
                                     char** argv,std::ostream& out){
    auto exact=[&](int n){require(argc==n,"Incorrect research finance arguments");};
    auto num=[&](int i){return parse_number(argv[i]);};
    if(cmd=="cvar-evaluate"||cmd=="cvar-optimize"){
        exact(7);
        auto scenarios=finance_research_scenarios(read_csv(argv[2]),argv[3],argv[4]);
        if(cmd=="cvar-evaluate")
            out<<"CVaRLoss "<<empirical_cvar_loss(scenarios,{num(5),1-num(5)},num(6))<<"\n";
        else finance_research_vector(out,"Weight",
            minimum_cvar_long_only(scenarios,num(6)));
    }else if(cmd=="risk-budget2"){
        exact(10);
        Matrix cov={{num(2),num(3)},{num(3),num(4)}};
        finance_research_vector(out,"Weight",
            risk_budget_weights(cov,{num(5),num(6)},
                                std::stoi(argv[7]),num(8)));
    }else if(cmd=="black-litterman-full"){
        exact(8);
        auto bl=black_litterman_full(
            {num(2)},{{num(3)}},{{1}},{num(4)},{num(5)},num(6));
        out<<"PosteriorMean "<<bl.mean[0]<<"\n"
           <<"PredictiveVariance "<<bl.covariance[0][0]<<"\n";
    }else if(cmd=="turnover-opt2"){
        exact(13);
        Matrix covariance={{num(2),num(3)},{num(3),num(4)}};
        finance_research_vector(out,"Weight",
            projected_turnover_allocation({num(5),num(6)},
             covariance,{num(7),num(8)},num(9),num(10),
             std::stoi(argv[11]),num(12)));
    }else if(cmd=="turnover-objective2"){
        exact(13);
        out<<"Objective "<<turnover_penalized_objective(
            {num(2),num(3)},{num(4),num(5)},
            {{num(6),num(7)},{num(7),num(8)}},
            {num(9),num(10)},num(11),num(12))<<"\n";
    }else if(cmd=="factor-cov2"){
        exact(8);
        auto matrix=factor_structured_covariance(
            {{num(3)},{num(4)}},{{num(2)}},{num(5),num(6)});
        out<<"Cov11 "<<matrix[0][0]<<"\n"
           <<"Cov12 "<<matrix[0][1]<<"\n"
           <<"Cov22 "<<matrix[1][1]<<"\n";
    }else if(cmd=="factor-exposure2"){
        exact(8);
        auto values=portfolio_factor_exposure(
            {{num(2)},{num(3)}},{num(4),num(5)});
        out<<"Exposure "<<values[0]<<"\n";
    }else if(cmd=="online-cov2"){
        exact(5);
        auto t=read_csv(argv[2]),a=t.numbers(argv[3]),b=t.numbers(argv[4]);
        auto state=new_online_covariance(2);
        for(int i=0;i<int(a.size());i++)
            update_online_covariance(state,{a[i],b[i]});
        auto covariance=finalized_online_covariance(state);
        out<<"Cov11 "<<covariance[0][0]<<"\n"
           <<"Cov12 "<<covariance[0][1]<<"\n"
           <<"Cov22 "<<covariance[1][1]<<"\n";
    }else if(cmd=="fifo-trade"){
        exact(10);
        FifoLedger l;
        fifo_buy(l,num(2),num(3),num(4));
        fifo_buy(l,num(5),num(6));
        fifo_sell(l,num(7),num(8),num(9));
        out<<"RealizedPnL "<<l.realized_pnl<<"\n"
           <<"RemainingShares "<<l.remaining_shares<<"\n"
           <<"RemainingBasis "<<l.remaining_basis<<"\n";
    }else if(cmd=="integer-allocation2"){
        exact(9);
        auto result=budgeted_whole_share_allocation(
            num(2),{num(3),num(4)},{num(5),num(6)},num(7));
        out<<"SharesA "<<result.shares[0]<<"\n"
           <<"SharesB "<<result.shares[1]<<"\n"
           <<"RemainingCash "<<result.cash<<"\n"
           <<"Commission "<<result.commission<<"\n";
    }else if(cmd=="drawdown-gross"){
        exact(6);
        finance_research_vector(out,"AllowedGross",
            causal_drawdown_leverage(read_csv(argv[2]).numbers(argv[3]),
                                     num(4),num(5)));
    }else if(cmd=="factor-stress2"){
        exact(12);
        out<<"Loss "<<portfolio_factor_scenario_loss(
            {num(10),num(11)},
            {{num(2),num(3)},{num(4),num(5)}},
            {num(6),num(7)},{num(8),num(9)})<<"\n";
    }else if(cmd=="par-bond-boot"){
        exact(4);
        auto result=bootstrap_annual_par_bonds(
            read_csv(argv[2]).numbers(argv[3]));
        finance_research_vector(out,"Discount",result.discounts);
    }else if(cmd=="par-coupon"){
        exact(4);
        out<<"ParCoupon "<<par_coupon_rate_from_discounts(
            read_csv(argv[2]).numbers(argv[3]))<<"\n";
    }else if(cmd=="credit-migration2"){
        exact(9);
        auto result=credit_rating_distribution(
            {num(2),1-num(2)},{{num(3),num(4)},{num(5),num(6)}},
            std::stoi(argv[7]));
        finance_research_vector(out,"StateProbability",result);
    }else if(cmd=="credit-migration-loss"){
        exact(7);
        out<<"ExpectedLoss "<<credit_migration_expected_loss(
            {num(2),num(3)},{num(4),num(5)})<<"\n";
    }else if(cmd=="credit-dv01"||cmd=="credit-duration"){
        exact(6);auto t=read_csv(argv[2]);
        auto times=t.numbers("Time"),flows=t.numbers("Cashflow");
        double v=cmd=="credit-dv01"?
              credit_spread_dv01(times,flows,num(3),num(4)):
              credit_spread_duration(times,flows,num(3),num(4));
        out<<(cmd=="credit-dv01"?"SpreadDV01 ":"SpreadDuration ")<<v<<"\n";
    }else if(cmd=="american-binomial-greeks"){
        exact(11);
        require(std::string(argv[10])=="call"||std::string(argv[10])=="put",
                "Option contract side must be call or put");
        auto g=american_binomial_greeks(
            num(2),num(3),num(4),num(5),num(6),
            num(7),std::stoi(argv[8]),std::string(argv[10])=="call");
        out<<"Value "<<g.value<<"\nDelta "<<g.delta<<"\n"
           <<"Gamma "<<g.gamma<<"\nVega "<<g.vega<<"\n";
    }else if(cmd=="barrier-downout-mc"){
        exact(12);
        auto r=down_and_out_call_monte_carlo(num(2),num(3),num(4),
            num(5),num(6),num(7),std::stoi(argv[8]),
            std::stoi(argv[9]),std::stoul(argv[10]));
        out<<"Price "<<r.mean<<"\nSE "<<r.standard_error<<"\n"
           <<"Lower95 "<<r.lower95<<"\nUpper95 "<<r.upper95<<"\n";
    }else if(cmd=="realized-variance-swap"){
        exact(5);
        out<<"AnnualVariance "<<annualized_realized_variance(
            read_csv(argv[2]).numbers(argv[3]),std::stoi(argv[4]))<<"\n";
    }else if(cmd=="variance-swap-settle"){
        exact(5);
        out<<"Settlement "<<variance_swap_settlement(
            num(2),num(3),num(4))<<"\n";
    }else if(cmd=="variance-notional"){
        exact(4);
        out<<"VarianceNotional "<<vega_notional_to_variance_notional(
            num(2),num(3))<<"\n";
    }else if(cmd=="continuous-future"){
        exact(6);
        auto prices=read_csv(argv[2]).numbers(argv[3]);
        auto t=read_csv(argv[4]);
        auto ids=t.numbers("RollIndex");
        auto old=t.numbers("Old"),next=t.numbers("New");
        std::vector<int> indexes;
        for(double id:ids){
            require(id>=0&&id==std::floor(id)&&id<=1000000000,
                    "Roll boundary must be integer");
            indexes.push_back(int(id));
        }
        finance_research_vector(out,"BackAdjusted",
            back_adjust_additive_futures(prices,old,next,indexes));
    }else if(cmd=="futures-held-return"){
        exact(6);auto t=read_csv(argv[2]);
        finance_research_vector(out,"HoldingReturn",
            held_contract_futures_returns(t.numbers(argv[3]),
                    t.numbers(argv[4]),t.numbers(argv[5])));
    }else if(cmd=="fx-covered"){
        exact(6);
        out<<"CoveredForward "<<covered_fx_forward(
            num(2),num(3),num(4),num(5))<<"\n";
    }else if(cmd=="fx-covered-basis"){
        exact(7);
        out<<"CoveredBasis "<<fx_covered_basis(
            num(2),num(3),num(4),num(5),num(6))<<"\n";
    }else if(cmd=="market-sweep"||cmd=="market-sweep-slip"){
        exact(8);auto t=read_csv(argv[2]);
        auto prices=t.numbers(argv[3]),quantities=t.numbers(argv[4]);
        std::vector<DepthLevel> levels;
        for(int i=0;i<int(prices.size());i++)
            levels.push_back({prices[i],quantities[i]});
        bool buying=std::string(argv[6])=="buy";
        require(buying||std::string(argv[6])=="sell",
                "Market side must be buy or sell");
        auto fill=sweep_market_order(levels,num(5),buying);
        out<<"Filled "<<fill.filled<<"\nUnfilled "<<fill.remaining<<"\n"
           <<"AveragePrice "<<fill.average_price<<"\n";
        if(cmd=="market-sweep-slip")
            out<<"ArrivalSlippage "<<market_order_slippage(fill,num(7),buying)<<"\n";
    }else if(cmd=="vwap-target"){
        exact(5);
        finance_research_vector(out,"ChildQuantity",
            volume_profile_schedule(num(4),
                read_csv(argv[2]).numbers(argv[3])));
    }else if(cmd=="vwap-actual"){
        exact(5);auto t=read_csv(argv[2]);
        out<<"ExecutionVWAP "<<executed_vwap(
            t.numbers(argv[3]),t.numbers(argv[4]))<<"\n";
    }else if(cmd=="borrow-fee"){
        exact(6);
        out<<"BorrowFee "<<daily_stock_borrow_fee(
            num(2),num(3),num(4),std::stoi(argv[5]))<<"\n";
    }else if(cmd=="short-dividend"){
        exact(4);
        out<<"Payment "<<short_dividend_payment(num(2),num(3))<<"\n";
    }else if(cmd=="short-collateral"){
        exact(5);
        out<<"RestrictedValue "<<restricted_short_proceeds(
            num(2),num(3),num(4))<<"\n";
    }else if(cmd=="liquidity-clip"||cmd=="liquidity-capacity-days"){
        exact(6);auto t=read_csv(argv[2]);
        auto desired=t.numbers(argv[3]),volume=t.numbers(argv[4]);
        if(cmd=="liquidity-clip")
            finance_research_vector(out,"ApprovedQuantity",
                capacity_constrained_shares(desired,volume,num(5)));
        else out<<"MinimumDays "<<participation_weighted_execution_time(
            desired,volume,num(5))<<"\n";
    }else if(cmd=="fills-reconcile"){
        exact(6);auto t=read_csv(argv[2]);
        auto shares=t.numbers(argv[3]),price=t.numbers(argv[4]),
             fees=t.numbers(argv[5]);
        std::vector<Fill> fills;
        for(int i=0;i<int(shares.size());i++)
            fills.push_back({shares[i],price[i],fees[i]});
        auto r=reconcile_fills(fills);
        out<<"NetShares "<<r.net_shares<<"\nCashChange "<<r.cash_change
           <<"\nGrossShares "<<r.gross_shares<<"\nCommission "<<r.commissions<<"\n";
    }else if(cmd=="implementation-shortfall"){
        exact(9);auto t=read_csv(argv[2]);
        auto shares=t.numbers(argv[3]),price=t.numbers(argv[4]),
             fees=t.numbers(argv[5]);
        std::vector<Fill> fills;
        for(int i=0;i<int(shares.size());i++)
            fills.push_back({shares[i],price[i],fees[i]});
        auto result=implementation_shortfall(
            num(6),num(7),num(8),fills);
        out<<"Execution "<<result.execution_cost<<"\nOpportunity "
           <<result.opportunity_cost<<"\nFees "<<result.commissions
           <<"\nTotal "<<result.total<<"\n";
    }else if(cmd=="lagged-trading"){
        exact(7);auto t=read_csv(argv[2]);
        auto trades=lagged_signal_strategy(
            t.numbers(argv[3]),t.numbers(argv[4]),num(5),num(6));
        out<<"Index,Weight,Turnover,Fee,NetReturn,Equity\n";
        for(int i=0;i<int(trades.size());i++){
            const auto& r=trades[i];
            out<<i<<","<<r.active_weight<<","<<r.turnover<<","
               <<r.transaction_fee<<","<<r.net_return<<","<<r.equity<<"\n";
        }
    }else if(cmd=="fill-intensity"){
        exact(5);
        out<<"Intensity "<<exponential_fill_intensity(
            num(2),num(3),num(4))<<"\n";
    }else if(cmd=="fill-probability"){
        exact(4);
        out<<"Probability "<<poisson_arrival_fill_probability(
            num(2),num(3))<<"\n";
    }else if(cmd=="queue-probability"){
        exact(4);
        out<<"Probability "<<queue_depletion_probability(
            num(2),num(3))<<"\n";
    }else return false;
    return true;
}
inline void research_finance_cli_usage(std::ostream& out){
    out<<"Advanced portfolio, derivatives, execution and research:\n"
       <<"  cvar-evaluate|cvar-optimize CSV RETURN_A RETURN_B WEIGHT_A CONFIDENCE\n"
       <<"  risk-budget2 C11 C12 C22 BUDGET1 BUDGET2 ITERATIONS STEP\n"
       <<"  black-litterman-full PRIOR VARIANCE VIEW_RETURN VIEW_VAR TAU\n"
       <<"  turnover-opt2 C11 C12 C22 MU1 MU2 PREV1 PREV2 RISK FEE ITERATIONS STEP\n"
       <<"  turnover-objective2 W1 W2 MU1 MU2 C11 C12 C22 PREV1 PREV2 RISK FEE\n"
       <<"  factor-cov2 FACTOR_VAR EXP1 EXP2 SPECIFIC1 SPECIFIC2\n"
       <<"  factor-exposure2 EXP1 EXP2 W1 W2\n"
       <<"  online-cov2 CSV SERIES_A SERIES_B\n"
       <<"  fifo-trade BUY1_Q BUY1_P BUY1_FEE BUY2_Q BUY2_P SELL_Q SELL_P SELL_FEE\n"
       <<"  integer-allocation2 EQUITY W1 W2 PRICE1 PRICE2 FEE_PER_SHARE\n"
       <<"  drawdown-gross CSV EQUITY LIMIT MAX_GROSS\n"
       <<"  factor-stress2 EX11 EX12 EX21 EX22 SHOCK1 SHOCK2 SPEC1 SPEC2 W1 W2\n"
       <<"  par-bond-boot CSV PAR_COUPON\n"
       <<"  par-coupon CSV ANNUAL_DISCOUNT\n"
       <<"  credit-migration2 P0 TRANS00 TRANS01 TRANS10 TRANS11 YEARS\n"
       <<"  credit-migration-loss P0 P1 LOSS_STATE0 LOSS_STATE1\n"
       <<"  credit-dv01|credit-duration CASHFLOW_CSV BASE_YIELD SPREAD\n"
       <<"  american-binomial-greeks S K RATE DIV VOL YEARS STEPS call|put\n"
       <<"  barrier-downout-mc S K BARRIER RATE VOL YEARS STEPS PATHS SEED\n"
       <<"  realized-variance-swap CSV LOG_RETURN PERIODS_PER_YEAR\n"
       <<"  variance-swap-settle REALIZED_VAR STRIKE_VAR NOTIONAL\n"
       <<"  variance-notional VEGA_NOTIONAL STRIKE_VOL\n"
       <<"  continuous-future PRICES_CSV COLUMN ROLL_CSV\n"
       <<"  futures-held-return CSV OLD NEXT NOTIONAL\n"
       <<"  fx-covered SPOT DOMESTIC FOREIGN YEARS\n"
       <<"  fx-covered-basis OBSERVED_FORWARD SPOT DOMESTIC FOREIGN YEARS\n"
       <<"  market-sweep|market-sweep-slip CSV PRICE QTY ORDER_QTY buy|sell ARRIVAL_MID\n"
       <<"  vwap-target CSV FORECAST_VOLUME QUANTITY\n"
       <<"  vwap-actual CSV FILL_PRICE FILL_QUANTITY\n"
       <<"  borrow-fee SHARES PRICE ANNUAL_RATE DAYS_PER_YEAR\n"
       <<"  short-dividend SHARES DIVIDEND\n"
       <<"  short-collateral SHARES PRICE HAIRCUT\n"
       <<"  liquidity-clip|liquidity-capacity-days CSV ORDER_QTY DAILY_VOLUME PARTICIPATION\n"
       <<"  fills-reconcile CSV SIGNED_QTY PRICE COMMISSION\n"
       <<"  implementation-shortfall CSV SIGNED_QTY PRICE FEE INTENDED_QTY ARRIVAL MID_LAST\n"
       <<"  lagged-trading CSV PRIOR_SIGNAL PERIOD_RETURN COST INITIAL_EQUITY\n"
       <<"  fill-intensity BASELINE EXPONENT DISTANCE\n"
       <<"  fill-probability INTENSITY YEARS\n"
       <<"  queue-probability EXPECTED_ORDERS QUEUE_AHEAD\n";
}
} // namespace kline
