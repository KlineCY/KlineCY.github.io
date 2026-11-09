import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
import katex from "katex";
import {gunzipSync} from "node:zlib";
const dir=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,dir),"utf8"));
const core=JSON.parse(gunzipSync(Buffer.concat([1,2,3,4].map(i=>fs.readFileSync(new URL("atlas/data.part"+i,dir))))).toString("utf8"));
const mathIds=new Set(read("../math/curriculum.json").nodes.map(x=>x.id));
const oldNames=["atlas-new-lessons.json","atlas-volatility-chapter.json","atlas-linear-regularization-chapter.json","atlas-evaluation-evidence-chapter.json","atlas-generalization-v2.json","atlas-kernel-covariance-v2.json","atlas-stability-kernel-v3.json","atlas-robust-validation-v3.json"];
const prior=oldNames.map(read),priorIds=new Set(prior.flatMap(d=>d.chapters.flatMap(c=>c.lessons.map(l=>l.id))));
const names=["atlas-quantile-calibration-v4.json","atlas-tailrisk-models-v4.json"],docs=names.map(read);
const rows=docs.flatMap(d=>d.chapters.flatMap(c=>c.lessons));
const expected=["m04-c09-p01","m12-c09-p01","m12-c09-p02","m22-c07-p01","m23-c09-p01","m23-c09-p02"];
assert.deepEqual(rows.map(x=>x.id),expected,"ordered V4 curriculum IDs");
assert.equal(new Set(expected).size,6);
for(const d of docs)assert(d.schema===1&&d.chapters.length>=2);
let units=0,tex=0,sources=0;
for(const l of rows){
 assert(!priorIds.has(l.id)&&!core.catalog.items.some(x=>x.id===l.id),"duplicate ML lecture ID "+l.id);
 assert(l.goal.length>=70&&l.protocol.length>=4&&l.sources.length>=2);
 assert.equal(l.sections.length,5);
 assert.equal(new Set(l.sections.map(s=>s.title)).size,5);
 for(const s of l.sections){
  assert(s.question?.length>=12&&s.paragraphs.length===2&&s.paragraphs.every(x=>x.length>=55),"incomplete Chinese reasoning "+l.id+" "+s.title);
  assert(s.worked.length>=40&&s.boundary.length>=35,"missing worked quantitative example or caveat "+l.id+" "+s.title);
  assert(s.equations.length>=1);
  for(const f of s.equations){assert(f.label&&f.tex);katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});tex++;}
  units++;
 }
 for(const link of l.prerequisites){
  assert(link.label);
  if(link.kind==="math")assert(mathIds.has(link.id),"broken mathematical link "+link.id);
  else{assert(core.catalog.items.some(x=>x.id===link.id)||priorIds.has(link.id)||expected.includes(link.id),"missing ML prerequisite "+link.id);if(expected.includes(link.id))assert(expected.indexOf(link.id)<expected.indexOf(l.id),"forward prerequisite "+l.id+" → "+link.id);}
 }
 for(const s of l.sources){assert(s.title&&/^https:\/\//.test(s.url));sources++;}
}
assert(!/\uFFFD/.test(JSON.stringify(rows)),"replacement character in Chinese or mathematical text");
const scripts=["atlas.js","new-topics.js"].map(x=>fs.readFileSync(new URL(x,dir),"utf8"));
for(const s of scripts)new vm.Script(s);
for(const f of names)assert(scripts[0].includes('"./'+f+'"'),"runtime V4 data missing: "+f);
const ctx={window:{},encodeURIComponent};
vm.runInNewContext(scripts[1],ctx);
for(const d of [...prior,...docs])ctx.window.quantMlExpansion.install(core,d);
assert.equal(core.catalog.items.length,770,"full 770-lesson runtime count");
assert.equal(new Set(core.catalog.items.map(x=>x.id)).size,770,"duplicate runtime lesson ID");
for(const id of expected){
 const lesson=core.domains[id.slice(0,3)][id];
 assert(lesson?.html?.includes("atlas-concept-toc"),"not rendered to article "+id);
 assert.equal((lesson.html.match(/class="atlas-concept-unit"/g)||[]).length,5,"depth lost in reader "+id);
 assert(core.catalog.items.some(x=>x.id===id));
}
const close=(a,b,t=1e-9)=>assert(Math.abs(a-b)<t,"arithmetic invariant "+a+" ≠ "+b);
close(6.8/5,1.36);
close(Math.ceil((9+1)*.8),8);
close(.8,.8);
close(.95*.02,.019);
close(1.96*.02,.0392);
close(1.645*.02,.0329);
close(.25*(.01**2+.02**2)+2*.5*.5*.25*.01*.02,.00015);
close(Math.sqrt(.00015),.01224744871391589);
close(Math.ceil(5*.8),4);
close(Math.sqrt(1000*.05*.95),Math.sqrt(47.5));
close((1+1+1+1-4)/5,0);
close(1-.95**20,.6415140775914581);
const html=fs.readFileSync(new URL("index.html",dir),"utf8"),css=fs.readFileSync(new URL("atlas.css",dir),"utf8");
for(const id of expected)assert(html.includes("#lesson/"+id),"new learning path missing "+id);
assert(html.includes("atlas.js?v=20261109-v115-reader-layout"));
assert(html.includes("atlas.css?v=20261109-v115-reader-layout"));
assert(html.includes("第四条高级学习链"));
assert(css.includes("V4 probabilistic-forecast reading")&&css.includes("overscroll-behavior-x:contain")&&css.includes("@media print"),"math layout regression");
console.log("PASS: six V4 distributional forecast and risk lectures, "+units+" full subchapters, "+tex+" KaTeX expressions, "+sources+" research references, 770 distinct navigable lessons and numeric/CJK layout checks.");
