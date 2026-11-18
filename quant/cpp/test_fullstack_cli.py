import csv
import math
import pathlib
import subprocess
import sys
import tempfile

binary = pathlib.Path(sys.argv[1])
assert binary.is_file()

def write_csv(path, header, rows):
    with open(path, "w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream)
        writer.writerow(header)
        writer.writerows(rows)

def execute(command, *args, valid=True):
    completed = subprocess.run(
        [str(binary), command, *[str(a) for a in args]],
        capture_output=True, text=True, timeout=25)
    if valid:
        assert completed.returncode == 0, (command, args, completed.stderr)
        assert completed.stdout.strip(), (command, args)
    else:
        assert completed.returncode != 0, ("Invalid inputs were accepted", command, args)
    return completed.stdout

with tempfile.TemporaryDirectory() as temporary:
    base = pathlib.Path(temporary)
    sample = base / "daily.csv"
    header = ["A", "B", "Signal", "Target", "ReturnA", "ReturnB",
              "Probability", "Label", "Maturity", "DollarVolume",
              "PriceChange", "SignedVolume", "WeightOld", "WeightNew",
              "Exposure", "DefaultProbability", "LossGivenDefault",
              "Low", "High", "PValue"]
    rows = []
    for t in range(60):
        a = 100 + 0.2*t + math.sin(t/3)
        b = 105 + 0.1*t + math.cos(t/5)
        signal = math.sin(t/4)
        ret_a = 0.02 * math.sin(t/3)
        ret_b = 0.01 * math.cos(t/6)
        rows.append([
            a, b, signal, 1 + 2*a - 3*signal, ret_a, ret_b,
            0.1 + 0.8*t/59, int(t >= 30), t+2, 100000+t*100,
            0.05*math.cos(t/5), 5*math.sin(t/5),
            0.3 + 0.1*math.sin(t/7), 0.2 + 0.1*math.cos(t/7),
            1000+t*10, 0.01 + 0.0001*t, 0.4,
            a-1, a+1, (t+1)/61,
        ])
    write_csv(sample, header, rows)
    roll = base / "trades.csv"
    write_csv(roll, ["Price"], [[100+i%2] for i in range(20)])
    bond = base / "cashflows.csv"
    write_csv(bond, ["Time", "Cashflow"], [[1, 5], [2, 105]])

    cases = [
        ("box-project", (sample, "WeightOld", "WeightNew", "A")),
        ("l1-project", (sample, "Signal", 2)),
        ("qp2", (2, 0, 2, 4, 2, 0, 1, 0, 1)),
        ("ols", (sample, "Target", "A", "Signal")),
        ("elastic", (sample, "Target", 0.01, 0.01, "A", "Signal")),
        ("kmeans", (sample, 2, "A", "Signal")),
        ("cointegration", (sample, "A", "B")),
        ("unitroot", (sample, "A")),
        ("spread-z", (sample, "Signal", 5)),
        ("kalman", (sample, "A", 100, 1, 0.1, 1)),
        ("regime", (0.5, 0.5, 0.8, 0.2, 0.2, 0.8, 3, 1)),
        ("factor-neutral", (sample, "Target", "A", "Signal")),
        ("factor-rank", (sample, "Signal", 1)),
        ("factor-buckets", (sample, "Signal", "ReturnA", 5)),
        ("credit-hazard", (0.012, 0.4)),
        ("credit-ecl", (100, 0.05, 0.6)),
        ("credit-bond", (100, 1, 0.03, 0.02, 0.4)),
        ("roll-spread", (roll, "Price")),
        ("kyle", (sample, "PriceChange", "SignedVolume")),
        ("realized-jump", (sample, "ReturnA")),
        ("option-hedge", (0.5, 0.02, 2, 2, 0.01)),
        ("futures-hedge", (1000000, 1, 100, 100)),
        ("stress-worst", (sample, 0.6, "ReturnA", 0.4, "ReturnB")),
        ("vol-ramp", (0.1, 0.2, 1, 0.25, 3)),
        ("amihud", (sample, "ReturnA", "DollarVolume")),
        ("impact-cost", (0.04, 0.2, 0.5)),
        ("alpha-after-cost", (0.01, 1, 10, 5, 5)),
        ("model-metrics", (sample, "Probability", "Label", 0.5)),
        ("oos-r2", (sample, "Target", "A", 100)),
        ("perm-corr", (sample, "A", "B", 40, 2026)),
        ("jackknife", (sample, "ReturnA")),
        ("zero-bootstrap", (2, 90.4837418, 100)),
        ("bond-ytm", (bond, 100)),
        ("futures-pnl", (100, 102, 3, 100)),
        ("futures-basis", (105, 100, 1)),
        ("fx-translate", (0.1, 7, 8)),
        ("fx-hedge", (100, 7.1, 7)),
        ("fx-cross", (7, 0.9)),
        ("series-lag", (sample, "Signal", 1)),
        ("price-jump", (sample, "A")),
        ("performance-tail", (sample, "ReturnA")),
        ("performance-trades", (sample, "ReturnA")),
        ("trend-ema", (sample, "A", 5, 20)),
        ("hysteresis", (sample, "Signal", 0.5, 0.2)),
        ("position-pnl", (sample, "ReturnA", "ReturnB", 0.001)),
        ("market-maker", (100, 2, 0.1, 4, 1, 2)),
        ("variance-forward", (1, 0.2, 2, 0.2)),
        ("maturity-count", (sample, "Maturity", 20, 1)),
        ("turnover-cap", (sample, "WeightOld", "WeightNew", 0.1)),
        ("neutral-weights", (sample, "Signal", 1)),
        ("gross-limit", (sample, "Signal", 1)),
        ("rank-turnover", (sample, "WeightOld", "WeightNew")),
        ("putcall-forward", (5, 3, 100, 1)),
        ("roll-futures", (100, 102, 3, 105, 10, 2)),
        ("credit-portfolio", (sample, "unused", "unused")),
        ("circular-boot", (sample, "ReturnA", 5, 2026)),
        ("stationary-boot", (sample, "ReturnA", 5, 2026)),
        ("butterfly", (12, 9, 7)),
        ("cost-of-carry", (100, 0.05, 0.01, 0.02, 1)),
    ]
    cases = [(key, args if key != "credit-portfolio" else (sample,))
             for key, args in cases]
    assert len(cases) == 59, len(cases)
    outputs = {}
    for command, arguments in cases:
        outputs[command] = execute(command, *arguments)

    assert abs(float(outputs["credit-hazard"].split()[1])-0.02) < 1e-7
    assert abs(float(outputs["credit-ecl"].split()[1])-3) < 1e-7
    assert abs(float(outputs["futures-hedge"].split()[1])+100) < 1e-7
    assert abs(float(outputs["fx-translate"].split()[1])-(1.1*8/7-1)) < 1e-7
    assert abs(float(outputs["variance-forward"].split()[1])-0.04) < 1e-7
    assert abs(float(outputs["zero-bootstrap"].split()[1])-0.05) < 1e-7
    assert abs(float(outputs["butterfly"].split()[1])-1) < 1e-7
    assert abs(float(outputs["putcall-forward"].split()[1])-102) < 1e-7
    assert abs(float(outputs["roll-futures"].splitlines()[1].split()[1])-12) < 1e-7
    assert "AUC" in outputs["model-metrics"] and "F1" in outputs["model-metrics"]
    assert "RSquared" in outputs["ols"]
    assert "Inertia" in outputs["kmeans"]
    assert "ResidualHalfLife" in outputs["cointegration"]
    assert "Index,SelectedValue" in outputs["circular-boot"]
    assert "Index,SelectedValue" in outputs["stationary-boot"]
    assert outputs["box-project"].count("\n") == 61

    invalid = [
        ("box-project", (sample, "A", "Missing", "Signal")),
        ("l1-project", (sample, "A", -1)),
        ("qp2", (0, 0, 0, 1, 1, 0, 1, 0, 1)),
        ("ols", (sample, "Target", "NoFeature")),
        ("elastic", (sample, "Target", -1, 0, "A")),
        ("kmeans", (sample, 100, "A")),
        ("unitroot", (sample, "Missing")),
        ("spread-z", (sample, "A", 100)),
        ("kalman", (sample, "A", 100, 1, -1, 1)),
        ("regime", (0.5, 0.5, 0.8, 0.1, 0.2, 0.8, 3, 1)),
        ("factor-rank", (sample, "Missing", 1)),
        ("credit-hazard", (0.01, 1)),
        ("credit-ecl", (100, 2, 0.5)),
        ("credit-bond", (100, -1, 0.03, 0.02, 0.4)),
        ("roll-spread", (sample, "Missing")),
        ("kyle", (sample, "A", "Missing")),
        ("option-hedge", (0, 0, 1, 2)),
        ("futures-hedge", (100, 1, 0, 100)),
        ("stress-worst", (sample, 0.6, "A", 0.4, "Missing")),
        ("vol-ramp", (0, 0.2, 1, 0.25, 3)),
        ("amihud", (sample, "ReturnA", "Missing")),
        ("impact-cost", (-1, 0.2, 0.5)),
        ("model-metrics", (sample, "Probability", "Label", 3)),
        ("perm-corr", (sample, "A", "B", 0, 2026)),
        ("zero-bootstrap", (0, 90, 100)),
        ("futures-basis", (100, 0, 1)),
        ("fx-translate", (0.1, 0, 8)),
        ("series-lag", (sample, "Signal", 0)),
        ("trend-ema", (sample, "A", 20, 5)),
        ("hysteresis", (sample, "Signal", 0.1, 0.2)),
        ("variance-forward", (2, 0.2, 1, 0.2)),
        ("maturity-count", (sample, "Maturity", -1, 1)),
        ("turnover-cap", (sample, "WeightOld", "WeightNew", -1)),
        ("gross-limit", (sample, "Signal", -1)),
        ("credit-portfolio", (sample, "wrong")),
        ("circular-boot", (sample, "ReturnA", 0, 2026)),
    ]
    for command, arguments in invalid:
        execute(command, *arguments, valid=False)

print("PASS: all 59 full-stack C++ CSV CLI commands and 36 invalid-input cases")
