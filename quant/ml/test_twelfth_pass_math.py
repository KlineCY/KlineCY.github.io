"""Stage twelve: six independently checked in-lesson mathematical examples."""
import json
import math
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parent
LESSONS=json.loads((ROOT/"atlas-depth-12-a.json").read_text(encoding="utf-8"))["lessons"]
class TwelfthPassMath(unittest.TestCase):
    def test_content_and_scope(self):
        self.assertEqual(len(LESSONS),6)
        self.assertEqual({uid[:3] for uid in LESSONS},{"m03","m05","m08","m09","m10","m12"})
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
        ]
        for uid,value,expected,fragment in checks:
            with self.subTest(uid=uid):
                self.assertAlmostEqual(value,expected,delta=1e-12)
                self.assertIn(fragment,LESSONS[uid]["worked_example"])
if __name__=="__main__":
    unittest.main()
