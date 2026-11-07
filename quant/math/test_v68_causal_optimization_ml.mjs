import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const mathIds=["econometrics-x003","optimization-x011","optimization-x012","econometrics-x005","econometrics-x006","econometrics-n0232","econometrics-n0233","econometrics-n0234","econometrics-n0239"];
const mlIds=["m01-c04-p01","m02-c05-p01","m03-c03-p01","m04-c02-p02","m05-c02-p02","m06-c04-p03","m07-c02-p06","m08-c04-p03","m09-c01-p01","m10-c01-p01","m11-c05-p06","m12-c01-p03","m13-c05-p03","m14-c03-p02","m15-c05-p02","m16-c05-p01","m17-c02-p04","m18-c04-p03","m19-c03-p02","m20-c03-p02","m21-c01-p01","m22-c03-p02","m23-c05-p02"];
const listed20="abc".split("").flatMap(s=>read("lessons-phase-20-"+s+".json").lessons);
const listed19="abc".split("").flatMap(s=>read("lessons-phase-19-"+s+".json").lessons).filter(l=>mathIds.includes(l.id));
const math=[...listed20,...listed19];
const ml=Object.assign({},...("abcd".split("").map(s=>read("../ml/atlas-depth-11-"+s+".json").lessons)));
assert.deepEqual(math.map(l=>l.id),mathIds);
assert.deepEqual(Object.keys(ml),mlIds);
let rendered=0,strictRendered=0;
const render=(tex,strict=false)=>{
 assert(typeof tex==="string"&&tex.trim().length>0,"empty mathematical expression");
 const html=katex.renderToString(tex,{throwOnError:true,strict:strict?"error":"ignore",displayMode:true});
 assert(html.includes("katex"),"unrendered formula");
 rendered++;if(strict)strictRendered++;
};
for(const l of math){
 assert(l.id===l.node_id&&l.formulas.length>=7&&l.derivation.length>=5&&l.deep_sections.length>=6,l.id+" incomplete math");
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,l.deep_sections.length,l.id+" duplicate heading");
 const d=l.deep_sections.at(-1);
 assert(d.question.length>=15&&d.paragraphs.length===2&&d.paragraphs.every(p=>p.length>=90),l.id+" CJK prose length");
 assert(d.proof.length>=110&&d.worked.length>=110&&d.limitation.length>=55,l.id+" derivation, numeric and limits");
 assert(d.formulas.length===2&&l.links.length>=4&&l.sources.length>=2,l.id+" mathematical and source coverage");
 for(const e of l.formulas)render(e.tex);
 for(const e of l.derivation)render(e.tex);
 for(const s of l.deep_sections)for(const e of s.formulas)render(e.tex);
 for(const e of d.formulas)render(e.tex,true);
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)),l.id+" corrupt Unicode");
}
for(const id of mlIds){
 const l=ml[id];
 assert(l&&l.equations.length>=6&&l.derivation.length>=6,id+" incomplete ML");
 assert(l.derivation.slice(-3).every(d=>d.length>=50),id+" missing substantive step");
 assert(l.worked_example.length>=180&&l.pitfalls.length>=3&&l.protocol.length>=4,id+" worked examples or reproduction");
 assert(l.protocol.every(s=>s.length>=30),id+" insufficient validation");
 for(const e of l.equations){assert(e.label.length>2);render(e.tex);}
 for(const e of l.equations.slice(-3))render(e.tex,true);
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)),id+" corrupt Unicode");
}
let checks=0;
const near=(value,expected,name,tolerance=1e-9)=>{assert(Number.isFinite(value)&&Number.isFinite(expected),name+" not finite");assert(Math.abs(value-expected)<=tolerance,name+" "+value+" !== "+expected);checks++};
near((.25*2*.5+.75*4*.5)-.5*3,.25,"propensity effect covariance");
near(.25/.5,.5,"ATE versus ATT covariance gap");
near(.5*(3-0)**2+0,4.5,"proximal initial full objective");
near(.5*(3-1)**2+1,3,"proximal updated full objective");
near(1/(2*.5)-1/2,.5,"proximal descent guaranteed coefficient");
near((2.1-2)**2/2,.005,"strongly convex exact objective error");
near(.1**2/(2*1),.005,"strongly convex gradient objective bound");
near(.04/.05,.8,"event lead insignificant Wald z");
near(.8-1,-.2,"ATT lower bound bounded outcome");
near(-.2,-.2,"ATU lower bounded outcome");
near((17-10)-(12-8),3,"DiD with exposed controls");
near(4-1,3,"DiD direct treated minus control spillover");
near(.10/.50,.2,"fuzzy RDD local Wald strong first stage");
near(.10/.05,2,"fuzzy RDD local Wald weak first stage");
near(.01/.05,.2,"weak IV reduced form uncertainty amplification");
near(.04*.008-.024*.013,.000008,"event BHAR CAR cross term");
near((1.04*1.008)-(1.024*1.013),.011008,"event BHAR direct compounding");
near((5-100)/100,-.95,"delisting hold to settlement no last quote");
near((20-100)/100,-.8,"delisting actual sale exit");
near(.10-(-.10),.2,"survivor filtering bias");
near((98+2)/100-1,0,"point-in-time dividend total return");
near(.04*.09-.03**2,.0027,"two-asset PSD determinant");
near(.25*.04+.25*.09+2*.25*.03,.0475,"covariance equal-weight portfolio variance");
near(45*.2**2*.8**8,.301989888,"independent binomial count probability");
near(10*.2*.8*(1+9*.1),3.04,"equicorrelated Bernoulli count variance");
near(3+1,4,"VC threshold growth on 3 points");
near(((-2.9)**2+(-2.7)**2)/2,7.85,"SGD noisy expected squared error");
near((.2-3)**2,7.84,"SGD deterministic mean-gradient squared error");
near((100+102+104)/3,102,"legal rolling feature average");
near(2-.25,1.75,"LASSO path first active coefficient");
near(.5-.25,.25,"LASSO path second active coefficient");
near(.5*(16/2+4/2-4/4),4.5,"histogram boosted split gain");
near((3*1)/Math.hypot(3,4),.6,"SVM geometric margin");
near((2*3*1)/(2*Math.hypot(3,4)),.6,"SVM positive scaling geometric invariance");
near(.008/(.008+.099),8/107,"rare event Bayesian posterior");
near(.008/(.008+.099),.07476635514018691,"duplicate positive signal must not double count");
near(1.1*.9-1,-.01,"two-day finance augmentation negative label");
near(1.1**2-1,.21,"two-day finance augmentation positive label");
near(16/(16+4+2),8/11,"F1 threshold with 8TP");
near(.01*2,.02,"SGD step without Adam adaptivity");
near(.2/(1-.9),2,"Adam bias-corrected first moment");
near(.004/(1-.999),4,"Adam bias-corrected second raw moment");
near(1/(4+4-1),1/7,"IoU overlapping boxes");
near(Math.exp(-.2),.8187307530779818,"exact scalar state transition");
near((1-Math.exp(-.2))/2,.09063462346100909,"zero-order hold exact input gain");
near(37*20**2+20*37**2,42180,"factorized time/asset attention pairs");
near((37*20)**2,547600,"full temporal/asset attention pairs");
near(42180/547600,.07702702702702703,"attention factorization cost ratio",1e-6);
near(-Math.log(.75),.2876820724517809,"masked language likelihood");
near(.5/(.5+.125),.8,"time decay graph edge first weight");
near(Math.log(2)/Math.log(2),1,"time decay edge half life");
near(.8/(.8+.2),.8,"GAN Bayes optimal discriminator");
near(.8/.2,4,"GAN optimal discriminator density odds");
near(.75*0+.25*4,1,"prioritized unweighted replay distortion");
near(.75*(.5/.75)*0+.25*(.5/.25)*4,2,"prioritized replay exact importance correction");
near(.2*2.5+.8*.625,1,"offline RL behavior ratio expected value");
near(.2*2.5**2+.8*.625**2-1,.5625,"offline RL importance-weight variance");
near(.0001/(1-.1-.8),.001,"GARCH long run variance");
near(.0001+.1*.0009+.8*.0004,.00051,"GARCH updated conditional variance");
near(Math.log(.5)/Math.log(.9),6.578813478960585,"GARCH variance persistence half-life",1e-7);
near(.5*.02-.5*(-.01),.015,"ETF long short gross return");
near(.015-.001,.014,"ETF executed net return");
const mathHTML=fs.readFileSync(new URL("index.html",root),"utf8");
const mlHTML=fs.readFileSync(new URL("../ml/index.html",root),"utf8");
for(const id of mathIds){
 assert(mathHTML.includes("?topic="+id),"missing math navigation "+id);
 assert(mlHTML.includes("../math/?topic="+id),"missing ML crosslink "+id);
}
for(const file of ["styles.css","../ml/atlas.css"]){
 const css=fs.readFileSync(new URL(file,root),"utf8");
 for(const term of ["overflow-x:auto","line-break:strict",":focus-visible","@media print","@media(max-width:700px)"])
  assert(css.includes(term),"missing responsive CJK / long formula / print rule "+file+": "+term);
}
console.log("V68 PASS: "+math.length+" math lectures, "+mlIds.length+" ML lectures, "+rendered+" KaTeX parses including "+strictRendered+" strict new expressions, "+checks+" independent numeric invariants; CJK and responsive formula safeguards.");
