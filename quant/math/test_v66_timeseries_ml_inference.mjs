import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const base=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,base),"utf8"));
const math="abcd".split("").flatMap(s=>read("lessons-phase-22-"+s+".json").lessons);
const ml=Object.assign({},...("abcd".split("").map(s=>read("../ml/atlas-depth-13-"+s+".json").lessons)));
const mid=["timeseries-n0244","timeseries-n0245","timeseries-n0248","timeseries-n0249","timeseries-n0250","timeseries-n0251","timeseries-n0254","timeseries-n0255"];
const aid=["m01-c03-p03","m02-c03-p05","m03-c02-p02","m04-c05-p04","m05-c05-p03","m06-c01-p02","m07-c01-p04","m08-c01-p06","m09-c03-p02","m10-c04-p04","m11-c05-p04","m12-c03-p06","m13-c02-p06","m14-c04-p04","m15-c01-p01","m16-c03-p01","m17-c02-p02","m18-c03-p01","m19-c02-p04","m20-c02-p02","m21-c02-p02","m22-c01-p06","m23-c04-p05"];
assert.deepEqual(math.map(l=>l.id),mid);
assert.deepEqual(Object.keys(ml),aid);
let expressions=0;
function formula(tex){
 assert(typeof tex==="string"&&tex.trim().length>0,"missing TeX");
 const html=katex.renderToString(tex,{throwOnError:true,strict:"ignore",displayMode:true});
 assert(html.includes("katex"),"unrendered expression");
 expressions++;
}
for(const l of math){
 assert(l.id===l.node_id&&l.deep_sections.length>=6&&l.derivation.length>=5&&l.formulas.length>=7,l.id+" full chapter coverage");
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,l.deep_sections.length,l.id+" duplicate title");
 const extra=l.deep_sections[5];
 assert(extra.question.length>=12&&extra.paragraphs.length===2&&extra.paragraphs.every(p=>p.length>=50),l.id+" substantive Chinese text");
 assert(extra.proof.length>=95&&extra.worked.length>=75&&extra.limitation.length>=38&&extra.formulas.length===2,l.id+" derivation/example/caveat");
 assert(l.links.length>=4&&l.sources.length>=2,l.id+" cross-links/references");
 for(const eq of [...l.formulas,...l.derivation,...l.deep_sections.flatMap(s=>s.formulas)])formula(eq.tex);
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)),l.id+" corrupt characters");
}
for(const id of aid){
 const l=ml[id];assert(l&&l.equations.length>=6&&l.derivation.length>=6,id+" six equations and derivations");
 assert(l.derivation.slice(-3).every(p=>p.length>=55),id+" mathematical reasoning");
 assert(l.worked_example.length>=180&&l.pitfalls.length>=3&&l.protocol.length>=4,id+" numerical and failure examples");
 assert(l.protocol.at(-1).length>=33,id+" verification requirements");
 for(const e of l.equations){assert(e.label.length>2);formula(e.tex);}
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)),id+" CJK characters");
}
let numericChecks=0;
const near=(a,b,id,tol=1e-9)=>{assert(Number.isFinite(a)&&Number.isFinite(b));assert(Math.abs(a-b)<=tol,id+": "+a+" != "+b);numericChecks++;};
near(3*1*1-1,2,"nonlinear white noise squared covariance");
near(.75*(1+.5**2+.5**4),.984375,"Wold 3-step forecast variance");
near(.75*.5**4,.046875,"Wold incremental forecast risk");
near(.5/(1+.5**2),.4,"MA inverse root first ACF");
near(2/(1+2**2),.4,"MA non-invertible root first ACF");
near(.5**2*(.5*2+.25*1),.3125,"ARMA forecast 3 steps");
near(-4/8,-.5,"over-differencing white noise MA lag ACF");
near(2*5*6/(40-5-1),60/34,"AICc five-parameter correction");
near(2*10*11/(40-10-1),220/29,"AICc ten-parameter correction");
near(.99**10,.9043820750088044,"near-unit-root ten-step fraction");
near(Math.log(.5)/Math.log(.99),68.96756393652842,"near-unit-root half-life",1e-6);
near((1.5**2+1**2+1.5**2),5.5,"KPSS level residual cumulative sum");
near(.3**2+.6**2+.3**2,.54,"KPSS detrended residual cumulative sum");
near(4+(.25+(.75/4)),4.4375,"MSE correlated ensemble four",1e-9);
near(2*(1+1),4,"nonsymmetric quadratic first coordinate");
near(.9*.05*4,.18,"conditional pinball upper action");
near(.1*.95*4,.38,"conditional pinball lower action");
near(.75*(2/3)**2+.25*2**2,4/3,"density ratio second moment");
near(400/(4/3),300,"density ratio ESS");
near(.25*4+.75*(-1),.25,"unclipped stochastic gradient");
near(.25*1+.75*(-1),-.5,"clipped stochastic gradient reverses sign");
near((.5*0+.25*2)/(.5+.25),2/3,"observed-only MAR mean");
near(.5*2,1,"IPW MAR target mean");
near((1+4)/4,1.25,"heteroskedastic OLS variance");
near(1/(1+.25),.8,"heteroskedastic GLS variance");
near((1+.75)/(1+.25),1.4,"heteroskedastic GLS mean");
near(9*(.25+.75/16),2.671875,"bagging sixteen trees");
near(9*.25,2.25,"bagging correlation floor");
near(.5+.5,1,"hard-margin SVM KKT stationarity");
near((1+.8)/(1-.8),9,"MCMC integration time");
near(Math.sqrt(9/1000),.09486832980505137,"MCMC standard error");
near(.6-1,-.4,"InfoNCE positive logit gradient");
near(.2/Math.sqrt(1),.2,"InfoNCE negative logit gradient");
near(.2*Math.sqrt(2*Math.log(4)),.33302184414349006,"DSR effective model search four");
near(.2*Math.sqrt(2*Math.log(100)),.6069708517540585,"DSR effective model search hundred");
near(3*2**2+.1**2,12.01,"cubic gradient central-difference error");
near(3*2**2+.01**2,12.0001,"cubic gradient smaller step");
near(785**2,616225,"ViT small patch attention pairs");
near(616225/38809,15.878404493802984,"ViT smaller-patch quadratic ratio",1e-6);
near(.8*.6+.2*.2,.52,"joint path marginal probability");
near(.48/.52,12/13,"joint path backward conditional probability");
near(1/(1+2+1),.25,"masked Encoder first key probability");
near(-Math.log(.5*.25)/2,1.0397207708399179,"response token NLL");
near(-Math.log(.5*.25)/5,.4158883083359672,"incorrect prompt-diluted mean demonstration");
near((4+1+0+0+1+4)/6,5/3,"GNN finite population two-neighbor mean variance");
near(.5*(1+1-1-Math.log(1)),.5,"VAE KL original");
near(2*.1/(1-.5),.4,"approximate policy degradation lower bound");
near(.99**5-.9**5,.3605000499,"model-based 5-step execution optimism");
near(5*(.99-.9),.45,"model-based 5-step stability bound");
near(Math.max(0,Math.max(0,1-.5)+(-1)-.5),0,"CUSUM reset after negative signal");
near(Math.max(0,2-.5),1.5,"CUSUM final likelihood ratio alarm");
near(.2/(2*(2+.2)),1/22,"two-asset one-sided rebalance turnover");
near((.2/(2+.2))*110,10,"two-sided trade notional");
near(110-.001*10,109.99,"self-financing equity after turnover costs");
const mathHtml=fs.readFileSync(new URL("index.html",base),"utf8");
const mlHtml=fs.readFileSync(new URL("../ml/index.html",base),"utf8");
for(const id of mid){
 assert(mathHtml.includes("?topic="+id),"math nav missing "+id);
 assert(mlHtml.includes("../math/?topic="+id),"ML nav missing "+id);
}
for(const name of ["styles.css","../ml/atlas.css"]){
 const css=fs.readFileSync(new URL(name,base),"utf8");
 for(const property of ["overflow-x:auto","line-break:strict",":focus-visible","@media print","@media(max-width:700px)"]){
  assert(css.includes(property),"CJK responsive long KaTeX/print/focus CSS missing "+name+" "+property);
 }
}
console.log("PASS V66: "+math.length+" foundational time-series math lectures; "+aid.length+" ML lectures; "+expressions+" KaTeX formula parses; "+numericChecks+" independent numerical checks; CJK, mobile and print CSS safeguards.");
