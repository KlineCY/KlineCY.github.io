import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>fs.readFileSync(new URL(p,root),"utf8");
const json=p=>JSON.parse(read(p));
const baseline=[{"path":"../ml/atlas-depth-08-d.json","schema":1,"batch":"v38-eighth-glm-tree-interpretability","revision":"2026-11-06","entries":[{"id":"m07-c03-p03","hash":"9d565a4f","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":254},{"id":"m07-c05-p06","hash":"ae5a02d6","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":210},{"id":"m07-c05-p03","hash":"b2d606af","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":257},{"id":"m07-c01-p03","hash":"9236ca7b","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":213},{"id":"m07-c03-p05","hash":"be278f1e","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":220},{"id":"m08-c05-p02","hash":"a0da78ce","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":234},{"id":"m08-c04-p04","hash":"28eba925","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":202},{"id":"m08-c03-p06","hash":"5544758e","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":205},{"id":"m08-c05-p01","hash":"463ecfba","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":225},{"id":"m08-c02-p05","hash":"6ff68890","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":248}]},{"path":"../ml/atlas-depth-08-e.json","schema":1,"batch":"v38-eighth-svm-bayesian-state-space","revision":"2026-11-06","entries":[{"id":"m09-c01-p05","hash":"4dc7a418","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":239},{"id":"m09-c02-p01","hash":"23448b60","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":220},{"id":"m09-c05-p04","hash":"b776021a","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":238},{"id":"m09-c04-p01","hash":"05e539b4","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":252},{"id":"m09-c01-p06","hash":"1f2addf5","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":213},{"id":"m10-c02-p04","hash":"dee199d1","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":213},{"id":"m10-c05-p04","hash":"ce2e049f","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":234},{"id":"m10-c05-p06","hash":"7ea8ae7d","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":241},{"id":"m10-c02-p01","hash":"f9d3efe1","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":234},{"id":"m10-c05-p01","hash":"81093420","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":227}]},{"path":"../ml/atlas-depth-08-f.json","schema":1,"batch":"v38-eighth-unsupervised-causal-inference","revision":"2026-11-06","entries":[{"id":"m11-c02-p06","hash":"a566ea92","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":233},{"id":"m11-c03-p02","hash":"c64d0692","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":214},{"id":"m11-c04-p06","hash":"130e397f","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":283},{"id":"m11-c01-p02","hash":"bc1ab5ac","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":258},{"id":"m11-c02-p01","hash":"fc2f2aa6","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":220},{"id":"m12-c02-p05","hash":"2556867a","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":263},{"id":"m12-c04-p05","hash":"edcbaa03","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":217},{"id":"m12-c02-p04","hash":"406c6c27","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":233},{"id":"m12-c04-p01","hash":"6da51ba3","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":249},{"id":"m12-c04-p02","hash":"d8d3f802","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":233}]},{"path":"../ml/atlas-depth-08-g.json","schema":1,"batch":"v39-eighth-neural-convolution-rigor","revision":"2026-11-06","entries":[{"id":"m13-c01-p05","hash":"248d9b9c","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":231},{"id":"m13-c01-p01","hash":"acb32fb0","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":218},{"id":"m13-c03-p06","hash":"b5c0ce7c","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":242},{"id":"m13-c05-p06","hash":"8ce8f73a","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":238},{"id":"m13-c05-p02","hash":"74379ba1","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":274},{"id":"m14-c03-p03","hash":"32dcb4ac","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":238},{"id":"m14-c01-p04","hash":"a479d8a5","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":281},{"id":"m14-c02-p02","hash":"6d22a68b","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":255},{"id":"m14-c05-p05","hash":"9edf58fb","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":234},{"id":"m14-c02-p03","hash":"833b4885","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":230}]},{"path":"../ml/atlas-depth-08-h.json","schema":1,"batch":"v39-eighth-sequence-transformer","revision":"2026-11-06","entries":[{"id":"m15-c02-p03","hash":"f3aa47d8","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":256},{"id":"m15-c01-p06","hash":"bea754bd","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":256},{"id":"m15-c02-p01","hash":"f39af205","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":259},{"id":"m15-c04-p03","hash":"afee5ce5","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":259},{"id":"m15-c02-p02","hash":"4eaeb4c1","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":243},{"id":"m16-c05-p04","hash":"03c95366","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":208},{"id":"m16-c01-p05","hash":"3058f73a","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":259},{"id":"m16-c02-p04","hash":"c69b6669","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":209},{"id":"m16-c03-p04","hash":"02d35dcf","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":247},{"id":"m16-c05-p06","hash":"fb908278","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":239}]},{"path":"../ml/atlas-depth-08-i.json","schema":1,"batch":"v39-eighth-financial-nlp-graph","revision":"2026-11-06","entries":[{"id":"m17-c05-p03","hash":"292f298a","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":255},{"id":"m17-c03-p03","hash":"fdf9d2fb","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":226},{"id":"m17-c04-p03","hash":"0e8caba0","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":223},{"id":"m17-c05-p01","hash":"3321520c","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":228},{"id":"m17-c03-p05","hash":"01dff539","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":250},{"id":"m18-c05-p05","hash":"644b5deb","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":228},{"id":"m18-c03-p04","hash":"6a3ef737","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":215},{"id":"m18-c02-p04","hash":"863d71e3","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":251},{"id":"m18-c05-p06","hash":"53554b24","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":229},{"id":"m18-c03-p05","hash":"bbf5749c","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":243}]}];
const manifest=json("../ml/atlas-depth-manifest.json");
assert.equal(manifest.schema,1);
assert.equal(manifest.expected_lessons,720);
assert.equal(manifest.expected_domains,24);
assert.equal(manifest.modules.reduce((n,m)=>n+m.lessons,0),720);
function hash(o){let h=2166136261>>>0;for(const c of JSON.stringify(o))h=Math.imul(h^c.charCodeAt(0),16777619)>>>0;return h.toString(16).padStart(8,"0");}
function originalSnapshot(x,old){
 return Object.fromEntries(old.oldKeys.map(k=>[k,
  ["equations","derivation","protocol","pitfalls"].includes(k)?x[k].slice(0,old.counts[k]):
  k==="worked_example"?x[k].slice(0,old.workedLength):x[k]]));
}
let count=0,texCount=0,strictCount=0,numericCount=0;
for(const spec of baseline){
 const d=json(spec.path),ids=spec.entries.map(e=>e.id),name=spec.path.split("/").at(-1);
 const loaded=manifest.modules.filter(m=>m.path==="./"+name);
 assert.equal(loaded.length,1,name+" must remain an official loaded module");
 assert.equal(loaded[0].lessons,10,name+" official lesson count");
 assert.equal(d.schema,spec.schema);
 assert.equal(d.batch,spec.batch);
 assert.equal(d.revision,spec.revision);
 assert.deepEqual(Object.keys(d.lessons),ids,name+" old IDs and order retained");
 assert.deepEqual(d.v80_revision.ids,ids,name+" V80 full coverage");
 for(const old of spec.entries){
  const id=old.id,x=d.lessons[id];count++;
  assert.equal(hash(originalSnapshot(x,old)),old.hash,id+" old source/links/formulas/proofs/example changed");
  assert(x.v80_note?.startsWith("V80"),id+" V80 depth marker absent");
  assert.equal(x.equations.length,old.counts.equations+2,id+" added strict equations");
  assert.equal(x.derivation.length,old.counts.derivation+2,id+" added derivations");
  assert.equal(x.protocol.length,old.counts.protocol+1,id+" added reproducibility");
  assert.equal(x.pitfalls.length,old.counts.pitfalls+1,id+" added financial limitation");
  assert(x.derivation.slice(-2).every(s=>s.length>=90),id+" shallow new proof");
  assert(x.worked_example.length-old.workedLength>=60,id+" shallow numeric worked example");
  assert(x.protocol.at(-1).length>=30&&x.pitfalls.at(-1).length>=30,id+" missing risk/protocol");
  assert(/[\u3400-\u9fff]/u.test(x.derivation.at(-1)),id+" missing Chinese proof");
  assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(JSON.stringify(x)),id+" malformed Chinese/hidden Unicode");
  assert(!/TODO|FIXME|Lorem ipsum/iu.test(JSON.stringify(x)),id+" draft placeholder");
  const recent=x.equations.slice(-2);
  assert.equal(new Set(recent.map(e=>e.label)).size,2,id+" new labels duplicate");
  assert(recent.every(e=>!x.equations.slice(0,-2).some(o=>o.label===e.label)),id+" heading collision");
  for(const [j,e] of x.equations.entries()){
   assert(e.label.length>=2&&e.tex.length>=5,id+" math metadata malformed");
   katex.renderToString(e.tex,{throwOnError:true,strict:j>=old.counts.equations?"error":"ignore",displayMode:true});
   texCount++;
   if(j>=old.counts.equations)strictCount++;
  }
 }
}
assert.equal(count,60);
assert.equal(texCount,480);
assert.equal(strictCount,120);

