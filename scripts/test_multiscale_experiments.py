import csv
import json
import math
import pathlib
import subprocess
import sys
from collections import defaultdict

root=pathlib.Path(__file__).resolve().parents[1]/'quant/research'
catalog=json.loads((root/'multiscale-experiment-catalog.json').read_text(encoding='utf8'))
exe=sys.argv[1]
assert len(catalog)==8 and len({x['id'] for x in catalog})==8
results={}
count=coordinates=checks=0
for p in catalog:
    with (root/p['csv']).open(newline='',encoding='utf8') as f:
        r=csv.DictReader(f);assert r.fieldnames==p['fields'];ref=list(r)
    raw=subprocess.check_output([exe,p['id']],text=True)
    r=csv.DictReader(raw.splitlines());assert r.fieldnames==p['fields'];actual=list(r)
    assert len(ref)==len(actual)==p['rows'],p['id']
    out=[]
    for i,(a,b) in enumerate(zip(actual,ref)):
        row={}
        for k in p['fields']:
            x,y=float(a[k]),float(b[k])
            assert math.isfinite(x) and math.isclose(x,y,rel_tol=4e-8,abs_tol=2e-8),(p['id'],i,k,x,y)
            row[k]=x;coordinates+=1
        out.append(row)
    count+=len(out);results[p['id']]=out
    print(f"PASS: {p['id']} {len(out)} parameter rows, {len(out)*len(p['fields'])} reference coordinates")
def close(a,b,tol=1e-8):return math.isclose(a,b,rel_tol=tol,abs_tol=tol)
def test(x,label):
    global checks
    assert x,label
    checks+=1
for r in results['mlmc']:
    test(r['PlannedVariance']<=r['SEBudget']**2+1e-10,r)
    test(r['ProductionCost']==2*r['N0']+2*r['N1']+4*r['N2']+8*r['N3'],r)
    test(r['PilotCost']==96*(2+2+4+8),r)
    test(all(r[f'N{i}'].is_integer() and r[f'N{i}']>=2 for i in range(4)),r)
for r in results['control']:
    test(0<=r['AnalyticResidualVariance']<=r['VarA']+1e-9,r)
    test(r['AdjustedExternalVariance']>=0,r)
for r in results['async']:
    test(r['RefreshPSDDet']>=-1e-10,r)
    test(abs(r['ShiftAlignedHY'])<=r['PeakAbsHY']+1e-10,r)
    test(r['RefreshNodes']>=2 and r['Overlap']>0,r)
for r in results['noise']:
    test(close(r['MeanCorrectedVarX'],.0004),r)
    test(close(r['MeanCorrectedVarY'],.0009),r)
    test(close(r['MeanCorrectedCov'],.0006),r)
    test(close(r['MeanPreavgCorrected'],.0004),r)
    test(r['EnumeratedPaths']==512,r)
by=defaultdict(list)
for r in results['universe']:
    test(close(r['PretradeWealth']-r['PostFeeWealth'],r['Fees']),r)
    test(close(r['Fees'],r['Fee']*r['TradeNotional']),r)
    test(close(r['RankGross'],2) and close(r['RankNet'],0),r)
    test(r['Cash']>=-1e-8,r)
    by[(r['AvailabilityDelay'],r['Fee'])].append(r)
for rows in by.values():
    test(close(rows[0]['PretradeWealth'],1000),'initial capital')
    for a,b in zip(rows,rows[1:]):test(close(a['EndWealth'],b['PretradeWealth']),'portfolio wealth carried across dates')
for r in results['maturity']:
    test(close(r['UnavailableLabelPerturbation'],0),r)
    test(close(r['ProperLoss'],(r['ActualLabel']-r['Forecast'])**2),r)
    test(r['TrainingCount']>=2 and r['TrainingCount']<r['Decision'],r)
for r in results['selection']:
    test(r['RawP']<=r['StepdownP']+1e-12<=r['MaxTP']+2e-12,r)
    test(r['BH']<=r['BY']+1e-12,r)
    test(close(r['RawP']*2**int(r['Dates']),round(r['RawP']*2**int(r['Dates']))),r)
for r in results['spanning']:
    test(close(r['BaseSR2']+r['Increment'],r['ExpandedSR2']),r)
    test(close(r['HJLowerBound'],r['SDFDistance']),r)
    test(close(r['HJLowerBound']**2,r['ExpandedSR2']),r)
    test(abs(r['MaxPricingError'])<1e-8 and abs(r['SpanningClosure'])<1e-8,r)
assert count==491 and coordinates==5587
assert subprocess.run([exe,'unknown'],capture_output=True).returncode!=0
print(f'PASS: {count} end-to-end output rows, {coordinates} reference coordinates, {checks} model invariants; 8192 exhaustive latent/noise paths across parameter pairs')
