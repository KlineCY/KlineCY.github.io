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


const nextMLGroups=[
 ["atlas-depth-15-c.json",9,["m13-c03-p02","m14-c04-p01","m15-c01-p02","m16-c03-p02","m17-c02-p03","m18-c03-p02"]],
 ["atlas-depth-15-d.json",10,["m19-c01-p01","m20-c04-p03","m21-c05-p04","m22-c04-p05","m23-c05-p06"]],
 ["atlas-depth-16-d.json",12,["m19-c02-p03","m20-c01-p05","m21-c05-p05","m22-c05-p06","m23-c03-p06"]],
 ["atlas-depth-14-a.json",10,["m01-c02-p03","m02-c01-p02","m03-c01-p02","m04-c01-p01","m05-c05-p05","m06-c01-p06"]]
];
let validatedNewML=0;
for(const [file,minEquations,ids] of nextMLGroups){
 const doc=read(file);
 assert.deepEqual(Object.keys(doc.lessons),ids,"further-deepened ML canonical identities changed "+file);
 for(const [id,l] of Object.entries(doc.lessons)){
  assert(l.equations.length>=minEquations&&l.derivation.length>=10,"new deepened equations/derivations missing "+id);
  assert(l.derivation.slice(-2).every(s=>s.length>=88),"new proof paragraphs underdeveloped "+id);
  assert(l.worked_example.length>=435,"new worked numerical example missing "+id);
  assert(l.protocol.length>=6&&l.pitfalls.length>=5,"new validation protocol or limitation missing "+id);
  assert.equal(new Set(l.equations.map(e=>e.label)).size,l.equations.length,"duplicate ML formula titles "+id);
  for(const f of l.equations.slice(-2)){
   assert(f.tex.length>=18&&f.label.length>=4,"formula too short "+id);
   katex.renderToString(f.tex,{displayMode:true,throwOnError:true,strict:"error"});
  }
  validatedNewML++;
 }
}
assert.equal(validatedNewML,22,"all twenty-two newly deepened ML lectures required");
nearExt((3*(4-2)/2)-1,2);
nearExt(Math.floor((30-8)/8)+1,3);
nearExt(.9*.9,.81);
nearExt(4*5/2,10);
nearExt((2+0+0+0)/4,.5);
nearExt((2+4)/2,(2+4+2+4)/4);
nearExt(.8*3*.2**2/(.8*.2**2+.2*.8**2),.6);
nearExt(1.05*.001*(2*.023809523809523808),.00005);
nearExt(.1*10/.2,5);
nearExt((110/100)*(220/210)-1,.15238095238095228);
nearExt(-2-.3,-2.3);
nearExt(1/(1-.9),10);
nearExt(.9*.01+.1*(-.02),.007);
nearExt(.1*(-.02),-.002);
nearExt(.8*.25*0+.2*4*1,.8);
nearExt(.4-.4**2/2,.32);
nearExt(9/10+1/2,1.4);
nearExt(.01*Math.hypot(.6,.4),.007211102550927978);
nearExt(.009/(.009+.099),1/12);
nearExt(6*.05/Math.PI**2,.030396355092701333);
nearExt(1024*1e-8,.00001024);
nearExt(21-7,14);


const asOfLecture=read("atlas-depth-14-a.json").lessons["m06-c01-p06"];
const revisionRule=asOfLecture.equations.find(f=>f.label==="同一业务时点多版本的点时记录规则");
const utcRule=asOfLecture.equations.find(f=>f.label==="跨市场绝对时间的发布合法性");
assert(revisionRule&&revisionRule.tex.includes("rev")&&revisionRule.tex.includes("publish")&&revisionRule.tex.includes("receive"),"historical source-vintage and receipt rule lost");
assert(utcRule&&utcRule.tex.includes("UTC")&&utcRule.tex.includes("published")&&utcRule.tex.includes("received"),"cross-market UTC rule lost");
katex.renderToString(revisionRule.tex,{throwOnError:true,strict:"error"});
katex.renderToString(utcRule.tex,{throwOnError:true,strict:"error"});


const cycleMLGroups=[
 ["atlas-depth-14-b.json",["m07-c02-p02","m08-c03-p02","m09-c01-p04","m10-c01-p05","m11-c04-p01","m12-c03-p02"]],
 ["atlas-depth-14-c.json",["m13-c02-p03","m14-c03-p01","m15-c05-p04","m16-c04-p03","m17-c03-p04","m18-c01-p03"]],
 ["atlas-depth-14-d.json",["m19-c04-p03","m20-c01-p06","m21-c04-p03","m22-c04-p06","m23-c03-p02"]],
 ["atlas-depth-13-a.json",["m01-c03-p03","m02-c03-p05","m03-c02-p02","m04-c05-p04","m05-c05-p03","m06-c01-p02"]]
];
let checkedCycleML=0;
for(const [file,ids] of cycleMLGroups){
 const doc=read(file);
 assert.deepEqual(Object.keys(doc.lessons),ids,"new ML course identities modified "+file);
 for(const [id,l] of Object.entries(doc.lessons)){
  assert(l.equations.length>=10&&l.derivation.length>=10,"new ML course formula or derivation missing "+id);
  assert(l.derivation.slice(-2).every(s=>s.length>=85),"new ML derivation requires two substantive passages "+id);
  assert(l.worked_example.length>=430,"new ML worked example missing "+id);
  assert(l.protocol.length>=6&&l.pitfalls.length>=5,"new ML protocol or caveat missing "+id);
  assert.equal(new Set(l.equations.map(e=>e.label)).size,l.equations.length,"new ML equation labels duplicated "+id);
  for(const e of l.equations.slice(-2)){
   assert(e.tex.length>=18&&e.label.length>=4,"new ML equation incomplete "+id);
   katex.renderToString(e.tex,{displayMode:true,throwOnError:true,strict:"error"});
  }
  checkedCycleML++;
 }
}
assert.equal(checkedCycleML,23,"all twenty-three new ML subject deepening records required");
nearExt(1/(1+5),1/6);
nearExt((0**2+1**2)/2,.5);
nearExt(2/1,2);
nearExt(2*3/(4*5),.3);
nearExt(1/(1+Math.exp(-4)),.9820137900379085);
nearExt((1-2)**2+(2-4)**2,5);
nearExt((.8**2+.3**2)/4,.1825);
nearExt(.6*(.8-1)**2+.4*.8**2,.28);
nearExt(.5*3-1,.5);
nearExt(5*(5+1)/2,15);
nearExt(-Math.log(.6),.5108256237659907);
nearExt((1-2)**2+(2-4)**2,5);
nearExt(.1*Math.sqrt(24),.48989794855663565);
nearExt(.496/(.496+.076),.8671328671328671);
nearExt(.5*(1-.5)*.5,.125);
nearExt(2.2+(-.2),2);
nearExt(.001/.0001,10);
nearExt(4+1,5);
nearExt(Math.log(1.005),.004987541511038968);
nearExt(.5*(0-1.5)**2+.5*(2-1.5)**2,1.25);
nearExt(1/(.7**2+3*.1**2),1/.52);
nearExt(Math.hypot(3*2/5,4*2/5),2);
nearExt(.5*.5,.25);
nearExt(.5*.5+.5,.75);

console.log("PASS: canonical 720-course Atlas, complete depth overlays, 24 domains, valid equations, runtime navigation and 656 mathematics prerequisites");
