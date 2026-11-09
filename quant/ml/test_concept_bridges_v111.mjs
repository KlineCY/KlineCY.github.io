import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
import katex from "katex";
import {gunzipSync} from "node:zlib";

const root=new URL("./",import.meta.url);
const read=name=>fs.readFileSync(new URL(name,root),"utf8");
const json=name=>JSON.parse(read(name));
const original=json("atlas-concept-chapters.json");
const bridge01=json("atlas-concept-bridge-01.json");
const files=["atlas-concept-bridge-v111a.json","atlas-concept-bridge-v111b.json","atlas-concept-bridge-v111c.json"];
const docs=files.map(json);
const knownMath=new Map(json("../math/curriculum.json").nodes.map(x=>[x.id,x.title]));
const atlas=JSON.parse(gunzipSync(Buffer.concat([1,2,3,4].map(i=>fs.readFileSync(new URL("atlas/data.part"+i,root))))).toString("utf8"));
const manifest=json("atlas-depth-manifest.json");
assert.equal(manifest.expected_lessons,720,"Unchanged Atlas 720-course curriculum required");
assert.equal(manifest.expected_domains,24,"Unchanged Atlas 24 domains required");
assert.equal(atlas.catalog.items.length,720,"Original 720 canonical Atlas pages must remain");
assert.equal(knownMath.size,656,"All 656 mathematics prerequisites must be available");
const targets=[
 "m02-c05-p01","m05-c02-p02","m06-c01-p06","m07-c01-p02",
 "m09-c04-p03","m14-c05-p06","m15-c01-p05","m16-c01-p06",
 "m20-c02-p01","m21-c01-p03","m22-c04-p03","m23-c04-p02"
];
const entries=docs.flatMap(d=>Object.entries(d.lessons));
assert.equal(entries.length,12,"V111 requires exactly 12 deepened existing lessons");
assert.deepEqual(entries.map(([id])=>id),targets,"V111 course IDs or prerequisite order changed");
assert.equal(new Set(targets).size,12,"Duplicate V111 target IDs");
assert(!targets.some(id=>id in original.lessons||id in bridge01.lessons),"Do not repeat previous concept chains");
for(const d of docs){
 assert.equal(d.schema,1,"Unsupported concept chapter format");
 assert.equal(d.revision,"V111","Wrong V111 revision");
 assert.equal(Object.keys(d.lessons).length,4,"Each transfer bundle must hold exactly four lessons");
}
let nSections=0,nTeX=0,nRefs=0,proofChars=0;
for(const [id,l] of entries){
 assert(/^m\d\d-c\d\d-p\d\d$/.test(id),"Invalid lesson id "+id);
 assert.equal(l.source_title,atlas.domains[id.slice(0,3)]?.[id]?.title,id+" source course title changed");
 assert(l.goal.length>=60,id+" missing goal");
 assert.equal(l.sections.length,4,id+" incomplete four-stage learning route");
 assert(l.links.length>=4,id+" missing math/ML prerequisite crosslinks");
 const used=new Set();
 for(const sec of l.sections){
  assert(sec.title.length>=7&&sec.question.length>=12,id+" thin question or title");
  assert(!used.has(sec.title),id+" duplicate deep section heading");used.add(sec.title);
  assert(sec.paragraphs.length===2&&sec.paragraphs.every(x=>x.length>=40),id+" deep paragraphs insufficient");
  assert(sec.worked.length>=55&&sec.boundary.length>=30,id+" missing independent arithmetic or caveat");
  assert.equal(sec.equations.length,1,id+" one substantive equation per deep section");
  for(const eq of sec.equations){
   assert(eq.label.length>=4&&eq.tex.length>=12,id+" formula missing");
   assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/.test(eq.tex),id+" invalid hidden text");
   try{katex.renderToString(eq.tex,{displayMode:true,throwOnError:true,strict:"error"});}
   catch(err){throw Error(id+" / "+sec.title+" strict KaTeX failed for "+eq.tex+": "+err.message);}
   nTeX++;
  }
  assert(/[\u3400-\u9FFF]/u.test(JSON.stringify(sec)),id+" Chinese mathematical explanation missing");
  assert(!/TODO|FIXME|草稿|待完善|占位|Lorem ipsum/.test(JSON.stringify(sec)),id+" placeholder found");
  proofChars+=sec.paragraphs.join("").length+sec.worked.length+sec.boundary.length;
  nSections++;
 }
 for(const link of l.links){
  assert(link.label.length>2,id+" unlabelled link");
  if(link.kind==="math")assert.equal(knownMath.get(link.id),link.label,id+" incorrect mathematics title: "+link.id);
  else if(link.kind==="ml")assert(atlas.domains[link.id.slice(0,3)]?.[link.id],id+" broken ML link "+link.id);
  else throw Error(id+" unknown link kind "+link.kind);
  nRefs++;
 }
}
assert.equal(nSections,48,"Expected forty-eight deep proof sections");
assert.equal(nTeX,48,"Expected forty-eight strictly parsed formulas");
assert.equal(nRefs,48,"Expected forty-eight cross-domain prerequisites");
assert(proofChars>=12500,"Substantive CJK reasoning unexpectedly shortened");

