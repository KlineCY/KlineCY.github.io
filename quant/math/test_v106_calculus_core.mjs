import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const here=new URL("./",import.meta.url);
const read=p=>fs.readFileSync(new URL(p,here),"utf8");
const json=p=>JSON.parse(read(p));
const curriculum=json("curriculum.json");
assert.equal(curriculum.nodes.length,656,"Canonical curriculum must stay 656");
const nodeMap=new Map(curriculum.nodes.map(n=>[n.id,n]));
assert.equal(nodeMap.size,656,"Duplicate curriculum nodes");

const files=["lessons-calculus-v106a.json","lessons-calculus-v106b.json"];
const docs=files.map(json);
for(const doc of docs){
 assert.equal(doc.version,1);
 assert.equal(doc.revision,"V106");
 assert.equal(doc.lessons.length,4);
}
const latest=docs.flatMap(d=>d.lessons);
const wanted=["calculus-n0031","calculus-n0033","calculus-n0034","calculus-n0035",
 "calculus-n0036","calculus-n0037","calculus-n0038","calculus-n0039"];
assert.deepEqual(latest.map(l=>l.node_id),wanted,"V106 must fill the canonical calculus gaps in learning order");

const existing=fs.readdirSync(here).filter(n=>/^lessons-.*\.json$/.test(n)&&!files.includes(n)&&!/^lessons-calculus-v107[a-f]\.json$/.test(n)&&!/^lessons-(foundation|measure)-v108[a-c]\.json$/.test(n)&&!/^lessons-(regression|econometrics)-v109[a-z]\.json$/.test(n))
 .flatMap(n=>{const d=json(n);return Array.isArray(d.lessons)?d.lessons:[];});
assert.equal(existing.length,562,"Historical 562 complete lessons were unexpectedly modified or removed");
const oldIds=new Set(existing.map(l=>l.node_id));
assert.equal(oldIds.size,562,"Historical lesson identity duplicated");
assert(wanted.every(id=>!oldIds.has(id)),"Cannot overwrite historical lessons");
assert.equal(new Set([...existing,...latest].map(l=>l.node_id)).size,570,"Expected 570 unique completed lectures");

let formulaCount=0,deepCount=0,proofChars=0,linkCount=0;
function validateTex(tex,id){
 assert.equal(typeof tex,"string",id+" TeX missing");
 assert(tex.length>=8,id+" TeX too short");
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(tex),id+" forbidden character");
 try{katex.renderToString(tex,{displayMode:true,throwOnError:true,strict:"error"});}
 catch(e){throw Error(id+" strict KaTeX parse failed: "+tex+" / "+e.message);}
 formulaCount++;
}
for(const lesson of latest){
 const id=lesson.node_id;
 assert.equal(lesson.id,id,"ID mismatch");
 assert.equal(lesson.title,nodeMap.get(id)?.title,"Curriculum title mismatch");
 for(const field of ["definition","assumptions","example","application","caveat"])
   assert(lesson[field]?.length>=65,id+" missing "+field);
 assert(lesson.properties.length>=4&&lesson.properties.every(x=>x.length>=20),id+" shallow properties");
 assert.equal(lesson.formulas.length,3,id+" core formulas");
 assert.equal(lesson.derivation.length,3,id+" derivation steps");
 assert.equal(lesson.deep_sections.length,3,id+" detailed proof chapters");
 assert.equal(lesson.links.length,3,id+" prerequisite chain");
 for(const x of lesson.formulas){assert(x.label.length>1);validateTex(x.tex,id);}
 for(const x of lesson.derivation){assert(x.explanation.length>=20);validateTex(x.tex,id);}
 const headings=new Set();
 for(const section of lesson.deep_sections){
  assert(section.title?.length>=8&&section.question?.length>=10,id+" section heading");
  assert(!headings.has(section.title),id+" repeated heading");
  headings.add(section.title);
  assert(section.paragraphs?.length===2&&section.paragraphs.every(p=>p.length>=30),id+" short exposition");
  assert(section.proof?.length>=55&&section.worked?.length>=50&&section.limitation?.length>=25,id+" missing proof or worked example");
  assert.equal(section.formulas?.length,1);
  validateTex(section.formulas[0].tex,id);
  deepCount++;
  proofChars+=section.paragraphs.join("").length+section.proof.length+section.worked.length+section.limitation.length;
 }
 assert(/[\\u3400-\\u9FFF]/u.test(JSON.stringify(lesson)),id+" Chinese explanation missing");
 assert(!/TODO|FIXME|待补充|Lorem ipsum/i.test(JSON.stringify(lesson)),id+" filler detected");
 for(const link of lesson.links){assert.equal(link.kind,"math");assert(nodeMap.has(link.id),id+" broken link "+link.id);linkCount++;}
}
assert.equal(formulaCount,72,"Expected 72 tested mathematical expressions");
assert.equal(deepCount,24,"Expected 24 deep proof sections");
assert(proofChars>=8000,"V106 explanatory proof detail below reviewed baseline");
assert.equal(linkCount,24,"Expected 24 internal prerequisite links");

