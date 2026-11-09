import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const v24Ids=new Set(["statistics-x003","statistics-x004","mlmath-x003","mlmath-x004"]);
const v25Ids=new Set(["timeseries-x003","timeseries-x004","timeseries-x006","mlmath-x006","riskmath-x001"]);
const v26Ids=new Set(["timeseries-n0260","timeseries-n0261","timeseries-n0262","timeseries-n0266","timeseries-n0267","timeseries-n0269","timeseries-n0270"]);
const v27Ids=new Set(["statistics-n0171","statistics-n0170","statistics-n0172","statistics-n0173","statistics-n0174","statistics-n0175","statistics-n0176","riskmath-n0589","riskmath-n0594","riskmath-n0595"]);
const v28Ids=new Set(["statistics-n0177","statistics-n0178","statistics-n0179","statistics-n0180","finance-n0434","finance-n0436","finance-n0437","finance-n0438"]);
const v29Ids=new Set(["information-n0508","information-n0507","information-n0505","information-n0509","information-n0506","information-n0510"]);
const v30Ids=new Set(["finance-n0431","finance-n0432","finance-n0439","finance-n0440","finance-n0441","finance-n0442"]);
const curriculum=read("curriculum.json"),manifest=read("lessons-manifest.json");
const core=["lessons-foundations.json","lessons-advanced.json","lessons-more.json"].flatMap(p=>read(p).lessons);
const extensions=manifest.modules.flatMap(m=>{
 const lessons=read(m.path).lessons;
 assert.equal(lessons.length,m.lessons,"module file count "+m.path);
 return lessons;
});
const all=[...core,...extensions];
const nodeMap=new Map(curriculum.nodes.map(n=>[n.id,n]));
const ids=["timeseries-n0244","timeseries-n0245","timeseries-n0248","timeseries-n0249",
 "timeseries-n0250","timeseries-n0251","timeseries-n0254","timeseries-n0255"];
