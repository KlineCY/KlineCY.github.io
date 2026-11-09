import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
import {gunzipSync} from "node:zlib";
import katex from "katex";
const here=new URL("./",import.meta.url);
const read=filename=>JSON.parse(fs.readFileSync(new URL(filename,here),"utf8"));
const atlas=fs.readFileSync(new URL("atlas.js",here),"utf8");
const html=fs.readFileSync(new URL("index.html",here),"utf8");
const nav=fs.readFileSync(new URL("../sidebar-v2.js",here),"utf8");
new vm.Script(atlas);
new vm.Script(nav);
for(const marker of ['id="quantSidebar"','id="atlas-search"','id="atlas-topic"','id="atlas-clear"','id="atlas-toc"','id="atlas-lesson"','id="atlas-status"','id="atlas-reader-section"'])assert(html.includes(marker),"published Atlas marker absent: "+marker);
assert(html.includes('src="./atlas.js?v=20261109-v115-reader-layout-v116-ml-bridges-v117-no-learning-progress"'),"published page must load versioned Atlas reader");
assert(nav.includes('["ml","机器学习 Machine Learning"]'));
assert(atlas.includes("loadArchive")&&atlas.includes("renderMath")&&atlas.includes("copyText"),"Atlas reader features missing");
assert(atlas.includes("applyDepth")&&atlas.includes('fetchAtlasJson("./atlas-depth-manifest.json"'),"published Atlas must load manifest with bounded retries");
assert(atlas.includes("quantMlConcepts.mount")&&atlas.includes("atlas-concept-jump"),"conceptual subchapters must be mounted");
assert(atlas.includes("quantMlExpansion.install")&&atlas.includes("atlas-new-lessons.json"),"additional in-curriculum full lessons must load");
assert(html.includes("new-topics.js?v=20261103-expanded-v1"),"new topic renderer should be loaded before Atlas");
assert(html.includes("concept-render.js?v=20261103-expanded-v1"),"concept renderer must load before atlas");
assert(atlas.includes("fetchAtlasModulesBounded(depthFiles,6,")&&atlas.includes("onProgress(++completed,files.length)"),"advanced modules must have bounded concurrency and progress callbacks");
const depthManifest=read("atlas-depth-manifest.json");
assert.equal(depthManifest.schema,1);
assert.equal(depthManifest.expected_lessons,720);
assert.equal(depthManifest.expected_domains,24);
assert.equal(depthManifest.modules.length,93);
assert.equal(new Set(depthManifest.modules.map(m=>m.path)).size,93);
assert.equal(depthManifest.modules.reduce((n,m)=>n+m.lessons,0),720);
const depthFiles=depthManifest.modules.map(m=>m.path.replace(/^\.\//,""));
const seen=new Set(),domains=new Set(),depthById=new Map();let workedCount=0;
for(let i=0;i<depthFiles.length;i++){
 const file=depthFiles[i],depth=read(file);
 assert.equal(depth.schema,1,"advanced overlay schema mismatch: "+file);
 assert.equal(Object.keys(depth.lessons).length,depthManifest.modules[i].lessons,"count mismatch: "+file);
 for(const [id,chapter] of Object.entries(depth.lessons)){
  assert(/^m\d\d-c\d\d-p\d\d$/.test(id),"advanced topic ID invalid: "+id);
  assert(!seen.has(id),"duplicate advanced lesson: "+id);seen.add(id);domains.add(id.slice(0,3));depthById.set(id,chapter);
  assert(chapter.title.length>=8&&chapter.overview.length>=60,"thin advanced chapter: "+id);
  for(const field of ["derivation","protocol","pitfalls"])assert(Array.isArray(chapter[field])&&chapter[field].length>=2,"missing "+field+": "+id);
  assert(chapter.equations.length>=2,"missing advanced formulas: "+id);
  assert(typeof chapter.worked_example==="string"&&chapter.worked_example.length>=50,"missing or thin worked example: "+id);workedCount++;
  if(/-(?:[3456]|(?:0[789]|1\d)-[a-z])\.json$/.test(file))assert(chapter.worked_example,"advanced lecture requires worked example: "+id);
  if(/-(?:[456]|(?:0[789]|1\d)-[a-z])\.json$/.test(file))assert(chapter.equations.length>=3&&chapter.worked_example.length>=60,"fourth-pass equation/example incomplete: "+id);
  for(const f of chapter.equations){
   assert(f.label&&f.tex,"missing advanced formula metadata: "+id);
   katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});
  }
 }
}
assert.equal(seen.size,720,"must load 720 advanced lessons");
assert.equal(domains.size,24,"must cover all twenty-four domains");
const perDomain=new Map();
for(const id of seen)perDomain.set(id.slice(0,3),(perDomain.get(id.slice(0,3))||0)+1);
assert([...perDomain.values()].every(n=>n>=22),"each of the 24 topics must have at least twenty-two rigorous in-lesson enhancements");
assert.equal(workedCount,720,"advanced worked-example count mismatch");
const binary=Buffer.concat([1,2,3,4].map(i=>fs.readFileSync(new URL("atlas/data.part"+i,here))));
assert.equal(binary.length,1969862,"published archive bytes changed unexpectedly");
const archive=JSON.parse(gunzipSync(binary).toString("utf8"));
assert.equal(archive.catalog.items.length,720,"Atlas must retain 720 original full lectures");
assert.equal(new Set(archive.catalog.items.map(x=>x.id)).size,720,"Original lecture IDs must be unique");
assert.equal(Object.keys(archive.domains).length,24,"Original domain count differs");
for(const [id,chapter] of depthById){
 const original=archive.domains[id.slice(0,3)]?.[id];
 assert(original,"missing original for "+id);
 assert.equal(chapter.source_title,original.title,"advanced source title mismatch for "+id);
}
const depthFnText=atlas.slice(atlas.indexOf("function applyDepth("),atlas.indexOf("function cleanCodeComments("));
assert(depthFnText.startsWith("function applyDepth("),"Atlas runtime function missing");
const escapeHtml=s=>String(s??"").replace(/[&<>"']/g,c=>({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[c]));
const applyRuntimeDepth=vm.runInNewContext(depthFnText+"\napplyDepth;",{esc:escapeHtml});
const runtimeIds=applyRuntimeDepth(archive,depthFiles.map(read),depthManifest);
assert.equal(runtimeIds.size,720,"runtime overlay count mismatch");
for(const id of seen){
 assert(runtimeIds.has(id),"runtime deep-dive missing: "+id);
 const article=archive.domains[id.slice(0,3)][id].html;
 assert(article.includes('aria-label="进阶推导与研究实践"'),"advanced lesson not attached: "+id);
 assert(article.includes("推导与适用边界")&&article.includes("研究与实现流程"),"advanced sections missing: "+id);
 const mod=depthById.get(id);
 if(mod.worked_example)assert(article.includes("数值或研究示例"),"runtime worked example missing: "+id);
}
assert(atlas.includes("visibleLessons")&&atlas.includes("chapterOpen")&&atlas.includes("domainOpen")&&atlas.includes("data-key"),"searchable hierarchical lesson directory missing");
assert(atlas.includes("atlas-jump-depth")&&atlas.includes("setCatalogOpen")&&atlas.includes("copyText"),"published reader must retain advanced navigation and link copying");
console.log("PASS: 720 archived courses and 720 canonical mathematical enhancements across 24 domains; main navigation and strict KaTeX checks pass.");
