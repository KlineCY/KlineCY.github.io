"""Numerical review of 16 examples in the early second-wave Atlas overlays."""
import json
import math
import unittest
from pathlib import Path

ROOT=Path(__file__).resolve().parent
FILES=("atlas-depth-quant.json","atlas-depth-foundations-2.json")
ROWS={}
for name in FILES:
    src=json.loads((ROOT/name).read_text(encoding="utf-8"))
    assert len(src["lessons"])==8
    for lid,body in src["lessons"].items():
        assert lid not in ROWS
        ROWS[lid]=body

class SecondWaveEarlyExamples(unittest.TestCase):
    def test_coverage(self):
        self.assertEqual(len(ROWS),16)
        for lid,body in ROWS.items():
            with self.subTest(lid=lid):
                self.assertTrue(body["source_title"])
                self.assertGreaterEqual(len(body["equations"]),3)
                self.assertGreaterEqual(len(body["worked_example"]),110)
    def test_arithmetic(self):
        examples=[
            ("m17-c01-p05",.75*(2/3)*0+.25*2*4,2,"加权目标风险"),
            ("m18-c04-p06",math.sqrt(2)/math.sqrt(2),1,"Δ_A="),
            ("m19-c05-p02",6/20,.30,"0.30"),
            ("m20-c02-p05",1+.1*(2+.5*4-1),1.3,"1.3"),
            ("m21-c04-p06",9-.5*1,8.5,"8.5"),
            ("m22-c01-p05",10+2*math.cos(math.pi),8,"y₆=8"),
            ("m23-c05-p04",100+30-10-25,95,"95手"),
            ("m24-c04-p06",.02*100,2,"2万元"),
            ("m01-c01-p06",.30*4,1.2,"1.20"),
            ("m02-c04-p02",1000000*1.5/100,15000,"15000元"),
            ("m03-c03-p05",2**2*4/(4-2),8,"Var(R)="),
            ("m04-c05-p05",.75*(2/3)*0+.25*2*4,2,"目标风险"),
            ("m05-c02-p05",.5*1-.1*(2+.5*1),.25,"0.25"),
            ("m06-c05-p06",.25*math.log(3),.274653072167,"0.27465"),
            ("m07-c04-p06",4+.25*4**2,8,"条件方差为"),
            ("m08-c02-p04",9*(.25+.75/4),3.9375,"3.9375"),
        ]
        self.assertEqual(len(examples),len(ROWS))
        for lid,value,expected,token in examples:
            with self.subTest(lid=lid):
                self.assertAlmostEqual(value,expected,delta=1e-9)
                self.assertIn(token,ROWS[lid]["worked_example"])
if __name__=="__main__":
    unittest.main()
