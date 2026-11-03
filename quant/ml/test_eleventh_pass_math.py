"""Stage eleven numerical and structural tests of first 12 enhanced lessons."""
import json
import math
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parent
LESSONS={}
for name in [f"atlas-depth-11-{c}.json" for c in "abcd"]:
    batch=json.loads((ROOT/name).read_text(encoding="utf-8"))
    assert batch["schema"]==1 and len(batch["lessons"])==(5 if name.endswith("-d.json") else 6)
    for uid,lesson in batch["lessons"].items():
        assert uid not in LESSONS
        LESSONS[uid]=lesson

class EleventhPassMath(unittest.TestCase):
    def test_structural_and_coverage(self):
        self.assertEqual(len(LESSONS),23)
        self.assertEqual({uid[:3] for uid in LESSONS},{f"m{i:02d}" for i in range(1,24)})
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
            ("m10-c01-p01",.008/.107,8/107,"7.48%",1e-12),
            ("m11-c05-p06",1.1*.9-1,-.01,"−1%",1e-12),
            ("m12-c01-p03",16/22,8/11,"0.7273",0),
            ("m13-c05-p03",.2/(1-.9),2,"偏差校正后分别为2和4",1e-12),
            ("m14-c03-p02",1/(4+4-1),1/7,"0.1429",1e-12),
            ("m15-c05-p02",math.exp(-.2),.8187307530779818,"0.81873",1e-12),
            ("m16-c05-p01",(37*20)**2,547600,"547600",0),
            ("m17-c02-p04",-math.log(.75),.2876820724517809,"0.28768",1e-12),
            ("m18-c04-p03",.5/(.5+.125),.8,"0.8",0),
            ("m19-c03-p02",.8/(.8+.2),.8,"0.8",0),
            ("m20-c03-p02",.75*0+.25*4,1,"变成1",0),
            ("m21-c01-p01",.5/.2,2.5,"2.5",0),
            ("m22-c03-p02",.0001+.1*.0009+.8*.0004,.00051,"0.00051",1e-12),
            ("m23-c05-p02",.5*.02-.5*(-.01)-.001,.014,"1.4%",1e-12),
        ]
        self.assertEqual(len(cases),len(LESSONS))
        for uid,value,expected,token,tolerance in cases:
            with self.subTest(uid=uid):
                self.assertAlmostEqual(value,expected,delta=tolerance)
                self.assertIn(token,LESSONS[uid]["worked_example"])

if __name__=="__main__":
    unittest.main()
