import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>fs.readFileSync(new URL(p,root),"utf8");
const load=p=>JSON.parse(read(p));
const math=load("lessons-phase-17-a.json");
const mlPaths=[
 "../ml/atlas-depth-16-a.json","../ml/atlas-depth-16-b.json",
 "../ml/atlas-depth-17-a.json","../ml/atlas-depth-17-b.json",
 "../ml/atlas-depth-15-a.json"
];
const modules=mlPaths.map(load);
const mathIds=["econometrics-n0211","econometrics-n0213","econometrics-n0214","econometrics-n0215"];
const groups=[
 ["m01-c04-p03","m02-c01-p01","m03-c01-p01","m04-c01-p03","m05-c01-p01","m06-c02-p01"],
 ["m07-c01-p02","m08-c01-p05","m09-c04-p03","m10-c01-p02","m11-c04-p03","m12-c01-p04"],
 ["m03-c03-p04","m04-c01-p02","m05-c02-p03","m06-c04-p05","m07-c02-p01","m08-c01-p02"],
 ["m09-c03-p01","m10-c01-p03","m11-c04-p04","m12-c03-p03","m13-c02-p02","m14-c01-p01"],
 ["m01-c05-p01","m02-c01-p04","m03-c02-p01","m04-c02-p04","m05-c02-p01","m06-c01-p01"]
];
assert.deepEqual(math.lessons.map(x=>x.id),mathIds);
assert.deepEqual(math.v73_revision.lessons,mathIds);
let texPass=0,strictPass=0;
function checkTex(tex,strict=false){
 assert(typeof tex==="string"&&tex.trim().length>=4);
 const result=katex.renderToString(tex,{throwOnError:true,strict:strict?"error":"ignore",displayMode:true});
 assert(result.includes("katex"));
 texPass++;
 if(strict)strictPass++;
}
function checkText(obj,label){
 const raw=JSON.stringify(obj);
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(raw),label+" broken Unicode");
 assert(!/TODO|FIXME|Lorem ipsum/iu.test(raw),label+" unfinished content");
 assert(/[\u3400-\u9FFF]/u.test(raw),label+" lost Chinese course content");
}
for(const x of math.lessons){
 assert(x.checked_status.startsWith("V73"),x.id+" latest review marker missing");
 const section=x.deep_sections.at(-1);
 assert(section.question.length>=14&&section.proof.length>=125);
 assert(section.worked.length>=90&&section.limitation.length>=50);
 assert.equal(section.paragraphs.length,2);
 assert(section.paragraphs.every(p=>p.length>=80));
 assert.equal(section.formulas.length,2);
 assert.equal(new Set(x.deep_sections.map(s=>s.title)).size,x.deep_sections.length);
 assert(x.sources?.length>=2&&x.links?.length>=3,x.id+" references missing");
 for(const f of x.formulas)checkTex(f.tex);
 for(const f of x.derivation)checkTex(f.tex);
 for(const f of x.deep_sections.flatMap(s=>s.formulas))checkTex(f.tex);
 for(const f of section.formulas)checkTex(f.tex,true);
 checkText(section,x.id);
}
for(let k=0;k<modules.length;k++){
 const d=modules[k],ids=groups[k];
 assert.deepEqual(Object.keys(d.lessons),ids,mlPaths[k]+" curriculum changed");
 assert.deepEqual(d.v73_revision.ids,ids,mlPaths[k]+" wrong revision ids");
 for(const id of ids){
  const x=d.lessons[id];
  assert(x.v73_note.startsWith("V73"),id+" latest review marker missing");
  assert(x.equations.length>=8&&x.derivation.length>=8,id+" insufficient equation/derivation coverage");
  assert(x.derivation.slice(-2).every(v=>v.length>=95),id+" thin new derivation");
  assert(x.protocol.length>=5&&x.pitfalls.length>=4,id+" missing applied verification/pitfalls");
  assert(x.worked_example.length>=250,id+" insufficient worked example");
  assert.equal(new Set(x.equations.map(f=>f.label)).size,x.equations.length,id+" duplicate equation headings");
  assert.equal(new Set(x.protocol).size,x.protocol.length,id+" duplicate validation steps");
  for(const f of x.equations)checkTex(f.tex);
  for(const f of x.equations.slice(-2))checkTex(f.tex,true);
  checkText(x,id);
 }
}
assert.equal(groups.flat().length+mathIds.length,34);
assert.equal(new Set(groups.flat()).size,30);
let arithmetic=0;
function near(label,got,want,tol=1e-9){
 assert(Number.isFinite(got)&&Number.isFinite(want)&&Math.abs(got-want)<=tol,label+" got "+got+" expected "+want);
 arithmetic++;
}
near("collider independent cov",.25-.25,0);
near("collider selected cov",0-.5*.5,-.25);
near("correlated dependent-label error",2+.5,2.5);
near("nonregular cube root",Math.cbrt(.001),.1);
near("nonregular sixth-root scaling",Math.cbrt(.000125),.05);
near("Wald IV treatment difference",.75-.25,.5);
near("Wald IV outcome difference",.5*4,2);
near("Wald LATE",2/.5,4);
near("ATE",.5*4+.25*0+.25*0,2);
near("conditional predictive mean",.5*2+.5*(-2),0);
near("conditional predictive moment",.5*2,1);
near("nonlinear correlation cross moment",(-1*1+0*0+1*1)/3,0);
near("nonlinear conditional mean",(1+0+1)/3,2/3);
near("BC convergent partial sum",Array.from({length:100},(_,i)=>1/(i+1)**2).reduce((a,b)=>a+b,0)<Math.PI**2/6?1:0,1);
for(let i=1;i<=3;i++)near("BH rank "+i,[.005,.02,.03][i-1]<=(i*.05/4)?1:0,1);
near("BH fourth rejected",.9<=.05?1:0,0);
near("Hessian error first",1-.25*1,.75);
near("Hessian error second",.75**2,.5625);
near("historical universe score",-2/Math.sqrt(8/3),-1.224744871391589);
near("LOOCV",(-4/3)/(1-1/3),-2);
near("CART parent SSE",(-2)**2+0**2+0**2+2**2,8);
near("CART split gain",2*2/4*(1-3)**2,4);
near("RBF original",Math.exp(-(.02**2)/(2*(.01**2))),Math.exp(-2));
near("RBF unit invariance",Math.exp(-(2**2)/(2*(1**2))),Math.exp(-2));
near("Beta boundary interior MAP",(1.5-1)/(1.5+1.5-2),.5);
near("mixture label invariance",.3*.2+.7*.6,.48);
near("mixture swapped",.7*.6+.3*.2,.48);
near("AUC tie halfcredit",(2+.5+1)/4,.875);
near("Poisson thinning P0",Math.exp(-4*.25),.36787944117144233);
near("anytime delta100",6*.05/(Math.PI**2*100**2),.0000030396355092701337);
near("anytime radius100",Math.sqrt(Math.log(2/(6*.05/(Math.PI**2*10000)))/200),.2588138339478868);
near("SGD independent batch variance",.25*25+.25*25,12.5);
near("SGD stratified batch mean",(0+10)/2,5);
near("quote A staleness",(15*60+30)-(15*60+20),10);
near("quote B staleness",(15*60+30)-13*60,150);
near("Ridge original prediction",1/(1+1),.5);
near("Ridge bad unit prediction",100/(100+1),100/101);
near("Ridge matched penalty prediction",100/(100+100),.5);
near("Gini memorization",1-.5**2-.5**2,.5);
near("SVM different cap",1>.5?1:0,1);
near("Beta Bernoulli param var",4/(4**2*5),.05);
near("Beta binomial predictive variance",2*.5*.5+2*.05,.6);
near("GMM soft membership",1+.5+.5,2);
near("GMM soft mean",(.5*2+.5*4)/2,1.5);
near("GMM weights ESS",2**2/(1+.25+.25),8/3);
near("MA1 gamma0",1+.5**2,1.25);
near("MA1 gamma1",-.5,-.5);
near("MA1 long variance",1.25+2*(-.5),.25);
near("MA1 Bartlett L1",1.25+2*.5*(-.5),.75);
near("autodiff shared parameter",(2+2**2)*(1+2*2),30);
near("causal dilated receptive field",1+2*1+2*2,7);
near("adjusted open",100*.5,50);
near("adjusted high",110*.5,55);
near("adjusted low",90*.5,45);
near("adjusted close",105*.5,52.5);
near("split volume turnover",105*.5*(1000/.5),105000);
near("pseudoinverse after scaling",2/101+10*20/101,2);
near("predictable position zero mean",.5*.1+.5*(-.1),0);
near("lookahead position false profit",.5*.1+.5*.1,.1);
near("Rademacher all patterns",3/3,1);
near("unstable GD loss",(.76**2+9*1.16**2)/2,6.344);
near("stable GD loss",(.8**2+9*.8**2)/2,3.2);
near("Simpson strategy unpaired",.9*2+.1*10,2.8);
near("Simpson baseline unpaired",.1*1+.9*9,8.2);
near("Simpson standardized difference",.5*(2-1)+.5*(10-9),1);
const mathHTML=read("index.html"), mlHTML=read("../ml/index.html");
for(const id of mathIds){
 assert(mathHTML.includes("?topic="+id),"math deep-link vanished "+id);
 assert(mlHTML.includes("../math/?topic="+id),"ML-to-math reference vanished "+id);
}
for(const [label,css] of [["math",read("styles.css")],["ml",read("../ml/atlas.css")]]){
 for(const marker of ["overflow-x:auto","line-break:strict",":focus-visible","@media print","@media(max-width:700px)"])
  assert(css.includes(marker),label+" missing responsive/CJK marker "+marker);
}
console.log("V73 PASS: 4 math + 30 ML deepened lectures, "+texPass+" strict/regular KaTeX checks ("+strictPass+" new strict), "+arithmetic+" independently calculated numerical assertions; Chinese integrity, backlinks and responsive CSS confirmed.");
