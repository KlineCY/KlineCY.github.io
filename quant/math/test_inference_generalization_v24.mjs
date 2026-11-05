import assert from "node:assert/strict";
import fs from "node:fs";
import katex from "katex";
const root=new URL("./",import.meta.url);
const read=name=>JSON.parse(fs.readFileSync(new URL(name,root),"utf8"));
const c=read("curriculum.json"),m=read("lessons-manifest.json");
const additions=read("lessons-phase-24-a.json").lessons;
const core=["lessons-foundations.json","lessons-advanced.json","lessons-more.json"].flatMap(f=>read(f).lessons);
const extras=m.modules.flatMap(row=>{
 const data=read(row.path).lessons;
 assert.equal(data.length,row.lessons,"module count "+row.path);
 return data;
});
const all=[...core,...extras],ids=new Set(all.map(l=>l.node_id));
const nodeMap=new Map(c.nodes.map(n=>[n.id,n]));
const targets=["statistics-x003","statistics-x004","mlmath-x003","mlmath-x004"];
assert.deepEqual(additions.map(l=>l.node_id),targets,"V24 canonical IDs and teaching order");
assert.equal(m.expected_lessons,189);
assert.equal(m.modules.length,73);
assert.equal(extras.length,189);
assert.equal(all.length,312);
assert.equal(ids.size,312,"new lectures do not collide with previous chapters");
assert.equal(c.nodes.length,656,"no curriculum ID churn");
let sections=0,formulaCount=0,links=0,sourceCount=0;
for(const l of additions){
 assert.equal(l.id,l.node_id);
 assert.equal(l.title,nodeMap.get(l.id)?.title,"title maps to official catalog");
 assert(l.definition.length>100&&l.assumptions.length>100&&l.example.length>130,"core assumptions and worked example "+l.id);
 assert(l.application.length>100&&l.caveat.length>90,"finance implementation and failure modes "+l.id);
 assert(l.formulas.length>=4&&l.derivation.length>=3&&l.properties.length>=3);
 assert.equal(l.deep_sections.length,4,"four reasoned proof sections "+l.id);
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,4);
 for(const f of l.formulas){katex.renderToString(f.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulaCount++}
 for(const d of l.derivation){assert(d.explanation.length>65);katex.renderToString(d.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulaCount++}
 for(const s of l.deep_sections){
  assert(s.title&&s.question.length>12&&s.paragraphs.length>=2);
  assert(s.paragraphs.every(p=>p.length>45));
  assert(s.proof.length>60&&s.worked.length>35&&s.limitation.length>45);
  for(const f of s.formulas){katex.renderToString(f.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulaCount++}
  sections++;
 }
 for(const lnk of l.links){assert(lnk.kind==="math"&&nodeMap.has(lnk.id),"invalid math learning edge");links++}
 for(const source of l.sources){assert(/^https:\/\//.test(source.url)&&source.title.length>25);sourceCount++}
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)),"Unicode corruption");
}
assert.equal(sections,16);
assert.equal(formulaCount,53);
assert.equal(links,12);
assert.equal(sourceCount,5);
const near=(a,b,tol=1e-10)=>assert(Math.abs(a-b)<tol,a+" != "+b);
near(Math.pow(.75,4),.31640625);
near(.02/Math.sqrt(250),.0012649110640673518);
near(Math.sqrt(1.5),1.224744871391589);
near((.0004+.000001)/(.02**3),50.125);
let rad=0;
for(let mask=0;mask<16;mask++){
 let sigmaSum=0;
 for(let i=0;i<4;i++)sigmaSum+=(mask&(1<<i))?1:-1;
 rad+=Math.abs(sigmaSum)/4/16;
}
near(rad,.375);
near(Math.sqrt(2*Math.log(2)/4),.5887050112577373);
near(2/(.5*100),.04);
near(2/(.5*200),.02);
for(const [id,token] of [
 ["statistics-x003","0.316406"],["statistics-x004","0.001265"],
 ["mlmath-x003","0.375"],["mlmath-x004","0.04"]
]){
 const x=additions.find(l=>l.id===id);
 assert(x.example.includes(token),"independent numeric example must be in public body: "+id);
}
const css=fs.readFileSync(new URL("styles.css",root),"utf8");
const html=fs.readFileSync(new URL("index.html",root),"utf8");
for(const token of ["math-deep-unit","math-deep-toc","overflow-x:auto","@media print"])assert(css.includes(token),"responsive proof layout "+token);
for(const id of targets)assert(html.includes("?topic="+id),"V24 chapter jump absent in public navigation "+id);
console.log("PASS V24: 4 connected lectures, 16 proof units, 53 strict KaTeX expressions, "+sourceCount+" academic references, 12 graph edges, independent numeric invariants and 282 unique full mathematics lectures.");
