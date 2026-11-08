import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
import katex from "katex";
const dir=new URL("./",import.meta.url);
const read=f=>JSON.parse(fs.readFileSync(new URL(f,dir),"utf8"));
const c=read("./curriculum.json");
const sources=["./lessons-foundations.json","./lessons-advanced.json","./lessons-more.json"].map(read);
const manifest=read("./lessons-manifest.json");
assert.equal(manifest.version,1);
assert.equal(manifest.expected_lessons,201);
assert.equal(manifest.modules.length,77);
assert.equal(new Set(manifest.modules.map(x=>x.path)).size,77);
assert.equal(manifest.modules.reduce((s,x)=>s+x.lessons,0),manifest.expected_lessons);
const extras=manifest.modules.flatMap(mod=>{
 assert(/^\.\/lessons-phase-\d\d-[a-z]\.json$/.test(mod.path),"invalid module path "+mod.path);
 const body=read(mod.path);
 assert.equal(body.version,1);
 assert.equal(body.lessons.length,mod.lessons);
 return body.lessons;
});
assert.equal(new Set(extras.map(x=>x.node_id)).size,extras.length);
for(const l of extras){
 for(const key of ["definition","assumptions","example","application","caveat"])
  assert(typeof l[key]==="string"&&l[key].length>=50,"thin math "+l.node_id+" "+key);
 assert(l.properties.length>=3&&l.formulas.length>=3&&l.derivation.length>=3,"incomplete proof "+l.node_id);
 for(const step of l.derivation)assert(step.explanation.length>=35&&step.tex,"missing proof step "+l.node_id);
 if(l.deep_sections){
  assert(l.deep_sections.length>=4,"deep mathematical lecture requires logical mini-chapters "+l.node_id);
  for(const s of l.deep_sections){
   assert(s.title&&s.question&&s.paragraphs.length>=2&&s.formulas.length&&s.proof&&s.worked&&s.limitation,"incomplete mini-chapter "+l.node_id);
   for(const eq of s.formulas)katex.renderToString(eq.tex,{throwOnError:true,strict:"ignore"});
  }
  for(const link of l.links||[]){
   if(link.kind==="math")assert(c.nodes.some(n=>n.id===link.id),"broken mathematics link "+link.id);
   else assert(/^m\d\d-c\d\d-p\d\d$/.test(link.id),"broken machine learning link "+link.id);
  }
 }
}
const lessons=[...sources.flatMap(f=>f.lessons),...extras];
const ids=new Set(),chIds=new Set(),deptIds=new Set(),nodeMap=new Map();
assert.equal(c.disciplines.length,20,"discipline coverage count");
assert.equal(c.chapters.length,109,"chapter coverage count");
assert.equal(c.nodes.length,656,"expected expanded topic catalog");
for(let i=0;i<c.disciplines.length;i++){
 const d=c.disciplines[i];
 assert(!deptIds.has(d.id),"duplicate discipline "+d.id);
 deptIds.add(d.id);
 assert.equal(d.order,i+1,"discipline progression order");
 assert.equal(d.chapter_ids.length,d.id==="optimization"?7:["statistics","econometrics","timeseries","finance","riskmath","mlmath","linear"].includes(d.id)?6:5,"discipline chapter count "+d.id);
 assert.equal(new Set(d.chapter_ids).size,d.chapter_ids.length,"duplicate chapter references "+d.id);
 for(const cid of d.chapter_ids)assert(c.chapters.some(ch=>ch.id===cid&&ch.discipline_id===d.id),"unlinked or misassigned chapter "+cid);
 assert(d.resources?.length>0,d.id+": no learning resources");
 for(const r of d.resources){assert(/^https:\/\//.test(r.url),d.id+": invalid resource URL")}
}
const knowledge=new Map(c.nodes.map(n=>[n.id,n]));
const memberships=new Map();
for(const chapter of c.chapters)for(const id of chapter.node_ids)memberships.set(id,(memberships.get(id)||0)+1);
for(const node of c.nodes)assert.equal(memberships.get(node.id),1,"topic appears in exactly one chapter: "+node.id);
const verified=new Set(),exploring=new Set();
function auditDependency(id){
 if(exploring.has(id))throw Error("cyclic mathematical prerequisite: "+id);
 if(verified.has(id))return;
 exploring.add(id);
 for(const p of knowledge.get(id).prerequisites||[]){
  assert(knowledge.has(p),"broken prerequisite: "+id+" -> "+p);
  auditDependency(p);
 }
 exploring.delete(id);verified.add(id);
}
for(const node of c.nodes)auditDependency(node.id);
for(const chapter of c.chapters){
 assert(!chIds.has(chapter.id),"duplicate chapter "+chapter.id);chIds.add(chapter.id);
 assert(deptIds.has(chapter.discipline_id),"missing department "+chapter.id);
 assert(chapter.node_ids.length>=4,chapter.id+": thin chapter");
}
for(const node of c.nodes){
 assert(!ids.has(node.id),"duplicate node "+node.id);ids.add(node.id);nodeMap.set(node.id,node);
 assert(chIds.has(node.chapter_id),"missing chapter "+node.id);
 assert(deptIds.has(node.discipline_id),"missing discipline "+node.id);
 assert(node.title&&node.title.trim(),"empty title");
 if(node.id.includes("-x"))assert(node.summary?.length>=28,"expanded topic must have a substantive scope: "+node.id);
 for(const p of node.prerequisites||[])assert(c.nodes.some(n=>n.id===p),"invalid prerequisite "+node.id+" -> "+p);
}
for(const chapter of c.chapters)for(const id of chapter.node_ids){
 assert(ids.has(id),"unknown topic "+id);
 assert.equal(nodeMap.get(id).chapter_id,chapter.id,"mismatched topic "+id);
}
const lessonsSeen=new Set(),bad=[];
for(const l of lessons){
 assert(nodeMap.has(l.node_id),"lesson missing from curriculum "+l.node_id);
 assert(!lessonsSeen.has(l.node_id),"duplicate lecture "+l.node_id);
 lessonsSeen.add(l.node_id);
 assert(l.title===nodeMap.get(l.node_id).title,"lesson mismatch "+l.node_id);
 for(const key of ["definition","assumptions","example","application","caveat"]){
  assert(typeof l[key]==="string"&&l[key].trim().length>=6,l.node_id+": missing "+key);
 }
 assert(Array.isArray(l.properties)&&l.properties.length>=2,l.node_id+": incomplete properties");
 assert(Array.isArray(l.formulas)&&l.formulas.length>=1,l.node_id+": missing formulas");
 assert(Array.isArray(l.derivation)&&l.derivation.length>=2,l.node_id+": incomplete derivation");
 for(const formula of l.formulas)try{
   katex.renderToString(formula.tex,{throwOnError:true,strict:"ignore"});
 }catch(e){bad.push(l.title+" formula "+formula.tex.slice(0,90)+" "+e.message)}
 for(const proof of l.derivation)try{
   assert(proof.explanation.trim().length>=3,"proof prose missing "+l.title);
   katex.renderToString(proof.tex,{throwOnError:true,strict:"ignore"});
 }catch(e){bad.push(l.title+" proof "+proof.tex.slice(0,90)+" "+e.message)}
}
assert.equal(bad.length,0,"Invalid KaTeX formulas:\n"+bad.join("\n"));
assert(lessons.length>=50,"expected substantial written proof coverage");
const html=fs.readFileSync(new URL("./index.html",dir),"utf8");
const app=fs.readFileSync(new URL("./app.js",dir),"utf8");
const sidebar=fs.readFileSync(new URL("../sidebar-v2.js",dir),"utf8");
new vm.Script(app,{filename:"quant/math/app.js"});
new vm.Script(sidebar,{filename:"quant/sidebar-v2.js"});
for(const marker of ['id="quantSidebar"','id="tree"','id="article"','id="disciplineFilter"','id="status"']){
 assert(html.includes(marker),"missing element "+marker);
}
assert(app.includes("loadMathAdditions()")&&app.includes("fetchMathJson("),"math module loader absent");
assert(app.includes("appendLearningSequence(node,article)")&&app.includes("math-learning-sequence-links"),"mathematics reader must show previous and following linked knowledge points");
 for(const name of ["curriculum.json","lessons-foundations.json","lessons-advanced.json","lessons-more.json"])
  assert(app.includes('fetch("./'+name+'",{cache:"no-cache"})'),"core math resource must revalidate cache "+name);
assert(html.includes("app.js?v=20261108-v99-dynamics-pde-complete"),"current versioned math loader missing");
assert(html.includes("styles.css?v=20261104-probscore-v6"),"versioned math styles missing");
for(const id of ["linear-x001","linear-x002","linear-x003","linear-x004","linear-x005","linear-x006"])
 assert(html.includes("?topic="+id),"missing mathematical spectral path entry "+id);
assert(html.includes("../ml/#lesson/m11-c06-p01"),"missing linked machine learning spectral path");
assert(app.includes("renderMathDeepSections")&&app.includes("math-deep-toc"),"mathematics subchapter reader missing");
assert(sidebar.includes('["math","量化数学 Mathematics"]'),"math missing from navigation");
console.log("Math validation PASS: "+c.disciplines.length+" disciplines, "+c.chapters.length+" chapters, "+c.nodes.length+" nodes, "+lessons.length+" complete lessons, all displayed LaTeX formulas parseable.");
