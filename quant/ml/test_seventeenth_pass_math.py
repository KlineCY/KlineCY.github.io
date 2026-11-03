"""Stage seventeen: exact-source structure and six independent worked cases."""
import json
import math
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parent
LESSONS={}
for suffix in "ab":
    data=json.loads((ROOT/f"atlas-depth-17-{suffix}.json").read_text(encoding="utf-8"))["lessons"]
    for lid,chapter in data.items():
        assert lid not in LESSONS
        LESSONS[lid]=chapter
class SeventeenthPassMath(unittest.TestCase):
    def test_structure(self):
        self.assertEqual(len(LESSONS),12)
        self.assertEqual({uid[:3] for uid in LESSONS},{f"m{i:02d}" for i in range(3,15)})
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
            ("m09-c03-p01",.5,.5,"原始目标为0.5，对偶目标为1−0.5=0.5"),
            ("m10-c01-p03",8/(36*7),8/252,"0.031746"),
            ("m11-c04-p04",.5*.8/(.5*.8+.5*.2),.8,"责任度为0.5×0.8/(0.5×0.8+0.5×0.2)=0.8"),
            ("m12-c03-p03",math.sqrt((1+2*.5*(-.75))/4),.25,"HAC标准误为√(0.25/4)=0.25"),
            ("m13-c02-p02",2*(3*2)*2,24,"梯度为(2z)×x=12×2=24"),
            ("m14-c01-p01",3-2,1,"输出长度为2"),
        ]
        for uid,got,expected,token in examples:
            with self.subTest(uid=uid):
                self.assertAlmostEqual(got,expected,delta=1e-9)
                self.assertIn(token,LESSONS[uid]["worked_example"])
if __name__=="__main__":
    unittest.main()
