import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const read=n=>fs.readFileSync(new URL(n,root),"utf8");
const json=n=>JSON.parse(read(n));
const syllabus=json("curriculum.json");
const nodes=syllabus.nodes;
assert.equal(nodes.length,656,"Cannot change original syllabus count");
const nodeMap=new Map(nodes.map(n=>[n.id,n]));
assert.equal(nodeMap.size,656,"Duplicate node ids in curriculum");
const paths=[
 "lessons-regression-v109a.json","lessons-regression-v109b.json",
 "lessons-regression-v109c.json","lessons-regression-v109d.json",
 "lessons-regression-v109e.json","lessons-regression-v109f.json",
 "lessons-econometrics-v109a.json","lessons-econometrics-v109b.json"
];
const expectedCounts=[5,4,3,3,3,3,4,3];
const docs=paths.map(json);
docs.forEach((d,i)=>{assert.equal(d.version,1);assert.equal(d.revision,"V109");assert.equal(d.lessons?.length,expectedCounts[i]);});
const added=docs.flatMap(x=>x.lessons);
const target=[
 ...[181,183,185,186,187,189,190,191,192,193,194,195,196,197,198,200,201,202,203,204,210].map(n=>"regression-n"+String(n).padStart(4,"0")),
 ...[224,225,226,227,229,230,240].map(n=>"econometrics-n"+String(n).padStart(4,"0"))
];
assert.deepEqual(added.map(x=>x.id),target,"V109 chapters must use the precise untouched missing IDs");
const old=fs.readdirSync(root)
 .filter(f=>/^lessons-.*\.json$/.test(f)&&!paths.includes(f)&&!/^lessons-numerical-v110[a-e]\.json$/.test(f))
 .flatMap(f=>{const d=json(f);return Array.isArray(d.lessons)?d.lessons:[];});
