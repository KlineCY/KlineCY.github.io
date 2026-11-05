"""Independent numerical audit for eighth-pass Atlas supplements.

This checks all 120 existing chapters structurally, including all 120 V38+V39 deepened chapters, and recomputes 72
representative worked examples (three per original ML domain).
"""
import json
import math
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parent
FILES = [f"atlas-depth-08-{letter}.json" for letter in "abcdefghijkl"]
LESSONS = {}
for filename in FILES:
    doc = json.loads((ROOT / filename).read_text(encoding="utf-8"))
    assert doc["schema"] == 1
    assert len(doc["lessons"]) == 10
    for uid, lesson in doc["lessons"].items():
        assert uid not in LESSONS, uid
        LESSONS[uid] = lesson


class EighthWaveMath(unittest.TestCase):
    def test_all_chapters_are_individualized_and_complete(self):
        self.assertEqual(len(LESSONS), 120)
        per_domain = {}
        for uid, lesson in LESSONS.items():
            per_domain[uid[:3]] = per_domain.get(uid[:3], 0) + 1
            with self.subTest(lesson=uid):
                self.assertRegex(uid, r"^m\d\d-c\d\d-p\d\d$")
                self.assertGreaterEqual(len(lesson["source_title"]), 2)
                self.assertGreaterEqual(len(lesson["title"]), 8)
                self.assertGreaterEqual(len(lesson["overview"]), 60)
                self.assertGreaterEqual(len(lesson["worked_example"]), 60)
                domain = int(uid[1:3])
                self.assertEqual(len(lesson["equations"]), 5)
                self.assertEqual(len(lesson["derivation"]), 5)
                self.assertEqual(len(lesson["protocol"]), 5)
                self.assertEqual(len(lesson["pitfalls"]), 3)
                self.assertEqual(len(set(lesson["protocol"])), 5)
                expected_revision = "V38" if domain <= 12 else "V39"
                self.assertTrue(lesson.get("revision_note", "").startswith(expected_revision))
                for item in lesson["equations"]:
                    self.assertTrue(item["label"] and item["tex"])
        self.assertEqual(per_domain, {f"m{i:02d}": 5 for i in range(1,25)})

    def test_numerical_examples_recomputed(self):
        # original lesson id, independently recomputed value, expected, excerpt, tolerance
        checks = [
            ("m01-c03-p04", .14-.06, .08, "额外恶化0.08", 1e-9),
            ("m01-c02-p04", .08+.01, .09, "为0.09", 1e-9),
            ("m01-c05-p06", 4-6, -2, "−2bp", 0),
            ("m02-c05-p02", 9/(9+1), .9, "90%", 1e-9),
            ("m02-c05-p05", 100/105, .9524, "95.24%", .0001),
            ("m02-c03-p06", ((3.01)**2-(2.99)**2)/.02, 6, "梯度为6", 1e-9),
            ("m03-c03-p03", -10*.2*.3, -.6, "−0.6", 1e-9),
            ("m03-c01-p03", .009/(.009+.0495), .154, "15.4%", .001),
            ("m03-c03-p02", .25*4+.25*4+.5*1, 2.5, "2.5", 1e-9),
            ("m04-c05-p01", .3/.1, 3, "权重为3", 1e-9),
            ("m04-c02-p05", math.sqrt(math.log(4000)/2000), .0644, "0.0644", .0001),
            ("m04-c04-p04", (.04+.05+.09)/3, .06, "均值0.06", 1e-9),
            ("m05-c05-p02", -math.log(1+math.exp(-1)), -.3133, "−0.3133", .0001),
            ("m05-c03-p02", .1*2**2, .4, "0.4", 1e-9),
            ("m05-c01-p05", (2-1.5*4)**2, 16, "升至16", 0),
            ("m06-c01-p05", 1.1*.9-1, -.01, "−1%", 1e-9),
            ("m06-c02-p02", (4-(-2))/(2-(-2)), 1.5, "1.5", 1e-9),
            ("m06-c02-p04", (2*.5+3*.2)/(2+3), .32, "0.32", 1e-9),
            ("m07-c03-p03", 2*(.8-1), -.4, "−0.4", 1e-9),
            ("m07-c05-p06", 3-5, -2, "−2bp", 0),
            ("m07-c01-p03", 1/4, .25, "0.25", 1e-9),
            ("m08-c05-p02", .5+.3-.1, .7, "0.7", 1e-9),
            ("m08-c04-p04", (2+3)/2, 2.5, "阈值2.5", 1e-9),
            ("m08-c03-p06", 10/3, 3.33, "3.33", .01),
            ("m09-c01-p05", 120/1000, .12, "120个非零α", 1e-9),
            ("m09-c02-p01", 1-.4, .6, "0.6", 1e-9),
            ("m09-c05-p04", .004/(.004+.0199), .167, "16.7%", .001),
            ("m10-c02-p04", 2/6, 1/3, "2/6", 1e-9),
            ("m10-c05-p04", 1/(.7**2+3*.1**2), 1.923, "1.923", .001),
            ("m10-c05-p01", .8**2*4+1, 3.56, "3.56", 1e-9),
            ("m11-c02-p06", math.sqrt(2**2/4+1), math.sqrt(2), "1.414", .001),
            ("m11-c03-p02", 9/(9+1), .9, "90%", 1e-9),
            ("m11-c04-p06", 200+5*math.log(100), 223.03, "223.03", .01),
            ("m12-c02-p05", len(list(range(111,116))), 5, "111至115", 0),
            ("m12-c04-p05", (16-10)-(11-8), 3, "估计量=(16−10)−(11−8)=3", 0),
            ("m12-c04-p01", .05-.02, .03, "3个百分点", 1e-9),
            ("m13-c01-p05", math.sqrt(2/100), .1414, "0.1414", .0001),
            ("m13-c01-p01", 5*3+3, 18, "18个参数", 0),
            ("m13-c03-p06", .001*250/1000, .00025, "0.00025", 1e-9),
            ("m14-c03-p03", 20/30, .667, "0.667", .001),
            ("m14-c01-p04", 3*3*4*16+16, 592, "592", 0),
            ("m14-c02-p03", 3*9*64**2, 110592, "110592", 0),
            ("m15-c02-p03", .8**10, .107, "0.107", .001),
            ("m15-c01-p06", (1+4)/2, 2.5, "2.5", 0),
            ("m15-c04-p03", 1+2*(1+2+4+8), 31, "31个时间点", 0),
            ("m16-c05-p04", math.sin(math.pi/2), 1, "(1,0)", 1e-9),
            ("m16-c01-p05", .8*2, 1.6, "(0.2,1.6)", 1e-9),
            ("m16-c02-p04", math.cos(math.pi/2), 0, "cos(π/2)=0", 1e-9),
            ("m17-c03-p03", 1/(1+math.exp(-1)), .731, "0.731", .001),
            ("m17-c04-p03", 1/math.sqrt(2), .707, "0.707", .001),
            ("m17-c05-p01", 150*1e6/1e8, 1.5, "1.5亿元", 1e-9),
            ("m18-c05-p05", 1, 1, "相关系数为1", 0),
            ("m18-c03-p04", .25*2+.75*6, 5, "聚合值为5", 0),
            ("m18-c02-p04", .5**6, 1/64, "1/64", 1e-9),
            ("m19-c03-p04", abs(30-0), 30, "距离为30", 0),
            ("m19-c03-p01", -math.log(4), -1.386, "−1.386", .001),
            ("m19-c02-p05", 1+2*(-.5), 0, "样本z=1+2×(−0.5)=0", 0),
            ("m20-c04-p06", 1/(1+math.exp(-1)), .731, "0.731", .001),
            ("m20-c02-p04", 1+.9*2, 2.8, "SARSA目标为2.8", 1e-9),
            ("m20-c05-p01", 2*2, 4, "轨迹权重为2×2=4", 0),
            ("m21-c03-p05", 90+20-5, 105, "q=90+20−5=105", 0),
            ("m21-c02-p03", .3-.5, -.2, "真实净亏0.2%", 1e-9),
            ("m21-c03-p03", .9*2+.1*(-8), 1, "B期望也为1", 1e-9),
            ("m22-c05-p01", .1*1.2**3, .1728, "0.1728", .0001),
            ("m22-c01-p01", 100*1, 100, "第100期方差100", 0),
            ("m22-c03-p01", .0001+.5*.02**2, .0003, "0.0003", 1e-9),
            ("m23-c05-p05", .5*110+.5*80, 95, "继续价值95", 0),
            ("m23-c02-p05", 10**2/((2*.75-1)*100), 2, "误选上界为", 0),
            ("m23-c05-p03", math.log(.5)/math.log(.5), 1, "半衰期为", 1e-9),
            ("m24-c01-p05", 5, 5, "5个交易日后", 0),
            ("m24-c02-p03", 20+30+10+15, 75, "75ms", 0),
            ("m24-c01-p02", .0101-.0100, .0001, "差值0.0001", 1e-9),
        ]
        self.assertEqual(len(checks), 72)
        domains = {}
        for uid, value, expected, marker, tolerance in checks:
            domains[uid[:3]] = domains.get(uid[:3], 0) + 1
            with self.subTest(lesson=uid):
                self.assertIn(uid, LESSONS)
                self.assertIn(marker, LESSONS[uid]["worked_example"])
                self.assertAlmostEqual(value, expected, delta=tolerance)
        self.assertEqual(domains, {f"m{i:02d}": 3 for i in range(1,25)})


if __name__ == "__main__":
    unittest.main()
