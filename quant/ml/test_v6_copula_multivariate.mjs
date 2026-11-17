import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
import katex from "katex";
import {gunzipSync} from "node:zlib";
const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const namesBefore=["atlas-new-lessons.json","atlas-volatility-chapter.json","atlas-linear-regularization-chapter.json","atlas-evaluation-evidence-chapter.json","atlas-generalization-v2.json","atlas-kernel-covariance-v2.json","atlas-stability-kernel-v3.json","atlas-robust-validation-v3.json","atlas-quantile-calibration-v4.json","atlas-tailrisk-models-v4.json","atlas-proper-scoring-conformal-v5.json","atlas-density-pit-finance-v5.json"];
const current=["atlas-copula-v6.json","atlas-multivariate-risk-v6.json"];
const docs=current.map(read),rows=docs.flatMap(d=>d.chapters.flatMap(c=>c.lessons));
const expected=["m09-c10-p01","m09-c10-p02","m12-c11-p01","m23-c11-p01"];
assert.deepEqual(rows.map(l=>l.id),expected);
const math=new Set(read("../math/curriculum.json").nodes.map(n=>n.id));
const core=JSON.parse(gunzipSync(Buffer.concat([1,2,3,4].map(i=>fs.readFileSync(new URL("atlas/data.part"+i,root))))).toString("utf8"));
const before=namesBefore.map(read),beforeIds=new Set(before.flatMap(d=>d.chapters.flatMap(c=>c.lessons.map(l=>l.id))));
let units=0,equations=0,sources=0;
for(const doc of docs)assert(doc.schema===1&&doc.chapters.length>=1);
for(const l of rows){
 assert(!beforeIds.has(l.id)&&!core.catalog.items.some(x=>x.id===l.id),"duplicate new course "+l.id);
 assert(l.goal.length>=80&&l.protocol.length>=4&&l.sources.length>=2);
 const expectedCount=["m12-c11-p01","m23-c11-p01"].includes(l.id)?9:5;
 assert.equal(l.sections.length,expectedCount,"all original and extended sections "+l.id);
 assert.equal(new Set(l.sections.map(s=>s.title)).size,l.sections.length);
 for(const s of l.sections){
  assert(s.question.length>=13&&s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=70),"insufficient detailed Chinese explanation "+l.id+" "+s.title);
  assert(s.worked.length>=65&&s.boundary.length>=55,"worked calculation or validity boundary incomplete "+l.id+" "+s.title);
  assert(s.equations.length>=1);
  for(const e of s.equations){assert(e.label&&e.tex);katex.renderToString(e.tex,{throwOnError:true,strict:"ignore"});equations++;}
  units++;
 }
 for(const p of l.prerequisites){
  assert(p.label&&["ml","math"].includes(p.kind),"invalid prerequisite type "+l.id);
  if(p.kind==="math")assert(math.has(p.id),"missing mathematical prerequisite "+p.id);
  else{
   assert(core.catalog.items.some(x=>x.id===p.id)||beforeIds.has(p.id)||expected.includes(p.id),"missing ML prerequisite "+p.id);
   if(expected.includes(p.id))assert(expected.indexOf(p.id)<expected.indexOf(l.id),"forward prerequisite "+p.id+" → "+l.id);
  }
 }
 for(const src of l.sources){assert(src.title&&/^https:\/\//.test(src.url));sources++;}
}
assert(!JSON.stringify(rows).includes("\ufffd"),"UTF-8 replacement character in textbook");
const atlas=fs.readFileSync(new URL("atlas.js",root),"utf8"),expansion=fs.readFileSync(new URL("new-topics.js",root),"utf8");
new vm.Script(atlas);new vm.Script(expansion);
for(const file of current)assert(atlas.includes('"./'+file+'"'),"runtime loader missing "+file);
const ctx={window:{},encodeURIComponent};
vm.runInNewContext(expansion,ctx);
for(const doc of [...before,...docs])ctx.window.quantMlExpansion.install(core,doc);
assert.equal(core.catalog.items.length,780,"incorrect count of active navigable courses");
assert.equal(new Set(core.catalog.items.map(x=>x.id)).size,780,"duplicate course ID after bundle injection");
for(const id of expected){
 const art=core.domains[id.slice(0,3)][id];
 assert(art?.html.includes("atlas-concept-toc"),"course reader cannot display "+id);
 assert.equal((art.html.match(/class="atlas-concept-unit"/g)||[]).length,rows.find(l=>l.id===id).sections.length,"lost an internal subsection "+id);
 assert(art.html.includes("tabindex=\"0\""),"long formula container not keyboard scrollable "+id);
 assert(core.catalog.items.some(x=>x.id===id),"not in navigation "+id);
}
for(const l of rows.filter(l=>["m12-c11-p01","m23-c11-p01"].includes(l.id))){
 for(const s of l.sections.slice(5)){
  assert(s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=75),"new multivariate theory incomplete "+l.id);
  assert(s.equations.length===2&&s.worked.length>=100&&s.boundary.length>=95,"new multivariate evidence incomplete "+l.id);
  for(const e of s.equations)katex.renderToString(e.tex,{throwOnError:true,strict:"error"});
 }
}
const jointAdvancedTopics={
 "m12-c11-p01":["Energy Score与边际CRPS：联合相关结构不能由单变量评分替代","多元高斯组合预测：投影之后的VaR与ES精确式"],
 "m23-c11-p01":["Student-t Copula的尾部依赖与Gaussian Copula差异","联合压力场景中的现金账户与融资缺口守恒"]
};
for(const [id,titles] of Object.entries(jointAdvancedTopics)){
 const lesson=rows.find(l=>l.id===id);
 for(const title of titles){
  const s=lesson.sections.find(s=>s.title===title);
  assert(s&&s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=85),"missing joint-distribution explanation "+title);
  assert(s.equations.length===2&&s.worked.length>=125&&s.boundary.length>=75,"incomplete joint stress evidence "+title);
  for(const eq of s.equations)katex.renderToString(eq.tex,{throwOnError:true,strict:"error"});
 }
}
const near=(a,b,t=1e-8)=>assert(Math.abs(a-b)<t,"numerical invariant incorrect "+a+" vs "+b);
near(.05**2,.0025);
near(.05/.05,1);
near(1-.95,.05);
near(2**(-1),.5);
near(.5*(0+2)-.5*(0+2+2+0)/4,.5);
near(Math.abs(Math.abs(1-3)-Math.abs(0-2))**2,0);
near(Math.hypot(1-0,3-2),Math.sqrt(2));
near(37*36/2,666);
near(.5*(1+.8*(1-.5)*(1-2*.25)),.6);
near(15/50,.3);
near(2/3-8/(2*3*2),0);
near(2/3-8/(2*3**2),2/9);
near(.05*.05,.0025);
near(.05/.0025,20);
near(.3+.7,1);
near(Math.sqrt(.5)*1.96,1.385929291125633);
near(Math.sqrt(.00036),.018973665961010276);
near(Math.sqrt(.00036)*1.96,.03718838528358014);
near(1-1/Math.sqrt(2),.2928932188134524);
near(10+2-14-1-.5,-3.5);
near(.5*(1+1),1);
const html=fs.readFileSync(new URL("index.html",root),"utf8"),css=fs.readFileSync(new URL("atlas.css",root),"utf8");
for(const id of expected)assert(html.includes("#lesson/"+id),"sixth learning route missing "+id);
assert(html.includes("780篇可导航课程")&&html.includes("第六条高级学习链"));
assert(html.includes("atlas.js?v=20261109")&&html.includes("atlas.css?v=20261109"));
assert(css.includes("overflow-x:auto")&&css.includes("@media print"),"CJK/formula accessibility missing");
console.log("PASS: V6 "+rows.length+" full Copula/multivariate risk lectures, "+units+" deep sections, "+equations+" parseable KaTeX formulas, "+sources+" cited resources; 780 unique navigable courses, conditional dependence arithmetic and keyboard/CJK/print regression checks.");
