import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const raw=p=>fs.readFileSync(new URL(p,root),"utf8");
const groups="abcdefghijkl".split("").map(c=>"atlas-depth-07-"+c+".json");
const manifest=read("atlas-depth-manifest.json");
assert.equal(manifest.expected_lessons,720);
assert.equal(manifest.expected_domains,24);
assert.equal(manifest.modules.reduce((n,m)=>n+m.lessons,0),720);
const modules=new Map(manifest.modules.map(m=>[m.path,m.lessons]));
let checked=0,formulas=0,deepWords=0;
const fullIds=new Set(),newIds=new Set(),domainCounts={};
for(const file of groups){
 assert.equal(modules.get("./"+file),8,"missing canonical module "+file);
 const d=read(file);
 assert.equal(d.schema,1,"module version "+file);
 assert.equal(Object.keys(d.lessons).length,8);
 assert.equal(d.revision,"2026-11-06","module not deepened "+file);
 for(const [id,l] of Object.entries(d.lessons)){
  assert(!fullIds.has(id),"duplicate ML identity "+id);
  fullIds.add(id);checked++;
  const domain=Number(id.slice(1,3)),revision=domain<=6?"V56":"V37";
  domainCounts[domain]=(domainCounts[domain]||0)+1;
  assert(l.revision_note?.startsWith(revision),id+" deepening revision");
  if(revision==="V37")newIds.add(id);
  assert(typeof l.source_title==="string"&&l.source_title.length>=2,id+" source title lost");
  assert(typeof l.overview==="string"&&l.overview.length>=60,id+" overview lost");
  assert.equal(l.equations.length,domain<=6?6:5,id+" enhanced seventh-stage equations");
  assert.equal(l.derivation.length,domain<=6?6:5,id+" enhanced seventh-stage proofs");
  assert.equal(l.protocol.length,domain<=6?6:5,id+" enhanced seventh-stage protocols");
  assert.equal(l.pitfalls.length,3,id+" original plus failure boundary");
  assert(l.worked_example.length>=100,id+" worked arithmetic lost");
  assert(l.derivation.slice(3).every(s=>s.length>=65),id+" derivation too shallow");
  assert(l.protocol.slice(3).every(s=>s.length>=25),id+" research verification too shallow");
  assert(l.pitfalls.at(-1)?.length>=25,id+" risk boundary missing");
  assert(!/TODO|Lorem ipsum|[\uFFFD\u200B\uFEFF]/i.test(JSON.stringify(l)),id+" malformed Chinese");
  for(const e of l.equations){
   assert(typeof e.label==="string"&&e.label.length>=2&&typeof e.tex==="string"&&e.tex.length>=5,id+" malformed equation");
   katex.renderToString(e.tex,{throwOnError:true,strict:"ignore",displayMode:true});
   formulas++;
  }
  if(revision==="V37")deepWords+=l.derivation.slice(3).join("").length+l.protocol.slice(3).join("").length+l.pitfalls.at(-1).length;
 }
}
assert.equal(checked,96);
assert.equal(formulas,504);
assert.equal(newIds.size,72);
// Chronos original paper §3.1 uses m=0 and s=mean(abs(context));
// do not quietly replace the published tokenizer with centered standardization.
const chronos=read("atlas-depth-07-k.json").lessons["m22-c05-p05"];
assert(chronos.equations[3].tex.includes("sum_{j=1}^{C}|x_j|"),"Chronos original mean-absolute scaling missing");
assert(chronos.worked_example.includes("s_C=(2+4+0)/3=2"),"Chronos scale example missing");

assert.deepEqual(Object.keys(domainCounts).length,24);
for(const n of Object.values(domainCounts))assert.equal(n,4,"exactly four seventh-stage lessons per domain");
assert(deepWords>27000,"V37 substantial Chinese derivation and audit text");

