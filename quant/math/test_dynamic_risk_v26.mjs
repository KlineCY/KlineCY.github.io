import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const catalog=read("curriculum.json"),manifest=read("lessons-manifest.json");
const added=[...read("lessons-phase-26-a.json").lessons,...read("lessons-phase-26-b.json").lessons];
const old=["lessons-foundations.json","lessons-advanced.json","lessons-more.json"].flatMap(x=>read(x).lessons);
const extras=manifest.modules.flatMap(mod=>{
 const v=read(mod.path).lessons;
 assert.equal(v.length,mod.lessons,"manifest count mismatch "+mod.path);
 return v;
});
const all=[...old,...extras],ids=new Set(all.map(x=>x.node_id)),nodeMap=new Map(catalog.nodes.map(x=>[x.id,x]));
const targets=["timeseries-n0260","timeseries-n0261","timeseries-n0262","timeseries-n0266","timeseries-n0267","timeseries-n0269","timeseries-n0270"];
assert.deepEqual(added.map(x=>x.node_id),targets);
assert.equal(manifest.version,1);
assert.equal(manifest.expected_lessons,181);
assert.equal(manifest.modules.length,71);
assert.equal(extras.length,181);
assert.equal(old.length,123);
assert.equal(all.length,304);
assert.equal(ids.size,304,"all complete courses must have unique IDs");
assert.equal(catalog.nodes.length,656);
assert.equal(catalog.chapters.length,109);
let chapters=0,formulas=0,sources=0,links=0;
for(const l of added){
 assert(l.id===l.node_id&&nodeMap.get(l.id)?.title===l.title,"canonical title/id mismatch "+l.node_id);
 for(const name of ["definition","assumptions","example","application","caveat"]){
  assert(typeof l[name]==="string"&&l[name].length>=90,"missing substantive "+name+" "+l.id);
 }
 assert(l.properties.length>=3&&l.formulas.length>=4&&l.derivation.length>=3);
 assert.equal(l.deep_sections.length,4,"each full lecture requires four linked mini-chapters");
 assert.equal(new Set(l.deep_sections.map(x=>x.title)).size,4);
 for(const part of l.formulas){
  assert(part.label&&part.tex);
  katex.renderToString(part.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++;
 }
 for(const step of l.derivation){
  assert(step.explanation.length>=55&&step.tex);
  katex.renderToString(step.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++;
 }
 for(const unit of l.deep_sections){
  assert(unit.title&&unit.question.length>=12&&unit.paragraphs.length>=2);
  assert(unit.paragraphs.every(p=>p.length>=35));
  assert(unit.proof.length>=45&&unit.worked.length>=25&&unit.limitation.length>=25);
  assert(unit.formulas.length>=1);
  for(const part of unit.formulas){
   assert(part.tex&&part.label);
   katex.renderToString(part.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++;
  }
  chapters++;
 }
 for(const x of l.links){
  assert(x.kind==="math"&&nodeMap.has(x.id),"invalid graph edge "+l.id+" -> "+x.id);
  links++;
 }
 for(const x of l.sources){
  assert(x.title.length>=20&&/^https:\/\//.test(x.url),"invalid academic reference "+l.id);
  sources++;
 }
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)),"CJK corruption "+l.id);
 assert(!/Lorem ipsum|TODO|占位|待填|余弦\?/.test(JSON.stringify(l)),"placeholder "+l.id);
}
assert.equal(chapters,28);
assert.equal(formulas,92);
assert.equal(links,25);
assert.equal(sources,16);
const near=(v,expect,label,tol=1e-9)=>assert(Math.abs(v-expect)<tol,"independent arithmetic mismatch "+label+" "+v+" vs "+expect);
near(.5*.2+.2*.3,.16,"IRF horizon two");
near(1.25/1.29,.9689922480620154,"FEVD shock1");
near(.04/1.29,.031007751937984496,"FEVD shock2");
near(1.25/1.29+.04/1.29,1,"FEVD row sum");
near(((120-80)/2)/(80/40),10,"Granger joint F");
near(.000002+.05*.0004+.85*.0001,.000107,"GJR positive");
near(.000002+(.05+.10)*.0004+.85*.0001,.000147,"GJR negative");
near(.000002/(1-.05-.10/2-.85),.00004,"GJR long-run variance",1e-8);
near(Math.exp(.4),1.4918246976412703,"EGARCH shock ratio");
near(.05+.05+.9,1,"DCC Q diagonal");
near(.02*.03*(-.05),-.00003,"DCC covariance");
near(.0004*.0009-(-.00003)**2,.0000003591,"DCC determinant",1e-13);
near(.3*1.3/2,.195,"ARFIMA second impulse weight");
near(.3*1.3*2.3/6,.1495,"ARFIMA third impulse weight");
near(.3*(.3-1)/2,-.105,"fractional difference coefficient");
near(.7*.2/(.7*.2+.3*.6),.4375,"Hamilton filtered state 0");
near(.4375*.9+.5625*.2,.50625,"Hamilton one step state0");
near(.2/(.1+.2),2/3,"Hamilton stationary state0");
near(1/(1-.9),10,"Markov duration 0");
near(1/(1-.8),5,"Markov duration 1");
for(const [id,token] of [
 ["timeseries-n0260","0.16"],["timeseries-n0261","0.968992"],["timeseries-n0262","=10"],
 ["timeseries-n0266","0.000147"],["timeseries-n0267","−0.00003"],
 ["timeseries-n0269","0.1495"],["timeseries-n0270","0.50625"]
])assert(added.find(x=>x.id===id).example.includes(token),"worked calculation missing in reading view "+id);
const math=fs.readFileSync(new URL("index.html",root),"utf8");
const ml=fs.readFileSync(new URL("../ml/index.html",root),"utf8");
const css=fs.readFileSync(new URL("styles.css",root),"utf8");
const cssMl=fs.readFileSync(new URL("../ml/atlas.css",root),"utf8");
assert(math.includes('aria-label="V26多元动态预测、波动与状态切换七课"'));
assert(ml.includes('aria-label="V26多资产冲击波动与状态预测跨模块学习链"'));
for(const id of targets){
 assert(math.includes("?topic="+id),"no mathematics study-path link "+id);
 assert(ml.includes("../math/?topic="+id),"no machine-learning study-path link "+id);
}
for(const token of ["overflow-x:auto","line-break:strict","@media print",":focus-visible"])
 assert(css.includes(token),"quant math responsive/print/CJK styling missing "+token);
assert(cssMl.includes("overflow-x:auto"),"ML mathematics overflow rule absent");
console.log("PASS V26: 7 complete lectures, 28 proof mini-chapters, 92 strict KaTeX expressions, "+
 sources+" verified academic source formats, "+links+" canonical topic links, 20 independent mathematical invariants, 304 unique full lectures / 656-node catalog.");
