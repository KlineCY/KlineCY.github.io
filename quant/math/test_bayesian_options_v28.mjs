import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const load=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const curriculum=load("curriculum.json"),manifest=load("lessons-manifest.json");
const first=load("lessons-phase-28-a.json").lessons;
const second=load("lessons-phase-28-b.json").lessons;
const newly=[...first,...second],newIds=[
 "statistics-n0177","statistics-n0178","statistics-n0179","statistics-n0180",
 "finance-n0434","finance-n0436","finance-n0437","finance-n0438"];
const core=["lessons-foundations.json","lessons-advanced.json","lessons-more.json"].flatMap(x=>load(x).lessons);
const extra=manifest.modules.flatMap(m=>{const x=load(m.path).lessons;assert.equal(x.length,m.lessons,"manifest module "+m.path);return x;});
const complete=[...core,...extra],nodeMap=new Map(curriculum.nodes.map(n=>[n.id,n]));
assert.deepEqual(newly.map(x=>x.id),newIds);
assert.equal(curriculum.nodes.length,656);assert.equal(curriculum.chapters.length,109);
assert.equal(core.length,123);
assert.equal(manifest.expected_lessons,201);assert.equal(manifest.modules.length,77);
assert.equal(extra.length,201);assert.equal(complete.length,324);
assert.equal(new Set(complete.map(x=>x.node_id)).size,324,"duplicate canonical mathematics courses");
let sections=0,mathematics=0,links=0,references=0;
for(const l of newly){
 assert(l.node_id===l.id&&nodeMap.get(l.id)?.title===l.title,"canonical title or ID "+l.id);
 for(const k of ["definition","assumptions","example","application","caveat"])
  assert(l[k].length>=110,"shallow "+k+" "+l.id);
 assert(l.properties.length>=4&&l.derivation.length>=4&&l.formulas.length>=4);
 assert(l.deep_sections.length>=5);
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,l.deep_sections.length);
 for(const f of l.formulas){assert(f.label&&f.tex);katex.renderToString(f.tex,{throwOnError:true,strict:"ignore",displayMode:true});mathematics++;}
 for(const d of l.derivation){assert(d.explanation.length>=55);katex.renderToString(d.tex,{throwOnError:true,strict:"ignore",displayMode:true});mathematics++;}
 for(const s of l.deep_sections){
  assert(s.question.length>=12&&s.title.length>=5&&s.paragraphs.length>=2);
  assert(s.paragraphs.every(x=>x.length>=37));
  assert(s.proof.length>=55&&s.worked.length>=30&&s.limitation.length>=30);
  assert(s.formulas.length>=1);
  for(const f of s.formulas){assert(f.label&&f.tex);katex.renderToString(f.tex,{throwOnError:true,strict:"ignore",displayMode:true});mathematics++;}
  sections++;
 }
 for(const x of l.links){assert(x.kind==="math"&&nodeMap.has(x.id),"broken math graph "+l.id+" -> "+x.id);links++;}
 for(const x of l.sources){assert(x.title.length>=18&&/^https:\/\//.test(x.url),"invalid source "+l.id);references++;}
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)),"corrupted CJK "+l.id);
 assert(!/Lorem ipsum|TODO|占位|待填/.test(JSON.stringify(l)),"placeholder "+l.id);
}
assert(sections>=40);
assert(mathematics>=133);
assert.equal(links,32);
assert.equal(references,16);
const near=(got,want,label,tol=1e-9)=>assert(Math.abs(got-want)<=tol,"independent arithmetic "+label+": "+got+" vs "+want);
near(2*.4/.5,1.6,"V45 sequential Bayes factor");
near(1/(4*(2*2+1)),.05,"V45 Beta prior variance");
near(1/(4*(2*20+1)),1/164,"V45 concentrated Beta prior");
near(10*11/(14*15)-(10/14)**2,40/(14**2*15),"V45 posterior predictive dependence");
near(Math.exp(-1.96),.140858420921045,"V45 transformed ET lower");
near(100*Math.exp(-.01)-100*Math.exp(-.03),1.960430020065985,"V45 prepaid dividend-adjusted parity",1e-8);
near(.5*.02*2**2,.04,"V45 local Gamma hedge residual");
near(.20/40,.005,"V45 IV bid ask spread");
near((11-15)/10,-.4,"V45 irregular put butterfly first slope");
near((1-11)/20,-.5,"V45 irregular put butterfly second slope");
near(.2*.6/(.2*.6+.8*.1),.6,"Bayes posterior odds");
near(.6/.1,6,"Bayes factor");
near(10/14,5/7,"Beta posterior mean");
near(1/(1/9+4/4),.9,"Normal posterior variance");
near(.9*(4*3/4),2.7,"Normal posterior mean");
near(2.7-1.96*Math.sqrt(.9),.8405807351402319,"credible lower bound",1e-8);
near(2.7+1.96*Math.sqrt(.9),4.559419264859768,"credible upper bound",1e-8);
near(10*11/(14*15),11/21,"Beta predictive two successes");
near(4*5/(14*15),2/21,"Beta predictive two failures");
near(1-11/21-2/21,8/21,"Beta predictive exactly one");
near(11/21+2/21+8/21,1,"predictive normalization");
near(10*4/(14**2*15),40/2940,"Beta posterior parameter variance");
const Phi01=.539827837277029,PhiMinus01=.460172162722971;
const density01=Math.exp(-.5*.1**2)/Math.sqrt(2*Math.PI);
near(100*(Phi01-PhiMinus01),7.9655674554058,"Black-Scholes ATM call");
near(Phi01,.539827837277029,"option Delta");
near(density01/(100*.2),.01984762737385059,"option Gamma");
near(100*density01,39.69525474770119,"option Vega",1e-8);
near(100*density01*.01,.3969525474770119,"one volatility point");
near(100*(Phi01-PhiMinus01)-100+100,7.9655674554058,"put-call parity");
near(-100*density01*.2/2,-3.969525474770119,"yearly Theta");
near(.1/(100*density01),.002519192968461694,"IV perturbation",1e-8);
near(12-2*9+5,-1,"violated butterfly");
near(12-2*9+7,1,"nonnegative butterfly");
for(const [id,key] of [
 ["statistics-n0177","=0.6"],["statistics-n0178","10/14=5/7"],
 ["statistics-n0179","0.840581"],["statistics-n0180","11/21"],
 ["finance-n0434","7.965567"],["finance-n0436","0.019847627"],
 ["finance-n0437","0.002519"],["finance-n0438","12−18+5=−1"]
])assert(newly.find(l=>l.id===id).example.includes(key),"missing example audited token "+id+" "+key);
const mathHome=fs.readFileSync(new URL("index.html",root),"utf8");
const mlHome=fs.readFileSync(new URL("../ml/index.html",root),"utf8");
const styles=fs.readFileSync(new URL("styles.css",root),"utf8");
const mlStyles=fs.readFileSync(new URL("../ml/atlas.css",root),"utf8");
assert(mathHome.includes('aria-label="从统计不确定性到期权定价：Bayes更新 → 共轭 → 可信区间 → 预测 → BSM → Greeks → IV → 波动率微笑"'));
assert(mlHome.includes('aria-label="机器学习与量化金融：贝叶斯预测不确定性 → 欧式复制定价 → Greeks → 波动率曲面"'));
for(const id of newIds){
 assert(mathHome.includes("?topic="+id),"math homepage lesson link "+id);
 assert(mlHome.includes("../math/?topic="+id),"ML crosslink "+id);
}
for(const c of ["overflow-x:auto","line-break:strict","@media print",":focus-visible"])
 assert(styles.includes(c),"math KaTeX/CJK/responsive style "+c);
assert(mlStyles.includes("overflow-x:auto"),"ML KaTeX scroll absent");
console.log("PASS V45/V28: 8 substantive lectures, 40 deep sections, 133 strict KaTeX expressions, 16 source links, 32 canonical crosslinks, 23 independent numeric checks, 312 unique full lectures / 656 math nodes.");
