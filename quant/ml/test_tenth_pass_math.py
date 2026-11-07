"""Independent arithmetic checks for 23 stage-ten advanced lessons."""
import json
import math
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parent
LESSONS = {}
for p in "abcd":
    doc = json.loads((ROOT / f"atlas-depth-10-{p}.json").read_text(encoding="utf8"))
    assert doc["schema"] == 1 and len(doc["lessons"]) == (5 if p == "d" else 6)
    for lid, lesson in doc["lessons"].items():
        assert lid not in LESSONS
        LESSONS[lid] = lesson

class StageTenMath(unittest.TestCase):
    def test_scope_and_structure(self):
        self.assertEqual(len(LESSONS),23)
        self.assertEqual(set(lid[:3] for lid in LESSONS),{f"m{i:02d}" for i in range(1,24)})
        for lid,lesson in LESSONS.items():
            with self.subTest(lid=lid):
                self.assertGreaterEqual(len(lesson["equations"]),3)
                self.assertGreaterEqual(len(lesson["derivation"]),3)
                self.assertGreaterEqual(len(lesson["protocol"]),3)
                self.assertGreaterEqual(len(lesson["pitfalls"]),2)
                self.assertGreaterEqual(len(lesson["worked_example"]),60)
                self.assertNotIn("独立稿",lesson["overview"])
    def test_twenty_three_numeric_examples(self):
        cases=[
            ("m01-c05-p03",98+5,103,"日103",0),
            ("m02-c01-p06",2*1,2,"(2,0)",0),
            ("m03-c01-p06",math.sqrt(.25),.5,"0.5",0),
            ("m04-c03-p03",math.ceil(math.log(4000)/.02),415,"415",0),
            ("m05-c04-p03",2-2**2/4,1,"零对偶间隙",0),
            ("m06-c02-p05",2/math.sqrt(2/3),2.4494897428,"2.449",1e-9),
            ("m07-c02-p05",1+1,2,"平方和为2",0),
            ("m08-c03-p01",2-.5,1.5,"1.5",0),
            ("m09-c02-p04",2*.5,1,"2×0.5=1",0),
            ("m10-c05-p02",.8*2,1.6,"1.6",0),
            ("m11-c03-p01",9/(9+1),.9,"90%",0),
            ("m12-c01-p02",16/(16+12+2),16/30,"0.5333",0),
            ("m13-c03-p01",2/100,.02,"0.02",0),
            ("m14-c05-p01",(105-90)/(110-90),.75,"0.75",0),
            ("m15-c05-p06",15+20+5+10,50,"50ms",0),
            ("m16-c04-p02",(2+3*6)/4,5,"=5",0),
            ("m17-c01-p06",80,80,"80秒",0),
            ("m18-c03-p03",.25*2+.75*6,5,"=5",0),
            ("m19-c02-p02",.5*(1+1-1-math.log(1)),.5,"0.5",0),
            ("m20-c04-p01",.8*.5,.4,"0.4",0),
            ("m21-c05-p01",50**2+50**2,5000,"5000",0),
            ("m22-c02-p01",4*(1+.5**2),5,"=5",0),
            ("m23-c03-p01",(.5**2)*(.1**2+.2**2),.0125,"0.0125",1e-12)
        ]
        for lid,v,w,fragment,tol in cases:
            with self.subTest(lid=lid):
                self.assertAlmostEqual(v,w,delta=tol)
                self.assertIn(fragment,LESSONS[lid]["worked_example"])

if __name__ == "__main__":
    unittest.main()
