"""Seventh-stage mathematical audit: 96 chapters, 48 independent calculations.

One check per lesson is not a formal proof, but prevents illustrative
arithmetic from silently drifting away from the published explanations.
"""
import json
import math
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parent
FILES = [f"atlas-depth-07-{letter}.json" for letter in "abcdefghijkl"]
LESSONS = {}
for file in FILES:
    doc = json.loads((ROOT / file).read_text(encoding="utf-8"))
    assert doc["schema"] == 1
    assert len(doc["lessons"]) == 8
    for key, lesson in doc["lessons"].items():
        assert key not in LESSONS, key
        LESSONS[key] = lesson


class SeventhPassMath(unittest.TestCase):
    def test_structure_and_domain_coverage(self):
        self.assertEqual(len(LESSONS), 96)
        domain_counts = {}
        for key, chapter in LESSONS.items():
            domain_counts[key[:3]] = domain_counts.get(key[:3], 0) + 1
            self.assertEqual(len(chapter["equations"]), 3, key)
            self.assertGreaterEqual(len(chapter["worked_example"]), 60, key)
            self.assertGreaterEqual(len(chapter["overview"]), 60, key)
            self.assertEqual(len(chapter["derivation"]), 3, key)
            self.assertEqual(len(chapter["protocol"]), 3, key)
            self.assertEqual(len(chapter["pitfalls"]), 2, key)
            self.assertTrue(chapter["source_title"], key)
        self.assertEqual(domain_counts, {f"m{i:02d}": 4 for i in range(1, 25)})

    def test_recalculate_48_worked_examples(self):
        # ID, independent calculation, expected result, textual evidence, tolerance
        examples = [
            ("m01-c02-p06", 100/(1+2*.3), 62.5, "62.5", .001),
            ("m01-c04-p05", 1-.95**20, .642, "64.2%", .001),
            ("m02-c02-p01", 1*2, 2, "拟合值2", .001),
            ("m02-c02-p06", .01/(.01**2+.01), .9901, "0.9901", .0001),
            ("m03-c01-p04", .7*.6+.3*.2, .48, "0.48", .001),
            ("m03-c03-p06", .2*.4/(.8*.1+.2*.4), .5, "0.5", .001),
            ("m04-c02-p06", .1+.01, .11, "0.11", .001),
            ("m04-c05-p06", .4-.1, .3, "0.30", .001),
            ("m05-c02-p04", .01*20/100, .002, "0.002", .001),
            ("m05-c03-p01", 1/math.sqrt(8), .354, "0.354", .001),
            ("m06-c05-p02", (14-10)/2, 2, "结果为2", .001),
            ("m06-c03-p01", 1000*.05, 50, "50", .001),
            ("m07-c05-p04", .03/.02, 1.5, "Beta=1.5", .001),
            ("m07-c04-p04", 5*1.2, 6, "变为6bp", .001),
            ("m08-c04-p01", .8*(1-.8), .16, "0.16", .001),
            ("m08-c04-p02", 6/(4+2), 1, "叶权为", .001),
            ("m09-c05-p03", 1/math.sqrt(2*math.pi), .399, "0.399", .001),
            ("m09-c05-p06", .009/(.009+.0495), .154, "15.4%", .001),
            ("m10-c05-p03", .9**2*4+1, 4.24, "4.24", .001),
            ("m10-c03-p03", -100-5, -105, "−105", .001),
            ("m11-c03-p04", 1/math.sqrt(.01), 10, "放大10倍", .001),
            ("m11-c02-p03", .2-.1, .1, "0.1", .001),
            ("m12-c04-p04", 1/.1, 10, "权重为1/0.1=10", .001),
            ("m12-c02-p02", 95+5, 100, "第100日", .001),
            ("m13-c04-p03", min(range(1,6),key=lambda i:[.40,.35,.33,.34,.36][i-1]), 3, "最优轮数是3", 0),
            ("m13-c04-p01", 2*(1-.1*.05), 1.99, "1.99", .001),
            ("m14-c04-p06", math.sqrt(.8*.2/20), .089, "0.089", .001),
            ("m14-c02-p04", 64*128, 8192, "8192", 0),
            ("m15-c03-p04", .75*2+.25*6, 3, "新状态为", .001),
            ("m15-c01-p04", 1.1**2*.1, .121, "0.121", .001),
            ("m16-c02-p01", 512/8, 64, "为64", 0),
            ("m16-c01-p02", math.sqrt(64), 8, "√64=8", 0),
            ("m17-c04-p06", 3/4, .75, "Recall@5=3/4=0.75", .001),
            ("m17-c04-p01", len(list(range(0,1000,200))), 5, "5个块", 0),
            ("m18-c04-p02", math.exp(-1), .368, "0.368", .001),
            ("m18-c05-p02", math.sqrt(2*(1-.8)), .632, "0.632", .001),
            ("m19-c03-p05", 10*(1.4-1)**2, 1.6, "1.6", .001),
            ("m19-c03-p03", math.log(2), .693, "0.693", .001),
            ("m20-c02-p06", .8+.2/4, .85, "0.85", .001),
            ("m20-c03-p05", min([7,8]), 7, "用7评价", 0),
            ("m21-c03-p06", 1-88/110, .2, "20%", .001),
            ("m21-c05-p06", 100*2, 200, "200元", 0),
            ("m22-c05-p04", .9*2, 1.8, "1.8", .001),
            ("m22-c03-p04", math.sqrt(.0006), .0245, "2.45%", .0001),
            ("m23-c01-p04", (1+.5+.5)/3, 2/3, "2/3", .001),
            ("m23-c02-p02", (.02+.04+.06)/3/.02, 2, "ICIR=2", .001),
            ("m24-c02-p01", 64/.016, 4000, "4000", 0),
            ("m24-c04-p04", 20/1000, .02, "2%", .001),
        ]
        self.assertEqual(len(examples), 48)
        domain_counts = {}
        for identity, actual, expected, marker, tolerance in examples:
            domain_counts[identity[:3]] = domain_counts.get(identity[:3], 0) + 1
            with self.subTest(lesson=identity):
                self.assertIn(marker, LESSONS[identity]["worked_example"])
                self.assertAlmostEqual(actual, expected, delta=tolerance)
        self.assertEqual(domain_counts, {f"m{i:02d}": 2 for i in range(1,25)})


if __name__ == "__main__":
    unittest.main()
