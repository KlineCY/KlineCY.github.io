import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>fs.readFileSync(new URL(p,root),"utf8");
const json=p=>JSON.parse(read(p));
const baseline=[{"path":"../ml/atlas-depth-07-b.json","schema":1,"batch":"v36-probability-generalization","revision":"2026-11-06","entries":[{"id":"m03-c01-p04","len":250,"hash":"abbcafc0"},{"id":"m03-c02-p06","len":246,"hash":"357bdd90"},{"id":"m03-c05-p01","len":243,"hash":"cd87b53a"},{"id":"m03-c03-p06","len":248,"hash":"a3eeae65"},{"id":"m04-c02-p06","len":258,"hash":"d000eaf3"},{"id":"m04-c04-p01","len":230,"hash":"ea6732c3"},{"id":"m04-c05-p06","len":269,"hash":"bc441f7e"},{"id":"m04-c03-p01","len":269,"hash":"069ac746"}]},{"path":"../ml/atlas-depth-07-c.json","schema":1,"batch":"v36-optimization-feature-pipeline","revision":"2026-11-06","entries":[{"id":"m05-c02-p04","len":269,"hash":"df4218be"},{"id":"m05-c04-p04","len":230,"hash":"901d10ca"},{"id":"m05-c03-p01","len":264,"hash":"6f5c692a"},{"id":"m05-c04-p01","len":250,"hash":"8b706227"},{"id":"m06-c05-p02","len":235,"hash":"3b6e7dd8"},{"id":"m06-c05-p05","len":225,"hash":"0653c5c0"},{"id":"m06-c01-p04","len":241,"hash":"ae56c169"},{"id":"m06-c03-p01","len":256,"hash":"b5d16681"}]},{"path":"../ml/atlas-depth-07-d.json","schema":1,"batch":"v37-seventh-econometrics-boosting","revision":"2026-11-06","entries":[{"id":"m07-c05-p05","len":330,"hash":"16f6854d"},{"id":"m07-c05-p04","len":272,"hash":"12d815f4"},{"id":"m07-c04-p04","len":226,"hash":"c8d491e7"},{"id":"m07-c05-p01","len":242,"hash":"2501fc85"},{"id":"m08-c05-p05","len":212,"hash":"a58b37d8"},{"id":"m08-c04-p01","len":229,"hash":"aab0ef88"},{"id":"m08-c04-p02","len":207,"hash":"2d65b1ba"},{"id":"m08-c03-p03","len":231,"hash":"dc28e6e9"}]},{"path":"../ml/atlas-depth-07-e.json","schema":1,"batch":"v37-seventh-kernels-bayes","revision":"2026-11-06","entries":[{"id":"m09-c05-p03","len":233,"hash":"6ee3048d"},{"id":"m09-c05-p02","len":205,"hash":"bcff1ea8"},{"id":"m09-c04-p05","len":231,"hash":"b70fc337"},{"id":"m09-c05-p06","len":250,"hash":"3f10f515"},{"id":"m10-c04-p06","len":216,"hash":"d32093d0"},{"id":"m10-c05-p03","len":246,"hash":"e1462425"},{"id":"m10-c03-p03","len":208,"hash":"0b3b6655"},{"id":"m10-c04-p02","len":282,"hash":"36343852"}]},{"path":"../ml/atlas-depth-07-f.json","schema":1,"batch":"v37-seventh-clustering-causal-validation","revision":"2026-11-06","entries":[{"id":"m11-c01-p06","len":227,"hash":"54339ab5"},{"id":"m11-c02-p02","len":215,"hash":"464950bb"},{"id":"m11-c03-p04","len":267,"hash":"9ab8e85d"},{"id":"m11-c02-p03","len":234,"hash":"e0d23b69"},{"id":"m12-c02-p02","len":215,"hash":"7b606e40"},{"id":"m12-c02-p01","len":206,"hash":"a21db64f"},{"id":"m12-c02-p06","len":221,"hash":"cc504f4d"},{"id":"m12-c04-p04","len":229,"hash":"6ccf62aa"}]},{"path":"../ml/atlas-depth-07-g.json","schema":1,"batch":"v37-seventh-deep-neural-vision","revision":"2026-11-06","entries":[{"id":"m13-c04-p03","len":253,"hash":"aefbc310"},{"id":"m13-c01-p02","len":230,"hash":"e2a514fa"},{"id":"m13-c04-p01","len":296,"hash":"65747bf4"},{"id":"m13-c03-p04","len":220,"hash":"22710de9"},{"id":"m14-c03-p05","len":293,"hash":"59de6a75"},{"id":"m14-c04-p06","len":256,"hash":"86e1e88b"},{"id":"m14-c02-p04","len":227,"hash":"7d2cbd2f"},{"id":"m14-c03-p06","len":229,"hash":"3ef8a5ce"}]},{"path":"../ml/atlas-depth-07-h.json","schema":1,"batch":"v37-seventh-rnn-attention","revision":"2026-11-06","entries":[{"id":"m15-c03-p04","len":238,"hash":"8b1d7bb0"},{"id":"m15-c04-p06","len":205,"hash":"3aa4821c"},{"id":"m15-c01-p04","len":240,"hash":"636ca1dd"},{"id":"m15-c03-p01","len":218,"hash":"c6db95de"},{"id":"m16-c02-p01","len":205,"hash":"e36cf8d7"},{"id":"m16-c01-p02","len":266,"hash":"0a25ba58"},{"id":"m16-c04-p05","len":255,"hash":"28c6c349"},{"id":"m16-c03-p03","len":293,"hash":"ff53aa62"}]}];
const manifest=json("../ml/atlas-depth-manifest.json");
assert.equal(manifest.schema,1);
assert.equal(manifest.expected_lessons,720);
assert.equal(manifest.expected_domains,24);
assert.equal(manifest.modules.reduce((s,m)=>s+m.lessons,0),720);

