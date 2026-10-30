"""Kline Quant browser-only research engine (educational; next-open execution).

Input CSV: Date,Open,Close[,Ticker,High,Low,Volume] or
date,ticker,open,high,low,close,volume.
Each signal uses closes strictly before the next bar's OPEN.
"""
import csv
import io
import json
import math
import statistics
from collections import defaultdict
from datetime import datetime

def _parse(text):
    reader=csv.DictReader(io.StringIO(text.replace("\ufeff","")))
    if not reader.fieldnames:
        raise ValueError("CSV 无表头。至少需要 Date,Open,Close，多个资产还需要 Ticker。")
    header={k.strip().lower():k for k in reader.fieldnames if k}
    def value(row,*keys,default=""):
        for k in keys:
            if k in header:
                return (row.get(header[k]) or "").strip()
        return default
    if not any(k in header for k in ["date","datetime","timestamp","time"]):
        raise ValueError("缺少 Date/Datetime/Timestamp 日期列")
    if "open" not in header or "close" not in header:
        raise ValueError("缺少 Open/Close；本引擎使用收盘后信号、次日开盘执行。")
    series=defaultdict(dict)
    count=0
    for line,row in enumerate(reader,start=2):
        raw_date=value(row,"date","datetime","timestamp","time")
        try:
            date=datetime.fromisoformat(raw_date[:10].replace("/","-")).date().isoformat()
            op=float(value(row,"open"))
            cl=float(value(row,"close"))
            voltext=value(row,"volume")
            vol=float(voltext) if voltext else 0.
        except (ValueError,OverflowError) as exc:
            raise ValueError(f"第{line}行日期或价格无法解析") from exc
        if not (math.isfinite(op) and math.isfinite(cl) and op>0 and cl>0 and math.isfinite(vol)):
            raise ValueError(f"第{line}行存在无效价格或成交量")
        ticker=value(row,"ticker","symbol","asset",default="ASSET").upper() or "ASSET"
        if ticker in ("__ALL__","__BENCHMARK__"):
            raise ValueError("Ticker 不得使用保留名 __ALL__ 或 __BENCHMARK__")
        if len(ticker)>45 or len(ticker)<1:
            raise ValueError(f"第{line}行Ticker异常")
        if date in series[ticker]:
            raise ValueError(f"重复记录 {ticker} {date}；请先去重")
        series[ticker][date]={"date":date,"open":op,"close":cl,"volume":vol}
        count+=1
        if count>120000:
            raise ValueError("浏览器回测最多处理120,000行CSV，请先按资产或时间筛选")
    if not series or len(series)>40:
        raise ValueError("资产数需为1到40；多资产CSV需带Ticker列")
    common=set.intersection(*(set(rows.keys()) for rows in series.values()))
    dates=sorted(common)
    if len(dates)<35:
        raise ValueError(f"共同交易日期仅{len(dates)}条，至少需要35条。多资产请核对Ticker和Date的交集。")
    if len(dates)>5000:
        raise ValueError("为避免浏览器卡顿，单次最多5000个交易日")
    tickers=sorted(series.keys())
    aligned={t:[series[t][d] for d in dates] for t in tickers}
    return dates,tickers,aligned,{"rows":count,"common_dates":len(dates),"discarded_rows":count-len(dates)*len(tickers),"assets":len(tickers)}

def _mean(data):
    return sum(data)/len(data) if data else None

def _std(data):
    return statistics.stdev(data) if len(data)>1 else 0.0

def _fmt(number):
    if number is None or not isinstance(number,(int,float)) or not math.isfinite(number):
        return None
    return round(float(number),7)

