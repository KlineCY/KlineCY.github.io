"""Stage eighteen: exact 720/720 Atlas coverage with independently computed finance/MDP examples."""
import json
import math
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parent
CHAPTERS=json.loads((ROOT/"atlas-depth-18-a.json").read_text(encoding="utf-8"))["lessons"]
EXPECTED={
    "m20-c02-p01":"策略评估：矩阵解与Bellman迭代",
    "m21-c01-p03":"重要性采样",
    "m22-c04-p03":"动态相关：DCC递推、条件协方差与压力时期的风险分散",
    "m23-c04-p02":"收益扣成本：自融资净值、漂移权重与交易时点核算",
}

class EighteenthPassMath(unittest.TestCase):
    def test_exact_coverage_and_structure(self):
        self.assertEqual(set(CHAPTERS),set(EXPECTED))
        for lid,chapter in CHAPTERS.items():
            with self.subTest(lid=lid):
                self.assertEqual(chapter["source_title"],EXPECTED[lid])
                self.assertGreaterEqual(len(chapter["equations"]),3)
                self.assertEqual(len({e["label"] for e in chapter["equations"]}),len(chapter["equations"]))
                self.assertTrue(all(e.get("tex", "").strip() for e in chapter["equations"]))
                self.assertGreaterEqual(len(chapter["derivation"]),3)
                self.assertGreaterEqual(len(chapter["protocol"]),3)
                self.assertGreaterEqual(len(chapter["pitfalls"]),2)
                self.assertGreaterEqual(len(chapter["worked_example"]),90)

    def test_policy_evaluation_exact_linear_solution(self):
        # P=((1/2,1/2),(0,1)), gamma=1/2, r=(1,2)
        v2=2/(1-.5)
        v1=(1+.5*.5*v2)/(1-.5*.5)
        self.assertAlmostEqual(v2,4)
        self.assertAlmostEqual(v1,8/3)
        self.assertIn("8/3",CHAPTERS["m20-c02-p01"]["worked_example"])

    def test_off_policy_importance_sampling(self):
        weights=(.5/.2,.5/.8)
        rewards=(4,0)
        ordinary=sum(w*r for w,r in zip(weights,rewards))/2
        self_normalized=sum(w*r for w,r in zip(weights,rewards))/sum(weights)
        effective=sum(weights)**2/sum(w*w for w in weights)
        self.assertAlmostEqual(ordinary,5)
        self.assertAlmostEqual(self_normalized,3.2)
        self.assertAlmostEqual(effective,25/17)
        self.assertIn("1.4706",CHAPTERS["m21-c01-p03"]["worked_example"])

    def test_dcc_covariance(self):
        q12=(1-.1-.8)*.2+.1*(-1)+.8*.2
        variance1=(1-.1-.8)*1+.1*1+.8*1
        variance2=(1-.1-.8)*1+.1*1+.8*1
        correlation=q12/math.sqrt(variance1*variance2)
        cov=.2*correlation*.3
        self.assertAlmostEqual(correlation,.08)
        self.assertAlmostEqual(cov,.0048)
        self.assertIn("0.0048",CHAPTERS["m22-c04-p03"]["worked_example"])

    def test_self_financing_net_wealth(self):
        initial=(50,50)
        before=(initial[0]*1.2,initial[1]*1)
        wealth=sum(before)
        # Equal post-cost holdings and zero cash: sell x, buy y, x+y=10, x-y=c(x+y).
        gross_to_rebalance=before[0]-before[1]
        sell=(gross_to_rebalance*(1+.01))/2
        buy=(gross_to_rebalance*(1-.01))/2
        orders=(-sell,buy)
        fee=.01*sum(abs(q) for q in orders)
        self.assertAlmostEqual(wealth,110)
        self.assertAlmostEqual(sum(abs(q) for q in orders),10)
        self.assertAlmostEqual(fee,.1)
        self.assertAlmostEqual(before[0]-sell,before[1]+buy)
        self.assertAlmostEqual(sell-buy-fee,0)
        self.assertAlmostEqual(before[0]-sell,54.95)
        self.assertAlmostEqual((wealth-fee)/100-1,.099)
        self.assertIn("9.9%",CHAPTERS["m23-c04-p02"]["worked_example"])

if __name__=="__main__":
    unittest.main()
