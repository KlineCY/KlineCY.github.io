"""Stage fourteen: audited example arithmetic for six initial lessons."""
import json
import math
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parent
LESSONS=json.loads((ROOT/"atlas-depth-14-a.json").read_text(encoding="utf8"))["lessons"]
class FourteenthPassMath(unittest.TestCase):
    def test_structure(self):
        self.assertEqual(len(LESSONS),6)
        self.assertEqual({uid[:3] for uid in LESSONS},{f"m{i:02d}" for i in range(1,7)})
        for uid,body in LESSONS.items():
            self.assertEqual(len(body["equations"]),3)
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
           ("m06-c01-p06",10*60+5,605,"10:05")
        ]
        for uid,got,expected,token in cases:
            with self.subTest(uid=uid):
                self.assertAlmostEqual(got,expected,delta=1e-10)
                self.assertIn(token,LESSONS[uid]["worked_example"])
if __name__=="__main__":
    unittest.main()
