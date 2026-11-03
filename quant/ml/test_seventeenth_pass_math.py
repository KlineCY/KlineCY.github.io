"""Stage seventeen: exact-source structure and six independent worked cases."""
import json
import math
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parent
LESSONS=json.loads((ROOT/"atlas-depth-17-a.json").read_text(encoding="utf-8"))["lessons"]
class SeventeenthPassMath(unittest.TestCase):
    def test_structure(self):
        self.assertEqual(len(LESSONS),6)
        self.assertEqual({uid[:3] for uid in LESSONS},{f"m{i:02d}" for i in range(3,9)})
        for uid,body in LESSONS.items():
            self.assertEqual(len(body["equations"]),3)
            self.assertGreaterEqual(len(body["derivation"]),3)
            self.assertGreaterEqual(len(body["protocol"]),3)
            self.assertGreaterEqual(len(body["pitfalls"]),2)
    def test_numeric(self):
        examples=[
            ("m03-c03-p04",math.exp(-2),.1353352832,"0.1353"),
            ("m04-c01-p02",2*math.exp(-8),.0006709253,"0.000671"),
            ("m05-c02-p03",5/2,2.5,"5/2=2.5"),
            ("m06-c04-p05",16*60-(15*60+30),30,"16:00收盘晚于15:30"),
            ("m07-c02-p01",.5**2*4+(1-.5)**2*2**2,2,"MSE降为1+1=2"),
            ("m08-c01-p02",1-(.75**2+.25**2),.375,"Gini=1−0.75²−0.25²=0.375"),
        ]
        for uid,got,expected,token in examples:
            with self.subTest(uid=uid):
                self.assertAlmostEqual(got,expected,delta=1e-9)
                self.assertIn(token,LESSONS[uid]["worked_example"])
if __name__=="__main__":
    unittest.main()
