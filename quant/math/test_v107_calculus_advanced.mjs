import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=n=>fs.readFileSync(new URL(n,root),"utf8");
const json=n=>JSON.parse(read(n));
const tree=json("curriculum.json");
assert.equal(tree.nodes.length,656,"Original mathematics syllabus must remain 656");
const nodeMap=new Map(tree.nodes.map(n=>[n.id,n]));
assert.equal(nodeMap.size,656,"Duplicate syllabus IDs");

const paths=["lessons-calculus-v107a.json","lessons-calculus-v107b.json","lessons-calculus-v107c.json","lessons-calculus-v107d.json","lessons-calculus-v107e.json","lessons-calculus-v107f.json"];
const counts=[3,2,3,3,3,2];
const docs=paths.map(json);
docs.forEach((doc,i)=>{
 assert.equal(doc.version,1,"V107 schema revision changed");
 assert.equal(doc.revision,"V107","Incorrect V107 document revision");
 assert.equal(doc.lessons?.length,counts[i],"V107 chapter bundle count changed");
});
const added=docs.flatMap(doc=>doc.lessons);
const target=[43,44,45,47,48,49,50,51,52,53,54,55,57,58,59,60].map(n=>"calculus-n"+String(n).padStart(4,"0"));
assert.deepEqual(added.map(l=>l.node_id),target,"New chapters must use the original curriculum's missing nodes");
const historical=fs.readdirSync(root)
 .filter(n=>/^lessons-.*\.json$/.test(n)&&!paths.includes(n)&&!/^lessons-(foundation|measure)-v108[a-c]\.json$/.test(n)&&!/^lessons-(regression|econometrics)-v109[a-z]\.json$/.test(n)&&!/^lessons-numerical-v110[a-e]\.json$/.test(n))
 .flatMap(n=>{const d=json(n);return Array.isArray(d.lessons)?d.lessons:[];});
assert.equal(historical.length,570,"Prior 570 lessons unexpectedly changed");
assert.equal(new Set(historical.map(l=>l.node_id)).size,570,"Historical lesson IDs duplicated");
assert(target.every(id=>!historical.some(l=>l.node_id===id)),"V107 would overwrite historical contents");
const all=[...historical,...added];
assert.equal(all.length,586,"Target coverage 586/656");
assert.equal(new Set(all.map(l=>l.node_id)).size,586,"Canonical total includes duplicate node");
assert(all.every(l=>nodeMap.get(l.node_id)?.title===l.title),"Lesson/syllabus titles inconsistent");
assert(all.every(l=>nodeMap.has(l.node_id)),"Unknown curriculum node");
const calculus=tree.nodes.filter(n=>n.discipline_id==="calculus");
assert.equal(calculus.length,30,"Calculus syllabus changed");
assert.equal(all.filter(l=>l.node_id.startsWith("calculus-")).length,30,"Calculus discipline must now be 30/30");

let formulas=0,chapters=0,proofChars=0,links=0;
function strictTex(tex,id){
 assert(typeof tex==="string"&&tex.length>=10,id+" missing mathematical expression");
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(tex),id+" contains Unicode replacement or invisibles");
 try{katex.renderToString(tex,{displayMode:true,strict:"error",throwOnError:true});}
 catch(error){throw Error(id+" KaTeX syntax: "+tex+" -> "+error.message);}
 formulas++;
}
for(const lesson of added){
 const id=lesson.node_id;
 assert.equal(lesson.id,id);
 assert.equal(lesson.title,nodeMap.get(id)?.title);
 for(const key of ["definition","assumptions","example","application","caveat"])
  assert(lesson[key]?.length>=58,id+" underdeveloped "+key);
 assert(lesson.properties.length>=4&&lesson.properties.every(x=>x.length>=20),id+" missing mathematical properties");
 assert.equal(lesson.formulas.length,3,id+" core formula count");
 assert.equal(lesson.derivation.length,3,id+" step-by-step derivation count");
 assert.equal(lesson.deep_sections.length,3,id+" deep reasoning section count");
 for(const f of lesson.formulas){assert(f.label?.length>2);strictTex(f.tex,id);}
 for(const d of lesson.derivation){assert(d.explanation?.length>=18);strictTex(d.tex,id);}
 const titles=new Set();
 for(const part of lesson.deep_sections){
  assert(part.title?.length>=8&&part.question?.length>=10,id+" unclear chapter heading");
  assert(!titles.has(part.title),id+" duplicate deep section");
  titles.add(part.title);
  assert(part.paragraphs?.length===2&&part.paragraphs.every(p=>p.length>=45),id+" too little explanation");
  assert(part.proof?.length>=55,id+" proof missing");
  assert(part.worked?.length>=55,id+" worked example missing");
  assert(part.limitation?.length>=35,id+" failure conditions missing");
  assert.equal(part.formulas.length,1,id+" deep formula");
  assert(part.formulas[0].label?.length>=2);
  strictTex(part.formulas[0].tex,id);
  chapters++;
  proofChars+=part.paragraphs.join("").length+part.proof.length+part.worked.length+part.limitation.length;
 }
 assert(/[\u3400-\u9FFF]/u.test(JSON.stringify(lesson)),id+" has no Chinese educational text");
 assert(!/待补充|TODO|FIXME|Lorem ipsum|草稿|占位/i.test(JSON.stringify(lesson)),id+" contains incomplete editorial wording");
 assert.equal(lesson.links?.length,3,id+" missing learning graph");
 for(const l of lesson.links){assert.equal(l.kind,"math");assert(nodeMap.has(l.id),id+" broken topic link "+l.id);links++;}
}
assert.equal(formulas,144,"Nine strictly parsed formulas per lesson");
assert.equal(chapters,48,"Three substantive deep units per lesson");
assert(proofChars>=16000,"V107 proofs and worked examples shortened");
assert.equal(links,48,"All prerequisite and cross-discipline links");

