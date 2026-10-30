# Research & Backtesting 技术说明

## 运行平台

- **站内轻量 Python 引擎**：浏览器通过 Pyodide 314.0.7（WebAssembly）执行真实 Python 代码，代码在独立 Web Worker 中运行，不使用 GitHub Pages 服务器端 Python。可停止 Worker；首次访问需要下载 Python 运行时。
- **真实 Backtrader 框架**：`backtrader_starter.ipynb` 可以在 Google Colab 中打开，运行 `pip install backtrader`，然后上传用户自己的 CSV。
- **Zipline Reloaded**：本站提供官方文档链接，不声称在 GitHub Pages 中安装/运行完整 Zipline。
- 只承诺这一套**基础日频OHLC/多资产目标权重简化回测**，不支持逐笔、期权行权、保证金与真实券商成交撮合。

## 文件格式

单资产：

```csv
Date,Open,High,Low,Close,Volume
2024-01-02,100,102,99,101,20000
2024-01-03,101,103,100,102,22000
```

多资产：

```csv
Date,Ticker,Open,High,Low,Close,Volume
2024-01-02,ETF_A,100,102,99,101,20000
2024-01-02,ETF_B,80,81,79,80.5,32000
```

日期统一按 `YYYY-MM-DD`；至少35个共同交易日、最多40个Ticker、120000行、5000个交易日。每个Ticker-Date必须唯一。由于取日期交集，缺失、停牌和退市信息不一定保真，正式回测需构建PIT交易日历并处理不可交易状态。需要 `Open`、`Close`，不支持只有Close的CSV。

**样例是合成价格，不是证券历史价格**，不得将其收益当成可实现的历史成绩。必须上传真实授权行情来得到真实历史回测曲线。用户CSV在浏览器侧解析；站点代码未设计将CSV上传到自己的服务器。

## 策略接口与时点

策略代码定义 `def signals(history):`，接受资产到此前历史bar的字典，返回 `{ticker: target_weight}`。目标权重在 `[-1, 1]`，总绝对敞口不超过1。

- 前一交易日 `t-1` 收盘后利用截止该日的历史，生成 `t` 日目标仓位。
- 在 `t` 日Open调仓，持仓至 `t+1` 日Open，按开盘价收益计算组合P&L。
- 每次仓位变化按 `sum(abs(new_weight-old_weight)) * (commission+slippage)/10000` 计线性成本。
- 做空借券按 `sum(short_abs_weight) * annual_borrow_bps / (10000*252)` 计费。
- 不支持杠杆与现金利息；默认等权每日再平衡基准**不扣手续费**。净策略与无费用基准差异需谨慎解读。
- 调仓事件为目标权重变化；持仓段胜率将每次调整视为上一持仓段结束，是近似分析，并非真实平仓胜率。

## 风险指标

- 年化收益：`equity_T**(252 / N) - 1`
- 年化波动率：`std(daily_returns, ddof=1) * sqrt(252)`
- Sharpe：`mean(strategy_daily - daily_rf) / sample_std(strategy_daily) * sqrt(252)`
- Sortino：以相对日度无风险利率的负超额收益均方根作为下行偏差。
- 最大回撤：`min(equity_t / running_peak_t - 1)`
- 胜率：正收益日占所有非零收益日的比例，同时另有持仓段近似胜率。
- 盈亏比：平均正日收益 / 平均负日收益绝对值；Profit Factor是累计正收益 / 累计负收益绝对值。
- 信息比率：`mean(strategy_return - benchmark_return) / std(active_return) * sqrt(252)`
- Beta：日度收益与等权基准的样本协方差 / 基准样本方差。
- Alpha：按日度超额收益的线性CAPM截距 × 252；与几何年化Alpha不完全相同。
- Tracking Error：主动日收益样本标准差 × sqrt(252)。
- 若收益方差为零、无亏损日、无基准方差等情形，相关指标返回 `null` 而非编造数字。

## 可复现、限制与验证

数据源、代码、费用和风险报告可保存JSON；净值可导出CSV。用户应保留数据版本、复权规则、模型超参数和策略迭代日志。平台未提供真实逐笔成交、事件驱动撮合、市场冲击、持仓约束、税务、信息时点审核或实时交易。

`python -m unittest discover quant/research -p 'test_*.py' -v` 覆盖所有策略模板、下一开盘时点、成本扣除、非法杠杆和重复日期等；GitHub Actions自动运行，但不能代替不同浏览器和网络环境下的Pyodide实际加载测试。

官方相关链接：[Pyodide](https://pyodide.org/en/stable/) · [Backtrader](https://www.backtrader.com/docu/) · [Zipline Reloaded](https://zipline.ml4trading.io/)
