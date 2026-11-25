import importlib.util
import json
import math
import pathlib
import subprocess
import sys
from collections import Counter

root=pathlib.Path(__file__).resolve().parents[1]
exe=sys.argv[1]
groups=['simulation','microstructure','engineering','empirical','extensions']
catalog=[c for g in groups for c in json.loads((root/'quant/research'/f'multiscale-{g}-cases.json').read_text(encoding='utf8'))]
ids={c['id'] for c in catalog}
assert len(ids)==40 and set(subprocess.check_output([exe,'--list'],text=True).splitlines())==ids

def batch(rows):
    source='\n'.join(' '.join([m]+[str(x) for x in a]) for m,a in rows)+'\n'
    raw=subprocess.run([exe,'--batch'],input=source,text=True,check=True,capture_output=True).stdout.splitlines()
    assert len(raw)==len(rows),(len(raw),len(rows))
    return [None if x=='ERROR' else list(map(float,x.split(','))) for x in raw]

def verify(result,expected,label):
    assert result is not None,label
    indices=expected.items() if isinstance(expected,dict) else enumerate(expected)
    # Quadrature and finite-difference references have independent numerical error.
    tol=3e-6 if label[0] in ('greek_estimators','stratified_call','sharpe_hac') else 3e-8
    for j,v in indices:
        j=int(j)
        assert j<len(result) and math.isfinite(result[j]) and math.isclose(result[j],v,rel_tol=tol,abs_tol=tol*1e-2),(label,j,result[j] if j<len(result) else None,v)
    return len(expected)

fixed=batch([(c['id'],c['input']) for c in catalog])
fail=[]
coords=0
for c,a in zip(catalog,fixed):
    try:coords+=verify(a,c['expected'],(c['id'],c['input']))
    except AssertionError as e:fail.append(str(e))
assert not fail,'\n'.join(fail)
print(f'PASS: {len(catalog)} fixed cases, {coords} selected analytical coordinates')
spec=importlib.util.spec_from_file_location('multiscale_reference',root/'quant/research/multiscale_reference.py')
ref=importlib.util.module_from_spec(spec);spec.loader.exec_module(ref)
studies=list(ref.studies())
spec2=importlib.util.spec_from_file_location('multiscale_extension_reference',root/'quant/research/multiscale_extension_reference.py')
ref2=importlib.util.module_from_spec(spec2);spec2.loader.exec_module(ref2)
studies.extend(ref2.studies())
assert set(m for m,_,_,_ in studies)==ids
results=batch([(m,a) for m,a,_,_ in studies])
checks=Counter();n=0;fail=[]
for result,(m,a,e,design) in zip(results,studies):
    try:
        n+=verify(result,e,(m,a,design));checks[m]+=1
        if m=='mlmc_plan':assert result[0]<=a[0]**2+1e-10 and result[1]>=result[2]-1e-9
        if m=='asian_coupling':assert abs(result[7])<1e-9
        if m=='stratified_call':assert result[2]<=result[1]+1e-8
        if m=='bridge_path':assert 0<=result[1]<=result[0]+1e-10
        if m=='refresh_covariance':assert result[0]**2<=result[1]*result[2]+1e-10
        if m=='fee_rebalance':assert abs(result[6])<1e-8 and result[4]>=-1e-8
        if m=='fractional_rank':assert abs(result[0])<1e-10 and abs(result[1]-a[1])<1e-10
        if m=='joint_sign_stepdown':
            k=int(a[1]);assert all(result[1+k+j]<=result[1+3*k+j]+1e-10<=result[1+2*k+j]+2e-10 for j in range(k))
        if m=='fdr_stepup':
            k=len(a)-1;assert all(result[3+j]<=result[3+k+j]+1e-10 for j in range(k))
    except AssertionError as e:
        if len(fail)<20:fail.append(str(e))
assert not fail,'\n'.join(fail)
for m,k in sorted(checks.items()):print(m,k)
print(f'PASS: {len(studies)} independently computed studies, {n} selected reference coordinates and structural certificates')
bad=[(m,[]) for m in ids]+[('unknown',[1]),('mlmc_plan',['nan',1,2]),('mlmc_plan',['0.1bad',1,2])]
bad += [
 ('mlmc_plan',[0,1,1]),('mlmc_estimate',[1,2,1,2,1,3,0]),('asian_coupling',[100,90,0,.2,1,1,2,3]),('asian_moments',[100,0,.2,1,.5]),
 ('stratified_call',[100,100,0,.2,1,1,0]),('greek_estimators',[100,100,0,0,1,0,1]),('bridge_path',[80,.2,0,90,100,1,1,100,.5,110]),('frozen_control',[2,0,1,1,2,1,3,2,4,3]),
 ('hy_covariance',[2,2,0,1,0,2,0,1,1,2]),('refresh_covariance',[2,2,0,1,1,2,5,1,6,2]),('hy_shift_grid',[2,0,0,2,2,0,1,1,2,0,1,1,2]),
 ('bipower_variation',[1,2]),('noise_moments',[1,2]),('lag_cov_noise',[1,0,0,1,0,0,1,1]),('weighted_preaverage',[2,0,0,0,1,2,3]),('volume_bars',[5,1,10,-2,1]),
 ('membership_vintage',[1,1,2,1,0,0,0,1,0,0,0,0]),('delist_wealth',[1,1,0,1,10,2,-1,0]),('delayed_orders',[1,1,0,-1,2,1,10,1,9,10,1,1]),
 ('fee_rebalance',[100,1,0,10,1,12]),('matured_ridge',[1,1,0,0,1,0,1,2,2,1,2]),('settlement_cash',[100,0,2,0,5,-1,0]),
 ('trade_revisions',[1,1,1,0,1,10,1,1,2,0,2]),('fractional_rank',[1,2,1,0,1,0]),('hj_bound',[1,3,1,1,1,1]),
 ('factor_spanning',[1,3,1,2,2,4,3,6]),('timing_regression',[1,1,1,2,1,3,1,4]),('sdf_distance',[1,2,1,1,0,1,0]),
 ('joint_sign_stepdown',[2,2,0,1,0,2]),('fdr_stepup',[.05,-.1]),('sharpe_hac',[0,1,1,1,1]),('rank_ic_panel',[2,0,3,1,1,1,2,1,3,3,1,1,2,2,3,3])]
bad += [('mlmc_bias_budget',[.01,.5,1,1,1,1,1,1,0]),('neyman_integer',[1,.5,1,1,1,.5,1,1,1]),('preavg_optimal',[1]),('two_scale_unknown',[1,0,0,1,2,2,4]),('cpcv_intervals',[3,3,0,0,0,1,1,1,2,2,2,3]),('two_way_cluster',[0,0,0,1,0,1,1,2,0,2,2,3,0,3,3,4]),('circular_sharpe_distribution',[2,1,1,1,1]),('reality_check_circular',[3,2,2,1,2,1,2,1,2])]
errors=[(m,a,r) for (m,a),r in zip(bad,batch(bad)) if r is not None]
assert not errors,errors
print(f'PASS: {len(bad)} rejected malformed or out-of-domain cases')
