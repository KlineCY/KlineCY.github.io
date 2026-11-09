import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
import katex from "katex";
import {gunzipSync} from "node:zlib";
const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const namesBefore=["atlas-new-lessons.json","atlas-volatility-chapter.json","atlas-linear-regularization-chapter.json","atlas-evaluation-evidence-chapter.json","atlas-generalization-v2.json","atlas-kernel-covariance-v2.json","atlas-stability-kernel-v3.json","atlas-robust-validation-v3.json","atlas-quantile-calibration-v4.json","atlas-tailrisk-models-v4.json","atlas-proper-scoring-conformal-v5.json","atlas-density-pit-finance-v5.json"];
const current=["atlas-copula-v6.json","atlas-multivariate-risk-v6.json"];
const docs=current.map(read),rows=docs.flatMap(d=>d.chapters.flatMap(c=>c.lessons));
const expected=["m09-c10-p01","m09-c10-p02","m12-c11-p01","m23-c11-p01"];
assert.deepEqual(rows.map(l=>l.id),expected);
const math=new Set(read("../math/curriculum.json").nodes.map(n=>n.id));
const core=JSON.parse(gunzipSync(Buffer.concat([1,2,3,4].map(i=>fs.readFileSync(new URL("atlas/data.part"+i,root))))).toString("utf8"));
const before=namesBefore.map(read),beforeIds=new Set(before.flatMap(d=>d.chapters.flatMap(c=>c.lessons.map(l=>l.id))));
let units=0,equations=0,sources=0;
for(const doc of docs)assert(doc.schema===1&&doc.chapters.length>=1);
for(const l of rows){
 assert(!beforeIds.has(l.id)&&!core.catalog.items.some(x=>x.id===l.id),"duplicate new course "+l.id);
 assert(l.goal.length>=80&&l.protocol.length>=4&&l.sources.length>=2);
 assert.equal(l.sections.length,5);
 assert.equal(new Set(l.sections.map(s=>s.title)).size,5);
 for(const s of l.sections){
  assert(s.question.length>=13&&s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=70),"insufficient detailed Chinese explanation "+l.id+" "+s.title);
  assert(s.worked.length>=65&&s.boundary.length>=55,"worked calculation or validity boundary incomplete "+l.id+" "+s.title);
  assert(s.equations.length>=1);
  for(const e of s.equations){assert(e.label&&e.tex);katex.renderToString(e.tex,{throwOnError:true,strict:"ignore"});equations++;}
  units++;
 }
 for(const p of l.prerequisites){
  assert(p.label&&["ml","math"].includes(p.kind),"invalid prerequisite type "+l.id);
  if(p.kind==="math")assert(math.has(p.id),"missing mathematical prerequisite "+p.id);
  else{
   assert(core.catalog.items.some(x=>x.id===p.id)||beforeIds.has(p.id)||expected.includes(p.id),"missing ML prerequisite "+p.id);
   if(expected.includes(p.id))assert(expected.indexOf(p.id)<expected.indexOf(l.id),"forward prerequisite "+p.id+" → "+l.id);
  }
 }
 for(const src of l.sources){assert(src.title&&/^https:\/\//.test(src.url));sources++;}
}
assert(!JSON.stringify(rows).includes("\ufffd"),"UTF-8 replacement character in textbook");
const atlas=fs.readFileSync(new URL("atlas.js",root),"utf8"),expansion=fs.readFileSync(new URL("new-topics.js",root),"utf8");
new vm.Script(atlas);new vm.Script(expansion);
for(const file of current)assert(atlas.includes('"./'+file+'"'),"runtime loader missing "+file);
const ctx={window:{},encodeURIComponent};
vm.runInNewContext(expansion,ctx);
for(const doc of [...before,...docs])ctx.window.quantMlExpansion.install(core,doc);
assert.equal(core.catalog.items.length,780,"incorrect count of active navigable courses");
assert.equal(new Set(core.catalog.items.map(x=>x.id)).size,780,"duplicate course ID after bundle injection");
for(const id of expected){
 const art=core.domains[id.slice(0,3)][id];
 assert(art?.html.includes("atlas-concept-toc"),"course reader cannot display "+id);
 assert.equal((art.html.match(/class="atlas-concept-unit"/g)||[]).length,5,"lost an internal subsection "+id);
 assert(art.html.includes("tabindex=\"0\""),"long formula container not keyboard scrollable "+id);
 assert(core.catalog.items.some(x=>x.id===id),"not in navigation "+id);
}
const near=(a,b,t=1e-8)=>assert(Math.abs(a-b)<t,"numerical invariant incorrect "+a+" vs "+b);
near(.05**2,.0025);
near(.05/.05,1);
near(1-.95,.05);
near(2**(-1),.5);
near(.5*(0+2)-.5*(0+2+2+0)/4,.5);
near(Math.abs(Math.abs(1-3)-Math.abs(0-2))**2,0);
near(Math.hypot(1-0,3-2),Math.sqrt(2));
near(37*36/2,666);
near(.5*(1+.8*(1-.5)*(1-2*.25)),.6);
near(15/50,.3);
near(Math.sqrt(.5)*1.96,1.385929291125633);
const html=fs.readFileSync(new URL("index.html",root),"utf8"),css=fs.readFileSync(new URL("atlas.css",root),"utf8");
for(const id of expected)assert(html.includes("#lesson/"+id),"sixth learning route missing "+id);
assert(html.includes("780篇可导航课程")&&html.includes("第六条高级学习链"));
assert(html.includes("atlas.js?v=20261109-v114-pit-regression-causal")&&html.includes("atlas.css?v=20261104-copula-v7"));
assert(css.includes("V6 multivariate risk:")&&css.includes("overflow-x:auto")&&css.includes("@media print"),"CJK/formula accessibility missing");
console.log("PASS: V6 "+rows.length+" full Copula/multivariate risk lectures, "+units+" deep sections, "+equations+" parseable KaTeX formulas, "+sources+" cited resources; 780 unique navigable courses, conditional dependence arithmetic and keyboard/CJK/print regression checks.");
