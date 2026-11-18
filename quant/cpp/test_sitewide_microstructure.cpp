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
#include <cmath>
#include <functional>
#include <iostream>
#include <stdexcept>
using namespace std;
int checks=0;
void check(bool ok,const char* why){checks++;if(!ok)throw runtime_error(why);}
void near(double x,double y,double tol,const char* why){
    check(isfinite(x)&&fabs(x-y)<=tol,why);
}
void rejected(const function<void()>& fn,const char* why){
    bool failed=false;try{fn();}catch(const exception&){failed=true;}
    check(failed,why);
}
int main(){
 try{
    kline::BookReplay book;
    kline::apply_orderbook_level(book,true,99,3);
    kline::apply_orderbook_level(book,true,100,4);
    kline::apply_orderbook_level(book,false,101,2);
    kline::apply_orderbook_level(book,false,102,5);
    kline::validate_uncrossed_book(book);
    near(kline::book_best_bid(book),100,1e-12,"Best bid highest quote");
    near(kline::book_best_ask(book),101,1e-12,"Best ask lowest offer");
    near(kline::total_visible_book_notional(book),
         99*3+100*4+101*2+102*5,1e-12,"Visible L2 orderbook notional");
    kline::apply_orderbook_level(book,true,100,0);
    near(kline::book_best_bid(book),99,1e-12,
         "Cancel bid level reveals next best price");
    kline::apply_orderbook_level(book,true,104,1);
    rejected([&]{kline::validate_uncrossed_book(book);},
             "Locked or crossed L2 orderbook must reject");
    near(kline::level_one_microprice(100,102,3,1),
         101.5,1e-12,"Microprice opposite-side weighting");
    near(kline::depth_imbalance(3,1),0.5,1e-12,
         "Normalized bid ask depth imbalance");
    auto signs=kline::tick_rule_trade_signs({100,101,101,100});
    check(signs==vector<int>({1,1,1,-1}),
          "Tick rule persists through tied trades");
    near(kline::signed_trade_response(
         {100,101,102},{1,1,1},1),
         (0.01+1.0/101)/2,1e-12,
         "Signed one-event impact as future return");
    near(kline::quoted_relative_spread(99,101),
         0.02,1e-12,"Relative quoted spread");
    near(kline::effective_spread(101,100,1),
         0.02,1e-12,"Buyer effective spread");
    near(kline::realized_spread(101,100.5,1),
         1/100.5,1e-12,"Buyer realized spread");
    near(kline::square_root_impact_fraction(
         0.02,2500,10000,1),0.01,1e-12,
         "Square root market impact scaling");
    near(kline::estimated_implementation_cost(
         10000,0.001,0.01,0.0001),111,
         1e-12,"Predicted spread impact commission cost");
    auto queue=kline::consume_fifo_queue({10,5,0},12);
    near(queue.queue_ahead,0,1e-12,"Front of FIFO queue consumed");
    near(queue.own_filled,2,1e-12,"FIFO own order partially filled");
    near(queue.own_remaining,3,1e-12,"FIFO own quantity unfilled");
    queue=kline::consume_fifo_queue(queue,10);
    near(queue.own_filled,5,1e-12,"FIFO order fills to cap");
    near(queue.own_remaining,0,1e-12,"FIFO fully filled");
    auto tca=kline::execution_cost_attribution(
         {{10,101,2},{-5,99,1}},{100.5,99.5},100);
    near(tca.spread_cost,7.5,1e-12,
         "Execution quoted-midpoint cost");
    near(tca.market_movement,7.5,1e-12,
         "Execution relative midpoint movement");
    near(tca.fees,3,1e-12,"Execution separate commissions");
    near(tca.total,18,1e-12,"TCA attribution total");
    near(kline::volume_bucket_imbalance_proxy(
         {70,20},{30,80}),0.5,1e-12,"Buy sell bucket imbalance proxy");
    kline::BookSnapshot a{100,102,3,2},b{100,102,5,3};
    near(kline::best_level_order_flow_imbalance(a,b),1,1e-12,
         "Top-of-book event OFI from changes");
    near(kline::microprice_deviation({100,102,3,1}),0.5,1e-12,
         "Microprice weighted midpoint deviation");
    auto schedule=kline::bounded_child_schedule(
         100,{30,50,50},{20,40,20});
    check(schedule.child_orders.size()==3,"Bounded child order count");
    near(schedule.child_orders[0],20,1e-12,
         "Child order must obey per interval liquidity");
    near(schedule.unfilled,20,1e-12,
         "Unfilled shares remain after execution horizon");
    auto quotes=kline::avellaneda_stoikov_quotes(
        100,2,0.1,0.2,1,2);
    check(quotes.reservation<100,
          "Positive inventory lowers reservation quote");
    near(quotes.ask-quotes.bid,quotes.spread,1e-12,
         "Market maker quotes symmetric about reservation");
    auto book2=kline::execute_cash_stock_trade({1000,0,0},5,100,1);
    near(book2.cash,499,1e-12,"Long stock trade cash debit");
    near(book2.shares,5,1e-12,"Long stock inventory");
    near(kline::mark_transaction_book(book2,100),999,1e-12,
         "Transaction fee reduces total account equity");
    auto book3=kline::execute_cash_stock_trade(book2,-5,110,1);
    near(book3.shares,0,1e-12,"Stock inventory returned to zero");
    near(kline::mark_transaction_book(book3,110),1048,1e-12,
         "Round trip realized transaction wealth");
    rejected([]{kline::level_one_microprice(100,99,1,1);},
             "Crossed microprice quotes rejected");
    rejected([]{kline::consume_fifo_queue({-1,5,0},10);},
             "Negative queued shares rejected");
    cout<<"PASS: "<<checks<<" new microstructure, market making and trade ledger checks\n";
 }catch(const exception& e){cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
