import importlib.util
import json
import math
import pathlib
import subprocess
import sys
from collections import Counter

root=pathlib.Path(__file__).resolve().parents[1]
exe=sys.argv[1]
catalog=[c for g in ('filter','pricing') for c in json.loads((root/'quant/research'/f'continuous-{g}-cases.json').read_text(encoding='utf8'))]
ids={c['id'] for c in catalog}
assert len(ids)==8 and ids==set(subprocess.check_output([exe,'--list'],text=True).splitlines())

def batch(rows):
    source='\n'.join(' '.join([m]+[str(x) for x in a]) for m,a in rows)+'\n'
    raw=subprocess.run([exe,'--batch'],input=source,text=True,check=True,capture_output=True).stdout.splitlines()
    assert len(raw)==len(rows),(len(raw),len(rows))
    return [None if x=='ERROR' else list(map(float,x.split(','))) for x in raw]

def verify(result,expected,label,complete=False):
    assert result is not None,label
    if complete and isinstance(expected,list):assert len(result)==len(expected)
    indices=expected.items() if isinstance(expected,dict) else enumerate(expected)
    for j,v in indices:
        j=int(j)
        assert j<len(result) and math.isfinite(result[j]) and math.isclose(result[j],v,rel_tol=2e-8,abs_tol=2e-9),(label,j,result[j] if j<len(result) else None,v)
    return len(expected)

fixed=batch([(c['id'],c['input']) for c in catalog])
coordinates=sum(verify(a,c['expected'],c['id'],True) for a,c in zip(fixed,catalog))
print(f'PASS: {len(catalog)} fixed cases and {coordinates} reference coordinates')
spec=importlib.util.spec_from_file_location('continuous_reference',root/'quant/research/continuous_reference.py')
ref=importlib.util.module_from_spec(spec);spec.loader.exec_module(ref)
studies=list(ref.experiments());results=batch([(m,a) for m,a,_,_ in studies])
assert {m for m,_,_,_ in studies}==ids
count=0;checks=0;coverage=Counter()
for out,(m,a,e,design) in zip(results,studies):
    count+=verify(out,e,(m,a,design));coverage[m]+=1
    if m=='kalman_missing':
        for t in range(int(a[0])):
            f=out[2+10*t:7+10*t];s=out[7+10*t:12+10*t]
            assert f[2]>0 and f[4]>0 and f[2]*f[4]>f[3]**2
            assert s[2]>0 and s[4]>0 and s[2]*s[4]>s[3]**2
            assert s[2]<=f[2]+1e-10 and s[4]<=f[4]+1e-10
            checks+=3
        assert out[-10:-5]==out[-5:];checks+=1
    elif m=='ensemble_analysis':
        assert out[15]<1e-10;checks+=1
        pts=list(zip(out[16::2],out[17::2]))
        assert len(pts)==int(a[0])
        assert all(math.isclose(sum(z[j] for z in pts)/len(pts),out[5+j],abs_tol=1e-10) for j in range(2))
        checks+=2
    elif m=='tridiagonal_adjoint':
        n=int(a[0]);assert max(out[-2:])<1e-10;checks+=1
        b=a[:];j=1+3*n-2;h=1e-5;b[j]+=h
        c=a[:];c[j]-=h
        slope=(ref.adjoint_dense(b)[0]-ref.adjoint_dense(c)[0])/(2*h)
        assert math.isclose(slope,out[1+5*n-2],rel_tol=1e-7,abs_tol=1e-8)
        checks+=1
    elif m=='bs_rannacher':
        assert out[-1]<1e-8;checks+=1
    elif m=='spread_conditional':
        assert 0<=out[1]<=a[0]*math.exp(-a[7]*a[9])+1e-8;checks+=1
    elif m=='ou_coupling':
        assert abs(out[4])<=math.sqrt(out[2]*out[3])+1e-10 and out[5]>=0;checks+=1
for k,v in sorted(coverage.items()):print(k,v)
print(f'PASS: {len(studies)} independent numerical studies, {count} coordinates, {checks} structural certificates')

# Refine temporal and spatial grids together, and verify a nontrivial error contraction.
grids=[30,60,120,240]
vals=batch([('bs_rannacher',[100,100,.02,0,.2,1,1.2,g,g]) for g in grids])
assert abs(vals[-1][4])<abs(vals[0][4])/8
assert abs(vals[-1][9]-vals[-1][10])<abs(vals[0][9]-vals[0][10])/8
print('PASS: independent refinement study for price and Gamma')

bad=[(m,[]) for m in ids]+[('unknown',[1]),('ou_coupling',[1,0,1,1,1,'nan']),
 ('nonlinear_gaussian_moments',[0,0,1,1,1,0,0]),
 ('gaussian_density_filter',[0,1,1,0,1,2,1]),
 ('ensemble_analysis',[2,1,0,1,0,0,0,1,1]),
 ('bs_rannacher',[100,100,.02,0,.2,1,1,9,10]),
 ('tridiagonal_adjoint',[2,1,1,-1,-1,1,0,1,2]),
 ('spread_conditional',[100,100,0,.2,.2,1,0,0,0,1,400]),
 ('ou_coupling',[1,0,0,1,1,10]),
 ('kalman_missing',[1,1,0,0,1,.1,0,.1,0,0,1,0,1,1,1,4,0,0])]
assert all(r is None for r in batch(bad))
print(f'PASS: {len(bad)} malformed and out-of-domain inputs rejected')