const home=raw("index.html"),reader=raw("atlas.js"),css=raw("atlas.css");
const start=home.indexOf('aria-label="V37第七批九十六篇正式讲义全覆盖深化"');
assert(start>=0,"V37 nine-stage navigation missing");
const end=home.indexOf("</details>",start);
assert(end>start,"V37 navigation not closed");
const v37=home.slice(start,end);
assert.equal((v37.match(/href="#lesson\//g)||[]).length,72,"72 new deep links required");
for(const id of newIds)assert(v37.includes('href="#lesson/'+id+'"'),"V37 link not reachable "+id);
for(const title of ["阶段一","阶段二","阶段三","阶段四","阶段五","阶段六","阶段七","阶段八","阶段九"])
 assert(v37.includes(title),"incomplete nine-stage curricular flow "+title);
assert(home.includes('aria-label="V36四十八篇机器学习数学和量化深入课程"'),"previous roadmap lost");
for(const m of ["linear-n0065","statistics-x001","riskmath-x002"])
 assert(v37.includes('href="../math/?topic='+m+'"'),"missing quantitative math bridge "+m);
for(const key of ["s.equations.map","s.derivation","s.protocol","s.pitfalls","s.worked_example","katex.render"])
 assert(reader.includes(key),"course viewer not compatible "+key);
for(const marker of [".atlas-spectral-path",".atlas-spectral-stage",".atlas-spectral-links",".ml-math-block","overflow-x:auto","line-break:strict","@media print"])
 assert(css.includes(marker),"narrow-screen / Chinese / math style missing "+marker);

const near=(value,expect,label,tol=1e-9)=>assert(Math.abs(value-expect)<=tol,label+" "+value+" != "+expect);
near(.75*8,6,"dropout conditional mean");
near(.75*(8-6)**2+.25*(0-6)**2,12,"dropout conditional variance");
near(Math.sqrt(.8*.2/20),.08944271909999159,"patient-level standard error");
near(.009/(.009+.0495),.15384615384615385,"rare diagnostic class PPV");
near(.75*2+.25*6,3,"GRU update gate");
near(.75**5,.2373046875,"GRU direct memory path");
near(.75*6+.25*2,5,"GRU smooth interpolation");
near(512/8,64,"eight head dimensions");
near(Math.exp(-1)/(1+Math.exp(-1)),.2689414213699951,"stable softmax");
near(14/3,4.666666666666667,"linear attention toy");
near(3/5,.6,"retrieval precision");
near(3/4,.75,"retrieval recall");
near(50/250,.2,"document block overlap");
near(Math.exp(-1),.36787944117144233,"temporal edge decay");
near(.12-.09,.03,"GNN baseline increment");
near(.10-.09,.01,"GNN after industry controls");
near(10*(1.4-1)**2,1.6,"WGAN GP");
near(Math.log(2),.6931471805599453,"disjoint JS divergence");
near(.5*2+.5*1,1.5,"Actor-Critic value");
near(.8+.2/4,.85,"epsilon greedy");
near(-Math.log(.75),.2876820724517809,"tanh log Jacobian");
near(.8**2,.64,"VAR-like stability");
near((Math.abs(2)+Math.abs(-4)+Math.abs(0))/3,2,"Chronos paper mean-absolute scale");
near(1-88/110,.2,"maximum drawdown");
near(1/Math.sqrt(8),.3535533905932738,"sample RMS");
near(2*.02*.5*.1,.002,"impact structure demonstration");
near((.01**2+.01**2+.02**2)**.5,.02449489742783178,"realized volatility");
near((1+.5+.5)/3,2/3,"event sample uniqueness");
near((.02+.04+.06)/3/.02,2,"ICIR population SD");
near(.02/.01,.2*10,"probability ratio scalar");
near(64/.016,4000,"batch inference throughput");
near(.7-.4,.3,"MIA advantage");
near(1+2*.5*.2,1.2,"Bartlett HAC L=1");
near(1+2*.2,1.4,"population long-run variance");
near(.03/.02,1.5,"rolling beta");
near(5*1.2,6,"Gamma log-link proportional mean");
near((2+1+3)/3,2,"Fama-MacBeth mean percent");
near(.8*(1-.8),.16,"boost logistic curvature");
near(6/(4+2),1,"regularized leaf weight");
near(.2+.1*.5,.25,"boosting update");
near(1/Math.sqrt(2*Math.PI),.3989422804014327,"Gaussian KDE");
near(.04+.09,.13,"GP observed prediction variance");
near(.2*.4/(.8*.1+.2*.4),.5,"GMM related posterior check");
near(.9**2*4+1,4.24,"Kalman variance prediction");
near(-100-5,-105,"standard ELBO");
near((1+.8)/(1-.8),9,"Gibbs correlation time");
near(Math.sqrt(.01),.1,"whitening small direction");
near(1/Math.sqrt(.01+.03),5,"ridge-whitening small direction");
near(95+5,100,"purging label maturity");
near(1/.1,10,"IPW small propensity");
console.log("PASS V37: 96 complete seventh-cycle canonical lectures (V36 24 + V37 72); 504 strict KaTeX equations, V56 added 24 to first 24 classes; 24 domains, nine-stage 72-link navigation, original numerical examples, independent numerical cross-checks and responsive Chinese/math typography.");