def _metrics(rets,baseline,equity,positions,closed_segments,turnover,fee_bps,rf_annual):
    n=len(rets)
    if n<2:
        raise ValueError("有效回测期不足")
    rf_daily=(1+rf_annual)**(1/252)-1
    sdev=_std(rets)
    excess=[v-rf_daily for v in rets]
    sharpe=(sum(excess)/n)/sdev*math.sqrt(252) if sdev>1e-12 else None
    down_dev=math.sqrt(sum(min(0,v)**2 for v in excess)/n)
    sortino=(sum(excess)/n)/down_dev*math.sqrt(252) if down_dev>1e-12 else None
    max_dd=0.0; peak=0
    for item in equity:
        current=item["strategy"]
        peak=max(peak,current)
        max_dd=min(max_dd,current/peak-1) if peak else max_dd
    total=equity[-1]["strategy"]-1
    annualized=equity[-1]["strategy"]**(252/n)-1 if equity[-1]["strategy"]>0 else -1
    bp=_mean(baseline) or 0
    sp=_mean(rets) or 0
    var_b=sum((v-bp)**2 for v in baseline)
    beta=sum((a-sp)*(b-bp) for a,b in zip(rets,baseline))/var_b if var_b>1e-18 else None
    alpha=((sp-rf_daily)-beta*(bp-rf_daily))*252 if beta is not None else None
    active=[a-b for a,b in zip(rets,baseline)]
    tracking=_std(active)*math.sqrt(252)
    ir=(_mean(active)*math.sqrt(252)/_std(active)) if _std(active)>1e-12 else None
    pos=sum(v>0 for v in rets)
    neg=sum(v<0 for v in rets)
    positive=[v for v in rets if v>0]
    negative=[-v for v in rets if v<0]
    payoff=(_mean(positive)/_mean(negative)) if positive and negative else None
    profit_factor=sum(positive)/sum(negative) if negative else None
    benchmark_end=equity[-1]["benchmark"]
    result={
        "periods":n,
        "total_return":_fmt(total),
        "annualized_return":_fmt(annualized),
        "annualized_volatility":_fmt(sdev*math.sqrt(252)),
        "sharpe":_fmt(sharpe),
        "sortino":_fmt(sortino),
        "max_drawdown":_fmt(max_dd),
        "calmar":_fmt(annualized/abs(max_dd)) if abs(max_dd)>1e-10 else None,
        "positive_day_rate":_fmt(pos/(pos+neg)) if pos+neg else None,
        "payoff_ratio":_fmt(payoff),
        "profit_factor":_fmt(profit_factor),
        "information_ratio":_fmt(ir),
        "tracking_error":_fmt(tracking),
        "alpha_annual":_fmt(alpha),
        "beta":_fmt(beta),
        "benchmark_total_return":_fmt(benchmark_end-1),
        "benchmark_annual_return":_fmt(benchmark_end**(252/n)-1),
        "turnover_oneway":_fmt(turnover/2),
        "adjustment_events":len(positions),
        "closed_segments":len(closed_segments),
        "segment_win_rate":_fmt(sum(v>0 for v in closed_segments)/len(closed_segments)) if closed_segments else None,
        "avg_segment_pnl":_fmt(_mean(closed_segments)),
        "fee_bps_per_oneway":fee_bps
    }
    return result

