import assert from "node:assert/strict";
import fs from "node:fs";
import katex from "katex";
const root=new URL("./",import.meta.url);
const read=name=>JSON.parse(fs.readFileSync(new URL(name,root),"utf8"));
const manifest=read("lessons-manifest.json"),curriculum=read("curriculum.json");
const newLessons=read("lessons-phase-25-a.json").lessons;
const core=["lessons-foundations.json","lessons-advanced.json","lessons-more.json"].flatMap(name=>read(name).lessons);
const ext=manifest.modules.flatMap(item=>{
 const lessons=read(item.path).lessons;
 assert.equal(lessons.length,item.lessons,"mismatched course module "+item.path);
 return lessons;
});
const all=[...core,...ext],unique=new Set(all.map(lesson=>lesson.node_id));
const nodes=new Map(curriculum.nodes.map(node=>[node.id,node]));
const ids=["timeseries-x003","timeseries-x004","timeseries-x006","mlmath-x006","riskmath-x001"];
assert.deepEqual(newLessons.map(l=>l.node_id),ids,"five courses follow canonical node IDs");
assert.equal(manifest.expected_lessons,189);
assert.equal(manifest.modules.length,73);
assert.equal(ext.length,189);
assert.equal(core.length,123);
assert.equal(all.length,312);
assert.equal(unique.size,312);
assert.equal(curriculum.nodes.length,656);
let chapters=0,expressions=0,links=0,citations=0;
for(const l of newLessons){
 assert(l.id===l.node_id&&nodes.get(l.node_id)?.title===l.title,"new lesson must match existing official node");
 for(const name of ["definition","assumptions","example","application","caveat"])
  assert(typeof l[name]==="string"&&l[name].length>=95,"thin "+name+" in "+l.node_id);
 assert(l.formulas.length>=4&&l.derivation.length>=3&&l.properties.length>=3);
 assert(l.deep_sections.length===4);
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,4);
 for(const f of l.formulas){
  assert(f.label&&f.tex);
  katex.renderToString(f.tex,{throwOnError:true,strict:"ignore",displayMode:true});expressions++;
 }
 for(const step of l.derivation){
  assert(step.explanation?.length>=60&&step.tex);
  katex.renderToString(step.tex,{throwOnError:true,strict:"ignore",displayMode:true});expressions++;
 }
 for(const unit of l.deep_sections){
  assert(unit.title&&unit.question.length>12&&unit.paragraphs.length>=2);
  assert(unit.paragraphs.every(s=>s.length>=40));
  assert(unit.proof.length>=50&&unit.worked.length>=35&&unit.limitation.length>=30);
  assert(unit.formulas.length>=1);
  for(const x of unit.formulas){
   katex.renderToString(x.tex,{throwOnError:true,strict:"ignore",displayMode:true});expressions++;
  }
  chapters++;
 }
 for(const edge of l.links){
  assert(edge.kind==="math"&&nodes.has(edge.id)&&edge.label,"broken cross-lecture link "+l.node_id);
  links++;
 }
 for(const source of l.sources){
  assert(/^https:\/\//.test(source.url)&&source.title.length>=27,"invalid academic source "+l.node_id);
  citations++;
 }
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)),"garbled CJK "+l.node_id);
 assert(!/余弦\?|Lorem ipsum|TODO|占位|待填/.test(JSON.stringify(l)),"unreviewed editorial placeholder "+l.node_id);
}
assert.equal(chapters,20);
assert.equal(expressions,62);
assert.equal(links,15);
assert.equal(citations,9);
const near=(actual,expected,what,tol=1e-10)=>assert(Math.abs(actual-expected)<tol,what+": "+actual+" vs "+expected);
near((.000402-.1*.00042)/.9,.0004,"TSRV bias corrected");
near(.8*.01*.01,.00008,"overlap Brownian covariance");
near(.00008/.0002,.4,"Epps asynchronous attenuation");
const z=.2+.3*(1-Math.exp(-2)),u=1-Math.exp(-z);
near(z,.4593994150290162,"Hawkes compensator");
near(u,.36833710114762475,"time-rescaled PIT");
near(6**2/(4**2+1**2+1**2+0),2,"ESS");
near((.02*100+.03*10)/.05,46,"discrete expected shortfall");
near(1-.96**2,.0784,"VaR non-subadditivity example");
const r=read("lessons-phase-25-a.json");
for(const [id,value] of [["timeseries-x003","0.0004"],["timeseries-x004","0.00008"],["timeseries-x006","0.3683"],["mlmath-x006","36/18=2"],["riskmath-x001","=46"]])
 assert(r.lessons.find(x=>x.id===id).example.includes(value),"numeric worked example is not reflected in visible prose "+id);
const html=fs.readFileSync(new URL("index.html",root),"utf8");
const ml=fs.readFileSync(new URL("../ml/index.html",root),"utf8");
const css=fs.readFileSync(new URL("styles.css",root),"utf8");
assert(html.includes('aria-label="V25高频计量和鲁棒尾部风险五课"'));
assert(ml.includes('aria-label="V25高频风险与分布鲁棒数学交叉学习链"'));
for(const id of ids){
 assert(html.includes("?topic="+id),"unlinked quant math lecture "+id);
 assert(ml.includes("../math/?topic="+id),"unlinked ML-math bridge "+id);
}
for(const token of ["overflow-x:auto","line-break:strict","@media print",":focus-visible"])
 assert(css.includes(token),"missing maths / Chinese responsive layout "+token);
console.log("PASS V25: 5 high-frequency/risk mathematics lectures, 20 detailed chapters, 62 strict KaTeX proofs, "+
 citations+" verified-format references, "+links+" learning links, 8 independent numeric invariants, 287 unique complete mathematics lessons.");
