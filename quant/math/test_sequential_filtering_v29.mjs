import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const catalog=read("curriculum.json"),manifest=read("lessons-manifest.json");
const added=[...read("lessons-phase-29-a.json").lessons,...read("lessons-phase-29-b.json").lessons];
const targets=["information-n0508","information-n0507","information-n0505","information-n0509","information-n0506","information-n0510"];
const core=["lessons-foundations.json","lessons-advanced.json","lessons-more.json"].flatMap(x=>read(x).lessons);
const extensions=manifest.modules.flatMap(x=>{const a=read(x.path).lessons;assert.equal(a.length,x.lessons,x.path);return a;});
const full=[...core,...extensions],nodeIds=new Map(catalog.nodes.map(x=>[x.id,x]));
assert.deepEqual(added.map(x=>x.id),targets);
assert.equal(manifest.expected_lessons,201);
assert.equal(manifest.modules.length,77);
assert.equal(core.length,123);assert.equal(extensions.length,201);
assert.equal(full.length,324);
assert.equal(new Set(full.map(x=>x.node_id)).size,324);
assert.equal(catalog.nodes.length,656);assert.equal(catalog.chapters.length,109);
let sections=0,formulas=0,refs=0,links=0;
for(const l of added){
 assert(l.id===l.node_id&&nodeIds.get(l.id)?.title===l.title,"canonical lecture and curriculum "+l.id);
 for(const k of ["definition","assumptions","example","application","caveat"])assert(l[k].length>=105,"substantive Chinese body "+l.id+" "+k);
 assert(l.properties.length>=4&&l.formulas.length>=4&&l.derivation.length>=4);
 assert.equal(l.deep_sections.length,5);
 assert.equal(new Set(l.deep_sections.map(x=>x.title)).size,5);
 for(const f of l.formulas){assert(f.label&&f.tex);katex.renderToString(f.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++;}
 for(const d of l.derivation){assert(d.explanation.length>=55&&d.tex,"proof narrative "+l.id);katex.renderToString(d.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++;}
 for(const s of l.deep_sections){
  assert(s.title.length>=5&&s.question.length>=12&&s.paragraphs.length>=2);
  assert(s.paragraphs.every(p=>p.length>=35));
  assert(s.proof.length>=45&&s.worked.length>=25&&s.limitation.length>=25);
  assert(s.formulas.length>=1);
  for(const f of s.formulas){katex.renderToString(f.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++;}
  sections++;
 }
 for(const a of l.links){assert(a.kind==="math"&&nodeIds.has(a.id),"broken canonical topic link "+l.id+" -> "+a.id);links++;}
 for(const s of l.sources){assert(/^https:\/\//.test(s.url)&&s.title.length>=19,"incomplete reference "+l.id);refs++;}
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)));
 assert(!/Lorem ipsum|TODO|占位|待填/.test(JSON.stringify(l)));
}
assert.equal(sections,30);assert.equal(formulas,108);assert.equal(links,28);assert.equal(refs,12);
const near=(a,b,k,tol=1e-9)=>assert(Math.abs(a-b)<=tol,"V29 checked computation "+k+": "+a+" != "+b);
near(9/(36*7),1/28,"V44 posterior parameter variance");
near(.4375*.74/(.4375*.74+.5625*.32),.6426799007444169,"V44 HMM smoothing",1e-9);
near(.5/Math.sqrt(1.25),.4472135954999579,"V44 innovation zscore");
near(.8*2,1.6,"state prediction mean");
near(.8**2+.36,1,"state prediction variance");
near(1/(1+.25),.8,"Kalman gain");
near(1.6+.8*(2.1-1.6),2,"state update mean");
near((1-.8)*1,.2,"state update variance");
near(.5*.4,.2,"Bayes two observations evidence");
near(.14/.32,.4375,"HMM posterior state0");
near(.18/.32,.5625,"HMM posterior state1");
near(.4375*.9+.5625*.2,.50625,"HMM next state probability");
near(Math.log(.32),-1.1394342831883648,"HMM log evidence");
near(2**2+1,5,"nonlinear propagated mean");
near(2*1**2+4*2**2*1,18,"nonlinear true variance");
near((2*2)**2*1,16,"EKF local variance");
near(1/((4/6)**2+(1/6)**2+(1/6)**2),2,"particle ESS");
near((4*1+1*2+1*4)/6,5/3,"particle weighted mean");
near(1-99/110,.1,"path observed drawdown");
near(1-105/110,1/22,"path rebound drawdown");
near(1-94.5/110,.14090909090909087,"max drawdown",1e-8);
for(const [id,token] of [
 ["information-n0508","1.6+0.8×(2.1−1.6)=2.0"],
 ["information-n0507","即 0.2"],["information-n0505","0.4375"],
 ["information-n0509","1+4=5"],["information-n0506","ESS=1/0.5=2"],
 ["information-n0510","0.140909"]
])assert(added.find(l=>l.id===id).example.includes(token),"example evidence missing "+id);
const mathHome=fs.readFileSync(new URL("index.html",root),"utf8"),mlHome=fs.readFileSync(new URL("../ml/index.html",root),"utf8");
assert(mathHome.includes('aria-label="V29状态空间与序贯滤波六课"'));
assert(mlHome.includes('aria-label="V29状态估计与序贯概率学习交叉路径"'));
for(const id of targets){
 assert(mathHome.includes("?topic="+id),"no math path "+id);
 assert(mlHome.includes("../math/?topic="+id),"no ML crosslink "+id);
}
const css=fs.readFileSync(new URL("styles.css",root),"utf8");
const mlcss=fs.readFileSync(new URL("../ml/atlas.css",root),"utf8");
for(const marker of ["overflow-x:auto","line-break:strict","@media print",":focus-visible"])assert(css.includes(marker),"CJK/KaTeX/layout rule "+marker);
assert(mlcss.includes("overflow-x:auto"));
console.log("PASS V44/V29: 6 live lessons, 30 deep proof units, 108 strict KaTeX expressions, 12 source references, 28 canonical lesson crosslinks, 18 independent numeric assertions, 318 unique complete courses.");
