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

    run("stats", returns, "MissingColumn", success=False)
    run("risk", returns, "A", 1, success=False)
    run("option", 100, 100, 0, 0, -1, 1, "call", success=False)
    run("implied", 100, 100, 0, 0, 200, 1, "call", success=False)
    run("american", 100, 100, 0, 0, 0.2, 1, 0, "put", success=False)
    run("quote", 101, 99, 10, 10, 101, "buy", 100, success=False)
    run("bond", 100, 0.05, 0.04, 0, 2, success=False)

print("PASS: all 12 CLI workflows and error-rejection cases")
