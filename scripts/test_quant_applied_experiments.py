import csv
import json
import math
import pathlib
import subprocess
import sys
from collections import defaultdict

root=pathlib.Path(__file__).resolve().parents[1]/'quant/research'
catalog=json.loads((root/'applied-experiment-catalog.json').read_text(encoding='utf8'))
exe=sys.argv[1]
assert len(catalog)==8 and len({p['id'] for p in catalog})==8
all_rows={}
count=0
coordinates=0
for p in catalog:
    with (root/p['csv']).open(newline='',encoding='utf8') as f:
        ref=csv.DictReader(f)
        assert ref.fieldnames==p['fields']
        expected=list(ref)
    output=subprocess.check_output([exe,p['id']],text=True)
    actual=csv.DictReader(output.splitlines())
    assert actual.fieldnames==p['fields']
    rows=list(actual)
    assert len(rows)==len(expected)==p['rows'],p['id']
    values=[]
    for i,(a,b) in enumerate(zip(rows,expected)):
        row={}
        for field in p['fields']:
            x,y=float(a[field]),float(b[field])
            assert math.isfinite(x) and math.isclose(x,y,rel_tol=2e-9,abs_tol=2e-10),(p['id'],i,field,x,y)
            row[field]=x
            coordinates+=1
        values.append(row)
    all_rows[p['id']]=values
    count+=len(rows)
    print(f"PASS: {p['id']} {len(rows)} grid scenarios, {len(rows)*len(p['fields'])} reference coordinates")
assert count==1073 and coordinates==11718
def close(a,b):
    return math.isclose(a,b,rel_tol=2e-9,abs_tol=2e-10)

checks=0
def test(cond,msg):
    global checks
    assert cond,msg
    checks+=1

for x in all_rows['forecast']:
    test(close(x['RawBrier']-x['CalibratedBrier'],x['Improvement']),x)
    test(x['TrainBlocks']==3,x)
p=all_rows['inference']
test(len({tuple(x[f'U{i}'] for i in range(1,7)) for x in p})==math.factorial(6),'all PIT permutations')
for x in p:
    test(all(close(x[k],p[0][k]) for k in ('KS','CvM','AD')),x)
    test(close(x['LjungBox'],48/5*x['FirstACF']**2),x)
test(max(x['LjungBox'] for x in p)-min(x['LjungBox'] for x in p)>1,'temporal discrimination')
by_target=defaultdict(list)
for x in all_rows['liquidity']:
    test(0<=x['Filled']<=x['Target'],x)
    test(close(x['Execution']+x['Opportunity']+x['Fees'],x['Shortfall']),x)
    test(close(x['IdealPnL']-x['ActualPnL'],x['Shortfall']),x)
    test(close(x['Closure'],0),x)
    by_target[x['Target']].append(x)
for rows in by_target.values():
    rows.sort(key=lambda x:x['Limit'])
    test(all(a['Filled']<=b['Filled'] for a,b in zip(rows,rows[1:])),'protected fill monotonicity')
for x in all_rows['rates']:
    test(close(x['AssetPV']-x['LiabilityPV'],x['Surplus']),x)
    test(close(x['Surplus']-x['ParallelQuadratic']-x['SlopeLinear'],x['Remainder']),x)
    if x['ParallelBp']==x['SlopeBpPerYear']==0: test(close(x['Surplus'],0),x)
    if x['SlopeBpPerYear']==0: test(x['Surplus']>=-1e-10,x)
for x in all_rows['options']:
    test(x['American']>=x['European']-1e-10,x)
    test(close(x['American']-x['European'],x['Premium']),x)
    if x['CallPut']==1 and x['Growth']>=1:
        test(close(x['Premium'],0),x)
        test(x['ExerciseNodes']==0,x)
by_h=defaultdict(list)
for x in all_rows['credit']:
    test(close(x['JointCVA']-x['IndependentCVA'],x['WWRAdjustment']),x)
    if x['Threshold']<=10: test(close(x['WWRAdjustment'],0),x)
    by_h[x['Threshold']].append(x)
for rows in by_h.values():
    rows.sort(key=lambda x:x['PDShiftBp'])
    test(all(close(x['IndependentCVA'],rows[0]['IndependentCVA']) for x in rows),'fixed marginal PD')
    test(all(a['WWRAdjustment']<=b['WWRAdjustment']+1e-10 for a,b in zip(rows,rows[1:])),'WWR monotonicity')
for x in all_rows['accounts']:
    test(close(x['TWR'],.08),x)
    g=1+x['AnnualMWR']
    test(close(100*g*g+x['Deposit']*g,x['TerminalValue']),x)
    test(close(x['TerminalValue']-100-x['Deposit'],x['Gain']),x)
by_alpha=defaultdict(list)
for x in all_rows['allocation']:
    test(close(x['ES'],x['TailRC1']+x['TailRC2']),x)
    test(0<=x['Weight1']<=.6+1e-12 and -.6-1e-12<=x['Weight2']<=0,x)
    by_alpha[x['Alpha']].append(x)
for rows in by_alpha.values():
    rows.sort(key=lambda x:x['Cost'])
    test(all(a['Turnover']>=b['Turnover']-1e-12 for a,b in zip(rows,rows[1:])),'cost-sensitive turnover')
assert subprocess.run([exe,'unknown'],capture_output=True).returncode!=0
assert subprocess.run([exe],capture_output=True).returncode!=0
print(f'PASS: {count} parameter-grid scenarios, {coordinates} reference coordinates, {checks} structural invariants')
