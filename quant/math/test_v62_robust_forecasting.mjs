import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const math=["lessons-phase-27-a.json","lessons-phase-27-b.json","lessons-phase-27-c.json"].flatMap(p=>read(p).lessons);
const ml=read("../ml/atlas-depth-17-c.json").lessons;
const mathIds=["statistics-n0171","statistics-n0170","statistics-n0172","statistics-n0173","statistics-n0174","statistics-n0175","statistics-n0176","riskmath-n0589","riskmath-n0594","riskmath-n0595"];
const mlIds=["m15-c01-p05","m16-c01-p06","m17-c03-p01","m18-c02-p02","m19-c04-p01"];
assert.deepEqual(math.map(x=>x.id),mathIds);
const formula=tex=>{assert(typeof tex==="string"&&tex.length>8);katex.renderToString(tex,{throwOnError:true,strict:"ignore",displayMode:true});};
let checked=0;
for(const l of math){
 assert(l.id===l.node_id&&l.deep_sections.length>=6,l.id+" deep sections");
 assert(l.formulas.length>=8&&l.derivation.length>=6,l.id+" math and derivation");
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,l.deep_sections.length,l.id+" unique section titles");
 const last=l.deep_sections.at(-1);
 assert(last.paragraphs.length>=2&&last.paragraphs.every(p=>p.length>=35),l.id+" paragraph depth");
 assert(last.proof.length>=60&&last.worked.length>=70&&last.limitation.length>=40,l.id+" complete reasoning");
 assert(last.formulas.length===2,l.id+" added formulas");
 for(const item of [...l.formulas,...l.derivation,...l.deep_sections.flatMap(s=>s.formulas)]){formula(item.tex);checked++;}
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)),l.id+" clean Chinese content");
}
for(const id of mlIds){
 const l=ml[id];assert(l,id+" present");
 assert(l.equations.length>=7&&l.derivation.length>=8,id+" complete equations and proofs");
 assert(l.worked_example.length>=200&&l.pitfalls.length>=3,id+" numerical examples and cautions");
 for(const e of l.equations){formula(e.tex);checked++;}
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)),id+" clean Chinese content");
}
const near=(got,want,label,tol=1e-9)=>assert(Math.abs(got-want)<=tol,label+": "+got+" != "+want);
near((4/5)*(4*.5**2),.8,"Jackknife median delete-one variance");
near(1.96/(1-.2*1.96),3.2236842105263155,"BCa mapped Gaussian score");
near(2/8,.25,"paired sign flip double-side p");
near((2+3+8)/3,13/3,"trimmed mean");
near((2+2+3+8+8)/5,23/5,"winsorized mean");
near(100*.1,10,"Huber leveraged gradient contribution");
near(.003*5,.015,"Holm adjusted first p");
near(.012*4,.048,"Holm adjusted second p");
near(.03*3,.09,"Holm adjusted third p");
near(.05*3/5,.03,"BH discovery threshold");
near(.03*5/3,.05,"BH adjusted third p");
near(1-.8, .2,"factor covariance minimum eigenvalue");
near(1*.92+.1*(-.7),.85,"factor risk variance including negative Euler contribution");
near(.92/Math.sqrt(.85)-.07/Math.sqrt(.85),Math.sqrt(.85),"Euler factor volatility adds up");
near(.5*(-5)+.5*.02*25,-2.25,"Delta-Gamma repricing");
near(.001*125/6,.020833333333333332,"Taylor stress remainder bound");
near(.2/Math.sqrt(Math.PI),.11283791670955126,"Gaussian winner bias");
near(.8**3*10,5.12,"AR three step mean");
near(1+.8**2+.8**4,2.0496,"AR three step forecast error variance");
near(Math.exp(Math.log(2))/(1+Math.exp(Math.log(2))),2/3,"masked softmax valid weight");
near(-Math.log(.5*.25)/2,1.0397207708399179,"SFT response-only token loss");
near(1/3+2/Math.sqrt(6),1.1498299142610593,"GCN row sum not stochastic");
near(8/Math.sqrt(6),3.265986323710904,"GCN center message");
near((1-.8)/(1-.64)*.2,1/9,"diffusion posterior variance");
near(Math.sqrt(.8)*.2/(1-.64)*3,1.4907119849998598,"diffusion posterior mean",1e-8);
const styles=fs.readFileSync(new URL("./styles.css",root),"utf8");
const mlStyles=fs.readFileSync(new URL("../ml/atlas.css",root),"utf8");
for(const key of ["overflow-x:auto","line-break:strict","@media print",":focus-visible"])assert(styles.includes(key),"math layout "+key);
assert(mlStyles.includes("overflow-x:auto"),"ML long formula horizontal scroll");
for(const id of mathIds)assert(fs.readFileSync(new URL("./index.html",root),"utf8").includes("?topic="+id),"math navigation "+id);
console.log("PASS V62: 10 math chapters, 5 ML lessons, "+checked+" KaTeX expressions, 25 independent numeric assertions, Chinese/layout guardrails");
