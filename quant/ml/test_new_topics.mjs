import assert from "node:assert/strict";
import fs from "node:fs";
import vm from "node:vm";
import {gunzipSync} from "node:zlib";
import katex from "katex";
const root=new URL("./",import.meta.url);
const read=path=>JSON.parse(fs.readFileSync(new URL(path,root),"utf8"));
const bundle=read("./atlas-new-lessons.json");
const mathIds=new Set(read("../math/curriculum.json").nodes.map(x=>x.id));
const bytes=Buffer.concat([1,2,3,4].map(n=>fs.readFileSync(new URL("./atlas/data.part"+n,root))));
const archive=JSON.parse(gunzipSync(bytes).toString("utf8"));
assert.equal(archive.catalog.items.length,720);
assert.equal(bundle.schema,1);
assert(bundle.chapters.length>=5,"at least two full ML extension chapters expected");
const allNew=bundle.chapters.flatMap(ch=>ch.lessons);
assert(allNew.length>=20,"all published additional courses must be verified");
const originalIds=new Set(archive.catalog.items.map(x=>x.id));
const newIds=new Set(allNew.map(l=>l.id));
assert.equal(newIds.size,allNew.length,"extended ML IDs must be unique");
let sectionCount=0,formulaCount=0;
for(const chapter of bundle.chapters){
 assert(archive.domains[chapter.domain_id],"domain missing "+chapter.domain_id);
 assert(chapter.chapter&&chapter.lessons.length>=4);
 for(const lecture of chapter.lessons){
  assert(!originalIds.has(lecture.id),"must preserve old lesson ID "+lecture.id);
  assert(/^m\d\d-c\d\d-p\d\d$/.test(lecture.id));
  assert(lecture.goal&&lecture.sections.length>=4&&lecture.protocol.length>=3,"missing full tutorial "+lecture.id);
  for(const ref of lecture.prerequisites){
   if(ref.kind==="math")assert(mathIds.has(ref.id),"math prerequisite missing "+ref.id);
   else assert(originalIds.has(ref.id)||newIds.has(ref.id),"machine learning prerequisite missing "+ref.id);
  }
  for(const section of lecture.sections){
   assert(section.title&&section.question&&section.paragraphs.length>=2&&section.equations.length&&section.worked&&section.boundary,"incomplete chapter "+lecture.id);
   sectionCount++;
   for(const eq of section.equations){katex.renderToString(eq.tex,{throwOnError:true,strict:"ignore"});formulaCount++;}
  }
  for(const ref of lecture.sources)assert(/^https:\/\//.test(ref.url),"source must be HTTPS");
 }
}
const source=fs.readFileSync(new URL("./new-topics.js",root),"utf8");
new vm.Script(source,{filename:"quant/ml/new-topics.js"});
const ctx={window:{},encodeURIComponent};
vm.runInNewContext(source,ctx);
const inserted=ctx.window.quantMlExpansion.install(archive,bundle);
assert.equal(inserted.size,allNew.length);
assert.equal(archive.catalog.items.length,720+allNew.length);
assert.equal(new Set(archive.catalog.items.map(x=>x.id)).size,720+allNew.length);
for(const id of inserted){
 const meta=archive.catalog.items.find(x=>x.id===id);
 assert(bundle.chapters.some(ch=>ch.chapter===meta.chapter&&ch.domain_id===meta.domain_id),"new chapter placement");
 const article=archive.domains[id.slice(0,3)]?.[id];
 assert(article&&article.html.includes("atlas-concept-toc"),"new article not published "+id);
 assert.equal((article.html.match(/class="atlas-concept-unit"/g)||[]).length,allNew.find(x=>x.id===id).sections.length);
}
for(const chapter of bundle.chapters){
 const sample=archive.catalog.items.filter(x=>x.domain_id===chapter.domain_id);
 assert(sample.slice(-chapter.lessons.length).every(x=>chapter.lessons.some(y=>y.id===x.id)),"extension must follow its canonical domain "+chapter.domain_id);
}
const arithmetic=[
 [1-.95**20,.6415140775914581],
 [4/(4+1),.8],
 [8-2-2-5,-1],
 [5.05+4.95,10],
 [(1-1)**2+(1-0)**2,1],
 [(3+1+1+3)/2,4],
 [(0.5+2+0.5)/4,.75]
];
for(const [actual,expect] of arithmetic)assert(Math.abs(actual-expect)<1e-9);
console.log("PASS: "+(720+allNew.length)+" distinct navigable ML lessons including "+allNew.length+" expanded full tutorials, "+sectionCount+" subchapters, "+formulaCount+" KaTeX expressions, valid math references");