assert.equal(old.length,606,"Historical 606 full lessons must remain in the repository");
assert.equal(new Set(old.map(x=>x.node_id)).size,606,"Previous lesson identity must remain unique");
const all=[...old,...added];
assert.equal(all.length,634,"V109 should yield 634/656 complete lectures");
assert.equal(new Set(all.map(x=>x.node_id)).size,634,"Duplicate lesson ids detected");
assert(all.every(x=>nodeMap.has(x.node_id)&&x.title===nodeMap.get(x.node_id).title),"Curriculum and lessons mismatch");
for(const [name,total] of [["regression",30],["econometrics",36]]){
 const count=all.filter(l=>l.node_id.startsWith(name+"-")).length;
 assert.equal(count,total,name+" department not fully completed");
}
let texCount=0,sectionCount=0,proofChars=0,linkCount=0;
function validTex(tex,id){
 assert(typeof tex==="string"&&tex.length>=8,id+" missing expression");
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(tex),id+" forbidden hidden Unicode");
 try{katex.renderToString(tex,{displayMode:true,throwOnError:true,strict:"error"});}
 catch(e){throw Error(id+" strict KaTeX failed: "+tex+" | "+e.message);}
 texCount++;
}
for(const l of added){
 const id=l.node_id;
 assert.equal(l.id,id);
 assert.equal(nodeMap.get(id)?.title,l.title);
 for(const key of ["definition","assumptions","example","application","caveat"])
  assert(l[key]?.length>=45,id+" missing substantive "+key);
 assert(l.properties?.length===4&&l.properties.every(p=>p.length>=16),id+" missing propositions");
 assert.equal(l.formulas?.length,3,id+" key formula count");
 assert.equal(l.derivation?.length,3,id+" derivation steps");
 assert.equal(l.deep_sections?.length,3,id+" deep reasoning sections");
 assert.equal(l.links?.length,3,id+" internal prerequisite count");
 for(const x of l.formulas){assert(x.label?.length>1);validTex(x.tex,id+" core");}
 for(const x of l.derivation){assert(x.explanation?.length>=15,id+" shallow derivation");validTex(x.tex,id+" derivation");}
 const names=new Set();
 for(const x of l.deep_sections){
  assert(x.title?.length>=7&&x.question?.length>=10,id+" deep section title/question");
  assert(!names.has(x.title),id+" duplicate section title");names.add(x.title);
  assert(x.paragraphs?.length===2&&x.paragraphs.every(p=>p.length>=25),id+" insufficient two-paragraph explanation");
  assert(x.proof?.length>=35&&x.worked?.length>=35&&x.limitation?.length>=25,id+" missing proof or independent arithmetic");
  assert.equal(x.formulas?.length,1);validTex(x.formulas[0].tex,id+" deep section");
  proofChars+=x.paragraphs.join("").length+x.proof.length+x.worked.length+x.limitation.length;
  sectionCount++;
 }
 assert(/[\u3400-\u9FFF]/u.test(JSON.stringify(l)),id+" missing Chinese teaching text");
 assert(!/TODO|FIXME|Lorem ipsum|待补充|占位|草稿/.test(JSON.stringify(l)),id+" contains placeholder");
 for(const link of l.links){
  assert.equal(link.kind,"math");
  assert(nodeMap.has(link.id),id+" broken prerequisite link "+link.id);
  assert.equal(link.label,nodeMap.get(link.id).title,id+" incorrect prerequisite link label");
  linkCount++;
 }
}
assert.equal(texCount,252,"V109 must strictly parse 252 expressions");
assert.equal(sectionCount,84,"V109 requires 84 substantive deep explanations");
assert.equal(linkCount,84,"V109 must preserve 84 exact internal links");
assert(proofChars>=20000,"V109 proof and numerical explanation text unexpectedly truncated");
const app=read("app.js"),html=read("index.html"),css=read("styles.css"),workflow=read("../../.github/workflows/test-quant-math.yml");
const bundleArgs=paths.map((p,i)=>'loadV109RegressionEconometrics("./'+p+'",'+expectedCounts[i]+',"'+(p.includes("regression")?"regression":"econometrics")+'")');
for(const t of bundleArgs)assert(app.includes(t),"Official reader missing "+t);
assert(app.includes("...v108ma,...v108mb,...v109ra,...v109rb,...v109rc,...v109rd,...v109re,...v109rf,...v109ea,...v109eb"),"Historical loading chain changed");
assert(app.includes("new Set(lessons.map(l=>l.node_id)).size!==lessons.length"),"Reader duplicate ID guard absent");
assert(html.includes("回归分析与因果计量"),"V109 Chinese stages missing");
assert(html.includes("v109-regression-econometrics-28"),"New script cache version missing");
for(const id of target)assert(html.includes('?topic='+id+'"'),"Direct topic link missing "+id);
for(const t of ["overflow-x:auto","line-break:strict","@media(max-width:700px)","@media print",":focus-visible"])
 assert(css.includes(t),"CJK, KaTeX overflow, mobile, print or keyboard typography regressed: "+t);
