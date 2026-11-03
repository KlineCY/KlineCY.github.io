"""Final 16 early Atlas numerical examples and complete lesson-level case coverage."""
import json
import math
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parent
FILES=("atlas-depth-learning-2.json","atlas-depth-quant-2.json")
ROWS={}
for name in FILES:
    src=json.loads((ROOT/name).read_text(encoding="utf-8"))
    assert src["schema"]==1 and len(src["lessons"])==8
    for lid,chapter in src["lessons"].items():
        assert lid not in ROWS
        ROWS[lid]=chapter

class ThirdWaveEarlyExamples(unittest.TestCase):
    def test_all_sixteen_have_examples(self):
        self.assertEqual(len(ROWS),16)
        for lid,chapter in ROWS.items():
            with self.subTest(lid=lid):
                self.assertTrue(chapter["source_title"])
                self.assertGreaterEqual(len(chapter["equations"]),3)
                self.assertGreaterEqual(len(chapter["worked_example"]),110)
    def test_independent_arithmetic(self):
        cases=[
            ("m09-c01-p03",2/.5,4,"总宽度"),
            ("m10-c04-p05",300/(1+2*sum(.5**k for k in range(1,70))),100,"100"),
            ("m11-c03-p03",9/(9+1),.9,"90%"),
            ("m12-c05-p03",3-(1+1),1,"变动成本为1"),
            ("m13-c04-p05",.25*0+.75*2,1.5,"(1.5,0.5)"),
            ("m14-c02-p05",32+16*3,80,"80"),
            ("m15-c04-p05",256**2,65536,"65536"),
            ("m16-c04-p04",1000**2/(1000*16),62.5,"62.5"),
            ("m17-c05-p04",.04-.01-.01,.02,"2%"),
            ("m18-c04-p05",(2+8)/2,5,"=5"),
            ("m19-c04-p06",100*2,200,"200毫秒"),
            ("m20-c04-p02",.25*.75*(4-1)+.75*(-.25)*(0-1),.75,"=0.75"),
            ("m21-c05-p02",.1*(4**2-3**2),.7,"0.7"),
            ("m22-c01-p04",math.log(121/100),.1906203596086497,"0.19062"),
            ("m23-c02-p01",115/100-1,.15,"15%"),
            ("m24-c04-p05",(1000-100)/100,9,"900%"),
        ]
        self.assertEqual(len(cases),len(ROWS))
        for lid,got,expect,token in cases:
            with self.subTest(lid=lid):
                self.assertAlmostEqual(got,expect,delta=1e-8)
                self.assertIn(token,ROWS[lid]["worked_example"])
if __name__=="__main__":
    unittest.main()