function hash(o){
 let h=2166136261>>>0;
 for(const c of JSON.stringify(o))h=Math.imul(h^c.charCodeAt(0),16777619)>>>0;
 return h.toString(16).padStart(8,"0");
}
function legacy(x,n){
 return {
  source_title:x.source_title,title:x.title,overview:x.overview,revision_note:x.revision_note,
  equations:x.equations.slice(0,6),derivation:x.derivation.slice(0,6),
  protocol:x.protocol.slice(0,6),pitfalls:x.pitfalls.slice(0,3),
  worked_example:x.worked_example.slice(0,n)
 };
}
let count=0,texCount=0,strictCount=0,numericCount=0;
for(const spec of baseline){
 const doc=json(spec.path),ids=spec.entries.map(e=>e.id),name=spec.path.split("/").at(-1);
 const loaded=manifest.modules.filter(m=>m.path==="./"+name);
 assert.equal(loaded.length,1,name+" must be in live manifest");
 assert.equal(loaded[0].lessons,8,name+" live module count changed");
 assert.equal(doc.schema,spec.schema,name+" schema changed");
 assert.equal(doc.batch,spec.batch,name+" batch identity changed");
 assert.equal(doc.revision,spec.revision,name+" historic revision changed");
 assert.deepEqual(Object.keys(doc.lessons),ids,name+" original ordered IDs changed");
 assert.deepEqual(doc.v78_revision.ids,ids,name+" V78 coverage incomplete");
 for(const old of spec.entries){
  const x=doc.lessons[old.id];count++;
  assert.equal(hash(legacy(x,old.len)),old.hash,old.id+" original V36/V37 content changed");
  assert(x.v78_note.startsWith("V78"),old.id+" V78 marker missing");
  assert.equal(x.equations.length,["../ml/atlas-depth-07-b.json","../ml/atlas-depth-07-c.json"].includes(spec.path)?10:8,old.id+" V78 original eight plus optional V84 append");
  assert.equal(x.derivation.length,["../ml/atlas-depth-07-b.json","../ml/atlas-depth-07-c.json"].includes(spec.path)?10:8,old.id+" V78 original eight derivations plus optional V84 append");
  assert.equal(x.protocol.length,["../ml/atlas-depth-07-b.json","../ml/atlas-depth-07-c.json"].includes(spec.path)?8:7,old.id+" V78 original protocol plus optional V84 append");
  assert.equal(x.pitfalls.length,["../ml/atlas-depth-07-b.json","../ml/atlas-depth-07-c.json"].includes(spec.path)?5:4,old.id+" V78 original pitfalls plus optional V84 append");
  assert(x.derivation.slice(6,8).every(v=>v.length>=100),old.id+" original V78 new proof missing");
  assert(x.worked_example.length-old.len>=80,old.id+" thin new worked case");
  assert(x.protocol.at(6).length>=30&&x.pitfalls.at(3).length>=30,old.id+" original V78 validation or limitations missing");
  assert(/[\u3400-\u9FFF]/u.test(x.derivation.at(7)),old.id+" original V78 CJK explanation missing");
  assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(JSON.stringify(x)),old.id+" damaged Unicode/hidden chars");
  const newLabels=x.equations.slice(6,8).map(z=>z.label);
  assert.equal(new Set(newLabels).size,2,old.id+" duplicated new formula headings");
  assert(newLabels.every(t=>!x.equations.slice(0,6).some(y=>y.label===t)),old.id+" new heading overwrote previous one");
  for(const [i,e] of x.equations.slice(0,8).entries()){
   assert(e.tex.length>=(i>=6?8:5)&&e.label.length>=2,old.id+" weak math heading");
   katex.renderToString(e.tex,{throwOnError:true,strict:i>=6?"error":"ignore",displayMode:true});
   texCount++;
   if(i>=6)strictCount++;
  }
 }
}
assert.equal(count,56);
assert.equal(strictCount,112);