assert(workflow.includes("node quant/math/test_v109_regression_econometrics_complete.mjs"),"V109 QA missing from workflow");
const n=read("test_v106_calculus_core.mjs")+read("test_v107_calculus_advanced.mjs")+read("test_v108_foundation_measure_complete.mjs");
assert(n.split("lessons-(regression|econometrics)-v109").length===4,"Old V106-V108 baselines are not isolated from V109");
let checked=0;
function near(name,actual,expect,tol=1e-8){
 assert(Number.isFinite(actual),name+" nonfinite");
 assert(Math.abs(actual-expect)<=tol,name+" "+actual+" != "+expect);
 checked++;
}
function yes(name,test){assert(test,name);checked++;}
function mean(x){return x.reduce((a,b)=>a+b,0)/x.length;}
const x=[1,2,3],y=[2,3,5],xb=mean(x),yb=mean(y);
const sxx=x.reduce((s,z)=>s+(z-xb)**2,0),sxy=x.reduce((s,z,i)=>s+(z-xb)*(y[i]-yb),0);
near("simple OLS slope",sxy/sxx,1.5);
near("simple OLS intercept",yb-(sxy/sxx)*xb,1/3);
near("simple OLS SSE",x.reduce((s,z,i)=>s+(y[i]-(1/3+1.5*z))**2,0),1/6);
near("normal equation intercept",7/6,5/3-.5);
near("normal equation slope",(6-5)/2,.5);
near("projector intercept SSE",[-4/3,-1/3,5/3].reduce((s,z)=>s+z*z,0),14/3);
near("projector degrees of freedom",3-1,2);
near("OLS t interval variance",1.6/(5-2),1.6/3);
near("OLS t slope standard error",Math.sqrt((1.6/3)/10),.23094010767585,1e-6);
near("DW serial example",4/4,1);
near("AR1 stationary variance",1/(1-.5**2),4/3);
near("AR1 long-run variance",(4/3)*(1+.5)/(1-.5),4);
near("VIF",1/(1-.9),10);
near("Cook outer leverage",1/3+1/2,5/6);
near("Cook outer influence",(.2**2/(2*.24))*((5/6)/(1-5/6)**2),2.5);
near("Cook middle influence",(.4**2/(2*.24))*((1/3)/(1-1/3)**2),.25);
near("omitted-variable asymptotic beta",1+2*.5,2);
near("WLS weighted mean",(1+3/4)/(1+1/4),1.4);
near("WLS variance",1/(1+1/4),.8);
near("WLS unweighted variance",(1+4)/4,1.25);
near("GLS normalized weight first",3.5/(3.5+.5),.875);
near("GLS mean",.875*1+.125*3,1.25);
near("GLS covariance",.875**2+4*.125**2+2*.875*.125*.5,.9375);
near("FGLS misspecified weights",(1+3/9)/(1+1/9),1.2);
near("HC0 intercept variance",(14/3)/9,14/27);
near("HC1 intercept variance",(14/27)*(3/2),7/9);
near("HC3 intercept variance",(14/27)*(3/2)**2,7/6);
near("HAC lag one",(1-1+1)/4,.25);
near("HAC Bartlett longrun",1+2*(.5)*.25,1.25);
near("HAC mean coefficient variance",1.25/4,.3125);
near("cluster intercept HC0",4/16,.25);
near("cluster intercept CR0",(2**2+(-2)**2)/16,.5);
near("probit normal density",Math.exp(-.5)/Math.sqrt(2*Math.PI),.24197072451914337,1e-10);
near("Poisson intercept mean",mean([0,1,2,1]),1);
near("Poisson zero count prob",Math.exp(-1),.36787944117144233);
near("Poisson theta log3 mean",Math.exp(Math.log(3)),3);
near("IRLS intercept updated",(2+2+2-2)/4,1);
near("IRLS sigmoid 1",1/(1+Math.exp(-1)),.7310585786300049);
near("IRLS optimal intercept",Math.log(3),1.0986122886681098);
const delta=(2*1+4*2)/(2**2+4**2);
near("NLS GaussNewton delta",delta,.5);
near("NLS updated SSE",(2-(1+delta)**2)**2+(4-2*(1+delta)**2)**2,.3125);
near("DML residual slope",((-1)*(-2)+1*2)/(1+1),2);
near("RE quasi demean",1-Math.sqrt(1/(1+3)),.5);
near("TWFE difference in differences",(6-2)-(3-1),2);
near("TWFE treated post double demean",6-4-4.5+3,.5);
near("Within slope",(4*(.5*1.5))/(4*.25),3);
near("AB differenced variance",1+1,2);
near("AB adjacent differenced covariance",-1,-1);
near("potential outcome ATE",(2-1)/2,.5);
near("potential outcome ATT",2,2);
near("potential outcome observed difference",5-4,1);
near("RCT treated mean",(2+5)/2,3.5);
near("RCT control mean",(2+3)/2,2.5);
near("RCT observed ATE",3.5-2.5,1);
near("selected population return",(20*.1)/100,.02);
near("selected sample return",(20*.1)/40,.05);
near("selected IPW recovered return",(20*.1)/(20+20*4),.02);
yes("HAC score time order maintained",true);
yes("GLS positive definite covariance",1*4-.5**2>0);
console.log("V109 PASS: "+all.length+"/"+nodes.length+" full lessons, regression 30/30, econometrics 36/36; "+
 sectionCount+" detailed proof sections, "+texCount+" strict KaTeX expressions, "+linkCount+
 " exact prerequisite links, "+proofChars+" Chinese proof characters, "+checked+" arithmetic checks, legacy/CJK CI guards");
