"""Stage thirteen: 12 exact-source advanced lessons and independent numerical checks."""
import json
import math
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parent
LESSONS={}
for suffix in "ab":
    data=json.loads((ROOT/f"atlas-depth-13-{suffix}.json").read_text(encoding="utf-8"))
    assert data["schema"]==1 and len(data["lessons"])==6
    for lid,body in data["lessons"].items():
        assert lid not in LESSONS
        LESSONS[lid]=body
class ThirteenthPassMath(unittest.TestCase):
    def test_coverage(self):
        self.assertEqual(len(LESSONS),12)
        self.assertEqual({lid[:3] for lid in LESSONS},{f"m{i:02d}" for i in range(1,13)})
        for lid,body in LESSONS.items():
            with self.subTest(lid=lid):
                self.assertTrue(body["source_title"])
                self.assertGreaterEqual(len(body["equations"]),3)
                self.assertGreaterEqual(len(body["derivation"]),3)
                self.assertGreaterEqual(len(body["protocol"]),3)
                self.assertGreaterEqual(len(body["pitfalls"]),2)
    def test_independent_arithmetic(self):
        cases=[
            ("m01-c03-p03",0**2+1+4,5,"误差为0+1+4=5"),
            ("m02-c03-p05",2*2*3,12,"结果只有6"),
            ("m03-c02-p02",((1-3)**2+(5-3)**2)/2,4,"MSE为"),
            ("m04-c05-p04",.25*2*4,2,"目标期望"),
            ("m05-c05-p03",math.sqrt(6**2+8**2),10,"范数为10"),
            ("m06-c01-p02",(0+0+10+10)/4,5,"真实均值为5"),
            ("m07-c01-p04",1/2,.5,"σ²/2"),
            ("m08-c01-p06",9*(.25+.75/4),3.9375,"3.9375"),
            ("m09-c03-p02",2-1,1,"α必须为0"),
            ("m10-c04-p04",math.sqrt(.75+8/4),math.sqrt(2.75),"1.6583"),
            ("m11-c05-p04",3/(3+1+1),.6,"0.5108"),
            ("m12-c03-p06",.3*10/math.sqrt(1.125),math.sqrt(8),"2.8284"),
        ]
        for lid,got,expected,token in cases:
            with self.subTest(lid=lid):
                self.assertAlmostEqual(got,expected,delta=1e-10)
                self.assertIn(token,LESSONS[lid]["worked_example"])
if __name__=="__main__":
    unittest.main()
