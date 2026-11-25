import csv
import json
import math
import pathlib
import subprocess
import sys

root=pathlib.Path(__file__).resolve().parents[1]/'quant/research'
exe=sys.argv[1]
cat=json.loads((root/'multiscale-advanced-catalog.json').read_text(encoding='utf8'))
assert len(cat)==6
rows=coords=checks=0
for p in cat:
    with (root/p['csv']).open(newline='',encoding='utf8') as f:
        r=csv.DictReader(f);assert r.fieldnames==p['fields'];expected=list(r)
    r=csv.DictReader(subprocess.check_output([exe,p['id']],text=True).splitlines())
    assert r.fieldnames==p['fields'];actual=list(r)
    assert len(expected)==len(actual)==p['rows']
    for i,(a,b) in enumerate(zip(actual,expected)):
        z={k:float(v) for k,v in a.items()}
        for k,x in z.items():
            y=float(b[k])
            assert math.isfinite(x) and math.isclose(x,y,rel_tol=5e-8,abs_tol=2e-8),(p['id'],i,k,x,y)
            coords+=1
        if p['id']=='budget':
            assert z['MSE']<=z['RMSE']**2+1e-12 and z['Cost']>=z['ContinuousCost']-1e-8
        elif p['id']=='allocation':
            assert z['TotalSamples']==z['Budget'] and z['Gap']>=-1e-12
        elif p['id']=='noise':
            assert z['SineRatio']<=z['UniformRatio']+1e-12 and z['Paths']==512
        elif p['id']=='validation':
            assert z['Test']+z['Train']+z['Purged']+z['Embargoed']==3*z['Groups']
        elif p['id']=='cluster':
            assert math.isclose(z['TwoWayVariance'],z['AssetVariance']+z['DateVariance']-z['IntersectionVariance'],abs_tol=1e-10)
        elif p['id']=='blocks':
            assert 0<=z['RealityP']<=1 and z['SharpePaths']==z['Dates']**(z['Dates']/z['Block'])
            assert z['Sharpe025']<=z['SharpeMedian']<=z['Sharpe975']
        checks+=1
    rows+=len(actual)
    print(f"PASS: {p['id']} {len(actual)} rows, {len(actual)*len(p['fields'])} fields")
assert rows==966 and coords==10218
assert subprocess.run([exe,'unknown'],capture_output=True).returncode!=0
print(f'PASS: 6 advanced projects, {rows} parameter records, {coords} independent reference fields, {checks} structural checks')