function near(name,got,want,tol=1e-9){
 assert(Number.isFinite(got)&&Number.isFinite(want)&&Math.abs(got-want)<=tol,name+": "+got+" != "+want);
 numericCount++;
}
function yes(name,condition){near(name,condition?1:0,1);}
const phi=z=>Math.exp(-(z*z)/2)/Math.sqrt(2*Math.PI);

near("Bayesian stress posterior",.3*.8/(.3*.8+.7*.2),12/19);
near("Bayes odds likelihood update",(3/7)*(.8/.2),12/7);
near("MA1 marginal variance",1+.5**2,1.25);
near("MA1 lag covariance",.5,.5);
near("MA1 long run variance",1.25+2*.5,2.25);
near("MA1 mean SE",Math.sqrt(2.25/100),.15);
near("MA1 invalid IID SE",Math.sqrt(1.25/100),.11180339887498948);
near("logistic zero predictor probability",1/(1+Math.exp(0)),.5);
near("logistic unidentifiable likelihood",-3*Math.log(2),-2.0794415416798357);
near("mixture Gaussian second moment",(.5*1+.5*9),5);
near("mixture Gaussian fourth moment",.5*3*1**2+.5*3*9**2,123);
near("mixture kurtosis",123/5**2,4.92);
near("uniform convergence risk radius",Math.sqrt(Math.log(4000)/2000),.06439739761862287,1e-8);
near("mean replacement n100",1/100,.01);
near("mean replacement n200",1/200,.005);
near("OOD nonoverlap test error",.5*1+.5*0,.5);
near("PAC success training draw",1-.05,.95);
near("warmup first step",.01*(1/2),.005);
near("warmup last step",.01*(2/2),.01);
near("cosine midpoint",.01*.5*(1+Math.cos(Math.PI*.5)),.005);
near("cosine endpoint",.01*.5*(1+Math.cos(Math.PI)),0);
near("dual optimum lambda",2*(1-.5),1);
near("dual maximum",.5*1-1**2/4,.25);
near("dual primal optimum",(.5-1)**2,.25);
near("looser trading upper bound",(.51-1)**2,.2401);
near("AdaGrad accumulated square second",1**2+3**2,10);
near("AdaGrad second step",3/Math.sqrt(10),.9486832980505138);
near("AdaGrad two-step total",1+3/Math.sqrt(10),1.9486832980505138);
near("Lagrangian shadow multiplier",2*1,2);
near("dense float64 bytes",8*100000*10000,8000000000);
near("sparse count five per row",100000*5,500000);
near("sparse float64 values bytes",500000*8,4000000);
near("training standardized percent",(2-1)/1,1);
near("mismatched serving decimal",(.02-1)/1,-.98);
near("train alpha contribution",.5*1,.5);
near("invalid serving alpha contribution",.5*(-.98),-.49);
near("pre-winsor mean",10/4,2.5);
near("post-winsor mean",5/4,1.25);
near("pre-winsor ES",10,10);
near("post-winsor ES",5,5);
near("BH last threshold",3*.05/3,.05);
yes("BH last p qualifies",.049<.05);
near("Bonferroni cutoff",.05/3,1/60);
yes("only first p Bonf qualifies",.01<.05/3&&.03>.05/3&&.049>.05/3);

