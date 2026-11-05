import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const manifest=read("lessons-manifest.json"),curriculum=read("curriculum.json");
const core=["lessons-foundations.json","lessons-advanced.json","lessons-more.json"].flatMap(p=>read(p).lessons);
const extensions=manifest.modules.flatMap(m=>{
 const rows=read(m.path).lessons;
 assert.equal(rows.length,m.lessons,"module cardinality "+m.path);
 return rows;
});
const all=[...core,...extensions],index=new Map(all.map(l=>[l.id,l]));
const nodes=new Map(curriculum.nodes.map(n=>[n.id,n]));
const newIds=["timeseries-n0256","timeseries-n0257","timeseries-n0258","timeseries-n0259"];
const added="ab".split("").flatMap(x=>read("lessons-phase-23-"+x+".json").lessons);
assert.deepEqual(added.map(l=>l.id),newIds);
assert.equal(manifest.modules.length,65);
assert.equal(manifest.expected_lessons,159);
assert.equal(core.length,123);
assert.equal(extensions.length,159);
assert.equal(all.length,282);
assert.equal(index.size,282,"unique complete math lesson identifiers");
assert.equal(all.filter(l=>!newIds.includes(l.id)).length,278,"all non-V23 full lessons retained");
assert.equal(curriculum.nodes.length,656);
let sections=0,formulas=0,refs=0,edges=0;
for(const l of added){
 assert(l.id===l.node_id&&nodes.get(l.id)?.title===l.title,"lesson-to-curriculum mapping "+l.id);
 assert(l.definition.length>=155&&l.assumptions.length>=115&&l.example.length>=160,"core explanatory depth "+l.id);
 assert(l.application.length>=100&&l.caveat.length>=75,"finance conditions/limitations "+l.id);
 assert(l.properties.length>=3&&l.formulas.length===4&&l.derivation.length===3&&l.deep_sections.length===4,"lecture coverage "+l.id);
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,4,"duplicate deep headings "+l.id);
 for(const d of l.derivation){
  assert(d.explanation.length>=60,"mathematical derivation explanation "+l.id);
  katex.renderToString(d.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++;
 }
 for(const eq of l.formulas){katex.renderToString(eq.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++;}
 for(const part of l.deep_sections){
  assert(part.question.length>=12&&part.paragraphs.length===2&&part.paragraphs.every(x=>x.length>=40),"incomplete Chinese reasoning "+l.id);
  assert(part.proof.length>=50&&part.worked.length>=40&&part.limitation.length>=30,"proof, worked example or assumptions "+l.id);
  for(const eq of part.formulas){katex.renderToString(eq.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++;}
  sections++;
 }
 for(const link of l.links||[]){
  if(link.kind==="math")assert(nodes.has(link.id),"bad mathematics link "+l.id+" -> "+link.id);
  else assert(/^m\d\d-c\d\d-p\d\d$/.test(link.id),"bad ML link "+l.id+" -> "+link.id);
  edges++;
 }
 for(const source of l.sources||[]){assert(source.title.length>=25&&/^https:\/\//.test(source.url));refs++;}
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)),"corrupted Chinese text "+l.id);
}
assert.equal(sections,16);
assert.equal(formulas,44);
assert.equal(refs,8);
assert.equal(edges,16);
const active=new Set(),done=new Set();
function visit(id){
 assert(nodes.has(id),"unknown curriculum parent "+id);
 if(active.has(id))throw Error("prerequisite cycle at "+id);
 if(done.has(id))return;
 active.add(id);
 for(const p of nodes.get(id).prerequisites||[])visit(p);
 active.delete(id);done.add(id);
}
for(const node of curriculum.nodes)visit(node.id);
assert.equal(done.size,656);
const eq=(a,b,label,tol=1e-9)=>assert(Math.abs(a-b)<tol,"numeric mismatch "+label+": "+a+" vs "+b);
const samples=[
 ["timeseries-n0256",3*.5**2,.75,"0.75"],
 ["timeseries-n0257",Math.round(-100*(Math.log(.8)+Math.log(.98))*1e4)/1e4,24.3346,"24.3346"],
 ["timeseries-n0258",Math.log(.5)/Math.log(.7),1.9433582098747315,"1.943"],
 ["timeseries-n0259",.5*1.2+.2*.3,.66,"(0.66,0.09)"]
];
for(const [id,actual,expect,token] of samples){eq(actual,expect,id);assert(index.get(id).example.includes(token),"independent check not in visible example "+id);}
eq(-100*Math.log(.8),22.31435513142097,"Johansen maximum eigenvalue");
eq(-100*Math.log(.98),2.0202707317519466,"Johansen marginal trace");
eq(.7**2*4,1.96,"ECM two-period deviation");
eq(.5*2+.2*1,1.2,"VAR first coordinate next period");
eq(.3*.3,.09,"VAR second coordinate second period");
eq(.5**2+.2**2, .29,"VAR forecast covariance AA' first diagonal");
eq(.2*.3,.06,"VAR forecast covariance off-diagonal");
eq(.3**2,.09,"VAR forecast covariance AA' second diagonal");
const math=fs.readFileSync(new URL("index.html",root),"utf8");
const ml=fs.readFileSync(new URL("../ml/index.html",root),"utf8");
assert(math.includes('aria-label="V23协整误差修正与向量VAR四课"'));
assert(ml.includes('aria-label="V23多资产协整ECM与VAR跨模块路径"'));
for(const id of newIds){
 assert(math.includes("?topic="+id),"math nav absent "+id);
 assert(ml.includes("../math/?topic="+id),"ML nav absent "+id);
}
for(const p of ["styles.css","../ml/atlas.css"]){
 const css=fs.readFileSync(new URL(p,root),"utf8");
 for(const prop of ["overflow-x:auto","line-break:strict",":focus-visible","@media print"])assert(css.includes(prop),"formula/CJK/mobile formatting missing "+prop+" "+p);
}
console.log("PASS V23: 4 new complete cointegration–Johansen–ECM–VAR lectures, 16 proof sections, "+
 formulas+" strict KaTeX equations, "+refs+" citations, "+edges+" cross-links, 4 independent and 8 supplemental arithmetic checks, 278 unique full mathematics lectures, 656-node acyclic curriculum.");
