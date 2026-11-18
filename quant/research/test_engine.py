"""Integration tests for the browser-side Python backtest engine (pure Python).
Run: python -m unittest discover quant/research -p 'test_*.py' -v
"""
import importlib.util
import json
import pathlib
import unittest
from datetime import date,timedelta

HERE=pathlib.Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location("kline_backtest_engine",HERE/"engine.py")
engine=importlib.util.module_from_spec(spec)
spec.loader.exec_module(engine)

def sample(rows=110):
    out=["Date,Ticker,Open,High,Low,Close,Volume"]
    d=date(2024,1,2)
    i=0
    while i<rows:
        if d.weekday()<5:
            for j,ticker in enumerate(["SYN_A","SYN_B","SYN_C"]):
                # Deterministic synthetic market. No claim of real prices.
                op=90+j*18+i*(0.08 if j!=1 else -0.025)+(i%11-5)*0.13
                cl=op*(1+0.002*((i+j)%5-2))
                out.append(f"{d},{ticker},{op:.4f},{max(op,cl):.4f},{min(op,cl):.4f},{cl:.4f},{1000+i*2+j}")
            i+=1
        d+=timedelta(days=1)
    return "\n".join(out)

class BacktestTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.csv=sample()
        cls.strategies=json.loads((HERE/"strategies.json").read_text(encoding="utf-8"))["strategies"]

    def execute(self,code,**kwargs):
        conf={"csv":self.csv,"code":code,"label":"单元测试合成数据","commission_bps":3,
              "slippage_bps":2,"risk_free_annual":0.02,"borrow_bps_annual":20}
        conf.update(kwargs)
        return json.loads(engine.main(json.dumps(conf)))

    def test_all_templates_run_and_have_risk_metrics(self):
        for strategy in self.strategies:
            with self.subTest(strategy=strategy["id"]):
                result=self.execute(strategy["code"])
                self.assertEqual(result["meta"]["assets"],["SYN_A","SYN_B","SYN_C"])
                self.assertEqual(len(result["equity"]),109)
                self.assertTrue(-1<result["metrics"]["total_return"]<100)
                self.assertTrue(-1<=result["metrics"]["max_drawdown"]<=0)
                self.assertIn("information_ratio",result["metrics"])
                self.assertIn("alpha_annual",result["metrics"])
                self.assertIn("beta",result["metrics"])
                self.assertEqual(result["meta"]["data_label"],"单元测试合成数据")

    def test_strategy_library_research_design_and_uniqueness(self):
        ids=[s["id"] for s in self.strategies]
        self.assertEqual(len(ids),len(set(ids)))
        self.assertGreaterEqual(len(ids),70)
        self.assertGreaterEqual(sum(bool(s.get("research")) for s in self.strategies),64)
        for s in self.strategies:
            self.assertTrue(s["name"] and s["description"] and s["requirements"])
            self.assertIn("def signals(history):",s["code"])
            r=s.get("research")
            if not r:
                continue
            required={"mechanism":55,"formula":30,"derivation":40,"worked":35,"validation":35,"limitations":25}
            for field,minimum in required.items():
                self.assertGreaterEqual(len(r[field]),minimum,(s["id"],field))
            self.assertTrue(s["dataset"].startswith("仅") or "真实" in s["dataset"])
            self.assertNotIn("Sharpe 2.",s["dataset"])

    def test_all_new_models_no_nan_on_constant_market(self):
        # The engine must handle no-information prices and return finite reports.
        lines=["Date,Ticker,Open,High,Low,Close,Volume"]
        from datetime import date,timedelta
        start=date(2024,1,1)
        for i in range(110):
            ds=(start+timedelta(days=i)).isoformat()
            for t in ("SYN_A","SYN_B","SYN_C"):
                lines.append(f"{ds},{t},100,100,100,100,1000")
        csv="\n".join(lines)
        for strategy in self.strategies:
            with self.subTest(strategy=strategy["id"]):
                result=self.execute(strategy["code"],csv=csv)
                self.assertTrue(-1<result["metrics"]["total_return"]<100)
                self.assertTrue(all(e["strategy"]>0 for e in result["equity"]))

    def test_rebalance_cost_charges_drift_after_a_price_shock(self):
        # Flat quotes after a single +20% move on A, B always remains 100.
        # Repeated equal-weight targets still require real cash rebalancing.
        days=[]
        d=date(2024,1,2)
        while len(days)<45:
            if d.weekday()<5:
                days.append(d.isoformat())
            d+=timedelta(days=1)
        csv=["Date,Ticker,Open,Close,Volume"]
        for i,dt in enumerate(days):
            for ticker in ("SYN_A","SYN_B"):
                px=120 if ticker=="SYN_A" and i>=2 else 100
                csv.append(f"{dt},{ticker},{px},{px},10000")
        code="def signals(history):\\n    return {'SYN_A':0.5,'SYN_B':0.5}\\n".replace("\\n","\n")
        result=self.execute(code,csv="\n".join(csv),commission_bps=100,slippage_bps=0,borrow_bps_annual=0)
        # First trade costs 1% NAV (0.5+0.5), next open's 20% stock jump
        # creates drift: stale-target logic would incorrectly report 50% turnover.
        self.assertGreater(result["metrics"]["turnover_oneway"],0.53)
        self.assertGreater(result["metrics"]["adjustment_events"],2)
        self.assertLess(result["metrics"]["total_return"],0.09)

    def test_static_single_asset_remains_fee_free_after_initial_purchase(self):
        code="def signals(history):\\n    return {'SYN_A':1.0}\\n".replace("\\n","\n")
        result=self.execute(code,commission_bps=100,slippage_bps=0,borrow_bps_annual=0)
        # A 1% initial trade fee reduces the cash available for the initial
        # purchase; post-fee 100%-invested weights must not trigger re-trades.
        expected_initial_turnover=0.5/(1+0.01)
        self.assertAlmostEqual(result["metrics"]["turnover_oneway"],expected_initial_turnover,places=6)
        self.assertEqual(result["metrics"]["adjustment_events"],1)

    def test_next_open_no_lookahead(self):
        code="def signals(history):\n    return {'SYN_A':1.0}\n"
        result=self.execute(code,commission_bps=0,slippage_bps=0,borrow_bps_annual=0)
        opens=[]
        for line in self.csv.splitlines()[1:]:
            fields=line.split(",")
            if fields[1]=="SYN_A":opens.append(float(fields[2]))
        expected=opens[-1]/opens[1]-1
        self.assertAlmostEqual(result["metrics"]["total_return"],expected,places=6)

    def test_costs_reduce_performance(self):
        code="def signals(history):\n    return {'SYN_A':1.0}\n"
        gross=self.execute(code,commission_bps=0,slippage_bps=0,borrow_bps_annual=0)
        net=self.execute(code,commission_bps=25,slippage_bps=15,borrow_bps_annual=0)
        self.assertLess(net["metrics"]["total_return"],gross["metrics"]["total_return"])

    def test_reject_future_symbol(self):
        bad="def signals(history):\n    return {'DOES_NOT_EXIST':1.0}\n"
        with self.assertRaisesRegex(ValueError,"未知Ticker"):
            self.execute(bad)

    def test_reject_leverage(self):
        bad="def signals(history):\n    return {'SYN_A':1.0,'SYN_B':1.0}\n"
        with self.assertRaisesRegex(ValueError,"超过1"):
            self.execute(bad)

    def test_input_duplicates_are_rejected(self):
        bad=self.csv+"\n"+self.csv.splitlines()[1]
        with self.assertRaisesRegex(ValueError,"重复记录"):
            self.execute("def signals(history):\n return {}\n",csv=bad)

    def test_missing_open_rejected(self):
        with self.assertRaisesRegex(ValueError,"缺少 Open/Close"):
            self.execute("def signals(history):\n return {}\n",csv="Date,Close\n2024-01-01,100\n")

if __name__=="__main__":
    unittest.main()
