import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const dir=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,dir),"utf8"));
const curriculum=read("curriculum.json"),manifest=read("lessons-manifest.json");
const rows=[...["lessons-foundations.json","lessons-advanced.json","lessons-more.json"].flatMap(p=>read(p).lessons),...manifest.modules.flatMap(x=>read(x.path).lessons)];
const lesson=new Map(rows.map(x=>[x.node_id,x])),nodes=new Map(curriculum.nodes.map(x=>[x.id,x]));
assert.equal(curriculum.nodes.length,656);
assert.equal(manifest.expected_lessons,181);
assert.equal(manifest.modules.length,71);
assert(rows.length>=146,"V5 baseline requires at least 146 full mathematics lessons");
assert.equal(lesson.size,rows.length,"duplicate completed math node ID");
const ids=["information-n0482","information-n0483","information-n0486","mlmath-x005","information-x001","information-x002"];
for(const id of ids)assert(nodes.has(id),"V5 mathematics node not indexed "+id);
assert(curriculum.chapters.find(x=>x.id==="information-c1").node_ids.includes("information-x001"));
assert(curriculum.chapters.find(x=>x.id==="information-c1").node_ids.includes("information-x002"));
assert(nodes.get("information-n0483").prerequisites.includes("information-n0482"));
assert(nodes.get("mlmath-x005").prerequisites.includes("information-n0483"));
assert(nodes.get("information-x002").prerequisites.includes("information-x001"));
let sections=0,formula=0,refs=0;
for(const id of ids){
 const l=lesson.get(id);
 assert(l&&l.title===nodes.get(id).title,"full V5 article missing "+id);
 for(const key of ["definition","assumptions","example","application","caveat"])assert(l[key].length>=75,"truncated V5 article "+id+" "+key);
 assert(l.properties.length>=4&&l.formulas.length>=4&&l.derivation.length>=4);
 assert(l.deep_sections.length>=4,"V5 lecture must preserve at least four proof sections: "+id);
 assert.equal(new Set(l.deep_sections.map(x=>x.title)).size,l.deep_sections.length,"duplicate deep section: "+id);
 for(const x of l.formulas){assert(x.label&&x.tex);katex.renderToString(x.tex,{throwOnError:true,strict:"ignore"});formula++;}
 for(const x of l.derivation){assert(x.explanation.length>=35);katex.renderToString(x.tex,{throwOnError:true,strict:"ignore"});formula++;}
 for(const x of l.deep_sections){
  assert(x.question.length>=12&&x.paragraphs.length===2&&x.paragraphs.every(p=>p.length>=65),"short mathematical subchapter "+id+" "+x.title);
  assert(x.proof.length>=55&&x.worked.length>=55&&x.limitation.length>=43,"missing proof, worked calculation or failure condition "+id+" "+x.title);
  for(const y of x.formulas){katex.renderToString(y.tex,{throwOnError:true,strict:"ignore"});formula++;}
  sections++;
 }
 assert(l.sources.length>=2&&l.links.some(x=>x.kind==="ml"));
 for(const src of l.sources){assert(src.title&&src.url.startsWith("https://"));refs++;}
 for(const x of l.links)if(x.kind==="math")assert(nodes.has(x.id),"broken mathematics link "+x.id);
}
const close=(a,b,t=1e-8)=>assert(Math.abs(a-b)<t,"numerical mismatch "+a+" vs "+b);
close(-.8*Math.log(.6)-.2*Math.log(.4),.5919186453876236);
close(.75*Math.log(1.5)+.25*Math.log(.5),.13081203594113697);
close(4*.2/(4*.2+.8),.5);
close(.75*(2/3)*1+.25*2*3,2);
close((2+1+1)**2/(4+1+1),16/6);
close((1-.7)**2,.09);
close(.7**2,.49);
close(2*.5,1);
close(.3+.7,1);
const html=fs.readFileSync(new URL("index.html",dir),"utf8");
const css=fs.readFileSync(new URL("styles.css",dir),"utf8");
const js=fs.readFileSync(new URL("app.js",dir),"utf8");
for(const id of ids)assert(html.includes("?topic="+id),"unlinked fifth math learning route "+id);
assert(html.includes("styles.css?v=20261104-probscore-v6"));
assert(html.includes("第五条高级知识链"));
assert(css.includes("V5 proper scoring")&&css.includes("overflow-x:auto")&&css.includes("@media print"),"CJK formula accessibility regression");
assert(js.includes('div.tabIndex=0')&&js.includes('div.setAttribute("role","region")'),"long math keyboard access missing");
console.log("PASS: V5 mathematics 6 complete scoring/shift lectures, "+sections+" detailed proof sections, "+formula+" strict KaTeX formulas, "+refs+" primary references, 656 nodes, "+rows.length+" full lessons, numeric and Chinese formula UX checks.");
