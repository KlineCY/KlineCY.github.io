#pragma once
#include "delta_hedge_cashbook.hpp"
#include "option_delta_gamma_hedge.hpp"
#include "swap_fixed_float_pv.hpp"
#include "credit_survival_curve.hpp"
#include "credit_portfolio_moments.hpp"
#include "fx_hedge_accounting.hpp"
#include "fixed_income_keyrate_hedge.hpp"
#include "tracking_error_attribution.hpp"
#include "volatility_target_rebalance.hpp"
#include "short_margin_stress.hpp"
#include "orderbook_replay.hpp"
#include "microprice_and_imbalance.hpp"
#include "signed_trade_impact.hpp"
#include "realized_spread_metrics.hpp"
#include "square_root_market_impact.hpp"
#include "queue_priority_execution.hpp"
#include "execution_tca_decomposition.hpp"
#include "intraday_vpin_proxy.hpp"
#include "book_event_features.hpp"
#include "execution_schedule_constraints.hpp"
#include "inventory_risk_quotes.hpp"
#include "trade_return_accounting.hpp"
#include "io.hpp"
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
namespace kline {
inline void sitewide_finance_series(std::ostream& out,const std::string& label,
                                    const std::vector<double>& values){
    out<<"Index,"<<label<<"\n";
    for(int i=0;i<int(values.size());i++)
        out<<i<<","<<values[i]<<"\n";
}
inline bool run_sitewide_finance_cli(const std::string& command,
                      int argc,char** argv,std::ostream& out){
    auto exact=[&](int n){require(argc==n,"Incorrect financial research arguments");};
    auto v=[&](int i){return parse_number(argv[i]);};
    if(command=="delta-hedge"){
        exact(8);auto t=read_csv(argv[2]);
        auto results=short_option_delta_hedge(
            t.numbers(argv[3]),t.numbers(argv[4]),
            t.numbers(argv[5]),v(6),v(7));
        out<<"Index,Cash,HedgeUnits,NetLiquidation,Commission\n";
        for(int i=0;i<int(results.size());i++){
            const auto& x=results[i];
            out<<i<<","<<x.cash<<","<<x.stock_units<<","
               <<x.net_liquidation<<","<<x.fees<<"\n";
        }
    }else if(command=="delta-gamma-hedge"){
        exact(6);
        auto result=delta_gamma_hedge(v(2),v(3),v(4),v(5));
        out<<"UnderlyingUnits "<<result.underlying<<"\n"
           <<"HedgeOptionUnits "<<result.option<<"\n";
    }else if(command=="greek-pnl"){
        exact(7);
        out<<"SecondOrderPnL "<<second_order_greeks_pnl(
            v(2),v(3),v(4),v(5),v(6))<<"\n";
    }else if(command=="payer-swap"){
        exact(8);auto table=read_csv(argv[2]);
        auto times=table.numbers(argv[3]),rates=table.numbers(argv[4]),
             accruals=table.numbers(argv[5]);
        out<<"SwapPV "<<payer_swap_present_value(
            {times,rates},times,accruals,v(6),v(7))<<"\n";
    }else if(command=="survival"){
        exact(6);auto t=read_csv(argv[2]);
        out<<"Survival "<<survival_probability(
            {t.numbers(argv[3]),t.numbers(argv[4])},v(5))<<"\n";
    }else if(command=="hazard-loss"){
        exact(9);auto t=read_csv(argv[2]);
        out<<"ExpectedDiscountedLoss "<<expected_discounted_default_loss(
            {t.numbers(argv[3]),t.numbers(argv[4])},v(5),v(6),v(7),v(8))
            <<"\n";
    }else if(command=="credit-el"||command=="credit-variance"){
        exact(6);auto t=read_csv(argv[2]);
        double x=command=="credit-el"?
            independent_credit_expected_loss(
                t.numbers(argv[3]),t.numbers(argv[4]),t.numbers(argv[5])):
            independent_credit_loss_variance(
                t.numbers(argv[3]),t.numbers(argv[4]),t.numbers(argv[5]));
        out<<(command=="credit-el"?"ExpectedLoss ":"LossVariance ")<<x<<"\n";
    }else if(command=="fx-hedged"){
        exact(7);
        out<<"TerminalDomesticValue "<<hedged_foreign_investment_terminal(
            v(2),v(3),v(4),v(5),v(6))<<"\n";
    }else if(command=="keyrate-hedge2"){
        exact(8);
        auto hedge=solve_keyrate_dv01_hedge(
            {{v(2),v(3)},{v(4),v(5)}},{v(6),v(7)});
        sitewide_finance_series(out,"HedgeUnits",hedge);
    }else if(command=="tracking-error2"){
        exact(7);
        double error=ex_ante_tracking_error(
            {{v(2),v(3)},{v(3),v(4)}},{v(5),v(6)});
        out<<"TrackingError "<<error<<"\n";
    }else if(command=="target-vol"){
        exact(7);
        sitewide_finance_series(out,"TargetGross",
            past_only_volatility_target(
                read_csv(argv[2]).numbers(argv[3]),
                std::stoi(argv[4]),v(5),v(6)));
    }else if(command=="short-margin"){
        exact(8);
        double equity=short_equity_after_price_move(
            v(2),v(3),v(4),v(5),v(6));
        out<<"Equity "<<equity<<"\nMarginCall "<<short_margin_call_required(
            equity,v(3),v(5),v(7))<<"\n";
    }else if(command=="book-replay"){
        exact(6);
        auto table=read_csv(argv[2]);
        auto side=table.numbers(argv[3]),price=table.numbers(argv[4]),
             available=table.numbers(argv[5]);
        BookReplay book;
        for(int i=0;i<int(side.size());i++){
            require(side[i]==0||side[i]==1,"Bid side must be 1 and ask 0");
            apply_orderbook_level(book,side[i]==1,price[i],available[i]);
        }
        validate_uncrossed_book(book);
        out<<"BestBid "<<book_best_bid(book)<<"\n"
           <<"BestAsk "<<book_best_ask(book)<<"\n"
           <<"Notional "<<total_visible_book_notional(book)<<"\n";
    }else if(command=="microprice"){
        exact(6);
        out<<"Microprice "<<level_one_microprice(
            v(2),v(3),v(4),v(5))<<"\n";
    }else if(command=="depth-imbalance"){
        exact(4);
        out<<"Imbalance "<<depth_imbalance(v(2),v(3))<<"\n";
    }else if(command=="tick-sign"){
        exact(4);
        auto signs=tick_rule_trade_signs(
            read_csv(argv[2]).numbers(argv[3]));
        out<<"Index,TradeSign\n";
        for(int i=0;i<int(signs.size());i++)
            out<<i<<","<<signs[i]<<"\n";
    }else if(command=="signed-response"){
        exact(6);auto t=read_csv(argv[2]);
        auto sign_raw=t.numbers(argv[4]);
        std::vector<int> signs;
        for(double sign:sign_raw){
            require(sign==1||sign==-1,"Trade sign must be +1 or -1");
            signs.push_back(int(sign));
        }
        out<<"Response "<<signed_trade_response(
            t.numbers(argv[3]),signs,std::stoi(argv[5]))<<"\n";
    }else if(command=="effective-spread"){
        exact(5);
        double side=v(4);
        require(side==1||side==-1,"Trade side must be buy +1 or sell -1");
        out<<"EffectiveSpread "<<effective_spread(
            v(2),v(3),int(side))<<"\n";
    }else if(command=="market-impact"){
        exact(6);
        out<<"ImpactFraction "<<square_root_impact_fraction(
            v(2),v(3),v(4),v(5))<<"\n";
    }else if(command=="fifo-queue"){
        exact(5);
        auto queue=consume_fifo_queue({v(2),v(3),0},v(4));
        out<<"Filled "<<queue.own_filled<<"\n"
           <<"Remaining "<<queue.own_remaining<<"\n"
           <<"Ahead "<<queue.queue_ahead<<"\n";
    }else if(command=="tca-decompose"){
        exact(8);auto t=read_csv(argv[2]);
        auto shares=t.numbers(argv[3]),price=t.numbers(argv[4]),
             fees=t.numbers(argv[5]),mid=t.numbers(argv[6]);
        std::vector<Fill> fills;
        for(int i=0;i<int(shares.size());i++)
            fills.push_back({shares[i],price[i],fees[i]});
        auto report=execution_cost_attribution(fills,mid,v(7));
        out<<"SpreadCost "<<report.spread_cost<<"\n"
           <<"MarketMove "<<report.market_movement<<"\n"
           <<"Fees "<<report.fees<<"\n"
           <<"Total "<<report.total<<"\n";
    }else if(command=="vpin-proxy"){
        exact(5);auto t=read_csv(argv[2]);
        out<<"VolumeImbalance "<<volume_bucket_imbalance_proxy(
            t.numbers(argv[3]),t.numbers(argv[4]))<<"\n";
    }else if(command=="ofi-books"){
        exact(10);
        out<<"OrderFlowImbalance "<<best_level_order_flow_imbalance(
            {v(2),v(3),v(4),v(5)},
            {v(6),v(7),v(8),v(9)})<<"\n";
    }else if(command=="child-schedule"){
        exact(6);auto t=read_csv(argv[2]);
        auto result=bounded_child_schedule(
            v(5),t.numbers(argv[3]),t.numbers(argv[4]));
        sitewide_finance_series(out,"ChildExecuted",result.child_orders);
        out<<"Unfilled "<<result.unfilled<<"\n";
    }else if(command=="market-maker-quote"){
        exact(8);
        auto quote=avellaneda_stoikov_quotes(
            v(2),v(3),v(4),v(5),v(6),v(7));
        out<<"Reservation "<<quote.reservation<<"\nBid "<<quote.bid
           <<"\nAsk "<<quote.ask<<"\nSpread "<<quote.spread<<"\n";
    }else if(command=="trade-book"){
        exact(8);
        auto b=execute_cash_stock_trade({v(2),v(3),0},v(4),v(5),v(6));
        out<<"Cash "<<b.cash<<"\nShares "<<b.shares
           <<"\nEquity "<<mark_transaction_book(b,v(7))<<"\n";
    }else return false;
    return true;
}
inline void sitewide_finance_usage(std::ostream& out){
    out<<"Cross-asset derivatives, trading and market microstructure:\n"
       <<"  delta-hedge CSV SPOT OPTION DELTA RATE_PER_PERIOD FEE_PER_UNIT\n"
       <<"  delta-gamma-hedge PORTFOLIO_DELTA PORTFOLIO_GAMMA HEDGE_DELTA HEDGE_GAMMA\n"
       <<"  greek-pnl DELTA GAMMA VEGA SPOT_CHANGE VOL_CHANGE\n"
       <<"  payer-swap CSV TENOR ZERO_RATE ACCRUAL COUPON NOTIONAL\n"
       <<"  survival CSV TENOR HAZARD MATURITY\n"
       <<"  hazard-loss CSV TENOR HAZARD RECOVERY DISCOUNT_RATE NOTIONAL MATURITY\n"
       <<"  credit-el|credit-variance CSV EAD PD LGD\n"
       <<"  fx-hedged NOTIONAL_F FUTURE_VALUE_F SPOT_D_PER_F FORWARD_D_PER_F HEDGE_RATIO\n"
       <<"  keyrate-hedge2 INSTR1_NODE1 INSTR1_NODE2 INSTR2_NODE1 INSTR2_NODE2 PORT_NODE1 PORT_NODE2\n"
       <<"  tracking-error2 C11 C12 C22 ACTIVE_WEIGHT1 ACTIVE_WEIGHT2\n"
       <<"  target-vol CSV RETURN LOOKBACK TARGET_ANNUAL_VOL MAX_GROSS\n"
       <<"  short-margin CASH SHARES SALE_PRICE CURRENT_PRICE BORROW_COST REQUIRED_RATIO\n"
       <<"  book-replay CSV IS_BID PRICE AVAILABLE\n"
       <<"  microprice BID ASK BID_SIZE ASK_SIZE\n"
       <<"  depth-imbalance BID_SIZE ASK_SIZE\n"
       <<"  tick-sign CSV TRADE_PRICE\n"
       <<"  signed-response CSV PRICE SIGN HORIZON\n"
       <<"  effective-spread TRADE_PRICE MID TRADE_SIDE\n"
       <<"  market-impact DAILY_VOL ORDER_SHARES DAILY_VOLUME COEFFICIENT\n"
       <<"  fifo-queue QUEUE_AHEAD OWN_SHARES MARKETABLE_SHARES\n"
       <<"  tca-decompose CSV SIGNED_SHARES PRICE COMMISSION MIDPOINT ARRIVAL_MID\n"
       <<"  vpin-proxy CSV BUY_VOLUME SELL_VOLUME\n"
       <<"  ofi-books OLD_BID OLD_ASK OLD_BID_SIZE OLD_ASK_SIZE NEW_BID NEW_ASK NEW_BID_SIZE NEW_ASK_SIZE\n"
       <<"  child-schedule CSV DESIRED MAX_PER_BUCKET TOTAL_SHARES\n"
       <<"  market-maker-quote MID INVENTORY RISK_AVERSION VOL REMAINING_TIME ARRIVAL_DECAY\n"
       <<"  trade-book CASH SHARES SIGNED_TRADE TRADE_PRICE COMMISSION MARK_PRICE\n";
}
} // namespace kline
