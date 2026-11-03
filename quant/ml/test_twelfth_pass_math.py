"""Stage twelve: six independently checked in-lesson mathematical examples."""
import json
import math
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parent
LESSONS={}
for filename in (f"atlas-depth-12-{c}.json" for c in "abcd"):
    batch=json.loads((ROOT/filename).read_text(encoding="utf-8"))["lessons"]
    for uid,lesson in batch.items():
        assert uid not in LESSONS, uid
        LESSONS[uid]=lesson
class TwelfthPassMath(unittest.TestCase):
    def test_content_and_scope(self):
        self.assertEqual(len(LESSONS),23)
        self.assertEqual({uid[:3] for uid in LESSONS},{"m01","m02","m03","m04","m05","m06","m07","m08","m09","m10","m11","m12","m13","m14","m15","m16","m17","m18","m19","m20","m21","m22","m23"})
        for uid,chapter in LESSONS.items():
            with self.subTest(uid=uid):
                self.assertEqual(len(chapter["equations"]),3)
                self.assertGreaterEqual(len(chapter["derivation"]),3)
                self.assertGreaterEqual(len(chapter["protocol"]),3)
                self.assertGreaterEqual(len(chapter["pitfalls"]),2)
    def test_numerical_cases(self):
        checks=[
           ("m03-c05-p06",2+((1-2)**2+(3-2)**2)/2,3,"总预测方差为3"),
           ("m05-c04-p06",(2+0)/(1+1),1,"第一次x更新"),
           ("m08-c01-p04",17-1,16,"下降量是16"),
           ("m09-c03-p04",math.exp(-.04*25),math.exp(-1),"0.3679"),
           ("m10-c02-p03",1/3,1/3,"1/3"),
           ("m12-c03-p05",1-.95**5,1-.95**5,"22.62%"),
           ("m01-c03-p02",2**3,8,"2³=8"),
           ("m02-c01-p05",2*3*4,24,"24次"),
           ("m04-c04-p03",(1+2)/2,1.5,"1.5"),
           ("m06-c03-p02",2**3-1,7,"7个"),
           ("m07-c02-p03",.8-.3,.5,"0.5"),
           ("m11-c05-p01",3/(3+1),.75,"0.75"),
           ("m13-c05-p04",math.ceil(100/32),4,"4个批次"),
           ("m14-c01-p05",math.floor((32+2-3)/2)+1,16,"16384"),
           ("m15-c05-p05",.8*.2,.16,"0.16"),
           ("m16-c02-p02",3*3,9,"9个"),
           ("m17-c03-p02",3/(3+1),.75,"0.75"),
           ("m18-c02-p01",(2+6)/2,4,"均值仍为4"),
           ("m19-c02-p01",2+.5*(-1),1.5,"1.5"),
           ("m20-c01-p01",2/(1-.5),4,"V=4"),
           ("m21-c03-p02",7,7,"CVaR_0.75=7"),
           ("m22-c02-p04",1.4+.8*(1.8-1.4),1.72,"1.72"),
           ("m23-c04-p06",1e6*.05,50000,"5万股"),
        ]
        for uid,value,expected,fragment in checks:
            with self.subTest(uid=uid):
                self.assertAlmostEqual(value,expected,delta=1e-12)
                self.assertIn(fragment,LESSONS[uid]["worked_example"])
if __name__=="__main__":
    unittest.main()
