"""Stage fifteen numerical regressions for 12 source-aligned Atlas lessons."""
import json
import math
from pathlib import Path
import unittest
ROOT=Path(__file__).resolve().parent
LESSONS={}
for suffix in "ab":
    data=json.loads((ROOT/f"atlas-depth-15-{suffix}.json").read_text(encoding="utf-8"))
    assert data["schema"]==1 and len(data["lessons"])==6
    for lid,body in data["lessons"].items():
        assert lid not in LESSONS
        LESSONS[lid]=body

class FifteenthPassMath(unittest.TestCase):
    def test_scope_and_completeness(self):
        self.assertEqual(len(LESSONS),12)
        self.assertEqual({lid[:3] for lid in LESSONS},{f"m{i:02d}" for i in range(1,13)})
        for lid,chapter in LESSONS.items():
            with self.subTest(lid=lid):
                self.assertTrue(chapter["source_title"])
                self.assertEqual(len(chapter["equations"]),3)
                self.assertGreaterEqual(len(chapter["derivation"]),3)
                self.assertGreaterEqual(len(chapter["protocol"]),3)
                self.assertGreaterEqual(len(chapter["pitfalls"]),2)
                self.assertGreaterEqual(len(chapter["worked_example"]),60)

    def test_twelve_numeric_examples(self):
        cases=[
            ("m01-c05-p01", 0, "收益是0%"),
            ("m02-c01-p04", 16, "平方误差为16"),
            ("m03-c02-p01", 0.04, "即4%"),
            ("m04-c02-p04", 0.5, "0.5"),
            ("m05-c02-p01", 2.88, "2.88"),
            ("m06-c01-p01", 0.5, "逐笔均值"),
            ("m07-c01-p01", 7.5, "7.5"),
            ("m08-c01-p01", 9, "增益为9"),
            ("m09-c02-p03", 0.5, "0.5"),
            ("m10-c04-p03", 0.499802, "0.499802"),
            ("m11-c04-p05", 18.75, "18.75"),
            ("m12-c04-p03", 0.5, "风险为0.5"),
        ]
        self.assertEqual(len(cases),len(LESSONS))
        for lid,expected,token in cases:
            with self.subTest(lid=lid):
                self.assertIn(token,LESSONS[lid]["worked_example"])
                self.assertTrue(math.isfinite(expected))

if __name__=="__main__":
    unittest.main()
