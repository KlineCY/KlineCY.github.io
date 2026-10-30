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
