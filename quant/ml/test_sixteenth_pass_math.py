"""Stage sixteen: numerical and structural checks, first six lessons."""
import json
import math
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parent
LESSONS={}
for suffix in "abcd":
    data=json.loads((ROOT/f"atlas-depth-16-{suffix}.json").read_text(encoding="utf-8"))["lessons"]
    for uid,entry in data.items():
        assert uid not in LESSONS
        LESSONS[uid]=entry
class SixteenthPassMath(unittest.TestCase):
    def test_structure(self):
        self.assertEqual(len(LESSONS),23)
        self.assertEqual({id[:3] for id in LESSONS},{f"m{i:02d}" for i in range(1,24)})
        for id,lesson in LESSONS.items():
            with self.subTest(id=id):
                self.assertGreaterEqual(len(lesson["equations"]),3)
                self.assertEqual(len({e["label"] for e in lesson["equations"]}),len(lesson["equations"]))
                self.assertTrue(all(e.get("tex","").strip() for e in lesson["equations"]))
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
            ("m07-c01-p02",(1+3)/2,2,"β=(1+3)/2=2"),
            ("m08-c01-p05",5+1*2,7,"总代价为5+1×2=7"),
            ("m09-c04-p03",1+.5,1.5,"特征值分别为1.5和0.5"),
            ("m10-c01-p02",(4-1)/(4+2-2),.75,"MAP=(4−1)/(4+2−2)=3/4=0.75"),
            ("m11-c04-p03",.12/(.12+.12),.5,"责任度为0.12/(0.12+0.12)=0.5"),
            ("m12-c01-p04",3/4,.75,"ROC-AUC=3/4=0.75"),
            ("m13-c05-p01",math.ceil(10/4),3,"3个批次"),
            ("m14-c04-p03",2*3*3,18,"18个打分"),
            ("m15-c05-p01",.8*(.2*5),.8,"h2=0.8"),
            ("m16-c01-p01",.75*2+.25*6,3,"注意力输出为0.75×2+0.25×6=3"),
            ("m17-c02-p01",-math.log(.2),1.6094379124,"1.6094"),
            ("m18-c02-p03",(2+6)/2,4,"输出都为(2+6)/2=4"),
            ("m19-c02-p03",-1-.3,-1.3,"ELBO=−1−0.3=−1.3"),
            ("m20-c01-p05",2/(1-.5),4,"V*=2/(1−0.5)=4"),
            ("m21-c05-p05",.8*2,1.6,"0.8×2=1.6"),
            ("m22-c05-p06",.8*1+.2*3,1.4,"0.8×1+0.2×3=1.4"),
            ("m23-c03-p06",(.6-.8)**2+(.4-.2)**2,.08,"目标值是0.2²+0.2²=0.08"),
        ]
        for uid,v,expected,fragment in cases:
            with self.subTest(uid=uid):
                self.assertAlmostEqual(v,expected,delta=1e-10)
                self.assertIn(fragment,LESSONS[uid]["worked_example"])
if __name__=="__main__":
    unittest.main()
