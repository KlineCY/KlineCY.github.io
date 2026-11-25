import importlib.util
import json
import math
import pathlib
import subprocess
import sys
from collections import Counter

root=pathlib.Path(__file__).resolve().parents[1]
exe=sys.argv[1]
groups=['inference','credit','derivatives','policy','exact','queue','optimization','control']
catalog=[c for g in groups for c in json.loads((root/'quant/research'/f'finite-state-{g}-cases.json').read_text(encoding='utf8'))]
ids={c['id'] for c in catalog}
assert len(ids)==32 and set(subprocess.check_output([exe,'--list'],text=True).splitlines())==ids

def batch(rows):
    source='\n'.join(' '.join([m]+[str(x) for x in a]) for m,a in rows)+'\n'
    raw=subprocess.run([exe,'--batch'],input=source,text=True,check=True,capture_output=True).stdout.splitlines()
    assert len(raw)==len(rows),(len(raw),len(rows))
    return [None if x=='ERROR' else list(map(float,x.split(','))) for x in raw]

def verify(result,expected,label,complete=False):
    assert result is not None,label
    if complete: assert len(result)==len(expected),(label,result,expected)
    indices=expected.items() if isinstance(expected,dict) else enumerate(expected)
    for j,v in indices:
        assert j<len(result) and math.isfinite(result[j]) and math.isclose(result[j],v,rel_tol=2e-8,abs_tol=2e-9),(label,j,result[j] if j<len(result) else None,v)
    return len(expected)

fixed=batch([(c['id'],c['input']) for c in catalog])
coordinates=sum(verify(a,c['expected'],(c['id'],c['input']),True) for a,c in zip(fixed,catalog))
print(f'PASS: {len(catalog)} complete hand-derived cases, {coordinates} reference coordinates')
spec=importlib.util.spec_from_file_location('finite_state_reference',root/'quant/research/finite_state_reference.py')
ref=importlib.util.module_from_spec(spec);spec.loader.exec_module(ref)
studies=list(ref.experiments())
assert set(m for m,_,_,_ in studies)==ids
results=batch([(m,a) for m,a,_,_ in studies])
n=0;coverage=Counter()
for result,(m,a,e,design) in zip(results,studies):
    n+=verify(result,e,(m,a,design));coverage[m]+=1
    if m=='duration_study':
        k=int(a[0]);t=int(a[3])
        assert math.isclose(sum(result[3:3+k]),1,abs_tol=1e-9)
        assert math.isclose(sum(result[3+k:3+2*k]),t,abs_tol=1e-8)
        assert result[1]<=result[0]+1e-10
    if m=='inventory_schedule':
        q=int(a[0]);trades=result[5:];rem=q
        for i,z in enumerate(trades):assert z.is_integer() and 0<=z<=min(rem,a[5+4*i]);rem-=z
        assert rem==0
    if m=='lot_rebalance':
        i,j,cash,fees,v,e,obj,closure=result
        assert i.is_integer() and j.is_integer() and cash>=0 and abs(closure)<1e-9
    if m=='transport_flow':
        m0,n0=int(a[0]),int(a[1]);flow=result[3:]
        assert all(v>=0 and v.is_integer() for v in flow)
        assert [sum(flow[i*n0:(i+1)*n0]) for i in range(m0)]==a[2:2+m0]
        assert [sum(flow[i*n0+j] for i in range(m0)) for j in range(n0)]==a[2+m0:2+m0+n0]
    if m=='assignment':
        assert sorted(result[2:])==list(range(int(a[0])))
    if m=='queue_execution_choice':
        assert math.isclose(sum(result[2:5]),1,abs_tol=1e-9)
    if m=='worst_tv':
        assert math.isclose(sum(result[4:]),1,abs_tol=1e-9)
        assert result[2]<=a[0]+1e-10 and result[1]<=result[0]+1e-10
for k,v in sorted(coverage.items()):print(k,v)
print(f'PASS: {len(studies)} independent numerical studies, {n} selected reference coordinates, plus model-specific feasibility certificates')
bad=[(m,[]) for m in ids]+[('unknown',[1]),('count_credit',['nan',.2]),('count_credit',[.95,'0.2x'])]
bad += [
 ('duration_study',[2,2,2,1]),('segmentation',[3,1,1,2]),('bayes_break',[1,1,0,2]),('two_way_within',[2,2,0,0,1,2]),
 ('count_credit',[.9,1.2]),('loss_distribution',[.9,0,1.5,.2]),('tranche_sensitivity',[2,1,1,.2]),('mixture_count',[1,1,1,1,0]),
 ('barrier_rebate',[100,100,1.2,.8,1,2,1,90,-1]),('swing_rights',[100,100,1.2,.8,1,2,1,0,1]),
 ('lookback_max',[100,100,1.2,.8,1,2,-1]),('state_price_chain',[2,1,1,0,-1,1,1,1,.5,.5,.5,.5]),
 ('inventory_schedule',[4,0,0,1,0,2]),('lot_rebalance',[100,0,0,10,20,1,.8,.8,0,4,4]),
 ('regime_policy',[1,2,2,0,0,1,0,1,-.1,.2,.5,.5,.5,.5]),('drawdown_chain',[2,2,0,0,.1,-1,1,.5,.5,.5,.5]),
 ('stratified_exact',[4,2,2,3]),('rank_sum_exact',[3,1,2,3]),('signed_rank_exact',[0,0]),('sequential_test',[4,.8,.2,-1,1,.5]),
 ('tagged_queue',[2,1,0,.6,.6,.1,.1]),('queue_regime',[1,0,0,.9,.2,.2,.3,.2,.1,.1,.1,2]),
 ('queue_execution_choice',[1,0,.8,.5,.1,.1,0,1,.5]),('queue_batch_fill',[2,1,2,1,.8,.8]),
 ('transport_flow',[2,2,1,1,2,2,4,1,2,5]),('band_dtw',[2,3,0,0,1,2,1,2,3]),('assignment',[3,1,2]),('wasserstein',[1,1,1,.6,2,1]),
 ('mdp_optimal',[1,2,1,0,1,0,.2,.2,0,.5,.5]),('policy_iteration',[1,2,1,0,1,0,.5,.5,0,.5,.5]),
 ('worst_tv',[1.2,.5,0,.5,1]),('stopping_policy',[1,2,0,1,-1,1,3,.5,.5,.5,.5])]
errors=[(m,a,r) for (m,a),r in zip(bad,batch(bad)) if r is not None]
assert not errors,errors
print(f'PASS: {len(bad)} rejected malformed or out-of-domain cases')
