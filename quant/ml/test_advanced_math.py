"""Numerically cross-check worked examples in the published advanced ML Atlas.

These tests verify arithmetic illustrations, not complete formal proofs.
"""
import json
import math
import pathlib
import unittest

ROOT = pathlib.Path(__file__).resolve().parent
FILES = [
    "atlas-depth-foundations-3.json",
    "atlas-depth-learning-3.json",
    "atlas-depth-quant-3.json",
]
LESSONS = {}
for name in FILES:
    doc = json.loads((ROOT / name).read_text(encoding="utf-8"))
    LESSONS.update(doc["lessons"])


class WorkedExamplesTest(unittest.TestCase):
    def lesson(self, lesson_id):
        chapter = LESSONS[lesson_id]
        self.assertGreaterEqual(len(chapter["worked_example"]), 50)
        self.assertGreaterEqual(len(chapter["equations"]), 3)
        return chapter

    def test_weighted_microbatch_gradient(self):
        self.lesson("m02-c04-p05")
        self.assertAlmostEqual((100 * 2 + 20 * 8) / (100 + 20), 3.0)
        self.assertNotEqual((2 + 8) / 2, 3.0)

    def test_label_shift_confusion_equation(self):
        self.lesson("m04-c05-p02")
        positive_prior = (0.62 - 0.30) / (0.80 - 0.30)
        self.assertAlmostEqual(positive_prior, 0.64)
        self.assertAlmostEqual(0.8 * positive_prior + 0.3 * (1 - positive_prior), 0.62)

    def test_adam_square_moment_bias_correction(self):
        self.lesson("m05-c03-p04")
        raw = (1 - 0.9) * 2**2
        corrected = raw / (1 - 0.9)
        self.assertAlmostEqual(raw, 0.4)
        self.assertAlmostEqual(corrected, 4)

    def test_smoothed_target_encoding(self):
        self.lesson("m06-c02-p06")
        value = (1 + 4 * 0.2) / (1 + 4)
        self.assertAlmostEqual(value, 0.36)

    def test_cost_sensitive_threshold(self):
        self.lesson("m07-c03-p04")
        self.assertAlmostEqual(3 / (3 + 1), 0.75)

    def test_rbf_feature_scale_change(self):
        self.lesson("m09-c05-p05")
        near = math.exp(-1)
        changed_units = math.exp(-100)
        self.assertAlmostEqual(near, 0.36787944117144233)
        self.assertLess(changed_units, 1e-40)

    def test_mutual_reachability(self):
        self.lesson("m11-c02-p04")
        self.assertEqual(max(2, 3, 1), 3)

    def test_dense_layer_parameters(self):
        self.lesson("m13-c01-p04")
        self.assertEqual(8 * (5 + 1) + 1 * (8 + 1), 57)

    def test_three_conv_layers_receptive_field(self):
        self.lesson("m14-c01-p06")
        r, jump = 1, 1
        for kernel, stride, dilation in [(3, 1, 1)] * 3:
            r += (kernel - 1) * dilation * jump
            jump *= stride
        self.assertEqual((r, jump), (7, 1))

    def test_forget_gate_direct_path(self):
        self.lesson("m15-c03-p03")
        self.assertAlmostEqual(0.9**10, 0.3486784401)
        self.assertAlmostEqual(0.99**10, 0.9043820750088044)

    def test_local_attention_connection_ratio(self):
        self.lesson("m16-c04-p06")
        n, window = 4096, 128
        self.assertEqual(n * window, 524288)
        self.assertEqual(n * n, 16777216)
        self.assertEqual(n * n // (n * window), 32)

    def test_har_realized_variance_weekly_mean(self):
        self.lesson("m22-c03-p05")
        self.assertAlmostEqual(sum([1, 2, 3, 2, 2]) / 5, 2)

    def test_market_making_reservation_quote(self):
        self.lesson("m21-c05-p03")
        reservation = 100 - 2 * 0.1
        self.assertAlmostEqual(reservation, 99.8)
        self.assertAlmostEqual(reservation - 0.4 / 2, 99.6)
        self.assertAlmostEqual(reservation + 0.4 / 2, 100.0)

    def test_factor_neutralization_example(self):
        self.lesson("m23-c02-p03")
        factors = [1, 2, 3]
        residuals = [x - sum(factors) / len(factors) for x in factors]
        self.assertEqual(residuals, [-1, 0, 1])
        self.assertEqual(sum(residuals), 0)


if __name__ == "__main__":
    unittest.main()
