"""Independent numerical and structural checks for deepened stage-16 and systemic-risk chapters."""
import json
import math
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent
ML = {}
for suffix in "abcd":
    block = json.loads((ROOT / f"atlas-depth-16-{suffix}.json").read_text(encoding="utf-8"))
    for key, lesson in block["lessons"].items():
        if key in ML:
            raise AssertionError(f"duplicate lesson id: {key}")
        ML[key] = lesson

MATH = {
    row["id"]: row
    for row in json.loads((ROOT.parent / "math" / "lessons-phase-15-a.json").read_text(encoding="utf-8"))["lessons"]
}


class DeepeningStageSixteen(unittest.TestCase):
    def test_23_original_ml_lessons_are_deepened_not_duplicated(self):
        self.assertEqual(len(ML), 23)
        self.assertEqual({k[:3] for k in ML}, {f"m{i:02d}" for i in range(1, 24)})
        for uid, lesson in ML.items():
            with self.subTest(uid=uid):
                self.assertGreaterEqual(len(lesson["equations"]), 6)
                self.assertGreaterEqual(len(lesson["derivation"]), 6)
                self.assertGreaterEqual(len(lesson["protocol"]), 5)
                self.assertGreaterEqual(len(lesson["pitfalls"]), 4)
                self.assertGreaterEqual(len(lesson["worked_example"]), 170)
                labels = [e["label"] for e in lesson["equations"]]
                self.assertEqual(len(labels), len(set(labels)))
                self.assertTrue(all(e.get("tex", "").strip() for e in lesson["equations"]))

    def test_statistical_foundations(self):
        self.assertAlmostEqual((4 - 1 + 1 - 1) / 2, 1.5)
        self.assertIn("t值仅为1", ML["m01-c04-p03"]["worked_example"])
        self.assertAlmostEqual(11 / (5 * math.sqrt(5)), 0.9838699100999075)
        self.assertIn("Pearson相关恰为1", ML["m02-c01-p01"]["worked_example"])
        self.assertAlmostEqual(.7 + .8 - .5, 1)
        self.assertIn("1.0", ML["m03-c01-p01"]["worked_example"])
        self.assertAlmostEqual(.05 / 20, .0025)
        self.assertIn("0.0025", ML["m04-c01-p03"]["worked_example"])
        self.assertAlmostEqual((1 - 2)**2, 1)
        self.assertIn("f=1", ML["m05-c01-p01"]["worked_example"])
        self.assertAlmostEqual(2 / math.sqrt(8 / 3), 1.224744871391589)
        self.assertIn("1.225", ML["m06-c02-p01"]["worked_example"])

    def test_classical_learning(self):
        self.assertAlmostEqual((1 + 3) / 2, 2)
        self.assertIn("最小范数解为(1,1)", ML["m07-c01-p02"]["worked_example"])
        self.assertAlmostEqual((5 - 2) / (3 - 1), 1.5)
        self.assertAlmostEqual(2 + 3 * 1.5, 5 + 1.5)
        self.assertIn("6.5", ML["m08-c01-p05"]["worked_example"])
        self.assertEqual((1 + .5) / (1 - .5), 3)
        self.assertIn("条件数降到2", ML["m09-c04-p03"]["worked_example"])
        self.assertAlmostEqual(4 / 6, 2 / 3)
        self.assertIn("2/3", ML["m10-c01-p02"]["worked_example"])
        self.assertAlmostEqual(.6 * .2 / (.6 * .2 + .4 * .3), .5)
        self.assertAlmostEqual(math.log(.24), -1.4271163556401458)
        self.assertIn("−1.4271", ML["m11-c04-p03"]["worked_example"])
        self.assertEqual(sum([1, 1, 1, 0]) / 4, .75)
        self.assertAlmostEqual(.8**3, .512)
        self.assertIn("0.512", ML["m12-c01-p04"]["worked_example"])

    def test_deep_learning(self):
        self.assertEqual(math.ceil(10 / 4), 3)
        self.assertAlmostEqual((4 + 4 + 2 * 4) / 10, 1.6)
        self.assertIn("1.6", ML["m13-c05-p01"]["worked_example"])
        self.assertEqual(2 * 6 * 6, 72)
        self.assertIn("72", ML["m14-c04-p03"]["worked_example"])
        self.assertAlmostEqual(1.05**2, 1.1025)
        self.assertIn("1.1025", ML["m15-c05-p01"]["worked_example"])
        self.assertAlmostEqual(.75 * 10 + .25 * 6, 9)
        self.assertIn("输出变为9", ML["m16-c01-p01"]["worked_example"])
        self.assertAlmostEqual((-math.log(.2) - math.log(.5)) / 2, 1.151292546497023)
        self.assertIn("1.1513", ML["m17-c02-p01"]["worked_example"])
        self.assertEqual((2 + 6) / 2, 4)
        self.assertIn("(4,4)", ML["m18-c02-p03"]["worked_example"])

    def test_finance_and_control(self):
        self.assertAlmostEqual(-1 - .1, -1.1)
        self.assertIn("−1.1", ML["m19-c02-p03"]["worked_example"])
        self.assertAlmostEqual(2 + .5 * (2 + .5 * 2), 3.5)
        self.assertIn("v3=3.5", ML["m20-c01-p05"]["worked_example"])
        self.assertAlmostEqual(.8 * 2 - .1, 1.5)
        self.assertIn("1.5", ML["m21-c05-p05"]["worked_example"])
        self.assertAlmostEqual(.5 * 1.6 * 1 + .5 * .4 * 3, 1.4)
        ess = (50 * 1.6 + 50 * .4)**2 / (50 * 1.6**2 + 50 * .4**2)
        self.assertAlmostEqual(ess, 73.52941176470588)
        self.assertIn("73.53", ML["m22-c05-p06"]["worked_example"])
        self.assertAlmostEqual(2 * (.61 - .8)**2, .0722)
        self.assertAlmostEqual(.08 - .0722, .0078)
        self.assertIn("0.0722", ML["m23-c03-p06"]["worked_example"])

    def test_mes_and_covar_both_have_complete_proof_paths(self):
        self.assertEqual(set(MATH), {"riskmath-x005", "riskmath-x006"})
        for key, lesson in MATH.items():
            with self.subTest(key=key):
                self.assertEqual(lesson["node_id"], key)
                self.assertGreaterEqual(len(lesson["deep_sections"]), 7)
                self.assertGreaterEqual(len(lesson["formulas"]), 7)
                self.assertGreaterEqual(len(lesson["derivation"]), 6)
                self.assertEqual(len({s["title"] for s in lesson["deep_sections"]}), len(lesson["deep_sections"]))
                self.assertTrue(all(s.get("proof") and s.get("worked") for s in lesson["deep_sections"]))
        samples = [0, 1, 1, 3]
        tail = (sum(sorted(samples)[-2:])) / 2
        self.assertEqual(tail, 2)
        self.assertIn("=2", MATH["riskmath-x005"]["deep_sections"][4]["worked"])
        rho, sig_s, sig_i, x = .5, 2, 1, 1
        self.assertAlmostEqual(rho * sig_s / sig_i * x, 1)
        self.assertIn("ΔCoVaR=1", MATH["riskmath-x006"]["deep_sections"][4]["worked"])


if __name__ == "__main__":
    unittest.main()
