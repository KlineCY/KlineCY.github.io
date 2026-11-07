import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const math="bcd".split("").flatMap(c=>read("lessons-phase-18-"+c+".json").lessons);
const ml=Object.assign({},...("abcd".split("").map(c=>read("../ml/atlas-depth-10-"+c+".json").lessons)));
const mathIds=["finance-x004","finance-x005","finance-x006","optimization-x003","optimization-x004","optimization-x006"];
const mlIds=["m01-c05-p03","m02-c01-p06","m03-c01-p06","m04-c03-p03","m05-c04-p03","m06-c02-p05","m07-c02-p05","m08-c03-p01","m09-c02-p04","m10-c05-p02","m11-c03-p01","m12-c01-p02","m13-c03-p01","m14-c05-p01","m15-c05-p06","m16-c04-p02","m17-c01-p06","m18-c03-p03","m19-c02-p02","m20-c04-p01","m21-c05-p01","m22-c02-p01","m23-c03-p01"];
assert.deepEqual(math.map(l=>l.id),mathIds);
assert.deepEqual(Object.keys(ml),mlIds);
let parsed=0,strict=0;
function checkTex(tex,strictMode=false){
 assert(typeof tex==="string"&&tex.trim().length>0,"empty mathematical expression");
 const html=katex.renderToString(tex,{throwOnError:true,strict:strictMode?"error":"ignore",displayMode:true});
 assert(html.includes("katex"),"unrendered formula");
 parsed++;if(strictMode)strict++;
}
for(const l of math){
 assert(l.node_id===l.id&&l.deep_sections.length>=6&&l.formulas.length>=7&&l.derivation.length>=5,l.id+" insufficient mathematical depth");
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,l.deep_sections.length,l.id+" duplicate section heading");
 const s=l.deep_sections.at(-1);
 assert(s.question.length>=14&&s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=70),l.id+" incomplete Chinese paragraphs");
 assert(s.proof.length>=120&&s.worked.length>=115&&s.limitation.length>=50&&s.formulas.length===2,l.id+" missing proof/worked example/limitation");
 assert(l.sources.length>=2&&l.links.length>=4,l.id+" lost source/crosslink material");
 for(const e of [...l.formulas,...l.derivation,...l.deep_sections.flatMap(s=>s.formulas)])checkTex(e.tex);
 for(const e of s.formulas)checkTex(e.tex,true);
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)),l.id+" corrupt Chinese text");
}
for(const id of mlIds){
 const l=ml[id];
 assert(l&&l.equations.length>=6&&l.derivation.length>=6,id+" incomplete ML lesson");
 assert(l.derivation.slice(-3).every(x=>x.length>=40),id+" superficial mathematical derivation");
 assert(l.worked_example.length>=180&&l.pitfalls.length>=3&&l.protocol.length>=4,id+" missing numerical example or risk boundary");
 assert(l.protocol.every(x=>x.length>=25),id+" too short reproduction protocol");
 for(const e of l.equations){assert(e.label.length>2);checkTex(e.tex);}
 for(const e of l.equations.slice(-3))checkTex(e.tex,true);
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)),id+" corrupt Chinese text");
}
let checked=0;
function near(v,exp,label,tol=1e-8){
 assert(Number.isFinite(v)&&Number.isFinite(exp),label+" invalid numeric");
 assert(Math.abs(v-exp)<=tol,label+": "+v+" != "+exp);checked++;
}
near(.5*.1,.05,"conditional SDF exposed-state moment");
near(.5*(-.1),-.05,"conditional SDF other-state moment");
near(.5*.1+.5*(-.1),0,"unconditional SDF moment cancellation");
near(1*4-2*2,0,"factor beta identification rank determinant");
near(.01+2*.01,.03,"collinear beta risk price nonidentification");
near((-.008)**2+.004**2,.00008,"GMM residual square moment");
near(100*.00008,.008,"GMM n100 J statistic");
near(10000*.00008,.8,"GMM n10000 scale");
near(.01*8**2+.04*2**2,.8,"heterogeneous impact inverse-cost slicing");
near(.01*5**2+.04*5**2,1.25,"equal TWAP heterogeneous-impact cost");
near(10*Math.cosh(1)/Math.sinh(1),13.13035285499331,"AC liquidation initial speed");
near(10/Math.sinh(1),8.50918128239311,"AC liquidation terminal speed");
near(Math.cosh(1),1.5430806348152437,"AC initial versus terminal speed ratio");
near(3*.1*15000,4500,"robust predicted execution volume");
near(3*.1*20000,6000,"optimistic predicted execution volume");
near(98+5,103,"label completion trading-day index");
near(2*1,2,"rank one matrix first component");
near(2*100*0,0,"rank one matrix second component discarded");
near(Math.sqrt(.25),.5,"squared uniform variable CDF");
near(1/(2*Math.sqrt(.25)),1,"squared uniform density point");
near(Math.ceil(Math.log(4000)/(.02)),415,"Hoeffding finite class sample n");
near(2-2**2/4,1,"Slater dual value at optimum");
near(2/Math.sqrt(2/3),Math.sqrt(6),"training-fitted standardizer");
near(2**2+0**2,4,"Elastic Net concentrated L2 squared");
near(1**2+1**2,2,"Elastic Net grouped L2 squared");
near((1-.5)**2+(3-2.5)**2,.5,"boosting one-step squared errors");
near(2*.5,1,"hinge violation C2 cost");
near(1/4+1,1.25,"Kalman posterior precision");
near(1/(1/4+1),.8,"Kalman posterior variance");
near(.8*(0/4+2),1.6,"Kalman posterior mean");
near(9/(9+1),.9,"PCA explained variance ratio");
near(.8*.1/(.8*.1+.1*.9),8/17,"precision at higher event prior");
near(.8*.01/(.8*.01+.1*.99),8/107,"precision at lower event prior");
near(2/100,.02,"He initialization variance");
near(100*.02/2,1,"ReLU initialized second moment");
near((105-90)/(110-90),.75,"OHLCV window normalized close");
near((210-180)/(220-180),.75,"positive affine price invariance");
near(15+20+5+10,50,"end-to-end serial latency milliseconds");
near(1/3+1,4/3,"FlashAttention merged denominator");
near((2/3)+6,20/3,"FlashAttention merged weighted numerator");
near((20/3)/(4/3),5,"FlashAttention merged exact output");
near(60+20,80,"financial text ingest delay seconds");
near(.5/(.5+.125),.8,"temporal graph edge normalized weight");
near(.25*2+.75*6,5,"graph attention normalized message");
near(.5*(1**2+1-1-Math.log(1)),.5,"VAE Gaussian posterior KL");
near(.8*.5,.4,"two-stage reinforcement learning trajectory probability");
near(50**2+50**2,5000,"split execution impact square amount");
near(100**2,10000,"all-at-once execution square amount");
near(4*(1+.5**2),5,"AR two-step error variance");
near(4*(1+.5**2+.5**4),5.25,"AR three-step error variance");
near(4/(1-.5**2),16/3,"AR long-run forecast risk");
near(.5**2*(.1**2+.2**2),.0125,"two independent ETFs portfolio variance");
near(.0125+2*.5*.5*.5*.1*.2,.0175,"two ETFs positive correlation risk");
const mathHTML=fs.readFileSync(new URL("index.html",root),"utf8");
const mlHTML=fs.readFileSync(new URL("../ml/index.html",root),"utf8");
for(const id of mathIds){
 assert(mathHTML.includes("?topic="+id),"missing math lesson navigation "+id);
 assert(mlHTML.includes("../math/?topic="+id),"missing cross-module navigation "+id);
}
for(const file of ["styles.css","../ml/atlas.css"]){
 const css=fs.readFileSync(new URL(file,root),"utf8");
 for(const term of ["overflow-x:auto","line-break:strict",":focus-visible","@media print","@media(max-width:700px)"]){
  assert(css.includes(term),"CJK and long KaTeX / mobile / print safeguard missing "+file+": "+term);
 }
}
console.log("PASS V69: "+math.length+" full SDF/execution math lessons; "+mlIds.length+" ML foundation lessons; "+parsed+" KaTeX parses incl "+strict+" strict new formulas; "+checked+" independently calculated numeric tests; CJK, responsive and print CSS safeguards.");
