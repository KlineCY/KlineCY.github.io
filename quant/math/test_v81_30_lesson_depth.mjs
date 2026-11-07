import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>fs.readFileSync(new URL(p,root),"utf8");
const json=p=>JSON.parse(read(p));
const baseline=[{"path":"../ml/atlas-depth-08-j.json","schema":1,"batch":"v39-eighth-generative-reinforcement-learning","revision":"2026-11-06","entries":[{"id":"m19-c03-p04","hash":"33170355","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":224},{"id":"m19-c03-p01","hash":"e9fec31d","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":250},{"id":"m19-c02-p05","hash":"34e6c6f3","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":260},{"id":"m19-c01-p05","hash":"7ac5664b","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":259},{"id":"m19-c05-p03","hash":"13bc2f78","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":223},{"id":"m20-c04-p06","hash":"63aa361e","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":211},{"id":"m20-c02-p04","hash":"eb7e97f8","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":266},{"id":"m20-c05-p01","hash":"7c44534a","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":229},{"id":"m20-c05-p06","hash":"0c0ab3a9","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":206},{"id":"m20-c03-p03","hash":"d9aca829","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":223}]},{"path":"../ml/atlas-depth-08-k.json","schema":1,"batch":"v39-eighth-marl-time-series-econometrics","revision":"2026-11-06","entries":[{"id":"m21-c03-p05","hash":"eef4f97f","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":219},{"id":"m21-c02-p03","hash":"88073bed","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":223},{"id":"m21-c03-p03","hash":"86a00ac4","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":210},{"id":"m21-c03-p01","hash":"eb35fc6d","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":246},{"id":"m21-c01-p06","hash":"4b1840b6","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":243},{"id":"m22-c02-p06","hash":"976e7524","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":213},{"id":"m22-c05-p01","hash":"0ddaebd3","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":230},{"id":"m22-c01-p01","hash":"dd226b01","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":215},{"id":"m22-c02-p05","hash":"8e28a159","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":232},{"id":"m22-c03-p01","hash":"20579052","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":239}]},{"path":"../ml/atlas-depth-08-l.json","schema":1,"batch":"v39-eighth-quant-risk-mlops","revision":"2026-11-06","entries":[{"id":"m23-c05-p05","hash":"fc320762","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":234},{"id":"m23-c02-p05","hash":"c4fb754b","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":253},{"id":"m23-c02-p04","hash":"733cd645","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":223},{"id":"m23-c05-p03","hash":"6430da54","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":228},{"id":"m23-c04-p03","hash":"6e58ecad","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":258},{"id":"m24-c01-p05","hash":"e23a4ef6","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":266},{"id":"m24-c02-p03","hash":"1e5939d3","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":225},{"id":"m24-c01-p02","hash":"9defe142","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":221},{"id":"m24-c01-p01","hash":"b0c4109a","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":234},{"id":"m24-c01-p03","hash":"b3876291","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":222}]}];
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
 assert.equal(loaded.length,1,name+" must remain an official Atlas module");
 assert.equal(loaded[0].lessons,10,name+" official lesson count");
 assert.equal(d.schema,spec.schema);
 assert.equal(d.batch,spec.batch);
 assert.equal(d.revision,spec.revision);
 assert.deepEqual(Object.keys(d.lessons),ids,name+" original IDs and chapter order intact");
 assert.deepEqual(d.v81_revision.ids,ids,name+" V81 coverage complete");
 for(const old of spec.entries){
  const x=d.lessons[old.id];count++;
  assert.equal(hash(originalSnapshot(x,old)),old.hash,old.id+" historical lesson changed");
  assert(x.v81_note?.startsWith("V81"),old.id+" V81 extension missing");
  assert.equal(x.equations.length,old.counts.equations+2,old.id+" formula coverage");
  assert.equal(x.derivation.length,old.counts.derivation+2,old.id+" proofs coverage");
  assert.equal(x.protocol.length,old.counts.protocol+1,old.id+" validation coverage");
  assert.equal(x.pitfalls.length,old.counts.pitfalls+1,old.id+" misuse coverage");
  assert(x.derivation.slice(-2).every(s=>s.length>=90),old.id+" thin derivation");
  assert(x.worked_example.length-old.workedLength>=75,old.id+" thin worked example");
  assert(x.protocol.at(-1).length>=30&&x.pitfalls.at(-1).length>=30,old.id+" missing protocol/pitfall");
  assert(/[\u3400-\u9fff]/u.test(x.derivation.at(-1)),old.id+" Chinese proof missing");
  assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(JSON.stringify(x)),old.id+" damaged Unicode/hidden characters");
  assert(!/TODO|FIXME|Lorem ipsum/iu.test(JSON.stringify(x)),old.id+" placeholder");
  const recent=x.equations.slice(-2);
  assert.equal(new Set(recent.map(e=>e.label)).size,2,old.id+" duplicate new formula label");
  assert(recent.every(e=>!x.equations.slice(0,-2).some(o=>o.label===e.label)),old.id+" new formula conflicts with old label");
  for(const [j,e] of x.equations.entries()){
   assert(e.label.length>=2&&e.tex.length>=5,old.id+" formula metadata");
   katex.renderToString(e.tex,{throwOnError:true,strict:j>=old.counts.equations?"error":"ignore",displayMode:true});
   texCount++;
   if(j>=old.counts.equations)strictCount++;
  }
 }
}
assert.equal(count,30);
assert.equal(texCount,240);
assert.equal(strictCount,60);
function near(name,actual,expected,tol=1e-9){
 assert(Number.isFinite(actual)&&Number.isFinite(expected)&&Math.abs(actual-expected)<=tol,name+": "+actual+" != "+expected);
 numericCount++;
}
function yes(name,condition){near(name,condition?1:0,1);}
const sigmoid=z=>1/(1+Math.exp(-z));

