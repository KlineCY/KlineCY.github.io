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
for(const id of ["quantSidebar","atlas-search","atlas-topic","atlas-clear","atlas-toc",
                 "atlas-lesson","atlas-status","atlas-reader-section"])
 assert(html.includes('id="'+id+'"'),"Published reader control missing "+id);
assert.match(html,/src="\.\/atlas\.js\?v=[0-9]{8}"/,
 "Published Atlas script resource missing");
assert(nav.includes('["ml","机器学习 Machine Learning"]'),"Global site curriculum navigation missing");
assert(atlas.includes("loadArchive")&&atlas.includes("renderMath")&&atlas.includes("copyText"),
 "Atlas reading and equation/clipboard controls missing");
assert(atlas.includes("applyDepth")&&atlas.includes('fetchAtlasJson("./atlas-depth-manifest.json"'),
 "Atlas depth manifest and module loader missing");
assert(atlas.includes("quantMlConcepts.mount")&&atlas.includes("atlas-concept-jump"),
 "Canonical in-lesson concept notes missing");
assert(atlas.includes("quantMlExpansion.install")&&atlas.includes("atlas-new-lessons.json"),
 "Sixty Atlas expansion lectures missing");
assert(atlas.includes("fetchAtlasModulesBounded(depthFiles,6,")&&atlas.includes("onProgress(++completed,files.length)"),
 "Bounded module loading missing");
const mathCurriculum=JSON.parse(fs.readFileSync(new URL("../math/curriculum.json",here),"utf8"));
assert.equal(mathCurriculum.nodes.length,656,"Mathematics syllabus missing");
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
assert.equal(archive.catalog.items.length,720,"Published Atlas course index changed");
assert.equal(Object.keys(archive.domains).length,24,"Published Atlas domain count changed");
const ids=new Set(archive.catalog.items.map(x=>x.id));
assert.equal(ids.size,720,"Duplicate Atlas canonical course IDs");
for(const topic of archive.catalog.items){
 assert(archive.domains[topic.domain_id]?.[topic.id],"Missing original canonical lecture "+topic.id);
 assert(typeof topic.title==="string"&&topic.title.length>=2,"Missing course title "+topic.id);
}
assert(atlas.includes("visibleLessons")&&atlas.includes("chapterOpen")&&atlas.includes("domainOpen"),
 "Searchable hierarchical course directory missing");
assert(atlas.includes("atlas-jump-depth")&&atlas.includes("setCatalogOpen"),
 "Course navigation and in-article derivation links missing");

const extendedMLGroups=[
 ["atlas-depth-15-a.json",["m01-c05-p01","m02-c01-p04","m03-c02-p01","m04-c02-p04","m05-c02-p01","m06-c01-p01"]],
 ["atlas-depth-15-b.json",["m07-c01-p01","m08-c01-p01","m09-c02-p03","m10-c04-p03","m11-c04-p05","m12-c04-p03"]],
 ["atlas-depth-16-b.json",["m07-c01-p02","m08-c01-p05","m09-c04-p03","m10-c01-p02","m11-c04-p03","m12-c01-p04"]],
 ["atlas-depth-16-a.json",["m01-c04-p03","m02-c01-p01","m03-c01-p01","m04-c01-p03","m05-c01-p01","m06-c02-p01"]],
 ["atlas-depth-16-c.json",["m13-c05-p01","m14-c04-p03","m15-c05-p01","m16-c01-p01","m17-c02-p01","m18-c02-p03"]]
];
let validatedMLExpansions=0;
for(const [file,expected] of extendedMLGroups){
 const data=read(file);
 assert.deepEqual(Object.keys(data.lessons),expected,"expanded canonical ML lesson order changed "+file);
 for(const [id,l] of Object.entries(data.lessons)){
  assert(l.equations.length>=10&&l.derivation.length>=10&&l.protocol.length>=6&&l.pitfalls.length>=5,"expanded ML article incomplete "+id);
  assert(l.derivation.slice(-2).every(s=>s.length>=85),"expanded ML reasoning underdeveloped "+id);
  assert(l.worked_example.length>=360,"expanded ML example missing "+id);
  assert.equal(new Set(l.equations.map(f=>f.label)).size,l.equations.length,"duplicate expanded ML equations "+id);
  for(const f of l.equations.slice(-2)){
   assert(f.label?.length>=3&&f.tex?.length>=14,"missing extended ML formula "+id);
   katex.renderToString(f.tex,{displayMode:true,throwOnError:true,strict:"error"});
  }
  validatedMLExpansions++;
 }
}
assert.equal(validatedMLExpansions,30,"thirty enriched ML subjects required");
const nearExt=(v,ref)=>assert(Math.abs(v-ref)<1e-7,"extended numerical invariant failed: "+v+" vs "+ref);
nearExt((100.4-100.1)/100.1,.002997002997002997);
nearExt(100**2,10000);
nearExt(.75**2*.04+.25**2*.09,.028125);
nearExt(Math.sqrt(2*Math.log(100)/1000),.09597051824376162);
nearExt(5*(-1.5)**2,11.25);
nearExt(500/(1+(5-1)*.5),500/3);
nearExt(4/(1+.75),16/7);
nearExt((2-1)**2+(0-1)**2,2);
nearExt(.7*0+.3*2,.6);
nearExt(.5*(.5**2+.75**2),.40625);
nearExt(1/(2*Math.PI*.01),15.915494309189535);
nearExt(1/.2,5);
nearExt((5/6)/(1-1/6),1);
nearExt((12-6)/(3-1),3);
nearExt(2/3,.6666666666666666);
nearExt(3/5,.6);
nearExt(1/(1+Math.exp(-4.5)),.9890130573694068);
nearExt(1/20,.05);
nearExt((3+5+0-5)/4,.75);
nearExt(20/Math.sqrt(406),.9925833339709303);
nearExt((.8-.5),.3);
nearExt(.5*.05+.3*.05+.2*.05,.05);
nearExt(1.1-1,.1);
nearExt(-2/Math.sqrt(8/3),-1.224744871391589);
nearExt((.75/(.75+.5+.75)),.375);
nearExt(2*3-1*3,3);
nearExt(.8-.5,.3);
nearExt((1/(1+Math.exp(-2)))*(1-1/(1+Math.exp(-2))),.10499358540350662);
nearExt(Math.log(2),.6931471805599453);
nearExt((.5*1+.5*(-1)),0);

console.log("PASS: canonical 720-course Atlas, complete depth overlays, 24 domains, valid equations, runtime navigation and 656 mathematics prerequisites");
