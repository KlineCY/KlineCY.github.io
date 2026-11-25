import fs from 'node:fs';
import path from 'node:path';
import assert from 'node:assert/strict';
import {spawnSync} from 'node:child_process';
import {methods as kernel} from '../quant/research/predictive_kernel_reference.mjs';
import {methods as online} from '../quant/research/predictive_online_reference.mjs';
import {methods as neural} from '../quant/research/predictive_neural_reference.mjs';
import {methods as portfolio} from '../quant/research/predictive_portfolio_reference.mjs';
import {methods as optimization} from '../quant/research/predictive_optimization_reference.mjs';
const root='quant/research/',read=p=>fs.readFileSync(p,'utf8');
const methods={...kernel,...online,...neural,...portfolio,...optimization};
const groups=['kernel','online','neural','portfolio','optimization'];
const exe=process.argv[2];assert(exe);
const cat=groups.flatMap(g=>JSON.parse(read(root+'predictive-'+g+'-catalog.json')));
const cases=cat.flatMap(c=>JSON.parse(read(root+c.cases)).map(z=>({id:c.id,...z})));
const result=spawnSync(exe,['--batch'],{input:cases.map(c=>[c.id,...c.input].join(' ')).join('\n')+'\n',encoding:'utf8',maxBuffer:64*1024*1024});
assert.equal(result.status,0,result.stderr);const rows=result.stdout.trim().split('\n');assert.equal(rows.length,cases.length);
let coords=0,invariants=0;const errors=[];
const close=(x,y,tol)=>Number.isFinite(x)&&Number.isFinite(y)&&Math.abs(x-y)<=tol*Math.max(1,Math.abs(y));
for(let i=0;i<cases.length;i++){
 const c=cases[i],ref=methods[c.id](c.input);
 assert.equal(ref.length,c.expected.length);
 const referenceTol=c.id==='gp_derivative'?5e-7:1e-7;
 ref.forEach((v,j)=>assert(close(v,c.expected[j],referenceTol),c.id+' reference dataset '+j));
 if(rows[i]==='ERROR'){errors.push(c.id+' unexpected ERROR '+i);continue;}
 const actual=rows[i].split(',').map(Number);if(actual.length!==ref.length){errors.push(c.id+' length '+i+': '+actual.length+' / '+ref.length);continue;}
 const tol=c.id==='gp_derivative'?5e-6:3e-7;
 for(let j=0;j<actual.length;j++){if(!close(actual[j],ref[j],tol)&&errors.length<50)errors.push(c.id+' case '+i+' coordinate '+j+': '+actual[j]+' != '+ref[j]);coords++;}
 const x=c.input,a=actual,n=x[0],sum=v=>v.reduce((s,z)=>s+z,0);
 function inv(ok,name){if(!ok&&errors.length<50)errors.push(c.id+' '+name+' case '+i);invariants++;}
 if(c.id==='kernel_center'){const K=a.slice(2+n);for(let j=0;j<n;j++)inv(Math.abs(sum(K.slice(j*n,(j+1)*n)))<1e-8,'centered row');}
 if(c.id==='fourier_gram'){for(let j=0;j<n;j++)inv(close(a[2+j*n+j],1,1e-9),'unit diagonal');}
 if(c.id==='gp_derivative'){inv(a[2]>=-1e-8&&a[3]>=-1e-8&&a[2]*a[3]>=a[4]*a[4]-1e-8,'posterior PSD');}
 if(c.id==='fixed_share'){const k=n;for(let j=3;j<a.length-k;j+=k+1)inv(close(sum(a.slice(j+1,j+1+k)),1,1e-9),'probability');}
 if(c.id==='adaptive_miscoverage')inv(Math.abs(a[3])<1e-9,'telescoping');
 if(c.id==='sparsemax_vjp')inv(close(a[2],1,1e-9)&&Math.abs(a[3])<1e-9,'projection and gradient');
 if(c.id==='layernorm_vjp')inv(Math.abs(a[3])<1e-9,'translation invariance');
 if(c.id==='causal_attention_vjp'){for(let j=0;j<n;j++){const p=a.slice(1+j*n,1+(j+1)*n);inv(close(sum(p),1,1e-9)&&p.slice(j+1).every(v=>v===0),'causal support');}}
 if(c.id==='stacking_simplex')inv(close(a[3],1,1e-9)&&a.slice(4,4+x[1]).every(v=>v>=0),'simplex');
 if(c.id==='neutral_layer_vjp')inv(Math.abs(a[2])+Math.abs(a[3])<1e-8,'neutrality');
 if(c.id==='spo_plus')inv(a[0]>=-1e-9&&a[1]+1e-9>=a[0],'decision bound');
 if(c.id==='policy_dr')inv(a[5]>0&&a[5]<=((x.length-2)/8)+1e-8,'ESS');
 if(c.id==='soft_portfolio_vjp')inv(Math.abs(a[4])<1e-9&&close(sum(a.slice(5,5+n)),1,1e-9),'budget gradient');
 if(c.id==='hutchinson_full')inv(close(a[0],a[2],1e-9)&&close(a[1],a[3],1e-9),'trace moments');
 if(c.id==='logistic_hvp')inv(a[1]>=-1e-9,'PSD curvature');
 if(c.id==='cg_ridge')inv(a[1]<1e-7,'normal-equation residual');
 if(c.id==='ensemble_risk')inv(a[1]>=0&&a[2]>=0&&close(a[3],a[1]+a[2],1e-9),'total variance');
}
assert.equal(errors.length,0,errors.join('\n'));
const bad=cat.map(c=>c.id).concat(['unknown','kernel_center nan 1 1 1 2','exp3_observed 2 0 .1 0 1','causal_attention_vjp 2 2 1','spo_plus 1 2 1 2 .1 .1','stacking_simplex 2 2 0 1 2 3 4 5 6','policy_dr 2 .1 0 0 1 1 0 1 1 1','gp_derivative 1 1 0 0 0 1','harmonic_graph 2 1 0 1 0 1']);
const b=spawnSync(exe,['--batch'],{input:bad.join('\n')+'\n',encoding:'utf8'});
assert.equal(b.status,0);assert.deepEqual(b.stdout.trim().split('\n'),bad.map(()=> 'ERROR'));
assert.deepEqual(spawnSync(exe,['--list'],{encoding:'utf8'}).stdout.trim().split('\n').sort(),Object.keys(methods).sort());
console.log('PASS: '+cat.length+' native methods, '+cases.length+' independent parameter cases, '+coords+' output coordinates, '+invariants+' structural checks, '+bad.length+' rejected invalid inputs');