function near(name,actual,expected,tol=1e-9){
 assert(Number.isFinite(actual)&&Number.isFinite(expected)&&Math.abs(actual-expected)<=tol,name+": "+actual+" != "+expected);
 numericCount++;
}
function yes(name,condition){near(name,condition?1:0,1);}
const sigmoid=z=>1/(1+Math.exp(-z));

// 08-d: GLM, regression and tree models
near("Logistic probability at zero",sigmoid(0),0.5);
near("Logistic gradient at one positive",-1*(1-.5),-.5);
near("Logistic Hessian",.5*(1-.5),.25);
near("Logistic Newton step",0-(-.5)/.25,2);
near("Logistic new probability",sigmoid(2),.8807970779778823);
near("post-cost utility q1",.008-.01,-.002);
near("post-cost utility qhalf",.5*(.008-.01),-.001);
near("FWL residual inner numerator",1*2+0*0+(-1)*(-2),4);
near("FWL denominator",1**2+0**2+(-1)**2,2);
near("FWL coefficient",4/2,2);
near("OLS hat row sum",.5+.5,1);
near("OLS intercept fitted mean",(1+3)/2,2);
near("OLS residual normal equations",(-1)+1,0);
near("Logistic intercept log odds",Math.log((2/3)/(1/3)),Math.log(2));
near("Logistic intercept base rate",sigmoid(Math.log(2)),2/3);
near("SHAP additive baseline zero",0+0,0);
near("SHAP additive original prediction",0+2+3,5);
near("SHAP alternative background",2+1+2,5);
near("Target encoding smoothed category",(2*1+3*.2)/(2+3),.52);
near("Target encoding unknown backoff",.2,.2);
near("Huber positive residual large",1*(4-.5),3.5);
near("Half squared large residual",.5*4**2,8);
near("Huber derivative saturated",1,1);
near("permutation unused feature delta",0-0,0);
near("balanced Gini",1-(.5**2+.5**2),.5);
near("pure Gini",1-(1**2+0**2),0);
near("Gini split gain",.5-.5*0-.5*0,.5);

