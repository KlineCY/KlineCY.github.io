"""Numerical and structural verification of the V36-enhanced sixth wave of ML lessons.

Checks are intentionally small, independently calculable quantities that
should remain in sync with the actual explanatory examples.
"""
import json
import math
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parent
FILES = [
    "atlas-depth-foundations-6.json",
    "atlas-depth-learning-6.json",
    "atlas-depth-quant-6.json",
]
LESSONS = {}
for file in FILES:
    document = json.loads((ROOT / file).read_text(encoding="utf-8"))
    assert document["schema"] == 1
    assert len(document["lessons"]) == 8
    for identity, chapter in document["lessons"].items():
        assert identity not in LESSONS, identity
        LESSONS[identity] = chapter


class SixthPassWorkedExamples(unittest.TestCase):
    def test_unique_substantive_topic_coverage(self):
        self.assertEqual(len(LESSONS), 24)
        self.assertEqual({k[:3] for k in LESSONS}, {f"m{i:02d}" for i in range(1, 25)})
        for identity, lesson in LESSONS.items():
            with self.subTest(id=identity):
                self.assertEqual(len(lesson["equations"]), 5)
                self.assertTrue(lesson.get("revision_note", "").startswith("V36"))
                self.assertGreaterEqual(len(lesson["worked_example"]), 60)
                self.assertEqual(len(lesson["derivation"]), 5)
                self.assertEqual(len(lesson["protocol"]), 5)
                self.assertEqual(len(lesson["pitfalls"]), 3)

    def test_one_arithmetic_case_per_topic(self):
        checks = [
            ("m01-c01-p01", 60-10, 50, "遮挡10日", 0),
            ("m02-c02-p03", (5**2+2**2)/(5**2+2**2+1), 29/30, "96.7%", .000001),
            ("m03-c05-p02", (1+8)/(1+1+10), .75, "0.75", .000001),
            ("m04-c04-p05", 1/100, .01, "0.01", .000001),
            ("m05-c01-p06", min(2,-2), -2, "diag(2,−2)", 0),
            ("m06-c03-p03", 2*1+2*0, 2*0+2*1, "完全相同的因子", 0),
            ("m07-c03-p06", .99-.98, .01, "0.99、0.98", .000001),
            ("m08-c02-p06", 1/(.2+.8/8), 3.33, "3.33", .01),
            ("m09-c03-p06", math.exp(-2), .135, "0.135", .001),
            ("m10-c02-p05", .2*.3, .06, "0.06", .000001),
            ("m11-c01-p04", (5-2)/5, .6, "0.6", .000001),
            ("m12-c02-p03", 98+5, 103, "103日", 0),
            ("m13-c04-p06", 1*2, 10*.2, "2x", .000001),
            ("m14-c02-p01", 5*5*6+6, 156, "=156", 0),
            ("m15-c03-p06", 20*(10+20+1), 620, "=620", 0),
            ("m16-c04-p01", 2048**2/1024**2, 4, "4倍", 0),
            ("m17-c04-p02", .6/math.hypot(.6,.8), .6, "0.6", .000001),
            ("m18-c05-p01", 3*(3-1), 6, "6条有向连接", 0),
            ("m19-c05-p05", (-.1+0+.1)/3, (-.05-.05+.1)/3, "样本均值均为0", .000001),
            ("m20-c05-p03", 1+.9*.8*2, 2.44, "2.44", .000001),
            ("m21-c04-p02", 3+3, 6, "总收益6", 0),
            ("m22-c04-p01", .8**2, .64, "0.64", .000001),
            ("m23-c01-p02", 100*(1+.05), 105, "上障碍105", 0),
            ("m24-c04-p02", .7-.4, .3, "优势是0.7−0.4=0.3", .000001),
        ]
        self.assertEqual(len(checks), 24)
        for identity, calculated, expected, phrase, tolerance in checks:
            with self.subTest(id=identity):
                self.assertIn(phrase, LESSONS[identity]["worked_example"])
                self.assertAlmostEqual(calculated, expected, delta=tolerance)


if __name__ == "__main__":
    unittest.main()
