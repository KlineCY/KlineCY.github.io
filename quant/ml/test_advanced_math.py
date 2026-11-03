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
    "atlas-depth-foundations-4.json",
    "atlas-depth-learning-4.json",
    "atlas-depth-models-4.json",
    "atlas-depth-deep-4.json",
    "atlas-depth-crossdomain-4.json",
    "atlas-depth-quant-4.json",
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


    def test_fourth_pass_numerical_illustrations(self):
        """Audit one named fourth-pass illustration per Atlas domain.

        The expected number is independently calculated; matching a stated
        numeral in the article guards against text drifting away from examples.
        """
        cases = [
            ("m01-c02-p01", 0.6*1-0.4*2-0.1, -0.3, "−0.3"),
            ("m02-c04-p04", .6**2*.04+2*.6*.4*.01+.4**2*.09, .0336, "0.0336"),
            ("m03-c04-p05", .9*math.log(1.8)+.1*math.log(.2), .368, "0.368"),
            ("m04-c01-p06", 1.1**3*.6-1, -.2014, "−20.14%"),
            ("m05-c05-p04", .9**20, .122, "0.122"),
            ("m06-c04-p04", sum([.01,-.01,.02,0,.03])/5, .01, "1%"),
            ("m07-c03-p01", (.25*2)/(1+.25*2), 1/3, "1/3"),
            ("m08-c05-p04", 0, 0, "重要性可能为0"),
            ("m09-c03-p05", 1-2, -1, "−1"),
            ("m10-c03-p04", 1/(1+math.exp(-.5*math.log(3))), .634, "0.634"),
            ("m11-c01-p03", 3**2/(1**2+2**2+3**2), 9/14, "9/14"),
            ("m12-c05-p02", 3*1+2*1, 5, "预测5"),
            ("m13-c04-p04", .9+.1/4, .925, "0.925"),
            ("m14-c02-p06", 3*3*32*64*28*28, 14450688, "14,450,688"),
            ("m15-c02-p05", .95**40, .129, "0.129"),
            ("m16-c05-p03", 98+5, 103, "103日"),
            ("m17-c01-p04", (.8*1+.6*0)/(math.sqrt(1)*math.sqrt(.8**2+.6**2)), .8, "0.8"),
            ("m18-c01-p06", 2/(1+2+1), .5, "1/2"),
            ("m19-c05-p06", 1-.95**20, .642, "64.2%"),
            ("m20-c05-p02", min(1.4*2,1.2*2), 2.4, "2.4"),
            ("m21-c02-p04", 2-3, -1, "−1"),
            ("m22-c03-p03", math.exp(.8), 2.23, "2.23"),
            ("m23-c01-p03", (1+.5)/2, .75, "0.75"),
            ("m24-c05-p06", .6*.01+(-.2)*(-.02)-.002, .008, "0.8%"),
        ]
        self.assertEqual(len(cases), 24)
        for lesson_id, observed, expected, anchor in cases:
            with self.subTest(lesson=lesson_id):
                lesson = self.lesson(lesson_id)
                self.assertIn(anchor, lesson["worked_example"])
                if lesson_id == "m22-c03-p03":
                    self.assertEqual(round(observed, 2), expected)
                elif abs(expected) >= 100:
                    self.assertAlmostEqual(observed, expected, delta=.5)
                else:
                    self.assertAlmostEqual(observed, expected, delta=.0011)


if __name__ == "__main__":
    unittest.main()