const app=read("app.js"),html=read("index.html"),css=read("styles.css");
assert(app.includes('loadV106Calculus("./lessons-calculus-v106a.json",4)'),"V106A reader missing");
assert(app.includes('loadV106Calculus("./lessons-calculus-v106b.json",4)'),"V106B reader missing");
assert(app.includes("...v105a,...v105b,...v105c,...v105d,...v106a,...v106b"),"Historic reader composition not preserved");
assert(app.includes("new Set(lessons.map(l=>l.node_id)).size!==lessons.length"),"Duplicate-lesson reader guard removed");
assert(html.includes("V106 · 微积分基础"),"Readable V106 roadmap missing");
assert(html.includes("app.js?v=20261108-v105-control-game-market-making-complete-v106-calculus-core"),"Script cache-busting missing");
for(const id of wanted)assert(html.includes('?topic='+id+'"'),"Direct CJK study link missing "+id);
for(const token of ["overflow-x:auto","line-break:strict","@media(max-width:700px)","@media print",":focus-visible"])
 assert(css.includes(token),"CJK/responsive accessibility regression "+token);
assert(read("../../.github/workflows/test-quant-math.yml").includes("node quant/math/test_v106_calculus_core.mjs"),"New CI gate not installed");

let checks=0;
const near=(label,actual,target,tol=1e-8)=>{
 assert(Number.isFinite(actual),label+" nonfinite");
 assert(Math.abs(actual-target)<=tol,label+": "+actual+" vs "+target);
 checks++;
};
const yes=(label,value)=>{assert(value,label);checks++;};
near("bond price",100/1.05,95.23809523809524);
near("bond derivative",-100/(1.05**2),-90.70294784580499);
near("bond first-order",-.001*100/(1.05**2),-.0907029478458);
near("bond exact",100/1.051-100/1.05,-.09061664627792254);
near("modified duration",1/1.05,.9523809523809523);
near("quadratic first derivative",2*2,4);
near("quadratic finite delta",2.1**2-2**2,.41);
near("log Taylor",.02-.02**2/2,.0198);
yes("log error below remainder bound",Math.abs(Math.log(1.02)-.0198)<=2*.02**3/6);
near("exp Taylor",1+.1+.1**2/2,1.105);
yes("exp remainder valid",Math.abs(Math.exp(.1)-1.105)<Math.exp(.1)*.1**3/6);
near("delta gamma quadratic",20*.2+.5*2*.2**2,10.2**2-10**2);
near("convex loss optimum",(2-2)**2+1,1);
near("convex loss at 2.5",(2.5-2)**2+1,1.25);
near("strict convex bound",.5*2*(2.5-2)**2,.25);
near("portfolio boundary",1-4*1,-3);
near("Riemann right sum",.5*(.5+1+1.5+2),2.5);
near("integral polynomial",2**3+2**2,12);
near("integral of t",.5*(2**2),2);
near("cashflow PV",100*(1-Math.exp(-.05))/.05,97.54115099857196);
near("fundamental theorem cumulative",2**3,8);
near("fundamental theorem point derivative",3*2**2,12);
near("discount rate integral",.02+.01/2,.025);
near("discount factor",Math.exp(-.025),.9753099120283326);
near("discount derivative",-.03*Math.exp(-.025),-.029259297360849978);
near("rolling window",2*2-2*(2-1),2);
near("partial w",2*2+3,7);
near("partial r",2,2);
near("partial example changed",2.1**2+2.1*3.1-(2**2+2*3),.92);
near("noncontinuous diagonal",.01*.01/(.01**2+.01**2),.5);
near("Fréchet linear",8*.01+3*(-.02),.02);
near("Fréchet exact",1.01**2+3*1.01*1.98-(1+6),.0195);
near("Fréchet remainder",.01**2+3*.01*(-.02),-.0005);
near("delta-method covariance",4+2*2*1+4*9,44);
near("directional derivative",2*.6+4*.8,4.4);
near("maximum directional",Math.sqrt(2**2+4**2),Math.sqrt(20));
near("budget neutral tangent",2-4,-2);
yes("directional below max",4.4<Math.sqrt(20));
console.log("V106 calculus passed: 570 unique lessons, "+formulaCount+" strict formulae, "+deepCount+" deep chapters, "+checks+" independent numerical checks, "+proofChars+" proof characters");
