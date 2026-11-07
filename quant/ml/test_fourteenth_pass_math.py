"""Stage fourteen: preserve original numerical tests while allowing deeper equations."""
import json
import math
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parent
LESSONS={}
for suffix in "abcd":
    data=json.loads((ROOT/f"atlas-depth-14-{suffix}.json").read_text(encoding="utf8"))["lessons"]
    for uid,body in data.items():
        assert uid not in LESSONS
        LESSONS[uid]=body
class FourteenthPassMath(unittest.TestCase):
    def test_structure(self):
        self.assertEqual(len(LESSONS),23)
        self.assertEqual({uid[:3] for uid in LESSONS},{f"m{i:02d}" for i in range(1,24)})
        for uid,body in LESSONS.items():
            self.assertGreaterEqual(len(body["equations"]),3)
            self.assertGreaterEqual(len(body["derivation"]),3)
            self.assertGreaterEqual(len(body["protocol"]),3)
            self.assertGreaterEqual(len(body["pitfalls"]),2)
    def test_numeric(self):
        cases=[
           ("m01-c02-p03",8/(4+1),1.6,"1.6"),
           ("m02-c01-p02",math.sqrt(3**2+4**2),5,"=5"),
           ("m03-c01-p02",.16/.24,2/3,"2/3"),
           ("m04-c01-p01",2*math.exp(-2*200*.1**2),2*math.exp(-4),"0.03663"),
           ("m05-c05-p05",1e-8*1024,1.024e-5,"1.024×10^-5"),
           ("m06-c01-p06",10*60+5,605,"10:05"),
           ("m07-c02-p02",9/(9+1),.9,"0.9"),
           ("m08-c03-p02",.5*(.5**2+.5**2),.25,"0.25"),
           ("m09-c01-p04",2/.5,4,"2/0.5=4"),
           ("m10-c01-p05",5/8,.625,"0.625"),
           ("m11-c04-p01",1/(1+math.exp(-2)),.8807970779778823,"0.8808"),
           ("m12-c03-p02",12-3+1,10,"10个"),
           ("m13-c02-p03",1*1+2*1,3,"权重梯度"),
           ("m14-c03-p01",-math.log(.5),math.log(2),"0.6931"),
           ("m15-c05-p04",.25*2+1,1.5,"h2=1.5"),
           ("m16-c04-p03",1+2+3+4,10,"共4次"),
           ("m17-c03-p04",-math.log(2/3),.4054651081081644,"0.4055"),
           ("m18-c01-p03",(1-3)**2,4,"能量"),
           ("m19-c04-p03",.8*2+.6,2.2,"2.2"),
           ("m20-c01-p06",.8*.5/(.8*.5+.2*.5),.8,"后验概率"),
           ("m21-c04-p03",2**5,32,"32"),
           ("m22-c04-p06",1-1.2,-.2,"−0.2"),
           ("m23-c03-p02",.03/(2*.01),1.5,"1.5")
        ]
        for uid,got,expected,token in cases:
            with self.subTest(uid=uid):
                self.assertAlmostEqual(got,expected,delta=1e-10)
                self.assertIn(token,LESSONS[uid]["worked_example"])
if __name__=="__main__":
    unittest.main()
