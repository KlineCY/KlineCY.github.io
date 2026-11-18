import csv
import math
import pathlib
import subprocess
import sys
import tempfile

exe=pathlib.Path(sys.argv[1])
assert exe.is_file()

def write(path, columns, rows):
    with open(path,"w",encoding="utf-8",newline="") as handle:
        w=csv.writer(handle)
        w.writerow(columns)
        w.writerows(rows)

def call(command,args,valid=True):
    result=subprocess.run([str(exe),command,*[str(a) for a in args]],
                          capture_output=True,text=True,timeout=30)
    if valid:
        assert result.returncode==0,(command,args,result.stderr)
        assert result.stdout.strip(),(command,args)
    else:
        assert result.returncode!=0,("Invalid input accepted",command,args)
    return result.stdout

with tempfile.TemporaryDirectory() as temp:
    d=pathlib.Path(temp)
    samples=d/"samples.csv"
    write(samples,["A","B","Signal","Target","Probability","Label",
                   "Return","Loss","Equity","Amount","X","Y"],
          [[0.02*math.sin(i/4)+0.0003*i,
            0.015*math.cos(i/5)+0.0001*i,
            math.sin(i/8),1+2*i-3*math.sin(i/8),
            0.05+0.9*i/79,int(i>=40),
            0.02*math.sin(i/4)+0.0001,
            i+1,100*(1.0005**i),
            1000+i*50, i, 1+2*i]
           for i in range(80)])
    tiny=d/"tiny.csv"
    write(tiny,["Value","P","Q"],[[1,0.2,0.3],[2,0.3,0.3],[3,0.5,0.4]])
    panel=d/"panel.csv"
    write(panel,["Entity","X","Y"],
          [[entity,t,10*entity+2*t] for entity in (1,2,3) for t in range(1,9)])
    positives=d/"positive.csv"
    write(positives,["V"],[[1+i] for i in range(30)])
    scores=d/"scores.csv"
    write(scores,["X","Label","Target"],
          [[i-40,int(i>=40),1+2*(i-40)] for i in range(80)])
    series=d/"series.csv"
    write(series,["Value"],[[1+math.cos(t*math.pi/4)] for t in range(24)])

    cases=[
        ("matrix-det2",[2,1,1,2]),
        ("matrix-inverse2",[2,1,1,2]),
        ("matrix-mahal2",[2,3,1,1,1,0,4]),
        ("robust-median",[samples,"Signal"]),
        ("robust-mad",[samples,"Signal"]),
        ("robust-autocorr",[samples,"Signal",1]),
        ("robust-trim",[samples,"Signal",0.1]),
        ("robust-huber",[samples,"Signal",0.5]),
        ("robust-z",[samples,"Signal",0.3]),
        ("normal-quantile",[0.975]),
        ("student-density",[0,3]),
        ("poisson-probability",[2,3]),
        ("lognormal-density",[1,0,1]),
        ("normal-interval",[-1,1,0,1]),
        ("periodogram",[series,"Value"]),
        ("dominant-frequency",[series,"Value"]),
        ("spectral-entropy",[series,"Value"]),
        ("arp-fit",[samples,"Target",1]),
        ("series-difference",[samples,"A",2]),
        ("garch-estimate",[samples,"Return",10]),
        ("garch-steady",[0.02,0.2,0.3]),
        ("garch-horizon",[0.04,0.02,0.2,0.3,3]),
        ("panel-fe",[panel,"Entity","Y","X"]),
        ("gpd-survival",[2,0.1,2]),
        ("gpd-quantile",[0.8,0.1,2]),
        ("hill-index",[positives,"V",5]),
        ("gaussian-es",[0,1,0.975]),
        ("welch-t",[samples,"A","B"]),
        ("paired-t",[samples,"A","B"]),
        ("entropy",[tiny,"P"]),
        ("kl-divergence",[tiny,"P","Q"]),
        ("binary-entropy",[0.5]),
        ("rls-train",[samples,"X","Y",0.01]),
        ("knn-regress",[scores,"X","Target",2,3]),
        ("knn-probability",[scores,"X","Label",2,3]),
        ("stump-regress",[scores,"X","Target",2]),
        ("boost-regress",[scores,"X","Target",2,30]),
        ("gaussian-nb",[scores,"X","Label",20]),
        ("isotonic-fit",[samples,"Probability","Label",0.6]),
        ("cov-eigen-floor",[samples,"A","B",0.01]),
        ("gmv2",[samples,"A","B"]),
        ("hrp2",[samples,"A","B"]),
        ("mean-frontier2",[samples,"A","B",0.04,0.03,0.05]),
        ("polynomial-features",[tiny,"Value",3]),
        ("rbf-value",[2,3,1]),
        ("interaction-features",[tiny,"Value"]),
        ("cosine-similarity",[samples,"A","B"]),
        ("mc-confidence",[samples,"A"]),
        ("antithetic-call",[100,90,0,0.2,1,0.3]),
        ("halton2",[12]),
        ("regime-compare",[samples,"Return",40]),
        ("cusum-change",[samples,"Target"]),
    ]
    assert len(cases)==52,len(cases)
    results={}
    for command,args in cases:
        results[command]=call(command,args)

    assert abs(float(results["matrix-det2"].split()[1])-3)<1e-8
    assert abs(float(results["normal-quantile"].split()[1])-1.95996398)<1e-6
    assert abs(float(results["garch-steady"].split()[1])-0.04)<1e-8
    assert abs(float(results["garch-horizon"].split()[1])-0.04)<1e-8
    assert abs(float(results["panel-fe"].splitlines()[0].split()[1])-2)<1e-6
    assert abs(float(results["entropy"].split()[1])-
               (-sum(p*math.log(p) for p in [0.2,0.3,0.5])))<1e-6
    assert abs(float(results["rls-train"].splitlines()[1].split()[1])-2)<0.01
    assert "Probability" in results["gaussian-nb"]
    assert "Row,Col0,Col1" in results["matrix-inverse2"]
    assert "Frequency" in results["dominant-frequency"]
    assert "Weight" in results["hrp2"]
    assert len(results["halton2"].splitlines())==13

    invalid=[
        ("matrix-inverse2",[1,1,1,1]),
        ("robust-z",[positives,"V",float("nan")]),
        ("robust-trim",[samples,"A",0.5]),
        ("normal-quantile",[1]),
        ("student-density",[0,-1]),
        ("poisson-probability",[-1,2]),
        ("periodogram",[samples,"Missing"]),
        ("arp-fit",[samples,"A",99]),
        ("garch-estimate",[tiny,"Value",10]),
        ("garch-steady",[0.02,0.5,0.5]),
        ("panel-fe",[samples,"Missing","Target","Signal"]),
        ("gpd-quantile",[1,0,1]),
        ("hill-index",[samples,"A",80]),
        ("gaussian-es",[0,1,1]),
        ("welch-t",[samples,"A","Missing"]),
        ("entropy",[samples,"Probability"]),
        ("kl-divergence",[tiny,"P","Missing"]),
        ("binary-entropy",[-1]),
        ("rls-train",[samples,"A","Y",-1]),
        ("knn-regress",[scores,"X","Target",0,1000]),
        ("stump-regress",[samples,"Missing","Y",0]),
        ("boost-regress",[scores,"X","Target",0,0]),
        ("gaussian-nb",[scores,"X","Target",2]),
        ("isotonic-fit",[scores,"X","Target",0]),
        ("gmv2",[samples,"A","Missing"]),
        ("polynomial-features",[tiny,"Value",6]),
        ("rbf-value",[2,3,0]),
        ("mc-confidence",[samples,"Missing"]),
        ("antithetic-call",[0,90,0,0.2,1,0]),
        ("halton2",[0]),
        ("regime-compare",[samples,"A",2]),
    ]
    for command,args in invalid:
        call(command,args,valid=False)
print("PASS: 52 expanded math/ML CLI commands and 31 invalid-input cases")
