import csv
import math
import pathlib
import subprocess
import sys
import tempfile

binary = pathlib.Path(sys.argv[1])
assert binary.is_file(), "Pass the compiled C++ command line program"


def run(*args, success=True):
    result = subprocess.run([str(binary), *map(str, args)],
                            capture_output=True, text=True, timeout=15)
    if success:
        assert result.returncode == 0, (args, result.stderr)
        assert result.stdout.strip(), "No output: " + str(args)
    else:
        assert result.returncode != 0, ("Accepted invalid arguments", args)
    return result.stdout


def csv_write(path, header, rows):
    with open(path, "w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream)
        writer.writerow(header)
        writer.writerows(rows)


with tempfile.TemporaryDirectory() as work:
    directory = pathlib.Path(work)
    returns = directory / "returns.csv"
    rows = []
    for i in range(60):
        a = 0.002 + 0.01 * math.sin(i / 5)
        b = 0.001 + 0.008 * math.cos(i / 7)
        rows.append([f"2026-01-{(i % 28) + 1:02d}", f"{a:.12f}", f"{b:.12f}",
                     f"{0.2 + i * 0.001:.12f}", f"{i * 0.0001:.12f}"])
    csv_write(returns, ["Date", "A", "B", "Factor", "ForwardReturn"], rows)

    stats = run("stats", returns, "A")
    assert "SampleVariance" in stats and "HAC_SE_1" in stats
    risk = run("risk", returns, "A", 0.95)
    assert "HistoricalES" in risk and "LongestUnderwaterDays" in risk
    portfolio = run("portfolio", returns, "A", "B")
    assert "MinVarianceWeight" in portfolio and "VolatilityContribution" in portfolio
    factor = run("factor", returns, "Factor", "ForwardReturn")
    assert "RankIC" in factor and "LongShortReturn" in factor
    relative = run("relative", returns, "A", "B")
    assert "TrackingErrorAnnual" in relative

    etf = run("etf", returns, "A", "B")
    assert "AnnualTrackingDifference" in etf
    assert "AnnualTrackingError" in etf
    curve = run("curve", 1, 0.03, 2, 0.04)
    assert abs(float(curve.splitlines()[0].split()[1]) - 0.05) < 1e-7
    fx = run("fx", 1.2, 0.02, 0.05, 1)
    assert abs(float(fx.split()[1]) - 1.2 * 1.05 / 1.02) < 1e-7
    hedge = run("duration-hedge", 1000000, 5, 10)
    assert "HedgeNotional -500000.00000000" in hedge
    roll = run("futures-roll", 98, 100)
    assert "StaticRollYield -0.02000000" in roll

    split = run("walkforward", 120, 60, 20, 5)
    assert "0,60,65,85" in split and "20,80,85,105" in split
    bootstrap = run("bootstrap", returns, "A", 5, 80, 2026)
    assert "Bootstrap025" in bootstrap and "Bootstrap975" in bootstrap
    mc = run("montecarlo", 100, 100, 0.05, 0.0, 0.2, 1, "call", 15000, 2026)
    assert "MonteCarloStandardError" in mc and "AntitheticPairs 15000" in mc
    calibration_csv = directory / "calibration.csv"
    csv_write(calibration_csv, ["Probability", "Label"],
              [[0.1, 0], [0.3, 1], [0.8, 1], [0.9, 1]])
    calibration = run("calibration", calibration_csv, "Probability", "Label", 2)
    assert "Brier" in calibration and "Bin,Count" in calibration
    attribution_csv = directory / "attribution.csv"
    csv_write(attribution_csv, ["WP", "WB", "RP", "RB"],
              [[0.6, 0.5, 0.08, 0.05], [0.4, 0.5, 0.03, 0.02]])
    attribution = run("brinson", attribution_csv, "WP", "WB", "RP", "RB")
    assert "ActiveReturn 0.02500000" in attribution
    assert "ExplainedActiveReturn 0.02500000" in attribution

    option = run("option", 100, 100, 0.02, 0.01, 0.2, 1, "call")
    premium = float(next(line.split()[1] for line in option.splitlines()
                         if line.startswith("Premium")))
    implied = run("implied", 100, 100, 0.02, 0.01, premium, 1, "call")
    assert abs(float(implied.split()[1]) - 0.2) < 1e-6
    assert "Delta" in option and "VegaPerUnitVol" in option
    assert "AmericanOption" in run("american", 100, 100, 0.02, 0.0, 0.2, 1, 100, "put")
    assert "DV01" in run("bond", 100, 0.05, 0.04, 3, 2)
    quote = run("quote", 99, 101, 30, 10, 101, "buy", 100.5)
    assert "EffectiveSpreadBps" in quote and "Microprice" in quote

    fills = directory / "fills.csv"
    csv_write(fills, ["Price", "Quantity"], [[100, 10], [102, 30]])
    output = run("fills", fills, 100, "buy")
    assert "101.50000000" in output and "150.00000000" in output

    bars = directory / "bars.csv"
    csv_write(bars, ["Date", "Open", "High", "Low", "Close", "Volume", "AdjustedClose"],
              [["2026-01-01", 100, 101, 98, 100, 1000, 100],
               ["2026-01-02", 102, 104, 101, 103, 1200, 102],
               ["2026-01-03", 104, 106, 103, 105, 1300, 104]])
    output = run("bars", bars)
    assert "Bars 3" in output and "AdjustedCumulativeReturn" in output

    events = directory / "events.csv"
    decisions = directory / "decisions.csv"
    csv_write(events, ["Date", "Published", "Received", "Value"], [
        ["2026-01-01", "2026-01-01", "2026-01-03", 2.5],
        ["2026-01-04", "2026-01-04", "2026-01-06", 4.5],
    ])
    csv_write(decisions, ["Decision"], [
        ["2026-01-02"], ["2026-01-03"], ["2026-01-06"],
    ])
    joined = run("asof", events, decisions)
    assert "2026-01-02,\n" in joined
    assert "2026-01-03,2.50000000" in joined
    assert "2026-01-06,4.50000000" in joined


    learning_csv = directory / "learning.csv"
    csv_write(learning_csv, ["X", "Y", "Maturity"], [
        [i, 1 + 2 * i, i + 2] for i in range(30)
    ])
    ridge = run("ridge", learning_csv, "Y", 0, "X")
    assert "Intercept 1.00000000" in ridge
    assert "X,2.00000000" in ridge
    assert "TrainingMSE 0.00000000" in ridge
    forward = run("ridge-forward", learning_csv, "Y", "Maturity", 12, 6, 12, 1, 0, "X")
    assert "12,12,10,25.00000000,25.00000000" in forward
    assert "18,18" in forward and "Index,ModelFitAt,TrainingRows" in forward

    shrink = run("ledoit-wolf", returns, "A", "B")
    assert "ShrinkageIntensity " in shrink
    intensity = float(shrink.splitlines()[0].split()[1])
    assert 0 <= intensity <= 1
    assert "Asset,A,B" in shrink

    ledger_csv = directory / "ledger.csv"
    csv_write(ledger_csv, ["WA", "RA", "WB", "RB"], [
        [1, 0.1, 0, 0], [1, 0, 0, 0], [0, 0, 0, 0]
    ])
    ledger = run("rebalance", ledger_csv, 100, 0, "WA", "RA", "WB", "RB")
    rows = [line.split(",") for line in ledger.strip().splitlines()[1:]]
    assert len(rows) == 3
    assert abs(float(rows[0][-1]) - 1.1 / 1.01) < 1e-7
    assert abs(float(rows[1][3])) < 1e-7
    assert abs(float(rows[2][-1]) - 1.1 / 1.01 * 0.99) < 1e-7
    run("ridge", learning_csv, "Y", -1, "X", success=False)
    run("ridge-forward", learning_csv, "Y", "Maturity", 2, 5, 3, 1, 0, "X",
        success=False)
    run("ledoit-wolf", returns, "A", success=False)
    run("rebalance", ledger_csv, -1, 0, "WA", "RA", success=False)

    run("stats", returns, "MissingColumn", success=False)
    run("risk", returns, "A", 1, success=False)
    run("option", 100, 100, 0, 0, -1, 1, "call", success=False)
    run("implied", 100, 100, 0, 0, 200, 1, "call", success=False)
    run("american", 100, 100, 0, 0, 0.2, 1, 0, "put", success=False)
    run("quote", 101, 99, 10, 10, 101, "buy", 100, success=False)
    run("bond", 100, 0.05, 0.04, 0, 2, success=False)
    run("fx", 1.2, -2, 0.05, 1, success=False)
    run("curve", 2, 0.03, 1, 0.04, success=False)
    run("calibration", calibration_csv, "Probability", "Label", 0, success=False)
    run("montecarlo", 100, 100, 0.05, 0, 0.2, 1, "call", 0, 2026, success=False)

print("PASS: 26 CLI workflows, time-safe Ridge and cash-ledger invalid-input checks")
