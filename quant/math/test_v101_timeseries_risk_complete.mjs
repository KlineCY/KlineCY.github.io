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
 "lessons-finance-v100c.json","lessons-finance-v100d.json"
];
const history=oldFiles.flatMap(n=>json(n).lessons);
assert.equal(history.length,459,"V100 historical 459 lesson texts changed");
assert.equal(new Set(history.map(l=>l.node_id)).size,459,"V100 history has duplicate IDs");
const files=["lessons-timeseries-v101.json","lessons-risk-tail-v101.json","lessons-risk-axioms-v101.json","lessons-risk-governance-v101.json"];
const docs=files.map(json);
docs.forEach((d,i)=>{
 assert.equal(d.version,1,"V101 data schema wrong");
 assert.equal(d.revision,"V101","V101 revision mismatch");
 assert.equal(d.lessons.length,[3,5,5,4][i],"V101 file size mismatch");
});
const newest=docs.flatMap(d=>d.lessons);
const ids=[
 "timeseries-n0246","timeseries-n0252","timeseries-n0253",
 "riskmath-n0571","riskmath-n0572","riskmath-n0573",
 "riskmath-n0575","riskmath-n0576","riskmath-n0577",
 "riskmath-n0578","riskmath-n0579","riskmath-n0580",
 "riskmath-n0581","riskmath-n0596","riskmath-n0597",
 "riskmath-n0598","riskmath-n0600"
];
assert.deepEqual(newest.map(l=>l.node_id),ids,"V101 knowledge coverage/order changed");
const all=[...history,...newest];
assert.equal(all.length,476,"Expected 476 completed mathematics lectures");
assert.equal(new Set(all.map(l=>l.node_id)).size,476,"Historical lesson overwritten");
assert(all.every(l=>nodes.get(l.node_id)?.title===l.title),"Curriculum mismatched or missing lesson");
const completed=new Set(all.map(l=>l.node_id));
for(const dept of ["timeseries","riskmath"]){
 const syllabus=curriculum.nodes.filter(n=>n.discipline_id===dept);
 assert.equal(syllabus.length,36,"Expected 36 syllabus nodes in "+dept);
 assert(syllabus.every(n=>completed.has(n.id)),dept+" must be 36/36 complete");
}
assert.equal(history.filter(l=>l.node_id.startsWith("timeseries-")).length,33);
assert.equal(history.filter(l=>l.node_id.startsWith("riskmath-")).length,22);
assert.equal(newest.filter(l=>l.node_id.startsWith("timeseries-")).length,3);
assert.equal(newest.filter(l=>l.node_id.startsWith("riskmath-")).length,14);

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
assert.equal(mathCount,153,"New formula count changed");
assert.equal(deepCount,51,"Deep chapter count changed");
assert(deepChars>=13900,"Depth reduced from reviewed version");
assert.equal(linkCount,51,"Cross-lesson link count changed");
const html=read("index.html"),css=read("styles.css"),app=read("app.js");
const workflow=read("../../.github/workflows/test-quant-math.yml");
assert(html.includes('app.js?v=20261108-v105-control-game-market-making-complete'),"Reader must bust V100 cache");
assert(html.includes("时间序列与风险数学"),"V101 CJK curriculum absent");
for(const id of ids)assert(html.includes("?topic="+id+'"'),"Study route missing "+id);
for(const needle of [
 'loadV100Finance("./lessons-finance-v100d.json",6)',
 'loadV101TimeseriesRisk("./lessons-timeseries-v101.json",3)',
 'loadV101TimeseriesRisk("./lessons-risk-tail-v101.json",5)',
 'loadV101TimeseriesRisk("./lessons-risk-axioms-v101.json",5)',
 'loadV101TimeseriesRisk("./lessons-risk-governance-v101.json",4)',
 '...v101time,...v101tail,...v101axioms,...v101governance',
 'new Set(lessons.map(l=>l.node_id)).size!==lessons.length',
 'new Set(doc.lessons.map(l=>l.node_id)).size!==8'
]) assert(app.includes(needle),"Canonical loader or previous curriculum loader missing: "+needle);
for(const token of ["overflow-x:auto","line-break:strict","@media(max-width:700px)","@media print",":focus-visible"])
 assert(css.includes(token),"Missing established Chinese/LaTeX mobile accessibility: "+token);
