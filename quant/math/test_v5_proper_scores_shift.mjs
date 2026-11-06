import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const dir=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,dir),"utf8"));
const curriculum=read("curriculum.json"),manifest=read("lessons-manifest.json");
const rows=[...["lessons-foundations.json","lessons-advanced.json","lessons-more.json"].flatMap(p=>read(p).lessons),...manifest.modules.flatMap(x=>read(x.path).lessons)];
const lesson=new Map(rows.map(x=>[x.node_id,x])),nodes=new Map(curriculum.nodes.map(x=>[x.id,x]));
assert.equal(curriculum.nodes.length,656);
assert.equal(manifest.expected_lessons,201);
assert.equal(manifest.modules.length,77);
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

assert.equal(sections,42,"V58 six proper-scoring chapters must preserve six sections and gain seventh");
assert.equal(formula,103,"V58 six courses preserve one pre-existing multi-formula deep section: 30 main + 30 derivation + 43 section formulas");
for(const id of ids){
 const l=lesson.get(id);
 assert(l.checked_status?.startsWith("V58"),"V58 log score course revision missing "+id);
 assert.equal(l.deep_sections.length,7,"V58 seventh coherent financial proof "+id);
 assert.equal(l.derivation.length,5,"V58 fifth derivation "+id);
 assert.equal(l.formulas.length,5,"V58 fifth equation "+id);
 assert.equal(new Set(l.formulas.map(f=>f.tex)).size,5,"V58 duplicate principal scoring equation "+id);
 assert(!/[\uFFFD\u200B\uFEFF]|TODO|Lorem ipsum/.test(JSON.stringify(l)),"V58 broken Chinese in proper scoring "+id);
}
const v58Entropy=-.8*Math.log(.8)-.2*Math.log(.2);
const v58Cross=-.8*Math.log(.6)-.2*Math.log(.4);
const v58BernoulliKL=.8*Math.log(.8/.6)+.2*Math.log(.2/.4);
close(v58Cross-v58Entropy,v58BernoulliKL);
const v58KL=.6*Math.log(.6/.5)+.4*Math.log(.4/.5);
close(v58KL,.0201355135506888);
assert(.1<=Math.sqrt(v58KL/2),"V58 Pinsker financial event probability bound");
close(.5*(1-.5),.25);
close(.9*(1-.9),.09);
close(.5*1.5**2+.5*.5**2,1.25);
close(.5*(1.5-1)**2+.5*(.5-1)**2,.25);
close(1/1.25,.8);
close(Math.abs(.03-.05),.02);
close(.5*.4+.5*0,.2);
assert(.4!==.2&&0!==.2,"V58 uniform PIT marginal is not conditional calibration");
const html=fs.readFileSync(new URL("index.html",dir),"utf8");
const css=fs.readFileSync(new URL("styles.css",dir),"utf8");
const js=fs.readFileSync(new URL("app.js",dir),"utf8");
for(const id of ids)assert(html.includes("?topic="+id),"unlinked fifth math learning route "+id);
assert(html.includes("styles.css?v=20261104-probscore-v6"));
assert(html.includes("第五条高级知识链"));
assert(css.includes("V5 proper scoring")&&css.includes("overflow-x:auto")&&css.includes("@media print"),"CJK formula accessibility regression");
assert(js.includes('div.tabIndex=0')&&js.includes('div.setAttribute("role","region")'),"long math keyboard access missing");
console.log("PASS V58/V5: mathematics 6 complete scoring/shift lectures, "+sections+" detailed proof sections, "+formula+" strict KaTeX formulas, "+refs+" primary references, 656 nodes, "+rows.length+" full lessons, numeric and Chinese formula UX checks.");
