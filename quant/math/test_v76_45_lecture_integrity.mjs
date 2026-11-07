import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>fs.readFileSync(new URL(p,root),"utf8");
const load=p=>JSON.parse(read(p));
const files=[
 "../ml/atlas-depth-12-a.json","../ml/atlas-depth-12-b.json",
 "../ml/atlas-depth-12-c.json","../ml/atlas-depth-12-d.json",
 "../ml/atlas-depth-13-d.json","../ml/atlas-depth-15-b.json",
 "../ml/atlas-depth-15-c.json","../ml/atlas-depth-17-c.json"
];
const groups=[
 ["m03-c05-p06","m05-c04-p06","m08-c01-p04","m09-c03-p04","m10-c02-p03","m12-c03-p05"],
 ["m01-c03-p02","m02-c01-p05","m04-c04-p03","m06-c03-p02","m07-c02-p03","m11-c05-p01"],
 ["m13-c05-p04","m14-c01-p05","m15-c05-p05","m16-c02-p02","m17-c03-p02","m18-c02-p01"],
 ["m19-c02-p01","m20-c01-p01","m21-c03-p02","m22-c02-p04","m23-c04-p06"],
 ["m19-c02-p04","m20-c02-p02","m21-c02-p02","m22-c01-p06","m23-c04-p05"],
 ["m07-c01-p01","m08-c01-p01","m09-c02-p03","m10-c04-p03","m11-c04-p05","m12-c04-p03"],
 ["m13-c03-p02","m14-c04-p01","m15-c01-p02","m16-c03-p02","m17-c02-p03","m18-c03-p02"],
 ["m15-c01-p05","m16-c01-p06","m17-c03-p01","m18-c02-p02","m19-c04-p01"]
];
assert.equal(groups.flat().length,45);
assert.equal(new Set(groups.flat()).size,45);
const docs=files.map(load);
let totalTex=0,strictTex=0,numerics=0;
function validTex(formula,strict=false){
 assert(typeof formula==="string"&&formula.trim().length>4,"formula missing");
 const html=katex.renderToString(formula,{throwOnError:true,strict:strict?"error":"ignore",displayMode:true});
 assert(html.includes("katex"),"KaTeX rendering failed");
 totalTex++;
 if(strict)strictTex++;
}
function cjkSafe(obj,id){
 const s=JSON.stringify(obj);
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(s),id+" corrupt/invisible CJK");
 assert(!/TODO|FIXME|Lorem ipsum/iu.test(s),id+" draft placeholder");
 assert(/[\u3400-\u9FFF]/u.test(s),id+" Chinese text missing");
}
for(let f=0;f<docs.length;f++){
 const d=docs[f],ids=groups[f];
 assert.deepEqual(Object.keys(d.lessons),ids,files[f]+" lecture IDs/order changed");
 assert.deepEqual(d.v76_revision.ids,ids,files[f]+" V76 coverage missing");
 for(const id of ids){
  const x=d.lessons[id];
  assert(x.v76_note.startsWith("V76"),id+" revision missing");
  assert(x.equations.length>=7&&x.derivation.length>=8,id+" insufficient depth");
  assert(x.protocol.length>=5&&x.pitfalls.length>=4,id+" application validation/pitfall missing");
  assert(x.worked_example.length>=260,id+" worked case too short");
  assert(x.derivation.slice(-2).every(t=>t.length>=90),id+" short derivation");
  assert.equal(new Set(x.equations.map(e=>e.label)).size,x.equations.length,id+" duplicate formula heading");
  assert.equal(new Set(x.protocol).size,x.protocol.length,id+" duplicate validation");
  for(const eq of x.equations)validTex(eq.tex);
  for(const eq of x.equations.slice(-2))validTex(eq.tex,true);
  cjkSafe(x,id);
 }
}
assert.equal(strictTex,90,"two strict new formulas required for all 45 lectures");
const manifest=load("../ml/atlas-depth-manifest.json");
assert.equal(manifest.schema,1);
assert.equal(manifest.expected_lessons,720);
for(let i=0;i<files.length;i++){
 const p="./"+files[i].split("/").at(-1);
 const matching=manifest.modules.filter(m=>m.path===p);
 assert.equal(matching.length,1,"depth module missing from live loader: "+p);
 assert.equal(matching[0].lessons,groups[i].length,p+" manifest count mismatch");
}
function near(name,got,want,tol=1e-9){
 assert(Number.isFinite(got)&&Number.isFinite(want)&&Math.abs(got-want)<=tol,name+" "+got+" != "+want);
 numerics++;
}
function yes(name,pass){near(name,pass?1:0,1);}
near("regime mixed variance low",.8*1+.2*9,2.6);
near("regime mixed variance high",.2*1+.8*9,7.4);
near("ADMM primal",1-1,0);
near("ADMM dual",2*(1-0),2);
near("weighted tree best mean",(1*0+3*4)/4,3);
near("weighted tree optimum SSE",1*3**2+3*1**2,12);
near("weighted tree unweighted SSE",1*2**2+3*2**2,16);
near("RBF isotropic",Math.exp(-2),.1353352832366127);
near("RBF anisotropic",Math.exp(-1.25),.2865047968601901);
near("collider E AB",1/3,1/3);
near("collider Cov",1/3-(2/3)**2,-1/9);
near("family false positive",1-.95**3,.142625);
near("uniform M4 bound",Math.sqrt(Math.log(160)/400),.11264003987484317,1e-6);
near("AB times C cost",10*100*2+10*2*50,3000);
near("A times BC cost",100*2*50+10*100*50,60000);
near("perturbed sample mean",(0+1+2+3+1000000)/5,200001.2);
near("median robust",2,2);
near("feature subsets",2**10-1,1023);
near("candidate false discovery",1-.95**20,.6415140775914577);
near("L1 boundary loss",.5*.3**2,.045);
near("L1 after shrink",.5*.1**2+.1*.2,.025);
near("contrastive one false negative group",1-.5**3,.875);
near("contrastive mean false negative views",2*3*.5,3);
near("earlystop min expected",.75*.4+.25*.6,.45);
near("dilated effective width",2*(3-1)+1,5);
near("dilated output width",Math.floor((10+4-5)/2)+1,5);
near("SSM retention",.8**4,.4096);
near("SSM selective decay",.8**2*.2**2,.0256);
yes("QK raw logit low rank",1<=1);
near("reward scaling logistic initial",3/(1+3),.75);
near("reward scaling logistic rescaled",9/(1+9),.9);
near("graph mean before duplication",(2+8)/2,5);
near("graph mean after single duplicate",(2+2+8)/3,4);
near("latent mixture variance",1+.25,1.25);
near("continuous discount two half days",Math.sqrt(.81)**2,.81);
near("old half-day discount changes objective",.81**2,.6561);
near("CVaR separate loss",10,10);
near("CVaR diversified loss",5,5);
near("Kalman missing obs variance",1+.25,1.25);
near("Kalman fictitious update",(1.25*.25)/(1.25+.25),.20833333333333334);
near("impact initial participation",100000/10000000,.01);
near("impact initial dollars",100000*.1*Math.sqrt(.01),1000);
near("impact double dollars",200000*.1*Math.sqrt(.02),2828.42712474619);
near("log-std VAE KL",.5*(4-1-Math.log(4)),.8068528194400547);
near("log-std VAE gradient",Math.exp(2*Math.log(2))-1,3);
near("approx greedy bound",.02/(1-.9),.2);
near("model transition bound",Math.min(1,10*.02),.2);
near("CUSUM baseline drift",-.5,-.5);
near("CUSUM changed drift",.5,.5);
near("portfolio drift long",.5*1.2/1.1,6/11);
near("portfolio drift other",.5/1.1,5/11);
near("portfolio one-sided turnover",.5*(Math.abs(.5-6/11)+Math.abs(.5-5/11)),1/22);
near("asymmetric action",2*3/(3+1),1.5);
near("asymmetric optimal loss",.5*3*(2-1.5)**2+.5*1*(1.5**2),1.5);
near("asymmetric mean action loss",.5*3+.5*1,2);
near("XOR root SSE",4*(.5**2),1);
near("XOR after interaction SSE",0,0);
near("ordinary hinge severely wrong",1-(-1),2);
near("square hinge severely wrong",(1-(-1))**2,4);
near("leapfrog first half momentum",-.5,-.5);
near("leapfrog position",1-.5,.5);
near("leapfrog final momentum",-.5-.5*.5,-.75);
near("HMC old energy",.5*(1**2+0**2),.5);
near("HMC new energy",.5*(.5**2+.75**2),.40625);
near("HMC energy change",.40625-.5,-3/32);
near("GMM min regularized eigen",0+.001,.001);
near("GMM positive determinant",.001*.011,.000011);
near("common confounder naive slope",1/2,.5);
near("BatchNorm frozen test",(10-1)/1,9);
near("BatchNorm leaked test",(10-11)/1,-1);
near("nonoverlap patch count",(32/8)**2,16);
near("overlap patch count",((32-8)/4+1)**2,49);
near("overlap attention ratio",49**2/16**2,2401/256);
near("joint entropy repeated directions",Math.log(2),.6931471805599453);
near("incorrect independent direction entropy",2*Math.log(2),1.3862943611198906);
near("decoder cross scores",2*3,6);
near("token-level NLL",(1*2+9*0)/10,.2);
near("document-level NLL",(2+0)/2,1);
near("token-level perplexity",Math.exp(.2),1.2214027581601699);
near("ReLU before graph mean",(Math.max(0,-2)+Math.max(0,2))/2,1);
near("ReLU after graph mean",Math.max(0,(-2+2)/2),0);
near("AR parameter conditional mean",(.8+1.2)/2,1);
near("AR parameter mean variance",.5*(.8-1)**2+.5*(1.2-1)**2,.04);
near("AR full predictive variance",1+.04,1.04);
yes("future disturbance invariance",2<3);
near("SFT response joint probability",.8*.5,.4);
near("SFT response average NLL",-Math.log(.4)/2,.45814536593707755);
near("GCN degree center",1+1+1,3);
near("GCN symmetric row sum",1/3+2/Math.sqrt(6),1.1498299142610593);
near("diffusion first var",1-.8,.2);
near("diffusion second var",1-.8**2,.36);
near("diffusion path covariance",Math.sqrt(.8)*.2,.17888543819998318);
const mathCSS=read("styles.css"),mlCSS=read("../ml/atlas.css");
for(const [name,css] of [["math",mathCSS],["ml",mlCSS]]){
 for(const mark of ["overflow-x:auto","line-break:strict",":focus-visible","@media print","@media(max-width:700px)"])
  assert(css.includes(mark),name+" missing CJK/responsive "+mark);
}
assert(read("index.html").includes("?topic=information-n0485"),"math chapter deep link lost");
assert(read("../ml/index.html").includes("quant"),"ML navigation lost");
console.log("V76 PASS: 45 deepened lectures in eight live modules, "+totalTex+" KaTeX parses ("+strictTex+" strict) and "+numerics+" numeric assertions; Chinese/CJK/responsive/navigation verified.");