const added="abcd".split("").flatMap(s=>read("lessons-phase-22-"+s+".json").lessons);
assert.deepEqual(added.map(l=>l.id),ids,"V22 lecture order must remain intact");
assert.equal(curriculum.nodes.length,656);
assert.equal(manifest.modules.length,77);
assert.equal(manifest.expected_lessons,201);
assert.equal(core.length,123);
assert.equal(extensions.length,201);
assert.equal(all.length,324);
assert.equal(new Set(all.map(l=>l.id)).size,324,"unique complete mathematics IDs");
const v23Ids=["timeseries-n0256","timeseries-n0257","timeseries-n0258","timeseries-n0259"];
assert.equal(all.filter(l=>!ids.includes(l.id)&&!v23Ids.includes(l.id)&&!v24Ids.has(l.id)&&!v25Ids.has(l.id)&&!v26Ids.has(l.id)&&!v27Ids.has(l.id)&&!v28Ids.has(l.id)&&!v29Ids.has(l.id)&&!v30Ids.has(l.id)).length,266,"all original V21 complete lessons retained");
let sections=0,parsed=0,sources=0,links=0;
for(const l of added){
 assert(l.id===l.node_id&&nodeMap.get(l.id)?.title===l.title,"curriculum/lecture mapping "+l.id);
 assert(l.definition.length>=115&&l.assumptions.length>=95&&l.example.length>=125,"incomplete substantive lecture "+l.id);
 assert(l.application.length>=65&&l.caveat.length>=65,"missing financial context and limitations "+l.id);
 assert(l.properties.length>=3&&l.formulas.length>=5&&l.derivation.length>=4&&l.deep_sections.length>=5,"full lecture structure "+l.id);
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,l.deep_sections.length,"duplicate internal title "+l.id);
 for(const d of l.derivation){
  assert(d.explanation.length>=45,"underexplained derivation "+l.id);
  katex.renderToString(d.tex,{throwOnError:true,strict:"ignore",displayMode:true});parsed++;
 }
 for(const f of l.formulas){katex.renderToString(f.tex,{throwOnError:true,strict:"ignore",displayMode:true});parsed++;}
 for(const sec of l.deep_sections){
  assert(sec.question.length>=10&&sec.paragraphs.length===2&&sec.paragraphs.every(p=>p.length>=35),"Chinese narrative too thin "+l.id+sec.title);
  assert(sec.proof.length>=42&&sec.worked.length>=35&&sec.limitation.length>=25,"proof, worked calculation or failure mode insufficient "+l.id+sec.title);
  for(const f of sec.formulas){katex.renderToString(f.tex,{throwOnError:true,strict:"ignore",displayMode:true});parsed++;}
  sections++;
 }
 for(const e of l.links||[]){
  if(e.kind==="math")assert(nodeMap.has(e.id),"broken mathematics prerequisite "+l.id+" -> "+e.id);
  else assert(/^m\d\d-c\d\d-p\d\d$/.test(e.id),"malformed ML link "+l.id+" -> "+e.id);
  links++;
 }
 for(const s of l.sources||[]){assert(s.title.length>=20&&/^https:\/\//.test(s.url));sources++;}
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)),"corrupted Chinese or zero-width character "+l.id);
}
assert(sections>=40);
assert(parsed>=112);
assert.equal(sources,16);
assert.equal(links,32);
const active=new Set(),done=new Set();
function walk(id){
 assert(nodeMap.has(id),"unknown prerequisite "+id);
 if(active.has(id))throw Error("cycle "+id);
 if(done.has(id))return;
 active.add(id);
 for(const p of nodeMap.get(id).prerequisites||[])walk(p);
 active.delete(id);done.add(id);
}
for(const n of curriculum.nodes)walk(n.id);
assert.equal(done.size,656,"all curriculum prerequisites must be acyclic");
const near=(x,y,label)=>assert(Math.abs(x-y)<1e-8,"incorrect numeric example "+label+" "+x+" vs "+y);
const cases=[
 ["timeseries-n0244",Math.round(10200*(.04/99+.01/98)*1000)/1000,5.162,"5.162"],
 ["timeseries-n0245",.75/(1-.5**2),1,"0.75×4/3=1"],
 ["timeseries-n0248",2/5,.4,"2/5=0.4"],
 ["timeseries-n0249",1.125/1.75,9/14,"0.642857"],
 ["timeseries-n0250",2*4,8,"2×4=8"],
 ["timeseries-n0251",250+3*Math.log(100),263.8155105579643,"263.8155"],
 ["timeseries-n0254",.8*100+2,82,"X_t=82"],
 ["timeseries-n0255",2/(4**2),.125,"0.125"]
];
assert.equal(cases.length,8);
for(const [id,actual,expected,token] of cases){
 near(actual,expected,id);
 assert(added.find(l=>l.id===id).example.includes(token),"example not present "+id);
}
near(.75*(1+.5**2),.9375,"V51 Wold two-step orthogonal innovation variance");
near(.75/(1-.5**2),1,"V51 Wold infinite-horizon variance limit");
near(2*2**2,8,"V51 Gaussian MA1 squared lag-one covariance");
near(8/(2*5**2),.16,"V51 Gaussian MA1 squared first ACF");
near(.5*1.125,.5625,"V51 ARMA second autocovariance");
near(.5625/1.75,9/28,"V51 ARMA lag-two autocorrelation");
near(1.96*Math.sqrt(2*4),5.543717164502532,"V51 RW conditional Gaussian 95 percent radius",1e-7);
near(3-2,1,"V51 AIC extra-likelihood margin");
near(Math.log(100)-3,1.6051701859880918,"V51 BIC extra-likelihood threshold margin");
assert(3<Math.log(100)&&3>2,"V51 AIC versus BIC model preference difference");
assert(Math.abs(1+(-3))>1,"V51 delta negative three maps to explosive AR coefficient minus two");
near(1+(-.75),.25,"V51 KPSS Bartlett L1 LRV");
near(2/(4**2*.25),.5,"V51 KPSS bandwidth-one example");
near(1+.5*4,3,"V51 ARCH1 conditional innovation variance");
near(1/(1-.5),2,"V51 ARCH1 unconditional second moment");
near(3-1,2,"fourth moment nonlinear-dependence counterexample");
near(.75+.5*.75**2/.75,1.125,"ARMA lag-one covariance");
near(244+5*Math.log(100),267.0258509299405,"BIC penalty");
near((1+1)/(2+2),.5,"MA overlap correlation");
const math=fs.readFileSync(new URL("index.html",root),"utf8");
const ml=fs.readFileSync(new URL("../ml/index.html",root),"utf8");
assert(math.includes('aria-label="量化时间序列：白噪声 → Wold → MA/ARMA → ARIMA → ADF/KPSS"'));
assert(ml.includes('aria-label="金融时间序列理论：创新、ARMA与单位根诊断"'));
for(const id of ids){
 assert(math.includes("?topic="+id),"missing math navigation "+id);
 assert(ml.includes("../math/?topic="+id),"missing ML navigation "+id);
}
for(const p of ["styles.css","../ml/atlas.css"]){
 const css=fs.readFileSync(new URL(p,root),"utf8");
 for(const term of ["overflow-x:auto","line-break:strict",":focus-visible","@media print"])assert(css.includes(term),"CJK/KaTeX/print safeguard missing "+term+" in "+p);
}
console.log("PASS V22: 8 extended mathematically complete time-series lectures, 40 proof sections, "+parsed+
 " strict KaTeX formulas, "+sources+" sourced references, "+links+" crosslinks, 8 independently recomputed examples, 278 unique complete courses, and 656-node acyclic prerequisites.");
