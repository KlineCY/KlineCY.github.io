"""Independently check 24 representative fifth-pass numerical illustrations.

All 48 canonical lessons are checked after V35 deepening without removing their original illustrations.
These 24 computations provide an additional one-per-domain arithmetic gate.
"""
import json
import math
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parent
FILES = [
    "atlas-depth-foundations-5.json",
    "atlas-depth-classical-5.json",
    "atlas-depth-models-5.json",
    "atlas-depth-deep-5.json",
    "atlas-depth-crossdomain-5.json",
    "atlas-depth-quant-5.json",
]
LESSONS = {}
for filename in FILES:
    document = json.loads((ROOT / filename).read_text(encoding="utf-8"))
    assert document["schema"] == 1
    assert len(document["lessons"]) == 8
    for identifier, lesson in document["lessons"].items():
        assert identifier not in LESSONS, identifier
        LESSONS[identifier] = lesson


class FifthPassWorkedExamples(unittest.TestCase):
    def test_all_48_examples_are_substantive_and_unique(self):
        self.assertEqual(len(LESSONS), 48)
        distribution = {}
        for identifier, chapter in LESSONS.items():
            distribution[identifier[:3]] = distribution.get(identifier[:3], 0) + 1
            self.assertEqual(len(chapter["equations"]), 5, identifier)
            self.assertTrue(chapter.get("revision_note", "").startswith("V35"), identifier)
            self.assertGreaterEqual(len(chapter["worked_example"]), 120, identifier)
            self.assertEqual(len(chapter["derivation"]), 5, identifier)
            self.assertEqual(len(chapter["protocol"]), 5, identifier)
            self.assertEqual(len(chapter["pitfalls"]), 3, identifier)
        self.assertEqual(distribution, {f"m{i:02d}": 2 for i in range(1, 25)})

    def test_one_recalculated_example_per_topic(self):
        # id, independent calculation, numeric expected, verbatim answer marker, tolerance
        checks = [
            ("m01-c01-p03", .7*1-.3*3-.1, -.3, "−0.3", .0001),
            ("m02-c02-p02", 2*1, 2, "diag(2,1)", .0001),
            ("m03-c04-p01", -.9*math.log2(.9)-.1*math.log2(.1), .469, "0.469", .001),
            ("m04-c04-p06", math.sqrt(math.log(100)/10000), .0215, "0.0215", .0001),
            ("m05-c03-p05", (1-.99)*3**2, .09, "v₁=0.09", .0001),
            ("m06-c03-p04", max(.6-.2,0), .4, "0.4", .0001),
            ("m07-c04-p05", .99*(1-.99), .0099, "0.0099", .0001),
            ("m08-c02-p01", math.exp(-1)*100, 36.8, "36.8%", .05),
            ("m09-c03-p03", 1+.5, 1.5, "1.5和0.5", .0001),
            ("m10-c01-p06", (20+8)/(20+20+10), .56, "0.56", .0001),
            ("m11-c01-p05", (0-5)**2*2+(10-5)**2*2, 100, "WCSS=100", .0001),
            ("m12-c01-p05", math.sqrt((1+1+100)/3), 5.83, "5.83", .01),
            ("m13-c03-p05", 1.05**20, 2.653, "2.653", .001),
            ("m14-c01-p03", math.floor((10-3)/2)+1, 4, "输出长度", .0001),
            ("m15-c02-p06", math.hypot(1.2,1.6), 2, "(1.2,1.6)", .0001),
            ("m16-c01-p04", .25*2+.75*6, 5, "=5", .0001),
            ("m17-c05-p06", .1+.25+.05, .4, "0.40%", .0001),
            ("m18-c01-p04", 1+1, 2, "0与2", .0001),
            ("m19-c02-p06", math.log(1), 0, "KL项为0", .0001),
            ("m20-c04-p04", (3+1)/2, 2, "V=2", .0001),
            ("m21-c03-p04", (0+1+2+10)/4, 3.25, "3.25", .0001),
            ("m22-c05-p02", 3+1, 4, "(1.5,1.5,4)", .0001),
            ("m23-c01-p05", (1+.5+.5)/3, 2/3, "2/3", .0001),
            ("m24-c01-p06", 1.1*.9, .99, "0.99", .0001),
        ]
        self.assertEqual(len(checks), 24)
        self.assertEqual({key[:3] for key, *_ in checks}, {f"m{i:02d}" for i in range(1,25)})
        for identifier, actual, expected, answer, tolerance in checks:
            with self.subTest(lesson=identifier):
                self.assertIn(answer, LESSONS[identifier]["worked_example"])
                self.assertAlmostEqual(actual, expected, delta=tolerance)


if __name__ == "__main__":
    unittest.main()
