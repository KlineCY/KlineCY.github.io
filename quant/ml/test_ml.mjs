import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
import {gunzipSync} from "node:zlib";
import katex from "katex";
const here=new URL("./",import.meta.url);
const read=filename=>JSON.parse(fs.readFileSync(new URL(filename,here),"utf8"));
const curriculum=read("curriculum.json");
const manifest=read("manifest.json");
const files=manifest.lesson_files;
const lessons=files.flatMap(p=>read(p).lessons);
const notes=(manifest.note_files||[]).flatMap(p=>read(p).notes);
const extraFiles=fs.readdirSync(new URL(".",here)).filter(name=>/^lessons-full-[a-z-]+\.json$/.test(name));
const examples=extraFiles.flatMap(p=>read(p).lessons);
const exampleIds=new Set();
const supplementalIds=new Set(notes.map(n=>n.node_id));
assert(extraFiles.length>0,"expected dedicated example drafts");
for(const item of examples){
  assert(supplementalIds.has(item.node_id)||lessons.some(l=>l.node_id===item.node_id),"focused example does not map to any catalog lesson "+item.node_id);
  assert(!exampleIds.has(item.node_id),"duplicated focused example "+item.node_id);
  assert(typeof item.code==="string"&&item.code.trim().length>10,"missing focused code "+item.node_id);
  exampleIds.add(item.node_id);
}