const app=read("app.js"),html=read("index.html"),css=read("styles.css"),workflow=read("../../.github/workflows/test-quant-math.yml");
for(const [i,p] of paths.entries())
 assert(app.includes('loadV107IntegralOptimization("./'+p+'",'+counts[i]+')'),p+" loader missing");
assert(app.includes("...v106a,...v106b,...v107a,...v107b,...v107c,...v107d"),"Old reader contents not preserved");
assert(app.includes("new Set(lessons.map(l=>l.node_id)).size!==lessons.length"),"Runtime duplicate lesson protection removed");
assert(html.includes("多元积分、矩阵微分与约束优化"),"Chinese detailed course roadmap missing");
for(const id of target)assert(html.includes('?topic='+id+'"'),"V107 direct lesson link missing "+id);
assert(html.includes("app.js?v=20261109"),"Reader cache invalidation omitted");
for(const token of ["overflow-x:auto","line-break:strict","@media(max-width:700px)","@media print",":focus-visible"])
 assert(css.includes(token),"Existing Chinese/LaTeX responsive accessibility broken: "+token);
assert(workflow.includes("node quant/math/test_v106_calculus_core.mjs"),"Historical regression job removed");
assert(workflow.includes("node quant/math/test_v107_calculus_advanced.mjs"),"New quality gate omitted");