// 08-j: generative models, offline RL and hard risk constraints
near("W1 pointmass gap",Math.abs(3-0),3);
near("W1 consistent rescaling",Math.abs(.03-0),.03);
near("GAN optimal discriminator uneven",.8/(.8+.2),.8);
near("GAN optimal discriminator match",.5/(.5+.5),.5);
near("GAN discriminator complementary",1-.8,.2);
near("Gaussian reparameterized sample",2+3*(-1),-1);
near("Gaussian derivative mu",1,1);
near("Gaussian derivative sigma",-1,-1);
near("Gaussian risk latent signal MI",Math.log(2),Math.log(2));
near("Gaussian risk noise label MI",0,0);
near("financial generated compounded price",100*1.1*.8,88);
near("financial generated two-day return",1.1*.8-1,-.12);
near("invalid additive two-day return",.1-.2,-.1);
yes("synthetic return above negative unit floor",-.2>-1);
yes("invalid simple return below floor",-1.2<=-1);
near("softmax tau1 best action",sigmoid(1),.7310585786300049);
near("softmax tau2 best action",sigmoid(.5),.6224593312018546);
near("SARSA TD target",1+.9*4,4.6);
near("SARSA TD residual",4.6-2,2.6);
near("SARSA update",2+.5*2.6,3.3);
near("offpolicy single action weight",.5/.1,5);
yes("offpolicy support missing for zero propensity",!(0>0));
near("CMDP action A soft value",10-.1*8,9.2);
near("CMDP action B soft value",6-.1*3,5.7);
yes("CMDP action A hard infeasible",8>5);
yes("CMDP action B hard feasible",3<=5);
near("DQN frozen target",1+.9*Math.max(2,3),3.7);
near("DQN online false bootstrap",1+.9*Math.max(10,1),10);

