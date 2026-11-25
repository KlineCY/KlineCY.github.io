import json
import math
import pathlib
import subprocess
import sys
from collections import Counter
root=pathlib.Path(__file__).resolve().parents[1]
sys.path.insert(0,str(root/'quant/research'))
import finite_state_project_reference as reference

exe=sys.argv[1]
cat=json.loads((root/'quant/research/finite-state-projects-cases.json').read_text(encoding='utf8'))
ids={c['id'] for c in cat}
assert len(ids)==8 and ids<=set(subprocess.check_output([exe,'--list'],text=True).splitlines())

def batch(rows):
    source='\n'.join(' '.join([m]+[str(x) for x in a]) for m,a in rows)+'\n'
    out=subprocess.run([exe,'--batch'],input=source,text=True,check=True,capture_output=True).stdout.splitlines()
    assert len(out)==len(rows)
    return [None if x=='ERROR' else list(map(float,x.split(','))) for x in out]

def verify(result,expected,label,complete=False):
    assert result is not None,label
    if complete:assert len(result)==len(expected),(label,result,expected)
    indices=expected.items() if isinstance(expected,dict) else enumerate(expected)
    for j,v in indices:
        assert j<len(result) and math.isfinite(result[j]) and math.isclose(result[j],v,rel_tol=2e-8,abs_tol=2e-9),(label,j,result[j] if j<len(result) else None,v)
    return len(expected)

fixed=batch([(c['id'],c['input']) for c in cat])
coords=sum(verify(r,c['expected'],(c['id'],c['input']),True) for r,c in zip(fixed,cat))
print(f'PASS: {len(cat)} complete project cases, {coords} reference coordinates')
cases=list(reference.experiments())
assert {m for m,_,_,_ in cases}==ids
out=batch([(m,a) for m,a,_,_ in cases])
coords=0;checks=0;coverage=Counter()
for result,(m,a,e,design) in zip(out,cases):
    coords+=verify(result,e,(m,a,design));coverage[m]+=1
    if m=='duration_filter':
        k,d,n,s=int(a[0]),int(a[1]),int(a[2]),int(a[3])
        assert abs(sum(result[1:1+k])-1)<1e-9
        assert abs(sum(result[2+k:2+2*k])-1)<1e-9
        assert abs(sum(result[2+2*k:2+2*k+n])-1)<1e-9
        assert abs(sum(result[-k*d:])-1)<1e-9
        assert 1-1e-9<=result[1+k]<=d+1e-9
        checks+=5
    elif m=='credit_posterior_stress':
        k=int(a[0]);assert abs(sum(result[6:6+k])-1)<1e-9 and abs(sum(result[6+k:6+2*k])-1)<1e-9
        assert result[3]>=result[2]-1e-9 and result[5]<=a[5]+1e-9
        checks+=4
    elif m=='robust_mdp':
        assert result[0]<=result[2]+1e-9<=result[1]+2e-9
        assert result[5]<=a[0]*a[1]+1e-9
        checks+=3
    elif m=='drawdown_policy':
        assert 0<=result[3]<=1+1e-9 and abs(result[2]-a[8]*result[3]-result[0])<1e-9
        checks+=2
    elif m=='asian_additive':
        assert result[0]<=result[1]+1e-9 and abs(result[2]-a[0])<1e-9
        checks+=2
    elif m=='hidden_queue_choice':
        assert result[2]<=result[0]+1e-9<=result[3]+2e-9
        assert 0<=result[6]<=1
        checks+=3
    elif m=='transport_stress':
        assert result[3]<=a[2]+1e-8 and abs(sum(result[5:])-1)<1e-9
        checks+=2
    elif m=='nonlinear_risk_jet':
        assert result[7]>=-1e-12 and result[8]>=-1e-12
        checks+=2
for k,v in sorted(coverage.items()):print(k,v)
print(f'PASS: {len(cases)} independent project studies, {coords} reference coordinates, {checks} structural certificates')
bad=[(m,[]) for m in ids]+[
 ('duration_filter',[2,2,2,1]),('credit_posterior_stress',[2,2,0,3,1,.2]),
 ('robust_mdp',[1.2,1,2,1,0,.9,0,.5,.5,0,.5,.5]),
 ('drawdown_policy',[1,2,2,0,0,1,0,.1,-1,0,0,-1,2,.25,.75,.25,.75]),
 ('asian_additive',[10,10,2,10,1]),('hidden_queue_choice',[1,0,.5,.1,0,1,.5,.8,.5,.1,.2,.1,.2,.1]),
 ('transport_stress',[2,2,-1,.5,.5,0,2,0,1,1,0]),
 ('nonlinear_risk_jet',[0,0,1,2,.3,.2,.1,.2,1])]
assert all(r is None for r in batch(bad))
print(f'PASS: {len(bad)} invalid project inputs')
