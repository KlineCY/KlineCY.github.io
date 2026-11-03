"""Cross-chapter audit: 16 foundational and architecture numerical invariants."""
import json, math, unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parent
ROWS={}
for file in ("atlas-depth-foundations-3.json","atlas-depth-learning-3.json"):
    src=json.loads((ROOT/file).read_text(encoding="utf-8"))
    assert len(src["lessons"])==8
    ROWS.update(src["lessons"])
class CrossChapterConsistency(unittest.TestCase):
    def test_complete(self):
        self.assertEqual(len(ROWS),16)
        for lid,chapter in ROWS.items():
            with self.subTest(lid=lid):
                self.assertEqual(len(chapter["equations"]),3)
                self.assertGreaterEqual(len(chapter["worked_example"]),160)
                self.assertTrue(chapter["source_title"])
    def test_numerics(self):
        cases=[
          ("m01-c01-p04",(2*2+.1*2**2)-2*2,.4,"放大100倍"),
          ("m02-c04-p05",(100*2+20*8)/120,3,"总体均值梯度"),
          ("m03-c05-p05",(10*.9**2+90*.1**2)/100,.09,"ROC-AUC=0.5"),
          ("m04-c05-p02",(.62-.3)/(.8-.3),.64,"q=0.64"),
          ("m05-c03-p04",2/math.sqrt(.4),math.sqrt(10),"3.162"),
          ("m06-c02-p06",(1+4*.2)/(1+4),.36,"0.36"),
          ("m07-c03-p04",3/(3+1),.75,"0.75"),
          ("m08-c05-p03",2,2,"深度2"),
          ("m09-c05-p05",math.exp(-1),.36787944117144233,"γ′"),
          ("m10-c03-p06",.9*math.log(1.8)+.1*math.log(.2),.3680642071684971,"0.3681"),
          ("m11-c02-p04",1/max(2,3,1),1/3,"0.3333"),
          ("m12-c05-p04",.5*.5**2,.125,"0.125"),
          ("m13-c01-p04",128*6+129,897,"897"),
          ("m14-c01-p06",5+(3-1)*2,9,"r₃=9"),
          ("m15-c03-p03",.99**10,.9043820750088044,"0.9044"),
          ("m16-c04-p06",4096*128-(128*127//2),516160,"516,160")
        ]
        self.assertEqual(len(cases),len(ROWS))
        for lid,actual,expect,token in cases:
            with self.subTest(lid=lid):
                self.assertAlmostEqual(actual,expect,delta=1e-10)
                self.assertIn(token,ROWS[lid]["worked_example"])
if __name__=="__main__":
    unittest.main()