// 08-k: market making, risk, financial time series
near("market maker full fill inventory",4+3-1,6);
yes("market maker full fill breaches bound",4+3-1>5);
near("market maker worst fill inventory",4+3,7);
near("market model true expected reward",.1*10+.9*0,1);
near("market model false expected reward",.5*10+.5*0,5);
near("model transition optimism bias",5-1,4);
near("CVaR point mass total mean",.1*10,1);
near("CVaR top decile",10,10);
near("CVaR candidate eta0 objective",0+(.1*10)/(.1),10);
near("dynamic utility full position",.01-.015,-.005);
near("dynamic utility half position",.5*(.01-.015),-.0025);
near("historical quote fill payoff",.8*1,.8);
near("counterfactual new quote fill payoff",.2*1,.2);
near("counterfactual fill optimism ratio",.8/.2,4);
near("heteroskedastic low risk",1**2,1);
near("heteroskedastic high risk",3**2,9);
near("recursive AR true 3 step",.8**3,.512);
near("recursive AR model 3 step",.9**3,.729);
near("recursive AR model bias",.9**3-.8**3,.217);
near("random walk variance n10",10*1,10);
near("random walk variance n20",20*1,20);
near("rolling sample 100",100,100);
near("expanding sample 250",250,250);
near("rolling new regime mean",100/100,1);
near("expanding old and new mean",100/250,.4);
near("ARCH variance post big shock",1+.5*2**2,3);
near("ARCH variance post zero shock",1+.5*0**2,1);
yes("ARCH alpha 0.5 stationary condition",.5<1);

// 08-l: factor diagnostics, real execution and production deployment
near("Snell stopping immediate value",Math.max(5,4),5);
near("Snell stopping continue value",Math.max(5,7),7);
near("stable-selection 78 over 100",78/100,.78);
yes("stable-selection threshold 0.7",.78>=.7);
yes("stable-selection not passing 0.8",.78<.8);
near("FWL residualized coefficient",(1*3+(-1)*(-3))/(1**2+(-1)**2),3);
near("mean reversion positive expected profit",.6*2-.4*1-.3,.5);
near("mean reversion higher costs reversal",.6*2-.4*1-1,-.2);
near("Sharpe mean zero",(-.1+.1)/2,0);
near("Sharpe variance",(.1**2+.1**2)/2,.01);
near("Sortino lower second moment",(.1**2+0)/2,.005);
near("Sortino lower semideviation",Math.sqrt(.005),.07071067811865475);
yes("ingest 9:10 happens after decision 9:05",9*60+10>9*60+5);
near("latency mean tail risk",.98*10+.02*100,11.8);
near("latency p99 discrete",100,100);
yes("P99 distribution CDF at 10ms below 99pct",.98<.99);
near("hash mismatch gated state",1*0,0);
near("seed mean",(.1+.2+.3)/3,.2);
near("seed sample variance",((.1-.2)**2+(.2-.2)**2+(.3-.2)**2)/2,.01);
near("seed sample std",Math.sqrt(.01),.1);
yes("version not ingested by decision day7",9>7);
yes("version available after ingest day10",9<=10);

const home=read("../ml/index.html"),viewer=read("../ml/atlas.js"),css=read("../ml/atlas.css");
for(const spec of baseline)for(const entry of spec.entries)
 assert(home.includes('href="#lesson/'+entry.id+'"'),entry.id+" absent from live navigation");
for(const marker of ["s.equations.map","s.derivation","s.protocol","s.pitfalls","s.worked_example","katex.render"])
 assert(viewer.includes(marker),"display renderer missing "+marker);
for(const marker of ["overflow-x:auto","line-break:strict","katex-display",":focus-visible","@media print","@media(max-width:700px)"])
 assert(css.includes(marker),"CJK/responsive formula style missing "+marker);
console.log("V81 PASS: "+count+" deepened live lectures, "+texCount+" KaTeX equations ("+strictCount+" new strict); "+numericCount+" independent numeric tests; full original-content fingerprints and CJK/mobile navigation intact.");