assert.equal(notes.length,manifest.notes_count,"supplemental note manifest mismatch");
const lessonIds=new Set(lessons.map(l=>l.node_id));
const activeNotes=notes.filter(n=>!lessonIds.has(n.node_id));
const catalogIds=new Set(curriculum.nodes.map(n=>n.id));
const activeIds=[...lessons,...activeNotes].map(n=>n.node_id);
const duplicateIds=activeIds.filter((id,i)=>activeIds.indexOf(id)!==i);
const missingIds=[...catalogIds].filter(id=>!activeIds.includes(id));
const unexpectedIds=[...new Set(activeIds)].filter(id=>!catalogIds.has(id));
assert.equal(missingIds.length,0,"catalog topics missing reading content: "+missingIds.join(","));
assert.equal(unexpectedIds.length,0,"unexpected tutorial or note IDs: "+unexpectedIds.join(","));
assert.equal(duplicateIds.length,0,"duplicated tutorial or active note IDs: "+duplicateIds.join(","));
assert.equal(lessons.length,manifest.total_lessons,"manifest count mismatch");
const topicIDs=new Set(),chapterIDs=new Set(),deptIDs=new Set();
assert.equal(curriculum.disciplines.length,24);
assert.equal(curriculum.chapters.length,120);
assert.equal(curriculum.nodes.length,720);
for(let i=0;i<curriculum.disciplines.length;i++){
 const d=curriculum.disciplines[i];
 assert(!deptIDs.has(d.id),"duplicate department "+d.id);deptIDs.add(d.id);
 assert.equal(d.order,i+1,"progression order");
 assert(d.resources.length>0,"missing resources "+d.id);
 for(const r of d.resources)assert(/^https?:\/\//.test(r.url),"invalid resource "+d.id);
}
for(const ch of curriculum.chapters){
 assert(!chapterIDs.has(ch.id),"duplicate chapter "+ch.id);chapterIDs.add(ch.id);
 assert(deptIDs.has(ch.discipline_id),"unknown department "+ch.id);
 assert(ch.node_ids.length>=5,"thin chapter "+ch.id);
}
const nodeMap=new Map();
for(const n of curriculum.nodes){
 assert(!topicIDs.has(n.id),"duplicate node "+n.id);
 topicIDs.add(n.id);nodeMap.set(n.id,n);
 assert(chapterIDs.has(n.chapter_id),"unknown chapter "+n.id);
 assert(deptIDs.has(n.discipline_id),"unknown discipline "+n.id);
}
for(const ch of curriculum.chapters)for(const id of ch.node_ids)assert.equal(nodeMap.get(id)?.chapter_id,ch.id);
assert(lessons.length>=150,"tutorial expansion unexpectedly missing");
const covered=new Set(),katexErrors=[];
for(const l of [...lessons,...activeNotes]){
 assert(topicIDs.has(l.node_id),"lesson missing from curriculum "+l.node_id);
 assert(!covered.has(l.node_id),"duplicate tutorial "+l.node_id);covered.add(l.node_id);
 assert.equal(l.title,nodeMap.get(l.node_id).title);
 for(const k of ["definition","assumptions","application","limits","code"]){
  assert(typeof l[k]==="string"&&l[k].trim().length>=12,l.title+": missing "+k);
 }
 assert(l.properties.length>=2,l.title+": missing properties");
 assert(l.formulas.length>=1,l.title+": missing formulas");
 assert(l.derivation.length>=2,l.title+": missing derivation");
 for(const f of l.formulas)try{katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});}
 catch(e){katexErrors.push(l.title+" FORMULA: "+e.message+" "+f.tex.slice(0,100));}
 for(const step of l.derivation)try{
  assert(step.explanation.trim().length>=3,"missing derivation explanation "+l.title);
  katex.renderToString(step.tex,{throwOnError:true,strict:"ignore"});
 }catch(e){katexErrors.push(l.title+" PROOF: "+e.message+" "+step.tex.slice(0,100));}
}
assert.equal(katexErrors.length,0,"LaTeX errors:\n"+katexErrors.join("\n"));
// The published ML entry is the Atlas reader. The former app is retained only as legacy.
const app=fs.readFileSync(new URL("app.js",here),"utf8");
const atlas=fs.readFileSync(new URL("atlas.js",here),"utf8");
const html=fs.readFileSync(new URL("index.html",here),"utf8");
const nav=fs.readFileSync(new URL("../sidebar-v2.js",here),"utf8");
new vm.Script(app);
new vm.Script(atlas);
new vm.Script(nav);
for(const marker of ['id="quantSidebar"','id="atlas-search"','id="atlas-topic"','id="atlas-toc"','id="atlas-lesson"','id="atlas-status"','id="atlas-depth-only"','id="atlas-depth-count"','id="atlas-unread-only"','id="atlas-resume"','id="atlas-export"','id="atlas-import"','id="atlas-import-file"'])assert(html.includes(marker),"published Atlas marker absent: "+marker);
assert(html.includes('src="./atlas.js"'),"published page must load Atlas reader");
assert(nav.includes('["ml","机器学习 Machine Learning"]'));
assert(atlas.includes("loadArchive")&&atlas.includes("renderMath")&&atlas.includes("copyText"),"Atlas reader features missing");
assert(atlas.includes("applyDepth")&&atlas.includes('fetchAtlasJson("./atlas-depth-manifest.json"'),"published Atlas must load manifest with bounded retries");
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
  if(chapter.worked_example){assert(chapter.worked_example.length>=50,"thin worked example: "+id);workedCount++;}
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
assert.equal(workedCount,664,"advanced worked-example count mismatch");
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
assert(atlas.includes("depthOnly")&&atlas.includes("atlas-advanced-mark")&&atlas.includes("state.worked"),"advanced lesson filter and directory labels missing");
assert(atlas.includes("unreadOnly")&&atlas.includes("continueUnread")&&atlas.includes("atlas-jump-depth"),"published reader must support unread filtering, resuming and in-lesson advanced navigation");
assert(app.includes("manifest.lesson_files"),"legacy lesson reader unexpectedly removed");
console.log("PASS: 720 rigorous advanced lessons across 24 domains; legacy curriculum 24 disciplines / 120 chapters / 720 nodes; published Atlas navigation and JS parse; "+lessons.length+" legacy tutorials, "+activeNotes.length+" active notes; all legacy LaTeX syntax valid.");
