"""Stage sixteen: numerical and structural checks, first six lessons."""
import json
import math
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parent
LESSONS=json.loads((ROOT/"atlas-depth-16-a.json").read_text(encoding="utf-8"))["lessons"]
class SixteenthPassMath(unittest.TestCase):
    def test_structure(self):
        self.assertEqual(len(LESSONS),6)
        self.assertEqual({id[:3] for id in LESSONS},{f"m{i:02d}" for i in range(1,7)})
        for id,lesson in LESSONS.items():
            with self.subTest(id=id):
                self.assertEqual(len(lesson["equations"]),3)
                self.assertGreaterEqual(len(lesson["derivation"]),3)
                self.assertGreaterEqual(len(lesson["protocol"]),3)
                self.assertGreaterEqual(len(lesson["pitfalls"]),2)
    def test_math(self):
        cases=[
            ("m01-c04-p03",(4+1)/2-(1+1)/2,1.5,"改善1.5"),
            ("m02-c01-p01",11/(5*math.sqrt(5)),.9838699101,"0.9839"),
            ("m03-c01-p01",.2+.5,.7,"0.7"),
            ("m04-c01-p03",.03+.04-.01,.06,"0.06"),
            ("m05-c01-p01",(2-2)**2,0,"值为0"),
            ("m06-c02-p01",(5-2)/1,3,"z=(5−2)/1=3"),
        ]
        for uid,v,expected,fragment in cases:
            with self.subTest(uid=uid):
                self.assertAlmostEqual(v,expected,delta=1e-10)
                self.assertIn(fragment,LESSONS[uid]["worked_example"])
if __name__=="__main__":
    unittest.main()
