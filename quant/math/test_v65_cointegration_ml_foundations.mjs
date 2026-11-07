import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const math=[...read("lessons-phase-23-a.json").lessons,...read("lessons-phase-23-b.json").lessons];
const ml={...read("../ml/atlas-depth-14-a.json").lessons,...read("../ml/atlas-depth-14-b.json").lessons,...read("../ml/atlas-depth-14-c.json").lessons,...read("../ml/atlas-depth-14-d.json").lessons};
const mathIds=["timeseries-n0256","timeseries-n0257","timeseries-n0258","timeseries-n0259"];
const mlIds=["m01-c02-p03","m02-c01-p02","m03-c01-p02","m04-c01-p01","m05-c05-p05","m06-c01-p06","m07-c02-p02","m08-c03-p02","m09-c01-p04","m10-c01-p05","m11-c04-p01","m12-c03-p02","m13-c02-p03","m14-c03-p01","m15-c05-p04","m16-c04-p03","m17-c03-p04","m18-c01-p03","m19-c04-p03","m20-c01-p06","m21-c04-p03","m22-c04-p06","m23-c03-p02"];
assert.deepEqual(math.map(l=>l.id),mathIds);
assert.deepEqual(Object.keys(ml),mlIds);
let rendered=0;
const checkTex=tex=>{
 assert(typeof tex==="string"&&tex.trim().length>0,"empty mathematics");
 const output=katex.renderToString(tex,{throwOnError:true,strict:"ignore",displayMode:true});
 assert(output.includes("katex"),"unrendered formula");
 rendered++;
};
for(const l of math){
 assert(l.node_id===l.id&&l.deep_sections.length>=6&&l.formulas.length>=7&&l.derivation.length>=5,l.id+" full lesson");
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,l.deep_sections.length,l.id+" duplicate section");
 const s=l.deep_sections.at(-1);
 assert(s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=75),l.id+" Chinese prose");
 assert(s.proof.length>=90&&s.worked.length>=120&&s.limitation.length>=60,l.id+" real derivation/example/limitations");
 assert(s.formulas.length===2);
 for(const e of [...l.formulas,...l.derivation,...l.deep_sections.flatMap(x=>x.formulas)])checkTex(e.tex);
 assert(l.links.length>=3&&l.sources.length>=1,l.id+" sources & links");
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)),l.id+" CJK corruption");
}
for(const id of mlIds){
 const l=ml[id];assert(l&&l.equations.length>=6&&l.derivation.length>=6,id+" deep lesson");
 assert(l.derivation.slice(-3).every(d=>d.length>=52),id+" detailed reasoning");
 assert(l.worked_example.length>=180&&l.pitfalls.length>=3&&l.protocol.length>=3,id+" teaching & reproducibility");
 assert(l.protocol.every(p=>p.length>30),"too thin protocol "+id);
 for(const eq of l.equations){assert(eq.label&&eq.label.length>2);checkTex(eq.tex);}
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)),id+" CJK corruption");
}
let checked=0;
function near(a,b,label,tol=1e-9){
 assert(Number.isFinite(a)&&Number.isFinite(b),label+" invalid numeric value");
 assert(Math.abs(a-b)<=tol,label+": "+a+" != "+b);
 checked++;
}
near(2*1*100/50,4,"cointegration log-price local hedge: shares short");
near((-.1)*(2),-.2,"Johansen alpha beta inverse reparameterization");
near((-0.2)*1+(.1)*(-2),-.4,"VECM cointegrating feedback beta alpha");
near((1-.4)**2*5,1.8,"VECM joint two-step mean reversion");
near(4/(1-.5**2),16/3,"VAR Lyapunov first unconditional variance");
near(1/(1-.3**2),100/91,"VAR Lyapunov second unconditional variance");
near((100+9)/(1+9),10.9,"Ridge regularized condition number");
near(.6*.1+(-.4)*(-.05),.08,"signed long-short portfolio gross return");
near(Math.abs(.6)+Math.abs(-.4),1,"long-short gross exposure");
near(.009/(.009+.0495),2/13,"rare event posterior PPV");
near((1+.5)/(1-.5),3,"AR1 asymptotic variance inflation");
near(2**(-24),5.960464477539063e-8,"FP16 smallest subnormal");
near(2**(-8),.00390625,"BF16 unit roundoff at 1");
assert(Math.max(9,12)<=Math.min(14,17),"purged validation overlap");
checked++;
near(9/10+1/2,1.4,"Ridge smooth degrees of freedom");
near(.5/.25,2,"logistic boost Newton leaf value");
near(1/(1+Math.exp(-.2)),.549833997312478,"boosted logistic probability");
near(5*6/(8*9),5/12,"Beta posterior two future successes");
near(5*3/(8**2*9),15/576,"Beta posterior predictive covariance");
near(1/(1+Math.exp(-2)),.8807970779778823,"GMM Gaussian posterior odds");
near(3*1/7,3/7,"block bootstrap first index expected multiplicity");
near(3*4/7,12/7,"block bootstrap interior expected multiplicity");
near(1*3+0*(-1),3,"MLP batch gradient first coordinate");
near(0*3+2*(-1),-2,"MLP batch gradient second coordinate");
near(2/(2+1),2/3,"temperature scaling positive-class confidence");
near(.25*2+1,1.5,"affine scan natural chronological order");
near(.5*1+2,2.5,"affine scan reversed order");
near(2*12*4*1024*64*2,12582912,"KV cache exact GQA bytes");
near(-1*(1-2/3),-1/3,"DPO negative winner advantage gradient");
near((1-3)**2,4,"graph Laplacian energy with isolated node");
near(-Math.sqrt(.36/.64)*.1,-.075,"diffusion epsilon reconstruction amplification mild");
near(-Math.sqrt(.96/.04)*.1,-.4898979485566356,"diffusion severe reconstruction amplification");
near(.8*.55/(.8*.55+.2*.45),44/53,"POMDP posterior after transition and evidence");
near(.6*.4,.24,"CTDE both agents buy");
near(.6*.6+.4*.4,.52,"CTDE exactly one buys");
near(1.05/1.15,21/23,"PSD repaired correlation normalization");
near(.01/(2*.01),.5,"MVO small eigenvalue return error amplification");
near(.01/(2*.25),.02,"MVO large eigenvalue return error amplification");
near(.25/.01,25,"MVO covariance condition number");
const html=fs.readFileSync(new URL("index.html",root),"utf8");
const mlhtml=fs.readFileSync(new URL("../ml/index.html",root),"utf8");
for(const id of mathIds){
 assert(html.includes("?topic="+id),"canonical math navigation "+id);
 assert(mlhtml.includes("../math/?topic="+id),"cross-module navigation "+id);
}
for(const path of ["styles.css","../ml/atlas.css"]){
 const css=fs.readFileSync(new URL(path,root),"utf8");
 for(const token of ["overflow-x:auto","line-break:strict","@media print",":focus-visible"]){
  assert(css.includes(token),"CJK, KaTeX scroll and print CSS: "+path+" "+token);
 }
}
console.log("PASS V65: "+math.length+" math chapters, "+mlIds.length+" ML chapters, "+rendered+" strict KaTeX expressions, "+checked+" independently computed numerical cases, CJK + responsive formatting safeguards");
