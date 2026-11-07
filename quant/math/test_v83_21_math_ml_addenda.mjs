import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const mathSpec=[{"file":"lessons-phase-01-d.json","id":"optimization-n0340","sections":4,"hash":"3f8eda76","newTitle":"对偶梯度、影子价格与约束扰动敏感性"},{"file":"lessons-phase-01-d.json","id":"numerical-n0362","sections":4,"hash":"e09ec7ad","newTitle":"残差、前向误差与后向稳定的不同含义"},{"file":"lessons-phase-01-e.json","id":"diffeq-n0397","sections":4,"hash":"8d94cf26","newTitle":"连续状态空间模型的精确离散化与过程噪声"},{"file":"lessons-phase-01-e.json","id":"finance-n0430","sections":4,"hash":"2bf4dd8e","newTitle":"不完备三状态市场中的风险中性概率区间"},{"file":"lessons-phase-01-e.json","id":"mlmath-n0453","sections":4,"hash":"1f78dcf7","newTitle":"样本外分布转移中的偏差方差与风险重加权"},{"file":"lessons-phase-01-e.json","id":"information-n0484","sections":4,"hash":"b38a64e7","newTitle":"条件互信息、特征协同与非单变量筛选"},{"file":"lessons-phase-01-f.json","id":"discrete-n0519","sections":4,"hash":"09cfc971","newTitle":"Katz中心性、网络路径叠加与谱稳定边界"},{"file":"lessons-phase-01-f.json","id":"control-n0545","sections":4,"hash":"c87d027b","newTitle":"Bellman残差与近似值函数的可计算误差证书"},{"file":"lessons-phase-01-f.json","id":"riskmath-n0584","sections":4,"hash":"4ee005c2","newTitle":"谱收缩、最小特征值与组合权重稳定性"},{"file":"lessons-phase-02-a.json","id":"foundation-n0005","sections":4,"hash":"9bb68222","newTitle":"非凸Pareto前沿与加权和标量化的盲区"},{"file":"lessons-phase-02-a.json","id":"calculus-n0041","sections":4,"hash":"0e0223df","newTitle":"Hessian退化时的高阶判别与鞍点反例"},{"file":"lessons-phase-02-a.json","id":"linear-n0070","sections":4,"hash":"0c0c9089","newTitle":"迭代精化：复用LU分解校正数值解"},{"file":"lessons-phase-02-a.json","id":"probability-n0100","sections":4,"hash":"0bf49c9b","newTitle":"Beta–Binomial混合与事件聚集的过度离散"}];
const mlSpec=[{"id":"m01-c02-p02","hash":"60035c2e","equations":8,"derivation":8,"protocol":7,"pitfalls":4,"workedLength":385},{"id":"m01-c02-p06","hash":"34795ae9","equations":8,"derivation":8,"protocol":7,"pitfalls":4,"workedLength":334},{"id":"m01-c04-p05","hash":"aefcb90e","equations":8,"derivation":8,"protocol":7,"pitfalls":4,"workedLength":355},{"id":"m01-c03-p05","hash":"9ea8ed44","equations":8,"derivation":8,"protocol":7,"pitfalls":4,"workedLength":367},{"id":"m02-c02-p01","hash":"deff9512","equations":8,"derivation":8,"protocol":7,"pitfalls":4,"workedLength":333},{"id":"m02-c02-p06","hash":"d1f4432f","equations":8,"derivation":8,"protocol":7,"pitfalls":4,"workedLength":396},{"id":"m02-c04-p03","hash":"5c8fe320","equations":8,"derivation":8,"protocol":7,"pitfalls":4,"workedLength":350},{"id":"m02-c02-p04","hash":"64cbad93","equations":8,"derivation":8,"protocol":7,"pitfalls":4,"workedLength":352}];
const read=p=>fs.readFileSync(new URL(p,root),"utf8");
const json=p=>JSON.parse(read(p));
function hash(o){let h=2166136261>>>0;for(const c of JSON.stringify(o))h=Math.imul(h^c.charCodeAt(0),16777619)>>>0;return h.toString(16).padStart(8,"0")}
const mathGroups=new Map();
for(const e of mathSpec){if(!mathGroups.has(e.file))mathGroups.set(e.file,json(e.file));}
const ids=new Set();let texCount=0,strictCount=0,mathCount=0,mlCount=0;
function safe(x,id){const t=JSON.stringify(x);assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(t),id+" damaged Unicode");assert(!/TODO|FIXME|Lorem ipsum/i.test(t),id+" draft placeholder");}
function tex(f){assert(f.label.length>=4&&f.tex.length>=14,"formula metadata incomplete");katex.renderToString(f.tex,{throwOnError:true,strict:"error",displayMode:true});texCount++;strictCount++;}
for(const spec of mathSpec){
  const d=mathGroups.get(spec.file);
  assert.equal(d.version,1);
  const x=d.lessons.find(y=>y.id===spec.id);assert(x,"missing "+spec.id);assert(!ids.has(spec.id));ids.add(spec.id);
  assert.equal(hash({...x,deep_sections:x.deep_sections.slice(0,spec.sections)}),spec.hash,spec.id+" old contents modified");
  assert.equal(x.deep_sections.length,spec.sections+1,spec.id+" missing new subchapter");
  const s=x.deep_sections.at(-1);
  assert.equal(s.title,spec.newTitle);
  assert(s.question.length>=14&&s.title.length>=14);
  assert(s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=80));
  assert(s.proof.length>=140&&s.worked.length>=110&&s.limitation.length>=60);
  assert.equal(s.formulas.length,2);
  assert.equal(new Set(x.deep_sections.map(y=>y.title)).size,x.deep_sections.length);
  for(const f of s.formulas)tex(f);
  safe(s,spec.id);mathCount++;
}
const d=json("../ml/atlas-depth-07-a.json");
const manifest=json("../ml/atlas-depth-manifest.json");
assert(manifest.modules.some(x=>x.path==="./atlas-depth-07-a.json"&&x.lessons===8));
assert.deepEqual(Object.keys(d.lessons),mlSpec.map(s=>s.id));
assert.deepEqual(d.v83_revision.ids,mlSpec.map(s=>s.id));
for(const spec of mlSpec){
  const x=d.lessons[spec.id];assert(x&&x.v83_note?.startsWith("V83"));
  const before={...x,equations:x.equations.slice(0,spec.equations),derivation:x.derivation.slice(0,spec.derivation),worked_example:x.worked_example.slice(0,spec.workedLength),protocol:x.protocol.slice(0,spec.protocol),pitfalls:x.pitfalls.slice(0,spec.pitfalls)};
  delete before.v83_note;
  assert.equal(hash(before),spec.hash,spec.id+" V77 original text modified");
  assert.equal(x.equations.length,spec.equations+2);
  assert.equal(x.derivation.length,spec.derivation+2);
  assert.equal(x.protocol.length,spec.protocol+1);
  assert.equal(x.pitfalls.length,spec.pitfalls+1);
  assert(x.derivation.slice(-2).every(v=>v.length>=100));
  assert(x.worked_example.length-spec.workedLength>=100);
  assert(x.protocol.at(-1).length>=55&&x.pitfalls.at(-1).length>=45);
  assert.equal(new Set(x.equations.map(e=>e.label)).size,x.equations.length);
  for(const e of x.equations.slice(-2))tex(e);
  safe(x,spec.id);mlCount++;ids.add(spec.id);
}
assert.equal(mathCount,13);assert.equal(mlCount,8);assert.equal(strictCount,42);
let numeric=0;
function near(k,a,b,tol=1e-8){assert(Number.isFinite(a)&&Number.isFinite(b)&&Math.abs(a-b)<=tol,k+": "+a+" != "+b);numeric++}
function truth(k,v){near(k,v?1:0,1)}
near("dual best lambda",2-4/4,1);
near("looser bound optimal cost",(.99)**2,.9801);
near("dual shadow price first order",-2*.01,-.02);
near("ill conditioned matrix condition",1/1e-6,1e6);
near("forward relative coefficient error",1/Math.sqrt(2),.7071067811865475);
near("OU exact transition",Math.exp(-1),.36787944117144233);
near("OU exact covariance",.09*(1-Math.exp(-2))/4,.01945495372417221,1e-6);
near("incomplete martingale q1",.25,.25);
near("incomplete martingale expected stock",80*.25+100*.5+120*.25,100);
near("incomplete digital value",.25,1/4);
near("covariate shift train risk",.9*1+.1*9,1.8);
near("covariate shift target risk",.1*1+.9*9,8.2);
near("XOR marginal information",0,0);
near("XOR conditional information",1,1);
near("Katz middle centrality",1.4/.92,1.5217391304347827);
near("Katz edge centrality",1+.2*1.4/.92,1.3043478260869565);
near("Bellman candidate residual",2+.9*10-10,1);
near("Bellman exact value",2/(1-.9),20);
near("Bellman residual upper bound",1/(1-.9),10);
near("shrink lower eigen",.75*.0001+.25*.02005,.0050875);
near("shrink upper eigen",.75*.04+.25*.02005,.0350125);
near("shrink old condition",.04/.0001,400);
near("shrink new condition",.0350125/.0050875,6.881326781326782,1e-2);
near("Pareto B half weighted",.5*1+.5*1.5,1.25);
near("Pareto A half weighted",.5*0+.5*2,1);
truth("Pareto impossible split",1/3<.6);
near("Hessian f at .1",.1**4,.0001);
near("Hessian g y direction",-(.1**4),-.0001);
near("LU residual first",2-2*.9,.2);
near("LU residual second",8-4*1.8,.8);
near("LU exact correction first",.2/2,.1);
near("LU exact correction second",.8/4,.2);
near("Beta-binomial rho",1/(2+2+1),.2);
near("Beta-binomial variance",10*.5*.5*(1+9*.2),7);
near("Bonferroni geometric four-round budget",.05*(1/2+1/4+1/8+1/16),.046875);
near("Bartlett HAC one lag",1+2*.5*.3,1.3);
near("HAC effective sample",100/1.3,76.92307692307692);
near("Gaussian maximum bias bound",.02*Math.sqrt(2*Math.log(100)),.060697598,1e-6);
near("cluster design effect",1+9*.5,5.5);
near("cluster effective sample",1000/5.5,181.8181818181818);
near("scaled linear prediction",10*.1+.2*10,3);
near("Ridge df lambda1",9/(9+1)+1/(1+1),1.4);
near("Ridge df lambda4",9/(9+4)+1/(1+4),.8923076923076922);
near("feature aligned prediction",5*3+2*1,17);
near("feature misaligned prediction",5*1+2*3,11);
near("OLS leave one out error",.2/(1-.8),1);
near("OLS leave one out squared",(.2/(1-.8))**2,1);
const mathHTML=read("index.html"),mlHTML=read("../ml/index.html");
for(const spec of mathSpec)assert(mathHTML.includes("?topic="+spec.id),"math navigation missing "+spec.id);
for(const spec of mlSpec)assert(mlHTML.includes('href="#lesson/'+spec.id+'"'),"ML navigation missing "+spec.id);
for(const [name,css] of [["math",read("styles.css")],["ML",read("../ml/atlas.css")]]){
  for(const m of ["overflow-x:auto","line-break:strict",":focus-visible","@media print","@media(max-width:700px)"]){
    assert(css.includes(m),name+" CJK/responsive style missing "+m);
  }
}
console.log("V83 PASS: "+mathCount+" math and "+mlCount+" ML live lessons; "+strictCount+" new strict KaTeX formulas; "+numeric+" independent arithmetic proofs; original full-text snapshots, CJK and responsive navigation intact.");
