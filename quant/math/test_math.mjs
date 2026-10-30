import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
import katex from "katex";
const dir=new URL("./",import.meta.url);
const read=f=>JSON.parse(fs.readFileSync(new URL(f,dir),"utf8"));
const c=read("./curriculum.json");
const sources=["./lessons-foundations.json","./lessons-advanced.json","./lessons-more.json"].map(read);
const lessons=sources.flatMap(f=>f.lessons);
const ids=new Set(),chIds=new Set(),deptIds=new Set(),nodeMap=new Map();
assert.equal(c.disciplines.length,20,"discipline coverage count");
assert.equal(c.chapters.length,100,"chapter coverage count");
assert(c.nodes.length>=600,"expected broad topic catalog");
for(let i=0;i<c.disciplines.length;i++){
 const d=c.disciplines[i];
 assert(!deptIds.has(d.id),"duplicate discipline "+d.id);
 deptIds.add(d.id);
 assert.equal(d.order,i+1,"discipline progression order");
 assert(d.resources?.length>0,d.id+": no learning resources");
 for(const r of d.resources){assert(/^https:\/\//.test(r.url),d.id+": invalid resource URL")}
}
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
assert(sidebar.includes('["math","量化数学 Mathematics"]'),"math missing from navigation");
console.log("Math validation PASS: "+c.disciplines.length+" disciplines, "+c.chapters.length+" chapters, "+c.nodes.length+" nodes, "+lessons.length+" complete lessons, all displayed LaTeX formulas parseable.");