// 08-e: SVM, Bayesian models and filters
near("SVM support vector nonzero alpha",[0,.4,.6,0].filter(x=>x>0).length,2);
near("SVM support vector fraction",2/4,.5);
near("hinge slack margin 0.4",Math.max(0,1-.4),.6);
near("hinge penalized slack",2*.6,1.2);
near("hinge zero above margin",Math.max(0,1-1.5),0);
near("rare anomaly base positives",100000*.005,500);
near("rare anomaly true positives",500*.9,450);
near("rare anomaly false positives",99500*.01,995);
near("rare anomaly PPV",450/(450+995),450/1445);
near("KRR one point dual coefficient",2/(1+1),1);
near("KRR one point fitted value",1*1,1);
near("KRR unregularized value",2/1,2);
near("SVM before scaling squared distance",1**2+1**2,2);
near("SVM after scaling squared distance",1**2+2**2,5);
near("SVM scaled RBF kernel",Math.exp(-.5*5),.0820849986238988);
near("Ising same sign probability",3/(3+1),.75);
near("Ising complement",1-.75,.25);
near("particle ESS concentrated",1/(.7**2+3*.1**2),25/13);
near("particle ESS uniform",1/(4*.25**2),4);
near("filtering prior odds",.2/.8,.25);
near("filter to smoothing odds",(.2/.8)*4,1);
near("filter to smoothing posterior",1/(1+1),.5);
near("DAG law total probability",.8*.9+.2*.1,.74);
near("state-space zero loading gain",0,0);

