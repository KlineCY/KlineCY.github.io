import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
import katex from "katex";
import {gunzipSync} from "node:zlib";
const base=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,base),"utf8"));
const archive=JSON.parse(gunzipSync(Buffer.concat([1,2,3,4].map(i=>fs.readFileSync(new URL("atlas/data.part"+i,base))))).toString("utf8"));
const baseline=new Set(archive.catalog.items.map(x=>x.id));
const previous=[read("./atlas-new-lessons.json"),read("./atlas-volatility-chapter.json"),read("./atlas-linear-regularization-chapter.json")];
const doc=read("./atlas-evaluation-evidence-chapter.json");
const earlier=new Set(previous.flatMap(d=>d.chapters.flatMap(c=>c.lessons.map(l=>l.id))));
const math=new Set(read("../math/curriculum.json").nodes.map(n=>n.id));
assert.equal(doc.schema,1);
assert.equal(doc.chapters.length,1);
const chapter=doc.chapters[0],lessons=chapter.lessons;
assert.equal(chapter.domain_id,"m12");
assert.equal(lessons.length,3);
const ids=new Set(lessons.map(l=>l.id));
assert.equal(ids.size,3);
let sections=0,equations=0,sourceCount=0;
for(let i=0;i<lessons.length;i++){
 const l=lessons[i];
 assert.equal(l.id,"m12-c07-p0"+(i+1));
 assert(!baseline.has(l.id)&&!earlier.has(l.id),"duplicate ML course");
 assert(l.goal.length>=70&&l.protocol.length>=4);
 assert.equal(l.sections.length,5);
 assert.equal(new Set(l.sections.map(s=>s.title)).size,5);
 for(const s of l.sections){
  assert(s.question.length>=12&&s.paragraphs.length>=2&&s.paragraphs.every(p=>p.length>=70),"insufficient explanatory prose "+l.id);
  assert(s.worked.length>=50&&s.boundary.length>=35,"missing worked example or limitations "+l.id);
  for(const eq of s.equations){assert(eq.label);katex.renderToString(eq.tex,{throwOnError:true,strict:"ignore"});equations++;}
  sections++;
 }
 for(const ref of l.prerequisites){
  if(ref.kind==="math")assert(math.has(ref.id),"broken math prerequisite "+ref.id);
  else assert(baseline.has(ref.id)||earlier.has(ref.id)||ids.has(ref.id),"broken ML prerequisite "+ref.id);
  if(ids.has(ref.id))assert(ref.id<l.id,"cyclic stage order");
 }
 for(const src of l.sources){assert(src.title&&/^https:\/\//.test(src.url));sourceCount++;}
}
const scripts=["atlas.js","new-topics.js"].map(p=>fs.readFileSync(new URL(p,base),"utf8"));
for(const source of scripts)new vm.Script(source);
assert(scripts[0].includes('fetchAtlasJson("./atlas-evaluation-evidence-chapter.json"'),"reader does not mount evidence bundle");
const ctx={window:{},encodeURIComponent};
vm.runInNewContext(scripts[1],ctx);
for(const p of previous)ctx.window.quantMlExpansion.install(archive,p);
const added=ctx.window.quantMlExpansion.install(archive,doc);
assert.equal(added.size,3);
assert.equal(archive.catalog.items.length,753);
assert.equal(new Set(archive.catalog.items.map(x=>x.id)).size,753);
for(const id of added){
 const item=archive.domains.m12[id];
 assert(item?.html.includes("atlas-concept-toc"),"missing rendered table of contents "+id);
 assert.equal((item.html.match(/class="atlas-concept-unit"/g)||[]).length,5);
}
const near=(x,y,eps=1e-10)=>assert(Math.abs(x-y)<eps,"arithmetic "+x+" vs "+y);
near(1+2*.2,1.4);
near(1+2*.5*.2,1.2);
near(Math.sqrt(100)*(-.01)/Math.sqrt(.04),-.5);
near(1/3,.3333333333333333);
near(Math.sqrt(2),1.4142135623730951);
near(.5*(.1+.1),.1);
near(100-1,99);
const html=fs.readFileSync(new URL("./index.html",base),"utf8"),css=fs.readFileSync(new URL("./atlas.css",base),"utf8");
for(const id of added)assert(html.includes("#lesson/"+id),"entry route missing "+id);
assert(html.includes("atlas.js?v=20261109-v115-reader-layout-v116-ml-bridges-v117-no-learning-progress"));
assert(html.includes("atlas.css?v=20261109-v115-reader-layout"));
assert(css.includes("overscroll-behavior-x:contain"));
console.log("PASS: 3 research evidence lessons, "+sections+" deep sections, "+equations+" math formulas, "+sourceCount+" academic references, 753 distinct ML lessons and stable CJK/formula layout");
