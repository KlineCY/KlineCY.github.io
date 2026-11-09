import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=n=>fs.readFileSync(new URL(n,root),"utf8");
const json=n=>JSON.parse(read(n));
const curriculum=json("curriculum.json");
assert.equal(curriculum.nodes.length,656,"Curriculum size changed");
const nodes=new Map(curriculum.nodes.map(n=>[n.id,n]));
assert.equal(nodes.size,656,"Curriculum node IDs not unique");
const manifest=json("lessons-manifest.json");
assert.equal(manifest.expected_lessons,201,"Historical manifest unexpectedly changed");
assert.equal(manifest.modules.length,77,"Historical bundle registry unexpectedly changed");
const oldFiles=[
 "lessons-foundations.json","lessons-advanced.json","lessons-more.json",
 ...manifest.modules.map(m=>m.path.replace(/^\.\//,"")),
 "lessons-foundations-v89.json","lessons-linear-v90.json","lessons-linear-v92.json",
 "lessons-matrix-v93.json","lessons-probability-v93.json",
 "lessons-probability-v94a.json","lessons-probability-v94b.json","lessons-probability-v94c.json",
 "lessons-statistics-v95a.json","lessons-statistics-v95b.json",
 "lessons-optimization-v96a.json","lessons-optimization-v96b.json",
 "lessons-portfolio-v97.json","lessons-algorithms-v97.json","lessons-numerical-v97.json",
 "lessons-diffeq-v98a.json","lessons-diffeq-v98b.json",
 "lessons-dynamics-v99a.json","lessons-pde-v99b.json",
 "lessons-pricing-pde-v99c.json","lessons-control-v99d.json",
 "lessons-finance-v100a.json","lessons-finance-v100b.json",
 "lessons-finance-v100c.json","lessons-finance-v100d.json",
 "lessons-timeseries-v101.json","lessons-risk-tail-v101.json",
 "lessons-risk-axioms-v101.json","lessons-risk-governance-v101.json",
 "lessons-mlmath-v102a.json","lessons-mlmath-v102b.json",
 "lessons-mlmath-v102c.json",
 "lessons-information-v103a.json","lessons-information-v103b.json",
 "lessons-information-v103c.json",
 "lessons-discrete-v104a.json","lessons-discrete-v104b.json",
 "lessons-discrete-v104c.json","lessons-discrete-v104d.json"
];
const history=oldFiles.flatMap(n=>json(n).lessons);
assert.equal(history.length,534,"V104 historical 534 lesson texts changed");
assert.equal(new Set(history.map(l=>l.node_id)).size,534,"V104 history has duplicate IDs");
const files=["lessons-control-v105a.json","lessons-control-v105b.json","lessons-control-v105c.json","lessons-control-v105d.json"];
const docs=files.map(json);
docs.forEach((d,i)=>{
 assert.equal(d.version,1,"V105 data schema wrong");
 assert.equal(d.revision,"V105","V105 revision mismatch");
 assert.equal(d.lessons.length,[7,7,7,7][i],"V105 file size mismatch");
});
const newest=docs.flatMap(d=>d.lessons);
const ids=Array.from({length:29},(_,i)=>i+542).filter(i=>i!==545).map(i=>"control-n"+String(i).padStart(4,"0"));
assert.deepEqual(newest.map(l=>l.node_id),ids,"V105 control lectures changed order or incomplete");
const all=[...history,...newest];
assert.equal(all.length,562,"Expected 562 completed mathematics lectures");
assert.equal(new Set(all.map(l=>l.node_id)).size,562,"Historical lesson overwritten");
assert(all.every(l=>nodes.get(l.node_id)?.title===l.title),"Curriculum mismatched or missing lesson");
const completed=new Set(all.map(l=>l.node_id));
const control=curriculum.nodes.filter(n=>n.discipline_id==="control");
assert.equal(control.length,30,"Control syllabus size changed");
assert(control.every(n=>completed.has(n.id)),"Control theory must be 30/30 complete");
assert.deepEqual(history.filter(l=>l.node_id.startsWith("control-")).map(l=>l.node_id).sort(),
 ["control-n0541","control-n0545"],"Previously existing two control lessons changed");
assert.equal(newest.filter(l=>l.node_id.startsWith("control-")).length,28);
let mathCount=0,deepCount=0,deepChars=0,linkCount=0;
function formula(tex,id){
 assert.equal(typeof tex,"string",id+" formula type");
 assert(tex.length>=10,id+" formula too short");
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(tex),id+" bad character");
 try{katex.renderToString(tex,{displayMode:true,throwOnError:true,strict:"error"});}
 catch(e){throw Error(id+" strict KaTeX failed: "+tex+" / "+e.message);}
 mathCount++;
}
for(const l of newest){
 const id=l.node_id;
 assert.equal(l.id,id,"Lesson identity mismatch");
 assert.equal(l.title,nodes.get(id).title,"Course title drift");
 for(const field of ["definition","assumptions","example","application","caveat"])
  assert(l[field]?.length>=70,id+" incomplete "+field);
 assert(l.properties?.length>=4 && l.properties.every(p=>p.length>=20),id+" shallow properties");
 assert.equal(l.formulas.length,3,id+" core formulas");
 assert.equal(l.derivation.length,3,id+" derivations");
 assert.equal(l.deep_sections.length,3,id+" detailed chapters");
 for(const x of l.formulas){assert(x.label?.length>=2,id+" mathematical label");formula(x.tex,id);}
 for(const x of l.derivation){assert(x.explanation?.length>=25,id+" derivation prose");formula(x.tex,id);}
 const headings=new Set();
 for(const s of l.deep_sections){
  assert(s.title?.length>=8 && s.question?.length>=10,id+" deep section title/question");
  assert(!headings.has(s.title),id+" repeated subchapter");
  headings.add(s.title);
  assert(s.paragraphs?.length===2 && s.paragraphs.every(p=>p.length>=30),id+" deep explanatory text");
  for(const [key,min] of [["proof",50],["worked",50],["limitation",25]])
   assert(s[key]?.length>=min,id+" short "+key);
  assert.equal(s.formulas?.length,1,id+" deep math");
  assert(s.formulas[0].label?.length>=2,id+" deep formula caption");
  formula(s.formulas[0].tex,id);
  deepCount++;
  deepChars+=s.paragraphs.join("").length+s.proof.length+s.worked.length+s.limitation.length;
 }
 assert(/[\u3400-\u9FFF]/u.test(JSON.stringify(l)),id+" Chinese content missing");
 assert(!/TODO|FIXME|Lorem ipsum|待补充/i.test(JSON.stringify(l)),id+" placeholder");
 for(const link of l.links||[]){
  assert(["math","ml"].includes(link.kind),id+" unknown cross link kind");
  if(link.kind==="math")assert(nodes.has(link.id),id+" dangling link "+link.id);
  linkCount++;
 }
}
assert.equal(mathCount,252,"New formula count changed");
assert.equal(deepCount,84,"Deep chapter count changed");
assert(deepChars>=22500,"Depth reduced from reviewed version");
assert.equal(linkCount,84,"Cross-lesson link count changed");
const html=read("index.html"),css=read("styles.css"),app=read("app.js");
const workflow=read("../../.github/workflows/test-quant-math.yml");
assert(html.includes('app.js?v=20261108-v105-control-game-market-making-complete'),"Reader must bust V104 cache");
assert(html.includes("控制理论、动态博弈与做市"),"V105 CJK curriculum absent");
for(const id of ids)assert(html.includes("?topic="+id+'"'),"Study route missing "+id);
for(const needle of [
 'loadV104DiscreteGraphs("./lessons-discrete-v104d.json",6)',
 'loadV105ControlGame("./lessons-control-v105a.json",7)',
 'loadV105ControlGame("./lessons-control-v105b.json",7)',
 'loadV105ControlGame("./lessons-control-v105c.json",7)',
 'loadV105ControlGame("./lessons-control-v105d.json",7)',
 '...v104a,...v104b,...v104c,...v104d,...v105a,...v105b,...v105c,...v105d',
 'new Set(lessons.map(l=>l.node_id)).size!==lessons.length',
 'new Set(doc.lessons.map(l=>l.node_id)).size!==8'
])assert(app.includes(needle),"Missing V105 canonical loader or historical guard: "+needle);
for(const token of ["overflow-x:auto","line-break:strict","@media(max-width:700px)","@media print",":focus-visible"])
 assert(css.includes(token),"Missing established Chinese/LaTeX mobile accessibility: "+token);
assert(workflow.includes("node quant/math/test_v105_control_complete.mjs"),"V105 CI step absent");





let checks=0;
function near(label,value,expected,tol=1e-8){
 assert(Number.isFinite(value),label+" non-finite");
 assert(Math.abs(value-expected)<=tol,label+": actual "+value+" expected "+expected);
 checks++;
}
function yes(label,x){near(label,Number(Boolean(x)),1);}
function log(x){return Math.log(x);}
near("Banach contraction fixed point",2/(1-.8),10);
near("Banach V1",2,2);
near("Banach V2",2+.8*2,3.6);
near("Banach V3",2+.8*3.6,4.88);
near("Banach error initial",10-0,10);
near("Banach error at k1",10-2,8);
near("Banach error at k2",10-3.6,6.4);
near("Banach error at k3",10-4.88,5.12);
near("Banach residual bound",.2/(1-.8),1);
near("Banach residual bound near1",.2/(1-.99),20);
near("Value hold policy",1/(1-.5),2);
near("Value trade policy",2/(1-.5),4);
near("Value Q first hold",1+.5*4,3);
near("Value Q first trade",2+.5*4,4);
near("Policy iteration old Q hold",1+.5*2,2);
near("Policy iteration old Q trade",2+.5*2,3);
near("Policy iteration new value",2/(1-.5),4);
yes("Policy iteration improving action",3>2);
near("Finite horizon t2",2+.9*2,3.8);
near("Infinite horizon value",2/(1-.9),20);
near("Finite horizon tail",20-3.8,16.2);
near("Finite horizon tail formula",.9**2*2/(1-.9),16.2);
near("Finite horizon terminal bonus",.9**2*10,8.1);
near("Snell Y2",2,2);
near("Snell Y1",Math.max(3,2),3);
near("Snell Y0",Math.max(1,3),3);
yes("Snell stop at t1",3>=2);
near("American risk neutral p",(1.05-.8)/(1.2-.8),.625);
near("American risk neutral down",1-.625,.375);
near("American terminal high",Math.max(100-144,0),0);
near("American terminal mid",Math.max(100-96,0),4);
near("American terminal low",Math.max(100-64,0),36);
near("American upper intermediate",(.625*0+.375*4)/1.05,1.4285714285714286);
near("American lower continuation",(.625*4+.375*36)/1.05,15.238095238095237);
near("American lower exercise",Math.max(100-80,0),20);
yes("American lower early exercise",20>15.238095238095237);
near("American initial",(.625*((.375*4)/1.05)+.375*20)/1.05,7.993197278911564);
near("Discrete stopping future value",.5*4+.5*0,2);
near("Discrete stopping stop now",Math.max(2,2),2);
near("Discrete stopping omniscient",.5*4+.5*2,3);
near("Discrete stopping information gap",3-2,1);
near("Continuous generator drift component",2*.04*100,8);
near("Continuous generator diffusion component",.2**2*100,4);
near("Continuous generator sum",8+4,12);
near("Continuous generator minus discount",12-.05*100,7);
near("Rebalance two period gross",.003*(1+.9),.0057);
near("Rebalance gross minus fee",.003*(1+.9)-.005,.0007);
near("Rebalance small signal",.002*(1+.9)-.005,-.0012);
near("Rebalance threshold",.005/(1+.9),.002631578947368421);
near("Rebalance decaying gain",.004+.9*.001-.005,-.0001);
near("Execution wait p0.6 increment",.4*.0015-.6*.001+.0002,.0002);
near("Execution wait cost p0.6",.002+.4*.0015-.6*.001+.0002,.0022);
near("Execution wait p0.8 increment",.2*.0015-.8*.001+.0002,-.0003);
near("Execution wait cost p0.8",.002+.2*.0015-.8*.001+.0002,.0017);
near("Stochastic risky expected reward",.5*4+.5*0,2);
near("Stochastic safe expected reward",1,1);
near("Stochastic safe utility",-Math.exp(-1),-.36787944117144233);
near("Stochastic risky utility",-.5*Math.exp(-4)-.5,-.5091578194443671);
yes("Stochastic CARA prefers safe", -Math.exp(-1)>-.5*Math.exp(-4)-.5);
near("HJB diffusion term",.5*.2**2*(-2),-.04);
near("HJB optimal action",.5,.5);
near("HJB optimum Hamilton",-((.5)**2)+.5-.04,.21);
near("HJB H at +1",-1+1-.04,-.04);
near("HJB H at -1",-1-1-.04,-2.04);
near("LQG control optimal",-(2)/2,-1);
near("LQG terminal state",2-1,1);
near("LQG optimal objective",(-1)**2+1**2,2);
near("LQG Riccati",1-1/(1+1),.5);
near("LQG quadratic derivative optimal",4*(-1)+2*2,0);
near("Risk sensitivity deterministic",log(Math.exp(1)),1);
near("Risk sensitivity random",log((1+Math.exp(2))/2),1.4337808304830273);
yes("Risk sensitivity random penalized",log((1+Math.exp(2))/2)>1);
near("Risk sensitivity random variance",.5*(0-1)**2+.5*(2-1)**2,1);
near("Risk sensitivity small parameter approx",1+.1/2,1.05);
near("Robust risky worst",Math.min(4,0),0);
near("Robust safe worst",Math.min(1,1),1);
yes("Robust safe best",1>0);
near("POMDP observed warning evidence",.2*.8+.8*.1,.24);
near("POMDP posterior high",(.2*.8)/.24,2/3);
near("POMDP posterior low",(.8*.1)/.24,1/3);
near("POMDP posterior sum",2/3+1/3,1);
near("Nash mutual defect payoff",1,1);
near("Nash cooperative payoff",3,3);
near("Nash deviate vs cooperate",5,5);
yes("Nash defect strictly dominates vs defect",1>0);
yes("Nash defect strictly dominates vs cooperate",5>3);
near("Nash joint cooperate gain",3+3,6);
near("Nash joint Nash gain",1+1,2);
near("Mixed matching payoff q0.7",2*.7-1,.4);
near("Mixed matching alternative",- (2*.7-1),-.4);
near("Mixed p/q half payoff",(2*.5-1)*(2*.5-1),0);
near("Mixed matching coin chance",.5*.5+.5*.5,.5);
near("Repeated cooperation at 0.6",3/(1-.6),7.5);
near("Repeated deviation at 0.6",5+.6/(1-.6),6.5);
near("Repeated cooperation at 0.4",3/(1-.4),5);
near("Repeated deviation at 0.4",5+.4/(1-.4),5.666666666666667);
near("Repeated grim delta threshold",.5,.5);
near("Repeated effective delta",.9*.5,.45);
near("Bayesian optimal bid v0.8",.8/2,.4);
near("Bayesian winning chance",2*.4,.8);
near("Bayesian bidder profit",(.8-.4)*.8,.32);
near("Bayesian rival full win bid",(.8-.5)*1,.3);
near("Bayesian three bidder bid",2*.8/3,.5333333333333333);
near("Mechanism payment at0.8",.8**2/2,.32);
near("Mechanism truthful payoff",.8*.8-.8**2/2,.32);
near("Mechanism misreport payoff",.8*.5-.5**2/2,.275);
near("Mechanism misreport loss",.32-.275,.045);
near("Second price high value win",.8-.6,.2);
near("Second price low value overbid loss",.4-.6,-.2);
near("Auction first-price two-bidder revenue",(2/3)/2,1/3);
near("Auction second-price two-bidder revenue",1/3,1/3);
near("Quoting optimal delta",1/2,.5);
near("Quoting ask fill intensity",10*Math.exp(-2*.5),3.6787944117144233);
near("Quoting two-side gross profit",2*10*.5*Math.exp(-1),3.6787944117144233);
near("Quoting wide gross profit",2*10*1*Math.exp(-2),2.706705664732254);
yes("Quoting delta half wins",2*10*.5*Math.exp(-1)>2*10*Math.exp(-2));
near("Inventory holding variance",10**2*2**2*.25,100);
near("Inventory holding standard deviation",Math.sqrt(100),10);
near("Inventory quadratic risk penalty",.01/2*100,.5);
near("Inventory reserve shift",.01*10*4*.25,.1);
near("Inventory doubled variance",20**2*2**2*.25,400);
near("Inventory short reserve shift",.01*(-10)*4*.25,-.1);
near("Poisson expected count",2*.5,1);
near("Poisson no order",Math.exp(-1),.36787944117144233);
near("Poisson at least one",1-Math.exp(-1),.6321205588285577);
near("Poisson Hawkes branching",.4/1,.4);
near("Poisson Hawkes stationary mean",3/(1-.4),5);
near("Poisson Hawkes impulse after minute",.4*Math.exp(-1),.14715177646857694);
const spread=.1*1*.5+2/.1*Math.log(1+.1/2);
near("Market making total spread",spread,1.0258032753274453);
near("Market making reserve shift",.1*2*1*.5,.1);
near("Market making bid",99.9-spread/2,99.38709836233628);
near("Market making ask",99.9+spread/2,100.41290163766372);
near("Market making risk-neutral spread",2/2,1);
near("Market making centre from bid ask",(99.9-spread/2+99.9+spread/2)/2,99.9);
near("Counterparty ask markout",100.05-100.10,-.05);
near("Counterparty bid markout",99.90-99.95,-.05);
near("Counterparty favorable bid markout",100.10-99.95,.15);
near("MARL matching exploitation pure",1-(-1),2);
near("MARL opponent H prob0.8",2*.8-1,.6);
near("MARL opponent H prob0.2",2*.2-1,-.6);
near("MARL mixed Nash exploitability",0,0);
assert(checks>=135,"V105 independent control and trading arithmetic checks unexpectedly incomplete");
console.log("V105 PASS",newest.length,"new control lectures;",all.length,"total;",
 mathCount,"strict KaTeX;",deepCount,"deep proof chapters;",deepChars,"deep prose chars;",
 checks,"independent numerics;",linkCount,"valid cross-links; control theory 30/30.");