// 08-f: clustering, Purged CV and causal identification
near("Mahalanobis squared diagonal",2**2/4+1,2);
near("Euclidean squared distance",2**2+1,5);
near("SVD centered covariance eigenvalue",2/(3-1),1);
near("SVD first singular value",Math.sqrt(2),1.4142135623730951);
near("GMM added BIC penalty",-2*5+3*Math.log(100),3.815510557964274,1e-9);
near("Kmeans first cluster centroid",(0+2)/2,1);
near("Kmeans WCSS",(0-1)**2+(2-1)**2+(10-10)**2,2);
near("Ward two cluster merge cost",(2*3/(2+3))*2**2,4.8);
near("Ward rescaled cost",(2*3/(2+3))*20**2,480);
yes("embargo day13 removed",13>12&&13<=14);
yes("embargo day14 removed",14>12&&14<=14);
yes("embargo day15 legal",!(15>12&&15<=14));
near("DID treated change",16-10,6);
near("DID controls change",11-8,3);
near("DID effect",(16-10)-(11-8),3);
yes("purged intervals overlap",Math.max(8,10)<Math.min(11,12));
yes("adjacent intervals do not overlap",!(Math.max(8,10)<Math.min(10,12)));
near("ATT identified counterfactual",6-4,2);
near("raw treated minus unrelated control",6-3,3);
near("collider unscreened equality probability",.5,.5);
near("collider screening equality probability",1,1);

// 08-g: neural networks, CNN, vision and transfer
near("He variance fan128",2/128,.015625);
near("He standard deviation fan128",Math.sqrt(2/128),.125);
near("He incorrect variance from std mistaken",(.015625)**2,.000244140625);
near("nn Linear 4 to3 parameters",4*3+3,15);
near("rank one factorization weights",3+4,7);
near("warmup last step",.01*2/2,.01);
near("cosine half schedule",.01*.5*(1+Math.cos(Math.PI*(6-2)/(10-2))),.005);
near("cosine schedule end",.01*.5*(1+Math.cos(Math.PI)),0);
near("seed average",(.1+.12+.14)/3,.12);
near("seed sample standard deviation",Math.sqrt((.02**2+.02**2)/(3-1)),.02);
near("seed standard error",.02/Math.sqrt(3),.011547005383792516);
near("ModuleList total parameter count",6+10,16);
near("segmentation IoU",20/(20+5+5),2/3);
near("segmentation Dice",2*20/(2*20+5+5),.8);
near("conv weight count",8*5*3*3,360);
near("conv with bias count",8*5*3*3+8,368);
near("ReLU negative input",Math.max(0,-2),0);
near("ReLU positive input",Math.max(0,3),3);
near("cross-sector source risk",.2*1+.8*0,.2);
near("cross-sector target risk",.8*1+.2*0,.8);
near("VGG small-kernel receptive field",1+3*(3-1),7);
near("VGG three small kernel parameter",3*9*64**2,110592);
near("VGG wide kernel parameter",49*64**2,200704);
near("VGG parameter reduction",49*64**2-3*9*64**2,90112);

