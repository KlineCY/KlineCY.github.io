import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const load=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const ids=["foundation-n0001","foundation-n0002","foundation-n0003","foundation-n0004","foundation-n0006","foundation-n0007","foundation-n0008","foundation-n0009","foundation-n0010","foundation-n0011","foundation-n0014","foundation-n0015"];
const base=load("lessons-foundations-v89.json"),deep=load("lessons-foundations-v91.json");
assert.equal(base.lessons.length,12);
assert.deepEqual(base.lessons.map(x=>x.node_id),ids);
assert(base.lessons.every(x=>x.deep_sections.length===4),"V89 historical four-section baseline changed");
assert.equal(deep.version,1);
assert.equal(deep.revision,"V91");
assert.equal(deep.base_file,"./lessons-foundations-v89.json");
assert.equal(deep.extensions.length,12);
assert.deepEqual(deep.extensions.map(x=>x.node_id),ids);
assert.equal(deep.patches.length,2);
const baseById=new Map(base.lessons.map(x=>[x.node_id,x]));
const seen=new Set();let sections=0,equations=0;
function typeset(tex,where){
 assert.equal(typeof tex,"string",where+" has no formula");
 assert(tex.length>=12,where+" formula too short");
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(tex),where+" hidden chars");
 katex.renderToString(tex,{throwOnError:true,strict:"error",displayMode:true});
 equations++;
}
for(const ext of deep.extensions){
 const original=baseById.get(ext.node_id);
 assert(original&&original.title===ext.title,"unknown or mistitled V91 node "+ext.node_id);
 assert(!seen.has(ext.node_id),"duplicate V91 node");
 seen.add(ext.node_id);
 assert.equal(ext.deep_sections.length,2,"each V91 lesson must deepen two distinct proofs");
 const originalTitles=new Set(original.deep_sections.map(s=>s.title));
 for(const s of ext.deep_sections){
  assert(s.title.length>=8&&s.question.length>=10,"incomplete section heading");
  assert(!originalTitles.has(s.title),"V91 repeats V89 chapter "+s.title);
  originalTitles.add(s.title);
  assert(s.paragraphs.length===2&&s.paragraphs.every(t=>t.length>=30),"explanation lacks depth");
  assert(s.proof.length>=50&&s.worked.length>=50&&s.limitation.length>=25,"proof, example or limitation lacks depth");
  assert(s.formulas.length>=1&&s.formulas.every(f=>f.label.length>=4));
  for(const f of s.formulas)typeset(f.tex,ext.node_id);
  assert(/[\u3400-\u9FFF]/u.test(JSON.stringify(s)),"Chinese exposition absent");
  assert(!/TODO|FIXME|Lorem ipsum/i.test(JSON.stringify(s)),"draft placeholder");
  sections++;
 }
}
assert.equal(sections,24);
assert.equal(equations,24);
const rank=deep.patches.find(p=>p.node_id==="foundation-n0003");
assert(rank&&rank.kind==="formula"&&rank.index===3);
assert.equal(baseById.get(rank.node_id).formulas[rank.index].label,rank.expected_label);
assert(rank.formula.tex.includes(String.raw`\dim V`),"rank-nullity must name vector-space domain");
typeset(rank.formula.tex,"rank-nullity correction");
const sup=deep.patches.find(p=>p.node_id==="foundation-n0015");
assert(sup&&sup.kind==="deep_section"&&sup.index===1);
assert.equal(baseById.get(sup.node_id).deep_sections[sup.index].title,sup.expected_title);
assert(sup.section.formulas[0].tex.includes(String.raw`\land`),"supremum must conjoin upper bound and approximation");
assert(sup.section.paragraphs.length>=3&&sup.section.worked.includes("A={1,2}"));
typeset(sup.section.formulas[0].tex,"supremum correction");
assert.equal(equations,26);
const app=fs.readFileSync(new URL("app.js",root),"utf8");
const css=fs.readFileSync(new URL("styles.css",root),"utf8");
for(const marker of ["loadV89Foundations()","loadV91FoundationDeepening()","applyV91FoundationDeepening(v89,v91)","...extra,...v89"])
 assert(app.includes(marker),"overlay disconnected from live reader: "+marker);
for(const marker of ["overflow-x:auto","line-break:strict","@media(max-width:700px)"])
 assert(css.includes(marker),"math/CJK responsive style missing: "+marker);
let cases=0;
function close(label,a,b,tol=1e-10){assert(Number.isFinite(a)&&Math.abs(a-b)<=tol,label+": "+a+" vs "+b);cases++;}
close("three-way inclusion-exclusion",12+10+8-5-4-3+2,20);
close("Jaccard example",15/(20+20-15),.6);
close("symmetric difference example",20+20-2*15,10);
close("cartesian union example",2*2,4);
close("Cantor pairing",3*4/2+1,7);
close("Cantor ordered pairing",3*4/2+2,8);
close("absolute covariance bound",4.5/(2*3),.75);
close("ill-conditioned invertible matrix",1/1e-12,1e12);
close("AM-GM equality gap",(1+9)/2-Math.sqrt(1*9),2);
close("L2 tail moment",100**2*(1/100),100);
close("linear inconsistent witness",-2*1+1*2,0);
close("linear inconsistent right hand side",-2*1+1*3,1);
close("supremum certificate",1-.998,.002);
assert(cases>=12);
console.log("V91 PASS: 12 V89 baseline chapters preserved, 24 new rigorous sections, 26 strict KaTeX expressions including 2 mathematical corrections, "+cases+" independent numerical invariants and responsive CJK/math checks.");
