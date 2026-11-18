import csv
import math
import pathlib
import subprocess
import sys
import tempfile

binary = pathlib.Path(sys.argv[1])
assert binary.is_file()

def run(*args, valid=True):
    process = subprocess.run([str(binary), *map(str, args)],
                             capture_output=True, text=True, timeout=30)
    if valid:
        assert process.returncode == 0, (args, process.stderr)
        assert process.stdout.strip(), args
    else:
        assert process.returncode != 0, ("Invalid input accepted", args)
    return process.stdout

def write_csv(path, names, rows):
    with open(path, "w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream)
        writer.writerow(names)
        writer.writerows(rows)

def first_value(output):
    return float(output.splitlines()[0].split()[1])

with tempfile.TemporaryDirectory() as temporary:
    d = pathlib.Path(temporary)
    samples = d / "samples.csv"
    data = []
    for i in range(80):
        a = 0.01 * math.sin(i / 4)
        b = 0.008 * math.cos(i / 5)
        close = 100 + i * 0.5
        target = close * 2 + 1
        data.append([a, b, 0.5 * a + 0.5 * b, close,
                     close+1, close-1, target, int(i >= 40),
                     i / 100, 1 + (i % 5), 0.01 + 0.9*i/79])
    write_csv(samples, ["A", "B", "C", "Close", "High", "Low",
                        "Target", "Label", "Signal", "Capacity", "PValue"], data)

    pca = run("pca", samples, "A", "B", "C")
    assert "Component,Eigenvalue" in pca and "Feature,PC1,PC2,PC3" in pca
    assert first_value(run("cov-condition", samples, "A", "B")) > 0

    ema = run("ema", samples, "Close", 0.5)
    assert "Index,EMA" in ema and len(ema.strip().splitlines()) == 81
    ewma = run("ewma-var", samples, "A", 0.9, 0.001)
    assert "Index,PriorVariance" in ewma
    assert abs(float(ewma.splitlines()[1].split(",")[1])-0.001) < 1e-8
    ar = run("ar1", samples, "Close")
    assert abs(float(next(line.split()[1] for line in ar.splitlines()
                          if line.startswith("Slope"))) - 1) < 1e-7
    garch = run("garch", samples, "A", 0.0001, 0.2, 0.7, 0.001)
    assert "PriorConditionalVariance" in garch
    assert len(garch.splitlines()) == 81

    momentum = run("momentum", samples, "Close", 12, 1)
    assert "Index,Momentum" in momentum
    assert len(momentum.splitlines()) == 68
    rsi = run("rsi", samples, "Close", 14)
    assert "Index,RSI" in rsi
    assert all(abs(float(s.split(",")[1])-100) < 1e-8
               for s in rsi.splitlines()[1:])
    atr = run("atr", samples, "High", "Low", "Close", 14)
    assert "Index,ATR" in atr and abs(float(atr.splitlines()[1].split(",")[1])-2)<1e-8

    lasso = run("lasso", samples, "Target", 0, "Close")
    assert "TrainingMSE" in lasso and "Close,2.00000000" in lasso
    logistic = run("logistic", samples, "Label", 0.1, "Close")
    assert "TrainingLogLoss" in logistic
    prob = [float(x.split(",")[1]) for x in logistic.splitlines()[2:]]
    assert prob[0] < 0.5 < prob[-1]

    budget = run("risk-budget", samples, "A", 1, "B", 1)
    weights = [float(x.split(",")[1]) for x in budget.splitlines()[1:]]
    assert len(weights) == 2 and abs(sum(weights) - 1) < 1e-7
    projected = run("cap-project", samples, "Signal", 0.1)
    targets = [float(x.split(",")[1]) for x in projected.splitlines()[1:]]
    assert len(targets) == 80
    assert abs(sum(targets)-1) < 1e-6 and max(targets) <= 0.1000001
    es = run("es-contrib", samples, 0.9, 0.6, "A", 0.4, "B")
    assert "Asset,ESContribution" in es

    black_call = first_value(run("black76", 100, 100, 0.2, 1, 0.95, "call"))
    black_put = first_value(run("black76", 100, 100, 0.2, 1, 0.95, "put"))
    assert black_call > 0 and abs(black_call-black_put) < 1e-7
    normal = first_value(run("bachelier", 100, 100, 10, 1, 1, "call"))
    implied = first_value(run("normal-iv", normal, 100, 100, 1, 1, "call"))
    assert abs(implied-10) < 1e-5

    curve = d / "curve.csv"
    flows = d / "flows.csv"
    payments = d / "payments.csv"
    write_csv(curve, ["Tenor", "ZeroRate"], [[1,0.05],[2,0.05],[3,0.05]])
    write_csv(flows, ["Time", "Cashflow"], [[1,5],[2,105]])
    write_csv(payments, ["PaymentTime"], [[1],[2]])
    assert first_value(run("curve-pv", curve, flows)) > 100
    assert first_value(run("swap-par", curve, payments)) > 0.05
    sensitivity = run("keyrate", curve, flows)
    assert "Index,KeyRateDV01" in sensitivity
    assert len(sensitivity.splitlines()) == 4

    book = d / "book.csv"
    write_csv(book, ["Price", "Quantity"], [[100,10],[101,5]])
    walked = run("book-walk", book, "buy", 12)
    assert "Executed 12.00000000" in walked and "Unfilled 0.00000000" in walked
    schedule = run("capped-schedule", samples, "Capacity", 30)
    children = [float(x.split(",")[1]) for x in schedule.splitlines()[1:]]
    assert abs(sum(children)-30) < 1e-5
    assert all(0 <= children[i] <= data[i][9]+1e-7 for i in range(80))

    alarm = run("cusum", samples, "Close", 110, 0, 20)
    assert "FirstAlarmIndex " in alarm
    holm = run("holm", samples, "PValue")
    assert "Index,AdjustedP" in holm
    interval = run("bootstrap-ci", samples, "A", 5, 90, 2026, 0.95)
    assert "Mean " in interval and "Lower " in interval and "Upper " in interval
    next_ou = first_value(run("ou-step", 3, 1, math.log(2), 0, 1, 0))
    assert abs(next_ou-2) < 1e-7

    run("pca", samples, "Missing", "A", valid=False)
    run("ema", samples, "Close", 0, valid=False)
    run("ewma-var", samples, "A", 1, 0.1, valid=False)
    run("garch", samples, "A", 0.01, 0.5, 0.5, 0.1, valid=False)
    run("momentum", samples, "Close", 100, 0, valid=False)
    run("rsi", samples, "Close", 0, valid=False)
    run("atr", samples, "High", "Low", "Close", -1, valid=False)
    run("lasso", samples, "Target", -1, "Close", valid=False)
    run("logistic", samples, "PValue", 0, "Close", valid=False)
    run("risk-budget", samples, "A", -1, "B", 1, valid=False)
    run("cap-project", samples, "Signal", 0.001, valid=False)
    run("es-contrib", samples, 1, 0.6, "A", 0.4, "B", valid=False)
    run("black76", 0, 100, 0.2, 1, 1, "call", valid=False)
    run("normal-iv", 0, 100, 90, 1, 1, "call", valid=False)
    run("book-walk", book, "unknown", 12, valid=False)
    run("capped-schedule", samples, "Capacity", 10000, valid=False)
    run("cusum", samples, "Close", 110, 0, 0, valid=False)
    run("bootstrap-ci", samples, "A", 0, 90, 2026, 0.95, valid=False)
    run("ou-step", 3, 1, -1, 0, 1, 0, valid=False)

print("PASS: 26 advanced C++ CLI workflows and invalid-input cases")