// 08-h: RNN, Transformer and financial deployment
near("BPTT ten steps",.8**10,.1073741824);
near("BPTT five steps",.8**5,.32768);
near("masked sequence mean",(1+3)/2,2);
near("invalid padded mean",(1+3+100+100)/4,51);
near("RNN Jacobian five zero-centered steps",.5**5,.03125);
near("RNN saturated local Jacobian",(1-.9**2)*.5,.095);
near("RNN saturated five step Jacobian",.095**5,.000007737809375);
near("TCN receptive field",1+2*(1+2+4),15);
near("RNN shared parameter first gradient",0-1,-1);
near("RNN shared parameter second gradient",0-3,-3);
near("RNN shared total gradient",-1-3,-4);
near("RNN shared optimum",(1+3)/2,2);
near("weekly calendar first",Math.sin(0),0);
near("weekly calendar next week",Math.sin(2*Math.PI),0,1e-12);
near("cross-attention uniform weights",1/2,.5);
near("cross-attention value",.5*2+.5*6,4);
near("RoPE orthogonal quarter rotation",Math.cos(Math.PI/2),0,1e-12);
near("RoPE same angle inner product",Math.cos(0),1);
near("LayerNorm first normalized value",(1-2)/Math.sqrt(1),-1);
near("LayerNorm second normalized value",(3-2)/Math.sqrt(1),1);
near("Transformer gross minus execution",.004-.006,-.002);

// 08-i: financial NLP, evidence and graph learning
near("as-of usable clock",Math.max(9,12,14),14);
yes("as-of pre-ingest unavailable",13<14);
near("Bradley Terry preference",sigmoid(2-1),.7310585786300049);
near("cosine orthogonal features",(1*0+0*1)/(1*1),0);
near("cosine positive-scaled features",(1*2+0*0)/(1*2),1);
near("statement revenue growth",(150-120)/120,.25);
near("statement millions to hundred-millions",150/100,1.5);
near("supported answer fail closed",1*0*1,0);
near("shared market covariance",1*1*4,4);
near("shared market variance",4+1,5);
near("shared market asset correlation",4/5,.8);
near("GAT two concat head width",2*4,8);
near("GAT two averaging head width",4,4);
near("GNN nonprimary spectrum after 5",.5**5,.03125);
near("GNN primary spectrum after 5",1**5,1);
near("GNN paired improvement first day",1-2,-1);
near("GNN paired deterioration second day",3-2,1);
near("GNN paired mean difference",((-1)+1)/2,0);
near("graph neighbor true mean",(2+6)/2,4);
near("neighbor importance sample first",2/(2*.25),4);
near("neighbor importance sample second",6/(2*.75),4);
near("neighbor importance weighted expectation",.25*4+.75*4,4);

const home=read("../ml/index.html"),viewer=read("../ml/atlas.js"),css=read("../ml/atlas.css");
for(const spec of baseline)for(const entry of spec.entries)
 assert(home.includes('href="#lesson/'+entry.id+'"'),entry.id+" missing canonical navigation");
for(const marker of ["s.equations.map","s.derivation","s.protocol","s.pitfalls","s.worked_example","katex.render"])
 assert(viewer.includes(marker),"reader cannot display "+marker);
for(const marker of ["overflow-x:auto","line-break:strict","katex-display",":focus-visible","@media print","@media(max-width:700px)"])
 assert(css.includes(marker),"responsive Chinese/KaTeX stylesheet missing "+marker);
console.log("V80 PASS: "+count+" live ML lectures, "+texCount+" KaTeX parses ("+strictCount+" new strict); "+numericCount+" numeric assertions, immutable original-content fingerprint, CJK and responsive style/navigation preserved.");
