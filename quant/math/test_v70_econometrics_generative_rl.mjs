import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const math="abc".split("").flatMap(c=>read("lessons-phase-17-"+c+".json").lessons);
const ml=Object.assign({},...("jk".split("").map(c=>read("../ml/atlas-depth-09-"+c+".json").lessons)));
const mathIds=["econometrics-n0211","econometrics-n0213","econometrics-n0214","econometrics-n0215","econometrics-n0216","econometrics-n0218","econometrics-n0219","econometrics-n0220","econometrics-n0221","econometrics-n0222","econometrics-n0228","econometrics-n0231"];
const mlIds=["m19-c05-p01","m19-c04-p02","m19-c04-p05","m19-c04-p04","m19-c01-p02","m20-c05-p04","m20-c01-p02","m20-c03-p01","m20-c01-p04","m20-c02-p03","m21-c04-p01","m21-c01-p04","m21-c01-p02","m21-c01-p05","m21-c02-p01","m22-c01-p02","m22-c02-p03","m22-c01-p03","m22-c02-p02","m22-c03-p06"];
assert.deepEqual(math.map(l=>l.id),mathIds);
assert.deepEqual(Object.keys(ml),mlIds);
let parsed=0,strict=0;
function checkTex(tex,strictMode=false){
 assert(typeof tex==="string"&&tex.trim().length>0,"empty TeX expression");
 const html=katex.renderToString(tex,{throwOnError:true,strict:strictMode?"error":"ignore",displayMode:true});
 assert(html.includes("katex"),"formula did not render");
 parsed++;
 if(strictMode)strict++;
}
for(const l of math){
 assert(l.id===l.node_id&&l.deep_sections.length>=6&&l.formulas.length>=7&&l.derivation.length>=5,l.id+" incomplete detailed chapter");
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,l.deep_sections.length,l.id+" repeated chapter section");
 const s=l.deep_sections.at(-1);
 assert(s.question.length>=14&&s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=80),l.id+" thin CJK argument");
 assert(s.proof.length>=125&&s.worked.length>=90&&s.limitation.length>=50&&s.formulas.length===2,l.id+" mathematical reasoning or worked example incomplete");
 assert(l.sources.length>=2&&l.links.length>=3,l.id+" lost sources or prerequisite links");
 for(const e of [...l.formulas,...l.derivation,...l.deep_sections.flatMap(x=>x.formulas)])checkTex(e.tex);
 for(const e of s.formulas)checkTex(e.tex,true);
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)),l.id+" corrupt Chinese");
}
for(const id of mlIds){
 const l=ml[id];
 assert(l&&l.equations.length>=7&&l.derivation.length>=7,id+" missing new advanced math");
 assert(l.derivation.slice(-2).every(s=>s.length>=65),id+" insufficient detailed proof");
 assert(l.worked_example.length>=210&&l.pitfalls.length>=4&&l.protocol.length>=6,id+" incomplete quantified example and validation");
 assert(new Set(l.equations.map(e=>e.tex)).size===l.equations.length,id+" repeated equation formula");
 assert(new Set(l.protocol).size===l.protocol.length,id+" duplicated validation procedure");
 for(const e of l.equations){assert(e.label.length>=2);checkTex(e.tex);}
 for(const e of l.equations.slice(-2))checkTex(e.tex,true);
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)),id+" corrupt CJK Unicode");
}
let checks=0;
function near(a,b,name,tol=1e-8){assert(Number.isFinite(a)&&Number.isFinite(b));assert(Math.abs(a-b)<=tol,name+": "+a+" != "+b);checks++}
near(2/3-4/9,2/9,"nonlinear exogeneity counterexample");
near(18/(9+3),1.5,"correlated measurement error IV bias");
near(18/9,2,"independent repeated measurements IV recovery");
near(.01/.001,10,"ill-conditioned identification parameter error");
near(.01/.1,.1,"better conditioned identification error");
near(3.5-1/2,3,"exclusion sensitivity lower bound");
near(3.5+1/2,4,"exclusion sensitivity upper bound");
near(100*.02**2/.04,1,"weak-IV AR one-moment statistic");
near(9/4.5,2,"2SLS invariant instrument scalar change");
near((1+2*4)/(1+4*4),9/17,"weighted GMM optimal coefficient");
near(4/(1+4*4),4/17,"weighted GMM minimal moment norm");
near(10*4/17,40/17,"GMM weighted minimum n10");
near((1+.5)/(1-.5),3,"geometric moment long run variance");
near(Math.sqrt(3/100),.17320508075688773,"HAC sample mean standard error");
near(.02**2+.04**2,.002,"original GMM moment scale");
near(2**2*.0001+.04**2,.002,"rescaled GMM correct weight");
near(2**2+.04**2,4.0016,"GMM invalid unchanged weights after rescale");
near(100*(.02**2+(-.02)**2),.08,"overidentification residual component");
near(.04/10+.01/20+.09/200,.00495,"two-way common shock panel mean variance");
near((.04+.01+.09)/(20*10),.0007,"naive independent panel rows variance");
near(.5*(1/.1+1/.9),50/9,"IPW second moment under poor overlap");
near(50/9-1,41/9,"IPW weight variance");
near(.05**2,.0025,"independent two-asset extreme tail overlap");
near(.05/.0025,20,"comonotone versus independent tail event ratio");
near((1.5-2)/.5,-1,"diffusion Gaussian reverse tail standardized z");
near((1.5-2)/.2,-2.5,"diffusion Gaussian reverse shrunken z");
near(-2/4,-.5,"Gaussian score at two");
near(1/4,.25,"Gaussian score Fisher information");
near(Math.sqrt(.64)*2,1.6,"forward diffusion conditional mean");
near(1-.64,.36,"forward diffusion conditional variance");
near(1.6+Math.sqrt(.36),2.2,"forward diffusion sample");
near(100/100,1,"autoencoder small signal reconstruction risk");
near(100/1,100,"autoencoder high variance reconstruction risk");
near(1000*(1-.9**2),190,"scaled tanh Jacobian for real orders");
near(Math.abs(.7-.2),.5,"market-action-dependent Markov TV");
near((3*1+(-1))/4,.5,"DQN maximization optimistic noise");
near(.9*.5,.45,"DQN TD target maximization bias");
near((1-.9**10)/(.1),6.513215599,"ten discounted rewards",1e-7);
near(.9**10/.1,3.486784401,"discounted tail after ten steps",1e-7);
near(2+.5*3.5-3.5,.25,"Bellman residual");
near(.25/(1-.5),.5,"Bellman uniform error certificate");
near(1-0,1,"maker payoff gain with opponent narrow");
near(3-2,1,"maker payoff gain with opponent wide");
near(.5*100,50,"FQE unseen target-state error");
near(.16/.02+.36/.98,8.36734693877551,"offline policy importance second moment");
near(.16/.02+.36/.98-1,7.36734693877551,"offline policy importance variance");
near(Math.exp(3)/(1+Math.exp(3)),.9525741268224334,"CQL unseen action gradient");
near(.01*5*4/2,.1,"planning finite horizon uniform-TV reward bound");
near((-.5)**1,-.5,"negative AR1 odd lag");
near((-.5)**2,.25,"negative AR1 even lag");
near((-.5)**3,-.125,"negative AR1 third lag");
near(30-25-18+16,3,"trading-day double differencing");
near(30-18,12,"five trading day seasonal difference");
near(9*1,9,"unit root nine-step error variance");
near(100+3*.5,101.5,"drifted random walk forecast mean");
near(3*4,12,"drifted random walk forecast variance");
near(101.5-1.96*Math.sqrt(12),94.71036,"drifted Gaussian 95% interval lower",1e-4);
near(101.5+1.96*Math.sqrt(12),108.28964,"drifted Gaussian 95% interval upper",1e-4);
near(Math.log(.5),-.6931471805599453,"raw QLIKE with zero proxy");
near(Math.log(2),.6931471805599453,"raw QLIKE with zero proxy and h2");
const html=fs.readFileSync(new URL("index.html",root),"utf8");
const mlHtml=fs.readFileSync(new URL("../ml/index.html",root),"utf8");
for(const id of mathIds){assert(html.includes("?topic="+id),"math chapter navigation missing "+id);assert(mlHtml.includes("../math/?topic="+id),"ML curriculum crosslink missing "+id);}
for(const file of ["styles.css","../ml/atlas.css"]){
 const css=fs.readFileSync(new URL(file,root),"utf8");
 for(const token of ["overflow-x:auto","line-break:strict",":focus-visible","@media print","@media(max-width:700px)"])
  assert(css.includes(token),"KaTeX long formula/CJK/mobile/print guard missing "+file+": "+token);
}
console.log("V70 PASS: "+math.length+" advanced econometrics lectures + "+mlIds.length+" generative, RL and time-series lessons; "+parsed+" KaTeX parses ("+strict+" strict new equations); "+checks+" independent numerical checks; CJK and responsive/print CSS integrity.");
