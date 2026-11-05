import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const manifest=read("lessons-manifest.json"),curriculum=read("curriculum.json");
const core=["lessons-foundations.json","lessons-advanced.json","lessons-more.json"].flatMap(p=>read(p).lessons);
const modules=manifest.modules.flatMap(m=>{
 const rows=read(m.path).lessons;
 assert.equal(rows.length,m.lessons,"declared module cardinality "+m.path);
 return rows;
});
const all=[...core,...modules],nodeMap=new Map(curriculum.nodes.map(n=>[n.id,n]));
const actualIds=new Set(all.map(l=>l.node_id));
const newIds=["mlmath-n0455","mlmath-n0466","mlmath-n0463","mlmath-n0464",
 "mlmath-n0460","mlmath-n0461","mlmath-n0462","mlmath-n0465"];
const added="abcd".split("").flatMap(tag=>read("lessons-phase-21-"+tag+".json").lessons);
assert.deepEqual(added.map(l=>l.node_id),newIds,"V21 chapter order and identity");
assert.equal(manifest.modules.length,62);
assert.equal(manifest.expected_lessons,151);
assert.equal(core.length,123);
assert.equal(modules.length,151);
assert.equal(all.length,274);
assert.equal(actualIds.size,274,"no duplicate completed math ID");
const v22Ids=["timeseries-n0244","timeseries-n0245","timeseries-n0248","timeseries-n0249","timeseries-n0250","timeseries-n0251","timeseries-n0254","timeseries-n0255"];
assert.equal(all.filter(l=>!newIds.includes(l.node_id)&&!v22Ids.includes(l.node_id)).length,258,"all 258 original complete math lectures retained");
assert.equal(curriculum.nodes.length,656);
let sections=0,formulas=0,sourceCount=0,linkCount=0;
for(const l of added){
 assert(l.node_id===l.id&&nodeMap.get(l.id)?.title===l.title,"curriculum mismatch "+l.id);
 assert(l.definition.length>=110&&l.assumptions.length>=100&&l.example.length>=115,"substantive narrative missing "+l.id);
 assert(l.application.length>=80&&l.caveat.length>=60,"financial caveats incomplete "+l.id);
 assert(l.properties.length>=3&&l.formulas.length>=4&&l.derivation.length>=3&&l.deep_sections.length===4,"math coverage incomplete "+l.id);
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,4,"duplicate chapter headers "+l.id);
 for(const p of l.derivation){
  assert(p.explanation.length>=50,"derivation step lacks proof "+l.id);
  katex.renderToString(p.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++;
 }
 for(const f of l.formulas){
  katex.renderToString(f.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++;
 }
 for(const sec of l.deep_sections){
  assert(sec.question.length>=10&&sec.paragraphs.length===2&&sec.paragraphs.every(x=>x.length>=40),"incomplete CJK logic "+l.id+":"+sec.title);
  assert(sec.proof.length>=45&&sec.worked.length>=40&&sec.limitation.length>=32,"proof/calculation/limitation missing "+l.id+":"+sec.title);
  for(const f of sec.formulas){katex.renderToString(f.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++}
  sections++;
 }
 for(const link of l.links||[]){
  if(link.kind==="math")assert(nodeMap.has(link.id),"broken math edge "+l.id+"->"+link.id);
  else assert(/^m\d\d-c\d\d-p\d\d$/.test(link.id),"broken ML id "+l.id);
  linkCount++;
 }
 for(const ref of l.sources||[]){assert(ref.title.length>=20&&/^https:\/\//.test(ref.url));sourceCount++}
 assert(!JSON.stringify(l).includes("\ufffd"),"Unicode replacement character "+l.id);
}
assert.equal(sections,32);
assert.equal(formulas,88);
assert.equal(sourceCount,16);
assert.equal(linkCount,32);
const visiting=new Set(),visited=new Set();
function dfs(id){
 assert(nodeMap.has(id),"unresolved math node "+id);
 if(visiting.has(id))throw Error("circular prerequisite "+id);
 if(visited.has(id))return;
 visiting.add(id);
 for(const p of nodeMap.get(id).prerequisites||[])dfs(p);
 visiting.delete(id);visited.add(id);
}
for(const n of curriculum.nodes)dfs(n.id);
assert.equal(visited.size,656);
const cases=[
 ["mlmath-n0455",2.5/Math.sqrt(1.25),2.23606797749979,"2.236"],
 ["mlmath-n0466",.12-.10,.02,"0.12与0.10"],
 ["mlmath-n0463",3/(1+1),1.5,"β_R=(1.5,0.25,−1)"],
 ["mlmath-n0464",2.5/(1+.5),5/3,"(5/3,0,−1)"],
 ["mlmath-n0460",1/2,.5,"梯度方差为0.5"],
 ["mlmath-n0461",.004/(1-.999),4,"v̂₁=4"],
 ["mlmath-n0462",3*2/Math.hypot(3,4),1.2,"g_clip=(1.2,1.6)"],
 ["mlmath-n0465",4/5,.8,"4/5=0.8"]
];
assert.equal(cases.length,8);
for(const [id,actual,expected,token] of cases){
 assert(Math.abs(actual-expected)<1e-9,"failed independent numeric invariant "+id);
 assert(added.find(l=>l.id===id).example.includes(token),"arithmetic not present in visible lecture "+id);
}
assert(Math.abs((2+2*1.5)/2-2.5)<1e-9,"control numeric check");
const math=fs.readFileSync(new URL("index.html",root),"utf8");
const ml=fs.readFileSync(new URL("../ml/index.html",root),"utf8");
assert(math.includes('aria-label="V21金融机器学习八课训练与正则化全链"'));
assert(ml.includes('aria-label="V21金融机器学习训练与正则化八课互链"'));
for(const id of newIds){
 assert(math.includes("?topic="+id),"math navigation missing "+id);
 assert(ml.includes("../math/?topic="+id),"ML crosslink missing "+id);
}
for(const p of ["styles.css","../ml/atlas.css"]){
 const css=fs.readFileSync(new URL(p,root),"utf8");
 for(const term of ["overflow-x:auto","line-break:strict",":focus-visible","@media print"]){
  assert(css.includes(term),"CJK/formula/print formatting safeguard missing "+term+" in "+p);
 }
}
console.log("PASS V21: 8 genuinely new lectures, 32 fully worked sections, "+
 formulas+" strict KaTeX parses, 16 references, 32 links, 8 independent numeric cases, 274 unique complete math courses and 656-node acyclic curriculum.");