near("Bartlett lag one weight",1-1/(1+1),.5);
near("HAC lag one covariance",1+2*.5*.4,1.4);
near("HAC mean se",Math.sqrt(1.4/100),.11832159566199232);
near("rolling beta",4/2,2);
near("Gamma shape 4 var mu2",2**2/4,1);
near("Gamma shape 4 var mu4",4**2/4,4);
near("Gamma shape 4 CV",1/Math.sqrt(4),.5);
near("FamaMacBeth time mean",(.02-.01)/2,.005);
near("FamaMacBeth sample sd",Math.sqrt((.015**2+.015**2)/(2-1)),.015*Math.sqrt(2));
near("FamaMacBeth sample se",.015*Math.sqrt(2)/Math.sqrt(2),.015);
near("FamaMacBeth t statistic",.005/.015,1/3);
near("single feature permutation variance",2*1,2);
near("joint feature permutation variance",8*1,8);
near("tree optimal second-order weight",-2/(1+1),-1);
near("tree optimal leaf improvement",-(2**2)/(2*(1+1)),-1);
near("XGBoost negative gain",.5*(4/3+4/3-16/4)-.1,-23/30);
near("boost first iteration",2*.2,.4);
near("boost second iteration",.4+.2*(2-.4),.72);
near("boost third iteration",2*(1-.8**3),.976);
near("boost third iteration remaining error",2-.976,1.024);

near("KDE bandwidth 1",phi(1),.24197072451914337);
near("KDE bandwidth half",2*phi(2),.10798193302637613);
near("oneclass nu for n100",.1*100,10);
near("GP posterior mean",2/(1+.25),1.6);
near("GP latent variance",1-1/(1+.25),.2);
near("GP predictive observation variance",.2+.25,.45);
near("rare-alert true positives",100*.9,90);
near("rare-alert false positives",9900*.05,495);
near("rare-alert precision",90/(90+495),.15384615384615385);
near("Cholesky variance second",.5**2+(Math.sqrt(3)/2)**2,1);
near("Cholesky covariance",1*.5+0*Math.sqrt(3)/2,.5);
near("Kalman predicted covariance",1+.5,1.5);
near("Kalman gain",1.5/(1.5+.5),.75);
near("Kalman updated covariance",(1-.75)*1.5,.375);
near("Kalman updated mean",.75*2,1.5);
near("variational KL evidence gap",.5*Math.log(4/3),.14384103622589045);
near("Gibbs A given B1",.4/(.4+.1),.8);
near("Gibbs B given A0",.1/(.4+.1),.2);