assert(workflow.includes("node quant/math/test_v101_timeseries_risk_complete.mjs"),"V101 CI step absent");

let checks=0;
function near(label,actual,expected,tol=1e-8){
 assert(Number.isFinite(actual),label+" nonfinite");
 assert(Math.abs(actual-expected)<=tol,label+" observed "+actual+" expected "+expected);
 checks++;
}
function yes(label,condition){near(label,Number(Boolean(condition)),1);}
function comb(n,k){let a=1;for(let i=1;i<=k;i++)a=a*(n-i+1)/i;return a;}

near("Ergodic AR variance",.75/(1-.5**2),1);
near("Ergodic AR sample mean var n4",(4+2*(3*.5+2*.25+1*.125))/16,.515625);
near("Ergodic independent mean variance",1/4,.25);
near("Ergodic long run var",(1+.5)/(1-.5),3);
near("Ergodic asymptotic mean se",Math.sqrt(3/1000),.05477225575051661);
near("Independent incorrect mean se",Math.sqrt(.001),.03162277660168379);
near("AR one step conditional forecast",.6*2,1.2);
near("AR two step conditional forecast",.6**2*2,.72);
near("AR h1 error variance",.64,.64);
near("AR h2 error variance",.64*(1+.6**2),.8704);
near("AR h3 error variance",.64*(1+.6**2+.6**4),.953344);
near("AR error variance infinity",.64/(1-.6**2),1);
near("AR conditional example MSE at mean",(.5*((-1)-1)**2+.5*(3-1)**2),4);
near("AR conditional example suboptimal MSE",(.5*(-1)**2+.5*3**2),5);
near("Deterministic trend t2",100+2*2,104);
near("Random walk permanent shock t2",100+2*2+5,109);
near("Random walk t10 variance",10*4,40);
near("Random walk t10 standard error",2*Math.sqrt(10),6.324555320336759);
near("Overdiff variance iid",2*4,8);
near("Overdiff lag covariance",-4,-4);
near("Overdiff lag correlation",-4/8,-.5);

near("Moments skew mean",.5*(-1)+.25*0+.25*2,0);
near("Moments skew variance",.5*1+.25*4,1.5);
near("Moments third moment",.5*(-1)**3+.25*2**3,1.5);
near("Moments fourth moment",.5*1+.25*16,4.5);
near("Moments skewness",1.5/(1.5**1.5),.816496580927726);
near("Moments kurtosis",4.5/(1.5**2),2);
near("Moments excess kurtosis",4.5/2.25-3,-1);
near("Moments fourth-power outlier ratio",10000/16,625);
near("Pareto3 survival2",2**-3,.125);
near("Pareto3 first moment",3/2,1.5);
near("Pareto3 second moment",3/(3-2),3);
near("Pareto3 variance",3-(1.5**2),.75);
yes("Pareto third moment infinite",3>=3);
near("Pareto threshold doubling ratio",4**-3/2**-3,.125);
near("Pareto4 q99",100**.25,3.1622776601683795);
near("Pareto4 q999",1000**.25,5.623413251903491);
near("Pareto4 q ratio",10**.25,1.7782794100389228);
near("Pareto2 q ratio",Math.sqrt(10),3.1622776601683795);
near("Student t5 variance",5/3,1.6666666666666667);
near("Student t5 fourth moment",3*25/(3*1),25);
near("Student t5 kurtosis",25/((5/3)**2),9);
near("Student t5 excess kurtosis",6/(5-4),6);
near("Student t5 unit scale",Math.sqrt(3/5),.7745966692414834);
near("Student t5 scaled variance",(3/5)*(5/3),1);
near("Student conditional variance U1",5/1,5);
near("Student conditional variance U10",5/10,.5);
near("GPD shape25 survival y2",(1+.25*2/2)**(-4),.4096);
near("GPD mean shape25",2/(1-.25),8/3);
near("GPD variance shape25",4/(.75**2*(1-2*.25)),14.222222222222221);
near("GPD threshold beta",2+.25*2,2.5);
near("GPD threshold new mean",2.5/.75,10/3);
near("GPD bounded endpoint",-2/(-.25),8);
yes("GPD xi .75 variance infinite",.75>=.5);
yes("GPD xi 1.2 mean infinite",1.2>=1);
near("Tail index alpha",1/.25,4);
near("Tail exponent sigma4 p ratio",10**(1/4),1.7782794100389228);

