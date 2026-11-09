import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
import katex from "katex";
import {gunzipSync} from "node:zlib";
const here=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,here),"utf8"));
const raw=Buffer.concat([1,2,3,4].map(i=>fs.readFileSync(new URL("atlas/data.part"+i,here))));
const core=JSON.parse(gunzipSync(raw).toString("utf8"));
const mathIds=new Set(read("../math/curriculum.json").nodes.map(x=>x.id));
const old=["atlas-new-lessons.json","atlas-volatility-chapter.json","atlas-linear-regularization-chapter.json","atlas-evaluation-evidence-chapter.json","atlas-generalization-v2.json","atlas-kernel-covariance-v2.json","atlas-stability-kernel-v3.json","atlas-robust-validation-v3.json","atlas-quantile-calibration-v4.json","atlas-tailrisk-models-v4.json"].map(read);
const names=["atlas-proper-scoring-conformal-v5.json","atlas-density-pit-finance-v5.json"],files=names.map(read);
const previous=new Set([...core.catalog.items.map(x=>x.id),...old.flatMap(d=>d.chapters.flatMap(ch=>ch.lessons.map(x=>x.id)))]);
const rows=files.flatMap(d=>d.chapters.flatMap(ch=>ch.lessons));
const expected=["m04-c10-p01","m12-c10-p01","m12-c10-p02","m09-c09-p01","m23-c10-p01","m23-c10-p02"];
assert.deepEqual(rows.map(x=>x.id),expected,"six-lesson topical graph and lesson-order stability");
let sections=0,formula=0,sources=0;
for(const file of files){assert.equal(file.schema,1);assert(file.chapters.length>=2);}
for(const x of rows){
 assert(!previous.has(x.id),"duplicate original course "+x.id);
 assert(x.goal.length>=80&&x.protocol.length>=4&&x.sources.length>=2);
 assert.equal(x.sections.length,5,"short V5 expanded textbook "+x.id);
 assert.equal(new Set(x.sections.map(s=>s.title)).size,5);
 for(const s of x.sections){
  assert(s.question?.length>=12&&s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=48),"thin explanatory paragraph "+x.id+" "+s.title);
  assert(s.worked.length>=48&&s.boundary.length>=35,"missing quantitative example or applicability limit "+x.id+" "+s.title);
  assert(s.equations.length>=1);
  for(const eq of s.equations){assert(eq.label&&eq.tex);katex.renderToString(eq.tex,{throwOnError:true,strict:"ignore"});formula++;}
  sections++;
 }
 for(const p of x.prerequisites){
  assert(["math","ml"].includes(p.kind)&&p.label);
  if(p.kind==="math")assert(mathIds.has(p.id),"broken mathematics bridge "+p.id);
  else{
   assert(previous.has(p.id)||expected.includes(p.id),"broken ML prerequisite "+x.id+" to "+p.id);
   if(expected.includes(p.id))assert(expected.indexOf(p.id)<expected.indexOf(x.id),"forward-only prerequisite "+x.id+" to "+p.id);
  }
 }
 for(const source of x.sources){assert(source.title&&/^https:\/\//.test(source.url));sources++;}
}
assert(!/\uFFFD/.test(JSON.stringify(rows)),"corrupted Chinese encoding");
const scripts=["atlas.js","new-topics.js"].map(n=>fs.readFileSync(new URL(n,here),"utf8"));
for(const src of scripts)new vm.Script(src);
for(const name of names)assert(scripts[0].includes('"./'+name+'"'),"reader V5 resource not mounted "+name);
const ctx={window:{},encodeURIComponent};
vm.runInNewContext(scripts[1],ctx);
for(const d of [...old,...files])ctx.window.quantMlExpansion.install(core,d);
assert.equal(core.catalog.items.length,776,"V5 expected 776 courses");
assert.equal(new Set(core.catalog.items.map(x=>x.id)).size,776,"duplicate navigable ID");
for(const id of expected){
 const lesson=core.domains[id.slice(0,3)][id];
 assert(lesson?.html.includes("atlas-concept-toc"),"missing lesson body "+id);
 assert.equal((lesson.html.match(/class="atlas-concept-unit"/g)||[]).length,5,"lost detailed sections "+id);
 assert(lesson.html.includes('tabindex="0" role="region"'),"formula keyboard scrolling missing "+id);
}
const near=(a,b,t=1e-8)=>assert(Math.abs(a-b)<t,"numeric invariant "+a+" != "+b);
near((1-.7)**2,.09);
near(.75*Math.log(1.5)+.25*Math.log(.5),.13081203594113697);
near(Math.ceil((4+1)*.8),4);
near(3.4,2+2*.7);
near(7.4,6+2*.7);
near((1+1+2)/4.5,4/4.5);
near(4/(4+2),2/3);
near(.75*(2/3)*1+.25*2*3,2);
near((2+1+1)**2/(4+1+1),16/6);
near(Math.log(2*Math.PI)/2,.9189385332046727);
near(Math.log(2*Math.PI)/2+Math.log(2),1.612085713764618);
near(Math.log(2*Math.PI)/2+2,2.9189385332046727);
near(1.612085713764618+.5,2.112085713764618);
near(.3+.7,1);
const html=fs.readFileSync(new URL("index.html",here),"utf8"),css=fs.readFileSync(new URL("atlas.css",here),"utf8");
for(const id of expected)assert(html.includes("#lesson/"+id),"fifth advanced learning route missing "+id);
assert(html.includes("atlas.js?v=20261109"));
assert(html.includes("atlas.css?v=20261109"));
assert(html.includes("第五条高级学习链"));
assert(html.includes("776 篇系统课程")&&html.includes("0 / 776 篇已标记"),"stale total count in user-facing interface");
assert(css.includes("focus-visible")&&css.includes("@media print"),"responsive Chinese formula style missing");
console.log("PASS: V5 six scoring/CQR/density/PIT/shift chapters, "+sections+" full lessons subchapters, "+formula+" KaTeX expressions, "+sources+" referenced sources; 776 unique reader courses, exact arithmetic and keyboard-accessible formulas.");