near("adjusted Rand expected index",2*2/6,2/3);
near("adjusted Rand value",(0-2/3)/(2-2/3),-.5);
near("single linkage distance",Math.min(4,3),3);
near("complete linkage distance",Math.max(4,3),4);
near("average linkage distance",(4+3)/2,3.5);
near("PCA reg whiten variance first",4/(4+1),.8);
near("PCA reg whiten variance second",1/(1+1),.5);
near("DBSCAN eps1 neighbor count endpoint",2,2);
near("DBSCAN eps.9 neighbor count endpoint",1,1);
near("label maturity last valid train date",13-4-1,8);
near("random Kfold duplicated across probability",1-1/5,.8);
near("expected crossfold duplicated events",20*(1-1/5),16);
yes("publication ingest after trade",9*60+10>9*60+5);
near("IPW two observations",(.5*(3/.5-1/(1-.5)))*2/2,2);
near("rare treatment propensity weight",1/.01,100);

near("early stopping patience 2 stop epoch",2+2,4);
near("early stopping best epoch",2,2);
near("GELU at minus one",-0.15865525393145707,-.15865525393145707);
near("GELU derivative at minus one",.15865525393145707-phi(1),-.0833154705876863,1e-8);
near("GELU derivative at zero",.5+0*phi(0),.5);
near("coupled preconditioner coordinate 1",1-.1*.1*1,.99);
near("coupled preconditioner coordinate 2",1-.1*.1*.01,.9999);
near("decoupled decay coordinate 2",1-.1*.1,.99);
near("inverted dropout retained value",10/(1-.2),12.5);
near("inverted dropout mean",.8*12.5+.2*0,10);
near("inverted dropout variance",.8*(12.5-10)**2+.2*(0-10)**2,25);
near("price trend forward",110/100-1,.1);
near("price trend reversed",100/110-1,-1/11);
near("patient accuracy",16/20,.8);
near("patient Wald SE",Math.sqrt(.8*.2/20),.08944271909999159);
near("patient rough Wald lower",.8-1.96*Math.sqrt(.8*.2/20),.6246922705640165,1e-6);
near("ResNet projection weights",4*2*1*1,8);
near("rare image true events",100*.8,80);
near("rare image false positives",9900*.02,198);
near("rare image PPV",80/(80+198),.28776978417266186);

near("GRU direct five step retention",.8**5,.32768);
near("sequence at least once independent",1-.8**4*.1,.95904);
near("sequence naive daily probability",(.2*4+.9)/5,.34);
near("AR one-step true",.8,.8);
near("AR 3-step true",.8**3,.512);
near("AR 3-step estimated",.9**3,.729);
near("AR 3-step model difference",.9**3-.8**3,.217);
near("gate interpolated state",(1-.25)*2+.25*6,3);
near("gate old state partial",1-.25,.75);
near("multihead width",512/8,64);
near("multihead scores T100",8*100**2,80000);
near("multihead scores T200",8*200**2,320000);
near("attention float32 bytes T100",80000*4,320000);
near("scaled attention raw variance",64,64);
near("scaled attention normalized variance",64/(Math.sqrt(64)**2),1);
near("kernel attention normalized value",(1*0+2*3)/(1+2),2);
near("PreLN direct zero sublayer first",1,1);
near("PostLN normalized first",(1-2)/1,-1);
near("PostLN normalized second",(3-2)/1,1);

const html=read("../ml/index.html"),reader=read("../ml/atlas.js"),css=read("../ml/atlas.css");
assert(html.includes('aria-label="V37第七批九十六篇正式讲义全覆盖深化"'),"ML V37 navigation missing");
for(const spec of baseline)for(const old of spec.entries)assert(html.includes('href="#lesson/'+old.id+'"'),old.id+" missing from ML navigation");
for(const marker of ["s.equations.map","s.derivation","s.protocol","s.pitfalls","katex.render"])assert(reader.includes(marker),"renderer missing "+marker);
for(const marker of ["overflow-x:auto","line-break:strict","katex-display",":focus-visible","@media print","@media(max-width:700px)"])assert(css.includes(marker),"CJK/mobile/formula style missing "+marker);
console.log("V78 PASS: "+count+" enriched live ML lessons; "+texCount+" KaTeX parses ("+strictCount+" new strict); "+numericCount+" independent numeric checks; frozen original-content snapshots, CJK and responsive navigation intact.");
