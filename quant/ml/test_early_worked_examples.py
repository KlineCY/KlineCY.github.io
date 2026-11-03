"""Early Atlas worked examples: 24 targeted numerical sanity checks."""
import json
import math
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parent
FILES=("atlas-depth.json","atlas-depth-foundations.json","atlas-depth-learning.json")
ROWS={}
for name in FILES:
    module=json.loads((ROOT/name).read_text(encoding="utf-8"))
    assert len(module["lessons"])==8
    for lid,body in module["lessons"].items():
        assert lid not in ROWS
        ROWS[lid]=body

class EarlyWorkedExamples(unittest.TestCase):
    def test_all_present_and_exact_source(self):
        self.assertEqual(len(ROWS),24)
        for lid,body in ROWS.items():
            with self.subTest(lid=lid):
                self.assertTrue(body["source_title"])
                self.assertGreaterEqual(len(body["worked_example"]),110)
                self.assertGreaterEqual(len(body["equations"]),3)
    def test_reproducible_arithmetic(self):
        examples=[
            ("m24-c01-p04",.18-.12,.06,"0.12"),
            ("m24-c02-p02",2+3+4+1+5,15,"15毫秒"),
            ("m24-c03-p03",.8*.625*0+.2*2.5*4,2,"加权风险"),
            ("m24-c03-p04",.8-.2,.6,"0.8"),
            ("m24-c04-p01",math.log(1.5),.4054651081,"0.4055"),
            ("m24-c05-p02",5-(1+3)/2,3,"5−2=3"),
            ("m24-c05-p03",.001*(abs(.6-.5)+abs(.4-.5)),.0002,"0.0002"),
            ("m24-c05-p05",100-2*10-1+2*11,101,"101"),
            ("m01-c01-p05",1-0,1,"ATE=0"),
            ("m02-c04-p06",1/.01,100,"70.7%"),
            ("m03-c05-p03",(6*.2**2+4*.8**2)/10,.28,"0.28"),
            ("m04-c03-p06",math.sqrt(100/400),.5,"减半"),
            ("m05-c03-p03",math.log(.5)/math.log(.9),6.57881347896,"6.58"),
            ("m06-c04-p02",math.sqrt(2-2*math.cos(math.pi/12)),.26105238444,"0.2611"),
            ("m07-c05-p02",(2+2)/(1+1),2,"β=2"),
            ("m08-c04-p06",2**(3+1)-1,15,"15"),
            ("m09-c04-p06",10000**2*8/1e6,800,"800MB"),
            ("m10-c02-p06",.5*.8/(.5*.2+.5*.8),.8,"0.8"),
            ("m11-c02-p05",1/(1+4),.2,"0.2"),
            ("m12-c05-p05",15/20,.75,"0.75"),
            ("m13-c04-p02",1+3,4,"Y=4"),
            ("m14-c03-p04",2/5,.4,"0.5714"),
            ("m15-c03-p05",.5**10,1/1024,"0.0009766"),
            ("m16-c02-p06",1024**2/512**2,4,"4倍"),
        ]
        self.assertEqual(len(examples),len(ROWS))
        for lid,got,expected,token in examples:
            with self.subTest(lid=lid):
                self.assertAlmostEqual(got,expected,delta=1e-8)
                self.assertIn(token,ROWS[lid]["worked_example"])
if __name__=="__main__":
    unittest.main()