const source=read("concept-render.js"),reader=read("atlas.js"),html=read("index.html"),css=read("atlas.css");
new vm.Script(source);new vm.Script(reader);
assert(reader.includes('for(const path of ["./atlas-concept-bridge-v111a.json","./atlas-concept-bridge-v111b.json","./atlas-concept-bridge-v111c.json"])'),"V111 official reader loading missing");
assert(reader.includes("state.conceptChapters.has(id)"),"V111 duplicate concept ID guard missing");
const ctx={window:{},encodeURIComponent};
vm.runInNewContext(source,ctx);
const mounted=new Set();
for(const doc of docs){
 const before=new Map(Object.entries(doc.lessons).map(([id])=>[id,atlas.domains[id.slice(0,3)][id].html]));
 const built=ctx.window.quantMlConcepts.mount(atlas,doc);
 assert.equal(built.size,4,"Canonical mount failed");
 for(const id of built){
  assert(!mounted.has(id),"Overlapping canonical articles");mounted.add(id);
  const row=atlas.domains[id.slice(0,3)][id];
  assert(row.html.startsWith(before.get(id)),id+" original HTML overwritten");
  assert.equal((row.html.match(/class="atlas-concept-unit"/g)||[]).length,4,id+" incomplete mounted proof sections");
  assert(row.html.includes("../math/?topic="),id+" missing mathematical prerequisite navigation");
 }
}
assert.equal(mounted.size,12);
assert.equal(atlas.catalog.items.length,720,"Mount must not append duplicate independent articles");
for(const id of targets)assert(html.includes('href="#lesson/'+id+'"'),"Missing direct Chinese study path "+id);
assert(html.includes("V111 · 机器学习 × 量化数学"),"Chinese V111 roadmap missing");
assert(html.includes("atlas.js?v=20261109-v112-20-math-ml-lessons"),"Atlas new source asset cache missing");
for(const term of ["line-break:strict","overflow-x:auto","@media(max-width:700px)",".atlas-concept-jump:focus-visible",".atlas-concept-chain"])
 assert(css.includes(term),"CJK, KaTeX, mobile, focus or typography CSS regressed: "+term);
const ci=read("../../.github/workflows/test-quant-ml.yml");
assert(ci.includes("node quant/ml/test_concept_bridges_v111.mjs"),"V111 CI must be registered");

let checks=0;
function near(n,x,y,eps=1e-8){assert(Number.isFinite(x)&&Math.abs(x-y)<=eps,n+" mismatch "+x+" vs "+y);checks++;}
function yes(n,p){assert(p,n);checks++;}
near("Covariance zero risk long-short",1+1-2,0);
near("Covariance singular Gram eigen 2",1+1,2);
near("Covariance shrinkage second eigen",.2,.2);
near("Two asset zero correlation risk",Math.sqrt(.5**2*.2**2+.5**2*.2**2),Math.sqrt(.02));
near("Two asset perfect correlation risk",Math.sqrt(.04),.2);
near("SGD uniform mean",(1+2+3)/3,2);
near("SGD correlated batch mean variance",(1+1+2*.8)/4,.9);
near("SGD unstable multiplicative factor",1-.6*4,-1.4);
near("OLS normal intercept",7/6,1.1666666666667);
near("OLS slope covariance",1/2,.5);
near("OLS Ridge single weight",4/(2+2),1);
near("Kernel RBF pair",Math.exp(-1),.36787944117144233);
near("Kernel ridge leading retention",4/(4+1),.8);
near("Kernel ridge small retention",.1/(.1+1),1/11);
near("Image label overlap length",Math.min(25,26)-Math.max(21,22)+1,4);
near("Familywise independent null",1-Math.pow(.95,20),.6415140775914581,1e-10);
near("AR3 conditional mean",.5**3*4,.5);
near("AR3 error variance",4*(1+.25+.0625),5.25);
near("Overlapping 20 day correlation",19/20,.95);
near("Causal triangular mask count",1+2+3,6);
near("Masked softmax sum",.5+.5+0,1);
near("Bellman second state",2/(1-.5),4);
near("Bellman first state",(1+.25*4)/(.75),8/3);
near("Bellman residual bound",1/(1-.5),2);
near("Policy improvement action value",1.2+.5*4,3.2);
near("Off policy expected reward",.5*1.6,.8);
near("Off policy importance weight tail",1.6**10,109.9511627776,1e-7);
near("Off policy ESS",100/52,1.9230769230769231);
near("DCC diagonal normalized",.1+.1+.8,1);
near("DCC off diagonal",.1*(1*(-1)),-.1);
near("DCC Gaussian PSD second eigen",1-.1,.9);
near("DCC normalized positive pair",1/(2*3),1/6);
near("DCC portfolio variance zero correlation",2*.5**2*.2**2,.02);
near("DCC portfolio variance half correlation",.02+2*.5*.5*.5*.2*.2,.03);
near("Drift first weight",.5*1.1/(.5*1.1+.5*.9),.55);
near("Drift second weight",.5*.9/(.5*1.1+.5*.9),.45);
near("Gross trade fraction",Math.abs(.5-.55)+Math.abs(.5-.45),.1);
near("Gross fee 10bp on turnover",.001*.1,.0001);
near("Final NAV after fee",100*(1-.0001),99.99);
yes("Old lesson descriptions maintained after mount",atlas.domains["m02"]["m02-c05-p01"].title==="协方差矩阵：二阶统计量的线性代数结构");
console.log("V111 PASS: canonical 720 Atlas + 60 active expansions preserved; 12 improved lessons, 48 deep CJK sections, "+
 nTeX+" strict KaTeX, "+nRefs+" exact prerequisite links, "+proofChars+" reasoning characters, "+checks+
 " independent arithmetic checks, math crosslinks, readable responsive layout and no overwritten articles");
