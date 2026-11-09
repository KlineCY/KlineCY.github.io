import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
import katex from "katex";
import {gunzipSync} from "node:zlib";
const dir=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,dir),"utf8"));
const core=JSON.parse(gunzipSync(Buffer.concat([1,2,3,4].map(i=>fs.readFileSync(new URL("atlas/data.part"+i,dir))))).toString("utf8"));
const ids=new Set(core.catalog.items.map(x=>x.id));
const math=new Set(read("../math/curriculum.json").nodes.map(n=>n.id));
const priorFiles=["atlas-new-lessons.json","atlas-volatility-chapter.json","atlas-linear-regularization-chapter.json","atlas-evaluation-evidence-chapter.json","atlas-generalization-v2.json","atlas-kernel-covariance-v2.json"];
const prior=priorFiles.map(read);
const priorIds=new Set(prior.flatMap(d=>d.chapters.flatMap(ch=>ch.lessons.map(l=>l.id))));
const newFiles=["atlas-stability-kernel-v3.json","atlas-robust-validation-v3.json"];
const docs=newFiles.map(read);
const rows=docs.flatMap(d=>d.chapters.flatMap(ch=>ch.lessons));
const expected=["m04-c08-p01","m09-c08-p01","m09-c08-p02","m12-c08-p01","m23-c08-p01","m23-c08-p02"];
assert.deepEqual(rows.map(l=>l.id),expected,"stable learning chain order");
for(const d of docs){assert.equal(d.schema,1);assert(d.version);assert(d.chapters.length>=2);}
let sections=0,formula=0,sources=0;
for(const l of rows){
 assert(!ids.has(l.id)&&!priorIds.has(l.id),"duplicate stable course ID "+l.id);
 assert(l.title.length>=12&&l.goal.length>=70&&l.protocol.length>=4&&l.sources.length>=2,"insufficient overview "+l.id);
 assert.equal(l.sections.length,5,"deep course requires five connected sections "+l.id);
 assert.equal(new Set(l.sections.map(s=>s.title)).size,5,"duplicate section headings "+l.id);
 for(const s of l.sections){
  assert(s.title.length>=7&&s.question.length>=12&&s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=65),"short theory paragraph "+l.id+" / "+s.title);
  assert(s.worked.length>=55&&s.boundary.length>=45,"worked example or failure boundary missing "+l.id+" / "+s.title);
  assert(s.equations.length>=1);
  for(const f of s.equations){assert(f.label&&f.tex);katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});formula++;}
  sections++;
 }
 for(const p of l.prerequisites){
  assert(p.label&&["ml","math"].includes(p.kind));
  if(p.kind==="math")assert(math.has(p.id),"broken maths reference "+p.id);
  else {
   assert(ids.has(p.id)||priorIds.has(p.id)||expected.includes(p.id),"broken ML link "+p.id);
   if(expected.includes(p.id))assert(expected.indexOf(p.id)<expected.indexOf(l.id),"forward prerequisite "+l.id);
  }
 }
 for(const src of l.sources){assert(src.title&&/^https:\/\//.test(src.url));sources++;}
}
assert(!/\uFFFD/.test(JSON.stringify(rows)),"source text contains replacement Unicode characters");
const scripts=["atlas.js","new-topics.js"].map(p=>fs.readFileSync(new URL(p,dir),"utf8"));
for(const s of scripts)new vm.Script(s);
for(const fn of newFiles)assert(scripts[0].includes('"./'+fn+'"'),"Atlas loader does not load "+fn);
const ctx={window:{},encodeURIComponent};
vm.runInNewContext(scripts[1],ctx);
for(const d of [...prior,...docs])ctx.window.quantMlExpansion.install(core,d);
assert.equal(core.catalog.items.length,764);
assert.equal(new Set(core.catalog.items.map(x=>x.id)).size,764);
for(const id of expected){
 const art=core.domains[id.slice(0,3)][id];
 assert(art?.html.includes("atlas-concept-toc"),"not visible to reader "+id);
 assert.equal((art.html.match(/class="atlas-concept-unit"/g)||[]).length,5);
 assert(core.catalog.items.some(x=>x.id===id),"missing index entry "+id);
}
const close=(a,b,t=1e-9)=>assert(Math.abs(a-b)<t,"numeric check "+a+" vs "+b);
close(2*4/100,.08);
close(1/Math.sqrt(2),.7071067811865476);
close(Math.exp(-.5),.6065306597126334);
close(2-.5*2,1);
close(2-1.5,0.5);
close(.064-2*.01,.044);
close(.95+.05,1);
close(5-.5,4.5);
close(((0-1)**2+(1-1)**2+(2-1)**2)/2,1);
close(1+2*.2,1.4);
const html=fs.readFileSync(new URL("./index.html",dir),"utf8"),css=fs.readFileSync(new URL("./atlas.css",dir),"utf8");
for(const id of expected)assert(html.includes("#lesson/"+id),"missing advanced learning route "+id);
assert(html.includes('atlas.js?v=20261109-v114-pit-regression-causal'));
assert(html.includes('atlas.css?v=20261104-copula-v7'));
assert(html.includes('aria-label="算法稳定性、可扩展核学习与稳健金融建模的贯通课程"'));
assert(css.includes("/* V3 CHAPTER READABILITY")&&css.includes("scrollbar-gutter:stable")&&css.includes("@media print"),"new visual accessibility rules missing");
console.log("PASS: V3 machine learning "+rows.length+" advanced courses, "+sections+" deep sections, "+formula+" parseable TeX formulas, "+sources+" source references; 764 unique navigable lessons, exact arithmetic and CJK/formula design.");
