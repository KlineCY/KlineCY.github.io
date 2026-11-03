"""Cross-chapter audit: 8 quantitative, inference and online-deployment invariants."""
import json
import math
import unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parent
ROWS=json.loads((ROOT/"atlas-depth-quant-3.json").read_text(encoding="utf-8"))["lessons"]
class QuantCrossChapterConsistency(unittest.TestCase):
    def test_complete(self):
        self.assertEqual(len(ROWS),8)
        for lid,body in ROWS.items():
            with self.subTest(lid=lid):
                self.assertTrue(body["source_title"])
                self.assertGreaterEqual(len(body["worked_example"]),160)
    def test_numbers(self):
        cases=[
            ("m17-c04-p05",1/5,.2,"Precision@5=1/5=0.2"),
            ("m18-c05-p03",40/800,.05,"5%"),
            ("m19-c01-p06",math.sqrt(2),1.41421356237,"√2≈1.414"),
            ("m20-c03-p04",.8*(.9*2-1),.64,"0.64"),
            ("m21-c05-p03",100-2*.1-.4/2,99.6,"99.6"),
            ("m22-c03-p05",.5+.2*2+.3*2+.1*(27/22),1.6227272727,"1.6227"),
            ("m23-c02-p03",-0.5+0+2,1.5,"1.5"),
            ("m24-c02-p06",(.9*8+.1*2)-(.1*8+.9*2),4.8,"4.8bp"),
        ]
        for lid,value,expected,token in cases:
            with self.subTest(lid=lid):
                self.assertAlmostEqual(value,expected,delta=1e-8)
                self.assertIn(token,ROWS[lid]["worked_example"])
    def test_multivariate_normal_radial_mode(self):
        for d in (2,3,10,1000):
            mode=math.sqrt(d-1)
            rms=math.sqrt(d)
            self.assertLess(mode,rms)
        self.assertAlmostEqual(math.sqrt(2-1),1)
        self.assertIn("众数是r=1",ROWS["m19-c01-p06"]["worked_example"])
if __name__=="__main__":
    unittest.main()