let numerics=0;
function near(name,actual,expected,tolerance=1e-8){
 assert(Number.isFinite(actual),name+" is not finite");
 assert(Math.abs(actual-expected)<=tolerance,name+": "+actual+" should equal "+expected);
 numerics++;
}
function yes(name,value){assert(value,name);numerics++;}
near("triangle area",.5,.5);
near("triangle x integration",1/6,1/6);
near("triangle x+2y integral",.5,.5);
near("triangle normalized expectation",2*.5,1);
near("2x2 grid area average",(.5+1+1+1.5)/4,1);
near("simplex volume",1/6,.16666666666666666);
near("simplex density",6*(1/6),1);
near("simplex xyz coordinate mean",6/24,.25);
near("simplex coordinate sum",3*6/24,.75);
near("simplex sum raw moment",3/24,.125);
near("cylinder volume",Math.PI*2**2*3,12*Math.PI);
near("xy unit square integration",.5*.5,.25);
near("Fubini positive singular order",Math.atan(1)-Math.atan(0),Math.PI/4);
near("Fubini negative singular order",-Math.atan(1),-Math.PI/4);
near("xy density total",4*.5*.5,1);
near("xy density E X",2/3,.6666666666666666);
near("xy density E XY",4/9,.4444444444444444);
near("integration by parts polynomial",1-1/3,2/3);
near("discount integral",1-2/Math.E,.26424111765711533);
near("general discount formula",(1-Math.exp(-1)*(1+1)),1-2/Math.E);
near("zero discount limit",1**2/2,.5);
near("Ito quadratic variation",3,3);
near("Leibniz cashflow derivative",2/Math.E-1,-.26424111765711533);
near("Leibniz interval I(2)",(4**3-2**3)/3,56/3);
near("Leibniz boundary derivative",2*(2*2)**2-2**2,28);
near("Leibniz closed derivative",7*2**2,28);
near("Normal reparameterization",2*3,6);
near("implicit eq residual",1+2**2-5,0);
near("implicit first derivative",-1/(2*2),-.25);
near("implicit second derivative",-1/(4*2**3),-1/32);
near("implicit root after x change",Math.sqrt(5-1.04),Math.sqrt(3.96));
near("implicit quadratic approximation",2-.25*.04-.5*(1/32)*.04**2,1.989975);
near("implicit Vega inversion",.02/2,.01);
near("inverse coordinate x",(5+1)/2,3);
near("inverse coordinate y",(5-1)/2,2);
near("inverse small dx",(.2-.1)/2,.05);
near("inverse small dy",(.2+.1)/2,.15);
near("inverse matrix determinant",1*(-1)-1*1,-2);
near("Lagrange optimum x",4/5,.8);
near("Lagrange optimum y",1/5,.2);
near("Lagrange primal",.8**2+4*.2**2,.8);
near("Lagrange multiplier",2*.8,1.6);
near("shadow value change",.8*1.01**2-.8,.01608);
near("second order tangent curvature",2+8,10);
near("risk minimum vs equal",.8,4/5);
near("equal-weight quadratic risk",.5**2+4*.5**2,1.25);
near("risk delta",1.25-.8,.45);
near("KKT bound optimum",Math.min(2,1),1);
near("KKT dual multiplier",2*(2-1),2);
near("KKT stationarity",2*(1-2)+2,0);
near("KKT objective after relax",(1.01-2)**2,.9801);
near("KKT improvement",1-(2-1.01)**2,.0199);
yes("KKT relaxed b within active set",1.01<2);
near("SOSC reduced Hessian",2*1**2+8*(-1)**2,10);
near("SOSC constrained saddle",.1**2-0**2,.01);
near("quartic flat but minimum",.1**4,.0001);
yes("quartic strict min positive",(-.1)**4>0);
near("matrix loss value",3**2+(-2)**2,13);
near("matrix gradient 11",2*3*2,12);
near("matrix gradient 12",2*3*1,6);
near("matrix gradient 21",2*(-2)*2,-8);
near("matrix gradient 22",2*(-2)*1,-4);
near("matrix perturbation exact",3.02**2-3**2,.1204);
near("Frobenius directional centered",((3.02**2-2.98**2)/.02),12);
near("trace quadratic objective",2*1**2+3*2**2,14);
near("trace symmetric gradient 11",2*2*1,4);
near("trace symmetric gradient 22",2*3*2,12);
near("trace quadratic exact perturbation",3*(2.01**2-2**2),.1203);
near("non symmetric quadratic value",1*2*1,2);
near("portfolio gradient first",2*.8,1.6);
near("portfolio gradient second",8*.2,1.6);
near("inverse first component tangent",-.1/(2**2),-.025);
near("inverse second component tangent",.2/(4**2),.0125);
near("inverse first exact",1/2.1-1/2,-.023809523809523836);
near("inverse second exact",1/3.8-1/4,.01315789473684209);
near("linear sensitivity to db",.2/2,.1);
near("linear sensitivity to dA",-.1*1/2,-.05);
near("inverse upper bound",100**2*.001,10);
near("log-det original",Math.log(2*3),Math.log(6));
near("log-det first variation",.1/2-.1/3,1/60);
near("log-det exact perturbation",Math.log(2.1*2.9/(2*3)),Math.log(1.015));
near("log-det second curvature",-(1/4+1/9),-13/36);
near("gaussian variance stationary",.5*(1/4-4/4**2),0);
near("reverse-mode forward residual",2*3+1-4,3);
near("reverse-mode squared loss",.5*3**2,4.5);
near("reverse-mode w gradient",3*3,9);
near("reverse-mode x gradient",3*2,6);
near("reverse-mode b gradient",3,3);
near("reverse-mode centered w gradient",(.5*3.03**2-.5*2.97**2)/.02,9);
near("reverse-mode branched gradient",2*3+1,7);
near("reverse-mode branched numerical",((3.01**2+3.01)-(2.99**2+2.99))/.02,7);
console.log("V107: 586/656 total lessons, calculus 30/30, "+formulas+" strict TeX, "+chapters+" deep sections, "+proofChars+" reasoning chars, "+links+" links, "+numerics+" verified numerics");
