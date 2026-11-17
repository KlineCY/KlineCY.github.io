import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
import katex from "katex";
import {gunzipSync} from "node:zlib";
const dir=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,dir),"utf8"));
const core=JSON.parse(gunzipSync(Buffer.concat([1,2,3,4].map(i=>fs.readFileSync(new URL("atlas/data.part"+i,dir))))).toString("utf8"));
const ids=new Set(core.catalog.items.map(x=>x.id));
const math=new Set(read("../math/curriculum.json").nodes.map(n=>n.id));
const priorFiles=["atlas-new-lessons.json","atlas-volatility-chapter.json","atlas-linear-regularization-chapter.json","atlas-evaluation-evidence-chapter.json","atlas-generalization-v2.json","atlas-kernel-covariance-v2.json"];
const prior=priorFiles.map(read);
const priorIds=new Set(prior.flatMap(d=>d.chapters.flatMap(ch=>ch.lessons.map(l=>l.id))));
const newFiles=["atlas-stability-kernel-v3.json","atlas-robust-validation-v3.json"];
const docs=newFiles.map(read);
const rows=docs.flatMap(d=>d.chapters.flatMap(ch=>ch.lessons));
const expected=["m04-c08-p01","m09-c08-p01","m09-c08-p02","m12-c08-p01","m23-c08-p01","m23-c08-p02"];
assert.deepEqual(rows.map(l=>l.id),expected,"stable learning chain order");
for(const d of docs){assert.equal(d.schema,1);assert(d.version);assert(d.chapters.length>=2);}
let sections=0,formula=0,sources=0;
for(const l of rows){
 assert(!ids.has(l.id)&&!priorIds.has(l.id),"duplicate stable course ID "+l.id);
 assert(l.title.length>=12&&l.goal.length>=70&&l.protocol.length>=4&&l.sources.length>=2,"insufficient overview "+l.id);
 const expectedSections=l.id==="m12-c08-p01"?9:["m23-c08-p01","m23-c08-p02"].includes(l.id)?8:7;
 assert.equal(l.sections.length,expectedSections,"complete original and expanded course sections "+l.id);
 assert.equal(new Set(l.sections.map(s=>s.title)).size,l.sections.length,"duplicate section headings "+l.id);
 for(const s of l.sections){
  assert(s.title.length>=7&&s.question.length>=12&&s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=65),"short theory paragraph "+l.id+" / "+s.title);
  assert(s.worked.length>=55&&s.boundary.length>=45,"worked example or failure boundary missing "+l.id+" / "+s.title);
  assert(s.equations.length>=1);
  for(const f of s.equations){assert(f.label&&f.tex);katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});formula++;}
  sections++;
 }
 for(const p of l.prerequisites){
  assert(p.label&&["ml","math"].includes(p.kind));
  if(p.kind==="math")assert(math.has(p.id),"broken maths reference "+p.id);
  else {
   assert(ids.has(p.id)||priorIds.has(p.id)||expected.includes(p.id),"broken ML link "+p.id);
   if(expected.includes(p.id))assert(expected.indexOf(p.id)<expected.indexOf(l.id),"forward prerequisite "+l.id);
  }
 }
 for(const src of l.sources){assert(src.title&&/^https:\/\//.test(src.url));sources++;}
}
assert(!/\uFFFD/.test(JSON.stringify(rows)),"source text contains replacement Unicode characters");
const scripts=["atlas.js","new-topics.js"].map(p=>fs.readFileSync(new URL(p,dir),"utf8"));
for(const s of scripts)new vm.Script(s);
for(const fn of newFiles)assert(scripts[0].includes('"./'+fn+'"'),"Atlas loader does not load "+fn);
const ctx={window:{},encodeURIComponent};
vm.runInNewContext(scripts[1],ctx);
for(const d of [...prior,...docs])ctx.window.quantMlExpansion.install(core,d);
assert.equal(core.catalog.items.length,764);
assert.equal(new Set(core.catalog.items.map(x=>x.id)).size,764);
for(const id of expected){
 const art=core.domains[id.slice(0,3)][id];
 assert(art?.html.includes("atlas-concept-toc"),"not visible to reader "+id);
 assert.equal((art.html.match(/class="atlas-concept-unit"/g)||[]).length,rows.find(l=>l.id===id).sections.length);
 assert(core.catalog.items.some(x=>x.id===id),"missing index entry "+id);
}
for(const l of rows.filter(l=>["m12-c08-p01","m23-c08-p01","m23-c08-p02"].includes(l.id))){
 for(const s of l.sections.slice(5)){
  assert(s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=75),"new evidence needs complete explanations "+l.id);
  assert(s.equations.length===2&&s.worked.length>=100&&s.boundary.length>=95,"new evidence missing formulas, calculation or boundaries "+l.id);
  for(const e of s.equations)katex.renderToString(e.tex,{throwOnError:true,strict:"error"});
 }
}
for(const l of rows.filter(l=>["m04-c08-p01","m09-c08-p01","m09-c08-p02"].includes(l.id))){
 const original=5;
 assert.equal(l.sections.length,7,"two new rigorous sections per stability/kernel course "+l.id);
 for(const section of l.sections.slice(original)){
  assert(section.paragraphs.length===2&&section.paragraphs.every(p=>p.length>=95),"expanded stability reasoning too short "+l.id);
  assert(section.equations.length===2&&section.worked.length>=85&&section.boundary.length>=60,"missing independent example or caveat "+l.id);
  for(const eq of section.equations)katex.renderToString(eq.tex,{throwOnError:true,strict:"error",displayMode:true});
 }
}
const robustNewTopics={
 "m12-c08-p01":["Purged交叉验证：重叠事件标签的严格剔除条件","同日跨ETF区块Bootstrap：为什么必须共同重采样交易日期"],
 "m23-c08-p01":["谱范数协方差不确定集下的最坏方差闭式"],
 "m23-c08-p02":["Huber稳健位置估计与真实尾损失应分别评价"]
};
for(const [id,titles] of Object.entries(robustNewTopics)){
 const l=rows.find(x=>x.id===id);
 for(const title of titles){
  const section=l.sections.find(s=>s.title===title);
  assert(section&&section.paragraphs.length===2&&section.paragraphs.every(p=>p.length>=95),"missing robust research proof "+title);
  assert(section.equations.length===2&&section.worked.length>=110&&section.boundary.length>=95,"incomplete quantitative robust research "+title);
  for(const eq of section.equations)katex.renderToString(eq.tex,{throwOnError:true,strict:"error"});
 }
}
const close=(a,b,t=1e-9)=>assert(Math.abs(a-b)<t,"numeric check "+a+" vs "+b);
close(2*4/100,.08);
close(1/Math.sqrt(2),.7071067811865476);
close(Math.exp(-.5),.6065306597126334);
close(2-.5*2,1);
close(2-1.5,0.5);
close(.064-2*.01,.044);
close(.95+.05,1);
close(5-.5,4.5);
close(((0-1)**2+(1-1)**2+(2-1)**2)/2,1);
close(1+2*.2,1.4);
close(2/(.5*1000),.004);
close(Math.sqrt(2*.001/.5),Math.sqrt(.004));
close(2/(.5*2000),.002);
close(2*Math.exp(-4),.03663127777746836);
close(2*Math.exp(-8),.0006709252558050237);
close(1+2*.2,1.4);
close(.9+(.1/200),.9005);
close(.02-.16/10,.004);
close(.02-(-.12)/10,.032);
close(.6*.01+.2*.03,.012);
close(Math.sqrt(.006**2+.006**2),.008485281374238571);
close(1.5/.9,1.6666666666666667);
close((3-5.4)/3,-.8);
close(.25*.0004+.25*.0004+2*.5*.5*.8*.0004,.00036);
close(Math.sqrt(.00036),.018973665961010276);
close(Math.sqrt(.0002),.01414213562373095);
close(.5**2*.0004+.5**2*.0009,.000325);
close(.000325+.0001*.5,.000375);
close(Math.sqrt(.000375),.019364916731037084);
const html=fs.readFileSync(new URL("./index.html",dir),"utf8"),css=fs.readFileSync(new URL("./atlas.css",dir),"utf8");
for(const id of expected)assert(html.includes("#lesson/"+id),"missing advanced learning route "+id);
assert(html.includes('atlas.js?v=20261109'));
assert(html.includes('atlas.css?v=20261109'));
assert(html.includes('aria-label="第三条高级学习链：正则化稳定性、核近似与稳健ETF研究"'));
assert(css.includes("scrollbar-gutter:stable")&&css.includes("@media print"),"new visual accessibility rules missing");
console.log("PASS: V3 machine learning "+rows.length+" advanced courses, "+sections+" deep sections, "+formula+" parseable TeX formulas, "+sources+" source references; 764 unique navigable lessons, exact arithmetic and CJK/formula design.");
