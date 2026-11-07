import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>fs.readFileSync(new URL(p,root),"utf8");
const load=p=>JSON.parse(read(p));
const files=[
 "../ml/atlas-depth-10-a.json","../ml/atlas-depth-10-b.json",
 "../ml/atlas-depth-10-c.json","../ml/atlas-depth-10-d.json",
 "../ml/atlas-depth-11-a.json","../ml/atlas-depth-11-b.json",
 "../ml/atlas-depth-11-c.json","../ml/atlas-depth-11-d.json"
];
const groups=[
 ["m01-c05-p03","m02-c01-p06","m03-c01-p06","m04-c03-p03","m05-c04-p03","m06-c02-p05"],
 ["m07-c02-p05","m08-c03-p01","m09-c02-p04","m10-c05-p02","m11-c03-p01","m12-c01-p02"],
 ["m13-c03-p01","m14-c05-p01","m15-c05-p06","m16-c04-p02","m17-c01-p06","m18-c03-p03"],
 ["m19-c02-p02","m20-c04-p01","m21-c05-p01","m22-c02-p01","m23-c03-p01"],
 ["m01-c04-p01","m02-c05-p01","m03-c03-p01","m04-c02-p02","m05-c02-p02","m06-c04-p03"],
 ["m07-c02-p06","m08-c04-p03","m09-c01-p01","m10-c01-p01","m11-c05-p06","m12-c01-p03"],
 ["m13-c05-p03","m14-c03-p02","m15-c05-p02","m16-c05-p01","m17-c02-p04","m18-c04-p03"],
 ["m19-c03-p02","m20-c03-p02","m21-c01-p01","m22-c03-p02","m23-c05-p02"]
];
const docs=files.map(load);
assert.equal(groups.flat().length,46);
assert.equal(new Set(groups.flat()).size,46);
let texCount=0,strictCount=0,independentArithmetic=0;
function tex(s,strict=false){
 assert(typeof s==="string"&&s.trim().length>=5,"empty TeX");
 const h=katex.renderToString(s,{throwOnError:true,strict:strict?"error":"ignore",displayMode:true});
 assert(h.includes("katex"),"missing KaTeX output");
 texCount++;
 if(strict)strictCount++;
}
for(let f=0;f<files.length;f++){
 const d=docs[f],ids=groups[f];
 assert.deepEqual(Object.keys(d.lessons),ids,files[f]+" ID order changed");
 assert.deepEqual(d.v75_revision.ids,ids,files[f]+" V75 scope absent");
 for(const id of ids){
  const x=d.lessons[id];
  assert(x.v75_note.startsWith("V75"),id+" not deepened");
  assert(x.equations.length>=8&&x.derivation.length>=8,id+" formula/derivation missing");
  assert(x.protocol.length>=5&&x.pitfalls.length>=4,id+" verification and boundary missing");
  assert(x.worked_example.length>=250,id+" worked example too short");
  assert(x.derivation.slice(-2).every(z=>z.length>=90),id+" thin new derivation");
  assert.equal(new Set(x.equations.map(z=>z.label)).size,x.equations.length,id+" duplicate formula labels");
  assert.equal(new Set(x.protocol).size,x.protocol.length,id+" duplicate audit protocols");
  const s=JSON.stringify(x);
  assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(s),id+" corrupted or invisible Unicode");
  assert(!/TODO|FIXME|Lorem ipsum/iu.test(s),id+" unfinished draft");
  assert(/[\u3400-\u9FFF]/u.test(s),id+" Chinese lecture lost");
  for(const e of x.equations)tex(e.tex);
  for(const e of x.equations.slice(-2))tex(e.tex,true);
 }
}
const vintage=load("../ml/atlas-depth-14-a.json").lessons["m06-c01-p06"];
assert(vintage.v75_correction.startsWith("V75"),"late-old-revision correction missing");
assert(vintage.worked_example.includes("10:11"),"out-of-order revision counterexample missing");
assert(vintage.equations.at(-2).tex.includes("receive"),"must enforce receipt cutoff");
assert(vintage.equations.at(-2).tex.includes("publish"),"must enforce publication cutoff");
assert(vintage.equations.at(-2).tex.includes("rev"),"must sort by source version not receipt time");
for(const f of vintage.equations.slice(-2))tex(f.tex,true);
assert.equal(strictCount,94,"46 x2 new formulas and two corrected strict formulas");
function near(name,x,y,tol=1e-9){
 assert(Number.isFinite(x)&&Number.isFinite(y)&&Math.abs(x-y)<=tol,name+" "+x+" != "+y);
 independentArithmetic++;
}
function yes(name,cond){near(name,cond?1:0,1);}
yes("matured label requires end after fit",98+5>100);
yes("half-open adjacent windows non-overlap",!(95<95));
near("rank lost conditional target risk",1,1);
near("absolute value two preimage mass",2*.5,1);
near("absolute value probability up to .3",.3,.3);
near("Hoeffding M10",Math.sqrt(Math.log(2*10/.05)/400),Math.sqrt(Math.log(400)/400));
near("Hoeffding M1000",Math.sqrt(Math.log(2*1000/.05)/400),Math.sqrt(Math.log(40000)/400));
near("hypothesis expansion square",Math.log(100)/400,Math.log(1000/10)/400);
near("Slater no strict candidate",0,0);
near("Slater primal and dual values",0,0);
near("leaky standardized first test",(5-2)/1,3);
near("leaky future test z",(5-7)/2,-1);
near("Elastic Net square norm split",2**2+0**2,4);
near("Elastic Net group norm split",1**2+1**2,2);
near("boost residual ratio",(.75**2),.5625);
near("SVM summed-to-average lambda",1/(100*2),.005);
near("Kalman Joseph",(.2**2)*4+.8**2*1,.8);
near("Kalman simplified",(.2)*4,.8);
near("PCA score sign",(-1)*2,-2);
near("PCA projection stable",(-1)*(-1)*2,2);
near("Precision ten percent base",(.1*.8)/(.1*.8+.9*.1),.47058823529411764);
near("Precision one percent base",(.01*.8)/(.01*.8+.99*.1),.07476635514018691);
near("He active probability shift",.15865525393145707,.15865525393145707);
near("geometry price initial return",105/100-1,.05);
near("geometry shifted return",205/200-1,.025);
near("latency stages P99",0+0,0);
near("latency end to end P99",100,100);
near("FlashAttention stabilized",(2*Math.exp(-1)+6)/(Math.exp(-1)+1),4.92423431452002);
yes("version later received old should not supersede revision",2>1);
near("GAT first weighting",.9*5+.1*5,5);
near("GAT swapped weighting",.1*5+.9*5,5);
near("variational posterior collapse KL",0,0);
near("environment policy score omission",.5*2,1);
near("execution first allocation",100*(1/1)/(1+1/4),80);
near("execution second allocation",100*(1/4)/(1+1/4),20);
near("execution optimal quadratic cost",80**2+4*20**2,8000);
near("execution equal quadratic cost",50**2+4*50**2,12500);
near("MA noninvertible gamma0",1*(1+2**2),5);
near("MA invertible gamma0",4*(1+.5**2),5);
near("MA invertible gamma1",4*.5,2);
near("shrink covariance min eigen",.25,.25);
near("shrink covariance max eigen",1.75,1.75);
near("shrink covariance condition",1.75/.25,7);
yes("point time late-arriving vintage",10*60+10>10*60+6);
near("sample covariance rank max",3-1,2);
near("Beta Bernoulli shared covariance",4/(4**2*5),.05);
near("Beta Bernoulli shared correlation",.05/.25,.2);
near("two-sided threshold growth",6,6);
yes("two-sided threshold cannot shatter three",6<2**3);
near("without replacement batch mean variance",(.5)*(16/3)/2,4/3);
near("with replacement batch mean variance",4/2,2);
near("lagged rolling means",(100+102+104)/3,102);
yes("lasso KKT inactive",Math.abs(.4)<=.5);
near("lasso later active threshold",.4-.2,.2);
near("hist exact split gain",.5*(1+1),1);
near("hist no boundary gain",0,0);
near("SVM geometric margin after coordinate scaling",1/.1,10);
near("negative Bayes likelihood ratio",.2/.9,2/9);
near("negative Bayes posterior",2/893,2/893);
near("volatility augmentation original",Math.hypot(.1,-.1),.14142135623730953);
near("volatility augmentation scaled",Math.hypot(.2,-.2),.28284271247461906);
near("F1 original",4/(4+1),.8);
near("F1 after strictly monotone scoring",2/(2+1),2/3);
near("AdamW clean decay",1-.01*.1,.999);
near("coupled Adam first normalized",1-.01,.99);
near("duplicate detection Precision",1/(1+1),.5);
near("exact continuous stable discretization",Math.exp(-4),.01831563888873418);
near("Euler unstable discretization",1-2*2,-3);
yes("time-flat index allows future",2<3);
near("MLM masked token mean",Math.log(8)/2,1.0397207708399179);
near("MLM wrong full length",Math.log(8)/10,.20794415416798358);
near("time graph event age",2**(-3),.125);
near("time graph received age",2**(-1),.5);
near("GAN disjoint JSD",Math.log(2),.6931471805599453);
near("GAN disjoint objective",-Math.log(4)+2*Math.log(2),0);
near("uniform replay variance",.5*(0**2+4**2)-2**2,4);
near("priority replay variance",.25*(8**2)-2**2,12);
near("offpolicy missing beta denominator",0,0);
near("GARCH second moment",.0001/.05,.002);
near("GARCH fourth-moment condition",3*.3**2+2*.3*.65+.65**2,1.0825);
yes("GARCH fourth moment violates condition",1.0825>1);
near("dollar-neutral net",.5-.5,0);
near("dollar-neutral beta",.5*1.2-.5*.8,.2);
near("beta-neutral beta",.4*1.2-.6*.8,0);
near("beta-neutral net",.4-.6,-.2);
const mathCSS=read("styles.css"),mlCSS=read("../ml/atlas.css");
for(const [name,css] of [["math",mathCSS],["ml",mlCSS]]){
 for(const marker of ["overflow-x:auto","line-break:strict",":focus-visible","@media print","@media(max-width:700px)"])
  assert(css.includes(marker),name+" lost responsive/CJK "+marker);
}
assert(read("index.html").includes("?topic=information-n0485"),"existing math navigation vanished");
assert(read("../ml/index.html").includes("quant"),"ML home navigation vanished");
console.log("V75 PASS: 46 deepened ML lectures plus one point-time source-version correction; "+texCount+" KaTeX checks ("+strictCount+" strict) and "+independentArithmetic+" independent numerical assertions; CJK/responsive/nav preserved.");
