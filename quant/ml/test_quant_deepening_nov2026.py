"""Independent numerical and structural regression checks for expanded ML/math lessons."""
import json
import math
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parent
L17 = json.loads((ROOT / "atlas-depth-17-c.json").read_text(encoding="utf-8"))["lessons"]
L18 = json.loads((ROOT / "atlas-depth-18-a.json").read_text(encoding="utf-8"))["lessons"]
JSD = json.loads((ROOT.parent / "math" / "lessons-phase-16-a.json").read_text(encoding="utf-8"))["lessons"][0]


class DeepeningRegression(unittest.TestCase):
    def test_nine_lessons_keep_auditable_mathematics(self):
        for lid, lesson in {**L17, **L18}.items():
            with self.subTest(lid=lid):
                self.assertGreaterEqual(len(lesson["equations"]), 5)
                self.assertGreaterEqual(len(lesson["derivation"]), 6)
                self.assertGreaterEqual(len(lesson["protocol"]), 5)
                self.assertGreaterEqual(len(lesson["pitfalls"]), 3)
                self.assertEqual(
                    len({e["label"] for e in lesson["equations"]}),
                    len(lesson["equations"]),
                )
                self.assertTrue(all(e["tex"].strip() for e in lesson["equations"]))

    def test_multistep_ar_three_step(self):
        self.assertAlmostEqual(0.8**3 * 10, 5.12)
        self.assertAlmostEqual(sum(0.8**(2 * j) for j in range(3)), 2.0496)
        self.assertIn("2.0496", L17["m15-c01-p05"]["worked_example"])

    def test_causal_softmax(self):
        weights = [math.exp(0), math.exp(math.log(2))]
        normalized = [x / sum(weights) for x in weights]
        self.assertAlmostEqual(normalized[0], 1 / 3)
        self.assertAlmostEqual(normalized[1], 2 / 3)
        lesson = L17["m16-c01-p06"]
        self.assertIn("1/3", lesson["worked_example"])
        self.assertIn(r"\mathcal V_i", next(e["tex"] for e in lesson["equations"] if e["label"] == "注意力概率"))

    def test_sft_token_weighting(self):
        self.assertAlmostEqual(3 * math.log(2) / 4, 0.5198603854199589)
        self.assertIn("0.5199", L17["m17-c03-p01"]["worked_example"])

    def test_gnn_and_diffusion(self):
        self.assertEqual(0.25 * 2 + 0.75 * 10, 8)
        self.assertIn("聚合为8", L17["m18-c02-p02"]["worked_example"])
        self.assertAlmostEqual(0.8**2, 0.64)
        self.assertAlmostEqual(math.sqrt(0.64) * 2 - math.sqrt(0.36), 1.0)
        self.assertIn("2.2", L17["m19-c04-p01"]["worked_example"])

    def test_bellman_residual_bound(self):
        gamma = 0.5
        v = (1.0, 2.0)
        bellman_v = (1 + gamma * (v[0] + v[1]) / 2, 2 + gamma * v[1])
        residual = max(abs(a - b) for a, b in zip(v, bellman_v))
        self.assertAlmostEqual(residual / (1 - gamma), 2)
        self.assertIn("误差界为1/(1−0.5)=2", L18["m20-c02-p01"]["worked_example"])

    def test_offpolicy_effective_sample_size(self):
        w = (2.5, 0.625)
        ess = sum(w)**2 / sum(x*x for x in w)
        self.assertAlmostEqual(ess, 25 / 17)
        self.assertIn("25/17", L18["m21-c01-p03"]["worked_example"])

    def test_dcc_positive_definite_covariance(self):
        determinant = 0.04 * 0.09 - 0.0048**2
        self.assertAlmostEqual(determinant, 0.00357696)
        self.assertIn("0.00357696", L18["m22-c04-p03"]["worked_example"])

    def test_self_financing_cash_ledger(self):
        sell, buy, fee = 5.05, 4.95, 0.1
        delta_cash = -(-sell + buy) - fee
        self.assertAlmostEqual(delta_cash, 0)
        self.assertAlmostEqual(110 - fee, 109.9)
        self.assertIn("ΔB=", L18["m23-c04-p02"]["worked_example"])

    def test_js_empirical_noise_and_measure_scope(self):
        p, q = (0.6, 0.4), (0.4, 0.6)
        js = sum((x * math.log(x / 0.5) for x in p)) / 2
        js += sum((x * math.log(x / 0.5) for x in q)) / 2
        self.assertAlmostEqual(js, 0.0201355135506888)
        self.assertEqual(JSD["id"], "information-n0485")
        self.assertGreaterEqual(len(JSD["deep_sections"]), 7)
        empirical = [s for s in JSD["deep_sections"] if s["title"] == "经验JSD偏差、检验方案与ETF样本依赖"]
        self.assertEqual(len(empirical), 1)
        self.assertIn("0.02014", empirical[0]["worked"])


if __name__ == "__main__":
    unittest.main()
