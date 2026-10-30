import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
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
  assert(supplementalIds.has(item.node_id),"focused example not in supplemental notes "+item.node_id);
  assert(!exampleIds.has(item.node_id),"duplicated focused example "+item.node_id);
  assert(typeof item.code==="string"&&item.code.trim().length>10,"missing focused code "+item.node_id);
  exampleIds.add(item.node_id);
}

assert.equal(notes.length,manifest.notes_count,"supplemental note manifest mismatch");
assert.equal(lessons.length+notes.length,curriculum.nodes.length,"some topics have no reading content");
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
for(const l of [...lessons,...notes]){
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
const app=fs.readFileSync(new URL("app.js",here),"utf8");
const html=fs.readFileSync(new URL("index.html",here),"utf8");
const nav=fs.readFileSync(new URL("../sidebar-v2.js",here),"utf8");
new vm.Script(app);
new vm.Script(nav);
for(const marker of ['id="quantSidebar"','id="tree"','id="article"','id="coverageFilter"','id="status"'])assert(html.includes(marker));
assert(nav.includes('["ml","机器学习 Machine Learning"]'));
assert(app.includes("复制代码"));
assert(app.includes("manifest.lesson_files"));
console.log("PASS: 24 ML disciplines, 120 chapters, 720 nodes, "+lessons.length+" full tutorials, "+notes.length+" supplementary notes, all topics covered, LaTeX parses, navigation and JS syntax valid.");