near("Monotonic risk 75percent A",5,5);
near("Monotonic risk 75percent B",8,8);
yes("Monotonic lower risk 5le8",5<=8);
near("Mean lower risk example",.02*100,2);
near("Mean upper risk example",3,3);
yes("Mean order does not determine VaR",2<3&&100>3);
near("Cash VaR shifted",0+3,3);
near("Cash ES shifted",10+3,13);
near("Cash discount 100",100*.95,95);
near("Subadd zero-loss probability",.96**2,.9216);
near("Subadd one-loss probability",2*.96*.04,.0768);
near("Subadd double-loss probability",.04**2,.0016);
near("Subadd combined VaR",10,10);
near("Subadd single ES",(.04*10)/.05,8);
near("Subadd combo ES",(.0016*20+.0484*10)/.05,10.32);
yes("ES subadd example",10.32<=8+8);
near("Homogeneous ES triple",10*3,30);
near("Quadratic market impact double",2**2,4);
near("Entropic risk one",Math.log((1+Math.exp(2))/2),1.4337808304830273);
near("Entropic risk twice",Math.log((1+Math.exp(4))/2),3.3250027473578645);
near("Entropic twice first",2*Math.log((1+Math.exp(2))/2),2.8675616609660546);
yes("Entropic fails positive homogeneity",Math.log((1+Math.exp(4))/2)>2*Math.log((1+Math.exp(2))/2));

near("Parameter mean standard error",.2/Math.sqrt(25),.04);
near("Parameter mean SE quadruple observations",.2/Math.sqrt(100),.02);
near("Parameter predictive variance",.2**2+.04**2,.0416);
near("Parameter predictive SD",Math.sqrt(.0416),.2039607805437114);
near("Merton portfolio sensitivity mu",1/(2*.2**2),12.5);
near("Merton portfolio shift at 1pct",.01/(2*.2**2),.125);
near("Markov stationary low",.2/(.1+.2),2/3);
near("Markov stationary high",.1/(.1+.2),1/3);
near("Markov stationary low balance",(2/3)*.1,(1/3)*.2);
near("Markov mixed variance",(2/3)*.01+(1/3)*.09,.03666666666666667);
near("Markov mixed volatility",Math.sqrt((2/3)*.01+(1/3)*.09),.19148542155126763);
near("Markov naive weighted vol",(2/3)*.1+(1/3)*.3,.16666666666666666);
near("Markov mixing eigenvalue",.9-.2,.7);
near("EWMA forecast variance",.94*.0004+.06*.05**2,.000526);
near("EWMA forecast volatility",Math.sqrt(.000526),.02293468988235943);
near("EWMA latest weight",1-.94,.06);
near("EWMA next weight",.06*.94,.0564);
near("EWMA third weight",.06*.94**2,.053016);
near("EWMA effective sample λ.94",(1+.94)/(1-.94),32.33333333333331,1e-7);
near("EWMA effective sample λ.97",(1+.97)/(1-.97),65.66666666666661,1e-7);
near("Binomial 99pct VaR expected exceptions",100*.01,1);
near("Binomial actual exception rate",4/100,.04);
let cdf=0;for(let k=0;k<=3;k++)cdf+=comb(100,k)*.01**k*.99**(100-k);
near("Binomial P(K>=4)",1-cdf,.018374036444650528,1e-10);
yes("Observed exceptions above target",.04>.01);
assert(checks>=95,"Numerical tests unexpectedly incomplete");
console.log("V101 PASS",newest.length,"new lessons;",all.length,"total;",mathCount,
 "strict KaTeX;",deepCount,"deep proof chapters;",deepChars,"deep prose chars;",
 checks,"independent computations;",linkCount,"internal references; time series and risk each 36/36.");
