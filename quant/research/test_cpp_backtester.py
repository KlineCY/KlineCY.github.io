import csv
import datetime
import importlib.util
import io
import json
import math
import pathlib
import subprocess
import tempfile
import unittest

HERE = pathlib.Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("quant_python_engine", HERE / "engine.py")
python_engine = importlib.util.module_from_spec(spec)
spec.loader.exec_module(python_engine)


def example_csv(days=65):
    output = io.StringIO()
    writer = csv.writer(output)
    writer.writerow(["Date", "Ticker", "Open", "Close"])
    first = datetime.date(2026, 1, 1)
    for t in range(days):
        day = (first + datetime.timedelta(days=t)).isoformat()
        for j, symbol in enumerate(["AAA", "BBB", "CCC"]):
            drift = 0.20 * t if j == 0 else (-0.07 * t if j == 1 else 0.06 * t)
            price = 100 + 10 * j + drift + 1.2 * math.sin(t * 0.21 + j)
            close = price * (1 + 0.004 * math.cos(t * 0.3 + j))
            writer.writerow([day, symbol, f"{price:.12f}", f"{close:.12f}"])
    return output.getvalue()


CODES = {
    "equal": """def signals(history):
    return {ticker: 1 / len(history) for ticker in history}
""",
    "trend": """def signals(history):
    selected = [ticker for ticker, bars in history.items()
                if len(bars) >= 12 and bars[-1]["close"] >
                sum(bar["close"] for bar in bars[-12:]) / 12]
    return {ticker: 1 / len(selected) for ticker in selected}
""",
    "momentum": """def signals(history):
    if len(history) < 2 or any(len(bars) <= 12 for bars in history.values()):
        return {}
    ranked = sorted((bars[-1]["close"] / bars[-13]["close"] - 1, ticker)
                    for ticker, bars in history.items())
    return {ranked[0][1]: -0.5, ranked[-1][1]: 0.5}
""",
}


class CppBacktesterParity(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temporary = tempfile.TemporaryDirectory()
        cls.directory = pathlib.Path(cls.temporary.name)
        cls.executable = cls.directory / "cpp_backtester"
        result = subprocess.run(
            ["g++", "-std=c++17", "-O2", "-Wall", "-Wextra", "-pedantic",
             str(HERE / "cpp_backtester.cpp"), "-o", str(cls.executable)],
            capture_output=True, text=True, timeout=50
        )
        if result.returncode:
            raise AssertionError(result.stderr)

    @classmethod
    def tearDownClass(cls):
        cls.temporary.cleanup()

    def run_cpp(self, csv_text, strategy, fee=3, slip=2, borrow=0):
        file = self.directory / "prices.csv"
        file.write_text(csv_text, encoding="utf-8")
        result = subprocess.run(
            [str(self.executable), str(file), strategy, "12", str(fee),
             str(slip), str(borrow)],
            capture_output=True, text=True, timeout=15
        )
        return result

    def test_nine_cross_language_scenarios(self):
        csv_text = example_csv()
        for strategy, code in CODES.items():
            for fee, slip, borrow in [(0, 0, 0), (3, 2, 0), (5, 3, 120)]:
                with self.subTest(strategy=strategy, fee=fee, slip=slip, borrow=borrow):
                    run_cpp = self.run_cpp(csv_text, strategy, fee, slip, borrow)
                    self.assertEqual(run_cpp.returncode, 0, run_cpp.stderr)
                    cpp_equity = list(csv.DictReader(io.StringIO(run_cpp.stdout)))
                    py_output = json.loads(python_engine.main(json.dumps({
                        "csv": csv_text,
                        "code": code,
                        "commission_bps": fee,
                        "slippage_bps": slip,
                        "borrow_bps_annual": borrow,
                    })))
                    python_equity = py_output["equity"]
                    self.assertEqual(len(cpp_equity), len(python_equity))
                    for cpp, py in zip(cpp_equity, python_equity):
                        self.assertEqual(cpp["Date"], py["date"])
                        self.assertAlmostEqual(float(cpp["Strategy"]), py["strategy"], delta=2e-7)
                        self.assertAlmostEqual(float(cpp["Benchmark"]), py["benchmark"], delta=2e-7)
                    self.assertIn("periods=", run_cpp.stderr)
                    self.assertIn("max_drawdown=", run_cpp.stderr)

    def test_rejects_duplicate_ticker_date(self):
        text = example_csv()
        duplicate = text.splitlines()[1] + "\n"
        self.assertNotEqual(self.run_cpp(text + duplicate, "equal").returncode, 0)

    def test_rejects_negative_prices_and_costs(self):
        text = example_csv()
        self.assertNotEqual(self.run_cpp(text, "equal", fee=-1).returncode, 0)
        self.assertNotEqual(self.run_cpp(text.replace("100.000000000000", "-100.000000000000"),
                                        "equal").returncode, 0)

    def test_requires_common_trading_dates(self):
        rows = example_csv(20)
        self.assertNotEqual(self.run_cpp(rows, "equal").returncode, 0)


if __name__ == "__main__":
    unittest.main()