def main(run_json):
    params=json.loads(run_json)
    csv_text=params["csv"]
    user_code=params["code"]
    fees=float(params.get("commission_bps",3))
    slip=float(params.get("slippage_bps",2))
    borrow=float(params.get("borrow_bps_annual",0))
    rf=float(params.get("risk_free_annual",0))
    if not all(math.isfinite(v) for v in [fees,slip,borrow,rf]) or not (0<=fees<=500 and 0<=slip<=500 and 0<=borrow<=5000 and -0.5<rf<0.5):
        raise ValueError("手续费、滑点、借券费率或无风险利率输入不合法")
    dates,tickers,series,stats=_parse(csv_text)
    strategy_ns={"__name__":"__user_strategy__"}
    exec(compile(user_code,"<editable_strategy>","exec"),strategy_ns)
    signal_fn=strategy_ns.get("signals")
    if not callable(signal_fn):
        raise ValueError("策略代码必须定义 def signals(history): 返回 {Ticker: 目标权重}")
    weights={t:0.0 for t in tickers}
    seg_pnl={t:0.0 for t in tickers}
    closed_segments=[]
    position_events=[]
    rets=[];baseline=[];turnover=0.0
    equity=[{"date":dates[1],"strategy":1.0,"benchmark":1.0}]
    eq=1.0;bench=1.0
    for i in range(1,len(dates)-1):
        history={t:series[t][:i] for t in tickers}   # through close(i-1)
        suggested=signal_fn(history)
        if not isinstance(suggested,dict):
            raise ValueError(f"{dates[i-1]}: signals必须返回dict，例如 {{'ETF':1.0}}")
        unknown=set(suggested)-set(tickers)
        if unknown:
            raise ValueError(f"{dates[i-1]}: 策略返回未知Ticker: {', '.join(sorted(unknown))}")
        requested={}
        for t in tickers:
            try: w=float(suggested.get(t,0))
            except (TypeError,ValueError) as e: raise ValueError(f"{t} 目标权重不是数值") from e
            if not math.isfinite(w) or abs(w)>1+1e-9:
                raise ValueError(f"{dates[i-1]}: {t} 的单票权重需在[-1,1]")
            requested[t]=w
        gross=sum(abs(v) for v in requested.values())
        if gross>1.000000001:
            raise ValueError(f"{dates[i-1]}: 总绝对敞口 {gross:.3f} 超过1，当前演示引擎不支持杠杆")
        old=weights
        change=sum(abs(requested[t]-old[t]) for t in tickers)
        period_cost=change*(fees+slip)/10000
        short_cost=sum(-min(0,requested[t]) for t in tickers)*borrow/10000/252
        market={}
        for t in tickers:
            current=series[t][i]["open"]
            following=series[t][i+1]["open"]
            market[t]=following/current-1.0
            if abs(requested[t]-old[t])>1e-9:
                if abs(old[t])>1e-9:
                    closed_segments.append(seg_pnl[t]-(fees+slip)/10000*abs(requested[t]-old[t]))
                seg_pnl[t]=0.0
                position_events.append({"date":dates[i],"ticker":t,"from":_fmt(old[t]),"to":_fmt(requested[t])})
            if abs(requested[t])>1e-9:
                seg_pnl[t]+=requested[t]*market[t]-abs(requested[t]-old[t])*(fees+slip)/10000
        day=sum(requested[t]*market[t] for t in tickers)-period_cost-short_cost
        bday=sum(market.values())/len(tickers) # equal-weight daily rebalanced, no trading friction
        if day<=-1:
            raise ValueError("组合本金损失达到或超过100%，停止回测")
        eq*=1+day;bench*=1+bday
        rets.append(day);baseline.append(bday)
        turnover+=change
        equity.append({"date":dates[i+1],"strategy":_fmt(eq),"benchmark":_fmt(bench)})
        weights=requested
    result={"metrics":_metrics(rets,baseline,equity,position_events,closed_segments,turnover,fees+slip,rf),
        "equity":equity,
        "trades":position_events[-1500:],
        "meta":{"assets":tickers,"first_date":dates[0],"last_date":dates[-1],"input_rows":stats["rows"],
                "intersection_dates":stats["common_dates"],"dropped_unaligned_rows":stats["discarded_rows"],
                "data_label":params.get("label","用户上传数据"),
                "execution":"收盘日t-1产生信号，t日Open按目标权重调仓，持有至t+1日Open。无当日收盘成交假设。",
                "benchmark":"多资产等权每日再平衡基准，单资产时为买入并持有；基准不计交易费用",
                "fee_bps":fees,"slippage_bps":slip,"borrow_bps_annual":borrow,"risk_free_annual":rf,
                "limitations":"资金比例简化模型，不模拟逐笔成交、现金利息、保证金、流动性、期权行权及真实退市；持仓段胜率将每次调仓作为一段，非交易所真实成交胜率。未包含终止时强制平仓费用。"}}
    return json.dumps(result,allow_nan=False,ensure_ascii=False)
