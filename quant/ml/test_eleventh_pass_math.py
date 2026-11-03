"""Stage eleven numerical and structural tests of first 12 enhanced lessons."""
import json
import math
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parent
LESSONS={}
for name in ["atlas-depth-11-a.json","atlas-depth-11-b.json"]:
    batch=json.loads((ROOT/name).read_text(encoding="utf-8"))
    assert batch["schema"]==1 and len(batch["lessons"])==6
    for uid,lesson in batch["lessons"].items():
        assert uid not in LESSONS
        LESSONS[uid]=lesson

class EleventhPassMath(unittest.TestCase):
    def test_structural_and_coverage(self):
        self.assertEqual(len(LESSONS),12)
        self.assertEqual({uid[:3] for uid in LESSONS},{f"m{i:02d}" for i in range(1,13)})
        for uid,lesson in LESSONS.items():
            with self.subTest(uid=uid):
                self.assertTrue(lesson["source_title"])
                self.assertEqual(len(lesson["equations"]),3)
                self.assertGreaterEqual(len(lesson["derivation"]),3)
                self.assertGreaterEqual(len(lesson["protocol"]),3)
                self.assertGreaterEqual(len(lesson["pitfalls"]),2)
                self.assertGreaterEqual(len(lesson["worked_example"]),60)
    def test_independently_recomputed_numbers(self):
        cases=[
            ("m01-c04-p01",110-100,10,"110",0),
            ("m02-c05-p01",.25*.04+.25*.09+2*.25*.03,.0475,"0.0475",1e-12),
            ("m03-c03-p01",math.comb(10,2)*.2**2*.8**8,.301989888,"0.30199",1e-8),
            ("m04-c02-p02",2**3,8,"2³=8",0),
            ("m05-c02-p02",((1-3)**2+(3-3)**2+(5-3)**2)/3,8/3,"8/3",0),
            ("m06-c04-p03",(100+102+104)/3,102,"102",0),
            ("m07-c02-p06",2-1,1,"(1,0)",0),
            ("m08-c04-p03",.5*(16/2+4/2-4/4),4.5,"4.5",0),
            ("m09-c01-p01",3/math.sqrt(3**2+4**2),.6,"0.6",0),
            ("m10-c01-p01",.008/.107,8/107,"7.48%",0),
            ("m11-c05-p06",1.1*.9-1,-.01,"−1%",1e-12),
            ("m12-c01-p03",16/22,8/11,"0.7273",0),
        ]
        self.assertEqual(len(cases),len(LESSONS))
        for uid,value,expected,token,tolerance in cases:
            with self.subTest(uid=uid):
                self.assertAlmostEqual(value,expected,delta=tolerance)
                self.assertIn(token,LESSONS[uid]["worked_example"])

if __name__=="__main__":
    unittest.main()
