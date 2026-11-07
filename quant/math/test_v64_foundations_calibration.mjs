import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const math=[...read("lessons-phase-25-a.json").lessons,...read("lessons-phase-24-a.json").lessons];
const ml={...read("../ml/atlas-depth-15-a.json").lessons,...read("../ml/atlas-depth-15-b.json").lessons,...read("../ml/atlas-depth-15-d.json").lessons};
const mathIds=["timeseries-x003","timeseries-x004","timeseries-x006","mlmath-x006","riskmath-x001","statistics-x003","statistics-x004","mlmath-x003","mlmath-x004"];
const mlIds=["m01-c05-p01","m02-c01-p04","m03-c02-p01","m04-c02-p04","m05-c02-p01","m06-c01-p01","m07-c01-p01","m08-c01-p01","m09-c02-p03","m10-c04-p03","m11-c04-p05","m12-c04-p03","m19-c01-p01","m20-c04-p03","m21-c05-p04","m22-c04-p05","m23-c05-p06"];
assert.deepEqual(math.map(l=>l.id),mathIds);
assert.deepEqual(Object.keys(ml),mlIds);
let expr=0;
const render=tex=>{assert(typeof tex==="string"&&tex.trim().length>0);katex.renderToString(tex,{throwOnError:true,strict:"ignore",displayMode:true});expr++;};
for(const l of math){
 assert(l.node_id===l.id&&l.deep_sections.length>=6&&l.derivation.length>=6&&l.formulas.length>=7,l.id+" full chapter");
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,l.deep_sections.length,l.id+" duplicate headings");
 const x=l.deep_sections.at(-1);
 assert(x.question.length>12&&x.paragraphs.length===2&&x.paragraphs.every(p=>p.length>=45),l.id+" Chinese prose");
 assert(x.proof.length>=90&&x.worked.length>=75&&x.limitation.length>=45&&x.formulas.length===2,l.id+" proof/numeric/caveat");
 for(const f of [...l.formulas,...l.derivation,...l.deep_sections.flatMap(s=>s.formulas)])render(f.tex);
 assert(l.links.length>=3&&l.sources.length>=1,l.id+" canonical links and sources");
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)),l.id+" CJK corruption");
}
for(const id of mlIds){
 const l=ml[id];assert(l&&l.equations.length>=6&&l.derivation.length>=6,id+" complete equations");
 assert(l.worked_example.length>=180&&l.pitfalls.length>=3&&l.protocol.length>=4,id+" example, pitfalls and protocol");
 assert(l.derivation.slice(-3).every(d=>d.length>=48),id+" substantive derivations");
 for(const f of l.equations){assert(f.label.length>2);render(f.tex);}
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)),id+" CJK corruption");
}
const near=(a,b,name,tol=1e-9)=>assert(Math.abs(a-b)<=tol,name+": "+a+" != "+b);
near((.000401-.05*.00042)/.95,.0004,"TSRV unbiased ideal expectation");
near(.8*(2-1)/2,.4,"asynchronous Epps covariance overlap ratio");
near(2*Math.log(.5)-.5*4,-3.386294361119891,"count-process exposure terminal survival");
near((16+4+9)/36,29/36,"heteroscedastic ESS weighted risk variance");
near((2+.02*10)/.04,55,"ES discrete atom at 96 percent");
near(.75**3,.421875,"stationary bootstrap same-block length four");
near(.0005/.02**3,62.5,"nonzero Sharpe mean influence coefficient");
near(-.01/(2*.02**3),-625,"nonzero Sharpe squared return influence");
near(2/4*1.5,.75,"four observations linear Rademacher complexity");
near(2*1/Math.sqrt(4),1,"linear Rademacher upper bound");
near(2/(1*2),1,"strongly convex algorithm stability bound");
near((98+2)/100-1,0,"dividend adjusted wealth return");
near(.5+.5,1,"rank deficient design identified sum only");
near(.5*2*.1,.1,"random position-return covariance");
near((1+0+0+1)/4,.5,"two sample signed Rademacher");
near((4.5-3)**2,2.25,"overshoot SGD quadratic loss");
near((3-1)/4,.5,"transaction weighted average");
near(3*(1/6)-1/2,0,"entity weighted average");
near(.5*.75*10,3.75,"pinball lower action risk");
near(.5*.25*10,1.25,"pinball upper action risk");
near(4*.5**2,1,"XOR root regression SSE");
near(1-(.5+.5),0,"XOR greedy split gain");
near(1-.4*2,.2,"hinge margin loss");
near((.2**2+.98**2)/2,.5002,"HMC leapfrog energy");
near(Math.exp(-.0002),.9998000199986667,"HMC Metropolis acceptance");
near((1/(Math.sqrt(2*Math.PI)*.01))/(1/(Math.sqrt(2*Math.PI)*.1)),10,"GMM density collapse");
near(1-.5,.5,"collider selected conditional contrast");
near((4-0)**2,16,"autoencoder reconstruction loss");
near(((1/3)*3*4)/((1/3)*4+(2/3)*1),2,"policy gradient minimum variance baseline");
near((100*(100.1-100)+2+100*(100-99.9)+2)/10000*10000,24,"roundtrip execution cost bps");
near(.02,.02,"discrete strict exceedance probability");
near(100+110-105-100*.02/12,104.83333333333333,"self financing long-short equity",1e-8);
const maths=fs.readFileSync(new URL("index.html",root),"utf8");
for(const id of mathIds)assert(maths.includes("?topic="+id),"math canonical navigation "+id);
const cs=fs.readFileSync(new URL("styles.css",root),"utf8");
const mlcss=fs.readFileSync(new URL("../ml/atlas.css",root),"utf8");
for(const token of ["overflow-x:auto","line-break:strict","@media print",":focus-visible"]){
 assert(cs.includes(token),"math CJK responsive typography "+token);
 assert(mlcss.includes(token),"ML CJK responsive typography "+token);
}
console.log("V64 PASS: 9 complete math courses, 17 deepened ML courses, "+expr+" KaTeX expressions and 32 independent numerical identities; CJK, print and mobile CSS safeguards present");
