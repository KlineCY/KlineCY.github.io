"""Independent arithmetic checks for 12 stage-ten advanced lessons."""
import json
import math
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parent
LESSONS = {}
for p in "ab":
    doc = json.loads((ROOT / f"atlas-depth-10-{p}.json").read_text(encoding="utf8"))
    assert doc["schema"] == 1 and len(doc["lessons"]) == 6
    for lid, lesson in doc["lessons"].items():
        assert lid not in LESSONS
        LESSONS[lid] = lesson

class StageTenMath(unittest.TestCase):
    def test_scope_and_structure(self):
        self.assertEqual(len(LESSONS),12)
        self.assertEqual(set(lid[:3] for lid in LESSONS),{f"m{i:02d}" for i in range(1,13)})
        for lid,lesson in LESSONS.items():
            with self.subTest(lid=lid):
                self.assertEqual(len(lesson["equations"]),3)
                self.assertGreaterEqual(len(lesson["derivation"]),3)
                self.assertGreaterEqual(len(lesson["protocol"]),3)
                self.assertGreaterEqual(len(lesson["pitfalls"]),2)
                self.assertGreaterEqual(len(lesson["worked_example"]),60)
                self.assertNotIn("独立稿",lesson["overview"])
    def test_twelve_numeric_examples(self):
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
            ("m12-c01-p02",16/(16+12+2),16/30,"0.5333",0)
        ]
        for lid,v,w,fragment,tol in cases:
            with self.subTest(lid=lid):
                self.assertAlmostEqual(v,w,delta=tol)
                self.assertIn(fragment,LESSONS[lid]["worked_example"])

if __name__ == "__main__":
    unittest.main()
