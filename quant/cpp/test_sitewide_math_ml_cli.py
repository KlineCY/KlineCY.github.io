import csv
import math
import pathlib
import subprocess
import sys
import tempfile

exe = pathlib.Path(sys.argv[1])
assert exe.is_file()

def write(path, columns, rows):
    with open(path, "w", newline="", encoding="utf-8") as file:
        csv_writer = csv.writer(file)
        csv_writer.writerow(columns)
        csv_writer.writerows(rows)

def invoke(command, *arguments, valid=True):
    result = subprocess.run(
        [str(exe), command, *[str(v) for v in arguments]],
        capture_output=True, text=True, timeout=35)
    if valid:
        assert result.returncode == 0, (command, arguments, result.stderr)
        assert result.stdout.strip(), ("No output", command)
    else:
        assert result.returncode != 0, ("Invalid command succeeded", command, arguments)
    return result.stdout

with tempfile.TemporaryDirectory() as folder:
    root = pathlib.Path(folder)
    train = root / "training.csv"
    test = root / "testing.csv"
    records = []
    for i in range(80):
        x = (i-40)/10
        target = 1+2*x+0.05*math.sin(i/4)
        binary = int(i >= 40)
        group = int(i >= 40)
        probability = min(0.98, max(0.02, 0.5+0.12*x))
        records.append([
            x,target,math.sin(i/5)*0.01,math.cos(i/7)*0.01,
            x+3,group,binary,2*binary-1,probability,
            i%3+1,target+0.1*math.cos(i/9),target-0.1*math.sin(i/8)
        ])
    columns = ["Feature","Target","ReturnA","ReturnB",
               "Shifted","Group","Label","Signed","Probability",
               "Weight","PredA","PredB"]
    write(train, columns, records)
    write(test, columns, records[-40:])
    valid_commands = [
        ("partial-corr2", (1, 0.4, 1, 0)),
        ("trend-smooth", (train,"Target",10)),
        ("huber-fit", (train,"Feature","Target",1)),
        ("empirical-cdf", (train,"Feature",0)),
        ("ks-two", (train,"Feature","Shifted")),
        ("permutation-means", (train,"Group","Target",200,17)),
        ("stationary-bootstrap", (train,"ReturnA",5,17)),
        ("pacf", (train,"ReturnA",3)),
        ("page-hinkley", (train,"Target",0.1,2)),
        ("aggregate-return", (train,"ReturnA",5)),
        ("bipower", (train,"ReturnA",5)),
        ("distance-cov", (train,"Feature","Target")),
        ("weighted-quantile", (train,"Target","Weight",0.5)),
        ("weighted-cvar", (train,"Target","Weight",0.9)),
        ("linear-svm", (train,"Feature","Signed",0.2,200,0.01)),
        ("quantile-linear", (train,"Feature","Target",0.5,0.2,500,0.1)),
        ("expectile-linear", (train,"Feature","Target",0.5,0.2)),
        ("ftrl-logit", (train,"Feature","Label",80,0.2)),
        ("binary-auc", (train,"Probability","Label")),
        ("brier", (train,"Probability","Label")),
        ("binary-logloss", (train,"Probability","Label")),
        ("psi", (train,"Feature","Shifted",5)),
        ("kernel-mmd", (train,"Group","Feature",1)),
        ("permutation-importance", (train,test,"Feature","Target",17,5)),
        ("crossfit-ridge", (train,"Feature","Target",4,0.01)),
        ("rolling-origin", (train,"Feature","Target",30,5,10,2,0.01)),
        ("isotonic-monotone", (train,"Feature","Target",0.2)),
        ("monotone-violation", (train,"Feature","Target")),
    ]
    assert len(valid_commands) == 28
    results = {command:invoke(command,*args) for command,args in valid_commands}
    assert abs(float(results["partial-corr2"].split()[1])-0.4)<1e-6
    assert abs(float(results["empirical-cdf"].split()[1])-41/80)<1e-6
    assert abs(float(results["binary-auc"].split()[1])-1)<1e-6
    assert "Index,SmoothedTrend" in results["trend-smooth"]
    assert "Index,PACF" in results["pacf"]
    assert "MSEIncrease " in results["permutation-importance"]
    assert "Index,OutOfSampleError" in results["rolling-origin"]
    assert "Probability " in results["ftrl-logit"]
    assert "Index,CompoundedReturn" in results["aggregate-return"]
    assert "PValue " in results["permutation-means"]
    assert "Statistic " in results["binary-logloss"]
    invalid_commands = [
        ("partial-corr2", (1, 2, 1, 0)),
        ("trend-smooth", (train,"Target",-1)),
        ("huber-fit", (train,"Feature","Target",0)),
        ("empirical-cdf", (train,"Missing",0)),
        ("ks-two", (train,"Feature","Missing")),
        ("permutation-means", (train,"Group","Target",1,17)),
        ("stationary-bootstrap", (train,"ReturnA",0,17)),
        ("pacf", (train,"ReturnA",200)),
        ("page-hinkley", (train,"Target",-1,2)),
        ("aggregate-return", (train,"ReturnA",0)),
        ("bipower", (train,"ReturnA",0)),
        ("distance-cov", (train,"Feature","Missing")),
        ("weighted-quantile", (train,"Target","Weight",2)),
        ("weighted-cvar", (train,"Target","Weight",1)),
        ("linear-svm", (train,"Feature","Label",0.2,200,0.01)),
        ("quantile-linear", (train,"Feature","Target",1,0.2,500,0.1)),
        ("expectile-linear", (train,"Feature","Target",0,0.2)),
        ("ftrl-logit", (train,"Feature","Label",0,0.2)),
        ("binary-auc", (train,"Probability","Missing")),
        ("brier", (train,"Target","Label")),
        ("binary-logloss", (train,"Target","Label")),
        ("psi", (train,"Feature","Shifted",1)),
        ("kernel-mmd", (train,"Group","Feature",0)),
        ("permutation-importance", (train,test,"Missing","Target",17,5)),
        ("crossfit-ridge", (train,"Feature","Target",1,0.01)),
        ("rolling-origin", (train,"Feature","Target",100,5,10,2,0.01)),
        ("isotonic-monotone", (train,"Missing","Target",0.2)),
        ("monotone-violation", (train,"Missing","Target")),
    ]
    assert len(invalid_commands) == 28
    for name,args in invalid_commands:
        invoke(name,*args,valid=False)
print("PASS: 28 additional mathematics and ML CLI commands, 28 invalid-input cases")
