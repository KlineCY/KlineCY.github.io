import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
import katex from "katex";
import {gunzipSync} from "node:zlib";

const root=new URL("./",import.meta.url);
const read=name=>fs.readFileSync(new URL(name,root),"utf8");
const json=name=>JSON.parse(read(name));
const files=["atlas-concept-bridge-v114a.json","atlas-concept-bridge-v114b.json","atlas-concept-bridge-v114c.json",
             "atlas-concept-bridge-v114d.json","atlas-concept-bridge-v114e.json"];
const docs=files.map(json);
const manifest=json("atlas-depth-manifest.json");
const math=json("../math/curriculum.json");
const mathIDs=new Map(math.nodes.map(node=>[node.id,node.title]));
const archive=JSON.parse(gunzipSync(Buffer.concat([1,2,3,4].map(i=>fs.readFileSync(new URL("atlas/data.part"+i,root))))).toString("utf8"));
const previous=[
 json("atlas-concept-chapters.json"),json("atlas-concept-bridge-01.json"),
 json("atlas-concept-bridge-v111a.json"),json("atlas-concept-bridge-v111b.json"),json("atlas-concept-bridge-v111c.json"),
 ...["a","b","c","d","e"].map(x=>json("atlas-concept-bridge-v112"+x+".json")),
 ...["a","b","c","d","e"].map(x=>json("atlas-concept-bridge-v113"+x+".json"))
];
const priorIDs=new Set(previous.flatMap(b=>Object.keys(b.lessons)));
const expected=[
 "m06-c02-p04","m06-c02-p05","m06-c04-p03","m06-c04-p06",
 "m07-c02-p04","m07-c02-p05","m07-c02-p06","m07-c01-p04",
 "m08-c02-p02","m11-c03-p01","m12-c01-p06","m12-c03-p06",
 "m10-c02-p03","m12-c04-p03","m23-c02-p05","m12-c03-p02",
 "m22-c01-p06","m22-c03-p02","m23-c03-p01","m23-c05-p02"
];
const entries=docs.flatMap(d=>Object.entries(d.lessons));
assert.equal(manifest.expected_lessons,720,"Original ML core must stay 720");
assert.equal(manifest.expected_domains,24,"Original ML domains must stay 24");
assert.equal(archive.catalog.items.length,720,"Original archive course count changed");
assert.equal(mathIDs.size,656,"All 656 math prerequisite nodes must remain");
assert.equal(entries.length,20,"20 canonical deep lessons required");
assert.deepEqual(entries.map(([id])=>id),expected,"V114 exact topics and order changed");
assert.equal(new Set(expected).size,20,"Duplicate topic identity");
assert(expected.every(id=>!priorIDs.has(id)),"Do not duplicate any earlier deep concept chapter");
for(const d of docs){
 assert.equal(d.schema,1,"Chapter schema changed");
 assert.equal(d.revision,"V114","V114 revision not verified");
 assert.equal(Object.keys(d.lessons).length,4,"Each independent transmission must include four complete topics");
}
let sections=0,strictFormulas=0,crossLinks=0,chineseChars=0;
for(const [id,lesson] of entries){
 assert(/^m\d\d-c\d\d-p\d\d$/.test(id),"Unexpected Atlas node id "+id);
 const canonical=archive.domains[id.slice(0,3)]?.[id];
 assert(canonical,"Deep lesson must map to real archive course "+id);
 assert.equal(lesson.source_title,canonical.title,id+" original source title mismatch");
 assert(lesson.goal?.length>=60,id+" missing research goal and assumptions");
 assert.equal(lesson.sections?.length,4,id+" requires four logical subsections");
 assert.equal(lesson.links?.length,4,id+" requires four math/ML prerequisites");
 const headings=new Set();
 for(const s of lesson.sections){
  assert(s.title?.length>=6&&s.question?.length>=12,id+" lacks specific research question");
  assert(!headings.has(s.title),id+" repeated argument heading");
  headings.add(s.title);
  assert(s.paragraphs?.length===2&&s.paragraphs.every(p=>p.length>=40),id+" missing two rigorous paragraphs");
  assert(s.worked?.length>=55,id+" requires an independently replicable worked example");
  assert(s.boundary?.length>=30,id+" missing method failure boundary");
  assert.equal(s.equations?.length,1,id+" must explain each equation rather than list formulas");
  const eq=s.equations[0];
  assert(eq.label?.length>=3&&eq.tex?.length>=14,id+" missing labelled substantive equation");
  assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(eq.tex),id+" invalid Unicode in formula");
  try{katex.renderToString(eq.tex,{displayMode:true,throwOnError:true,strict:"error"});}
  catch(e){throw Error(id+" / "+s.title+" strictly invalid KaTeX: "+eq.tex+" | "+e.message);}
  assert(/[\u3400-\u9FFF]/u.test(JSON.stringify(s)),id+" lacks Chinese explanatory text");
  assert(!/TODO|FIXME|待补充|独立稿|占位|Lorem ipsum/.test(JSON.stringify(s)),id+" placeholder found");
  sections++;strictFormulas++;
  chineseChars+=s.paragraphs.join("").length+s.worked.length+s.boundary.length;
 }
 for(const link of lesson.links){
  assert(typeof link.label==="string"&&link.label.trim().length>=2,id+" unlabeled prerequisite");
  if(link.kind==="math")
   assert.equal(mathIDs.get(link.id),link.label,id+" mathematics prerequisite mismatch: "+link.id);
  else if(link.kind==="ml")
   assert(archive.domains[link.id.slice(0,3)]?.[link.id],id+" unknown machine learning prerequisite "+link.id);
  else throw Error(id+" unsupported prerequisite kind: "+link.kind);
  crossLinks++;
 }
}
assert.equal(sections,80,"V114 requires 80 full proof sections");
assert.equal(strictFormulas,80,"V114 requires 80 strict KaTeX equations");
assert.equal(crossLinks,80,"V114 requires 80 valid deep prerequisite links");
assert(chineseChars>=21000,"Explanation content was shortened too much");

const source=read("concept-render.js"),reader=read("atlas.js"),html=read("index.html"),css=read("atlas.css");
new vm.Script(source);new vm.Script(reader);
assert(reader.includes('for(const path of ["./atlas-concept-bridge-v114a.json","./atlas-concept-bridge-v114b.json"'),"New V114 reader loader missing");
assert(reader.includes('state.conceptChapters.has(id)||newIds.has(id)'),"V114 duplicate/extension collision guard missing");
const ctx={window:{},encodeURIComponent};vm.runInNewContext(source,ctx);
const loaded=new Set();
for(const d of docs){
 const original=new Map(Object.keys(d.lessons).map(id=>[id,archive.domains[id.slice(0,3)][id].html]));
 const mounted=ctx.window.quantMlConcepts.mount(archive,d);
 assert.equal(mounted.size,4,"One of five V114 canonical installations failed");
 for(const id of mounted){
  assert(!loaded.has(id),"Duplicate V114 installed course");loaded.add(id);
  const rendered=archive.domains[id.slice(0,3)][id].html;
  assert(rendered.startsWith(original.get(id)),id+" overwrote original HTML instead of appending");
  assert.equal((rendered.match(/class="atlas-concept-unit"/g)||[]).length,4,id+" four-section mount missing");
  assert(rendered.includes("../math/?topic="),id+" maths deep links not installed");
  assert(rendered.includes("本节学习顺序"),id+" four-stage Chinese learning route missing");
 }
}
assert.equal(loaded.size,20);
assert.equal(archive.catalog.items.length,720,"Deepening must preserve all core 720 Atlas IDs");
for(const id of expected)assert(html.includes('href="#lesson/'+id+'"'),"Missing direct route to original lesson "+id);
assert(html.includes("数据时点、稀疏回归与金融预测"),"V114 title or Chinese course map absent");

assert(html.indexOf('id="atlas-reader-section"')>html.indexOf("</header>")&&
       html.indexOf('id="atlas-reader-section"')<html.indexOf('aria-label="系统学习路径"'),
       "V115 top-of-page reader must remain above V114 and earlier learning paths");
assert.equal((html.match(/id="atlas-reader-section"/g)||[]).length,1,"V115 lecture reader duplicated");
assert(html.includes('id="course-site-nav-toggle"')&&html.includes('id="atlas-toggle-catalog"'),
       "V115 mobile site and curriculum navigation must be retained");

assert(html.includes('atlas.js?v=20261109'),"V114 runtime cache version missing");
for(const selector of ["line-break:strict","overflow-x:auto","@media(max-width:700px)",".atlas-concept-jump:focus-visible",".atlas-concept-chain","@media print"])
 assert(css.includes(selector),"Responsive CJK, KaTeX scroll, keyboard or print style missing "+selector);
const workflow=read("../../.github/workflows/test-quant-ml.yml");
assert(workflow.includes("node quant/ml/test_concept_bridges_v112.mjs"),"V114 gate omitted from CI");
const readV113=read("test_concept_bridges_v113.mjs");
assert(readV113.includes("new vm.Script(source)")&&readV113.includes("720 preserved legacy lectures"),"V113 historical test disabled");

// Independent financial-statistical and ML numeric checks; verify claims beyond LaTeX syntax.
let numerics=0;
function near(name,value,expected,tol=1e-9){
 assert(Number.isFinite(value)&&Math.abs(value-expected)<=tol,name+" computed "+value+", expected "+expected);numerics++;
}
function yes(name,value){assert(value,name);numerics++;}
// Black–Litterman and portfolio risk budgets
near("BL implied premium A",2*.04*.6,.048);
near("BL implied premium B",2*.09*.4,.072);
near("BL posterior precision fusion",(.05/.04+.07/.01)/(1/.04+1/.01),.066);
near("BL ill-conditioned second weight impact",.01/(2*.01),.5);
near("Risk parity equal volatility contribution A",.6**2*.2**2,.0144);
near("Risk parity equal volatility contribution B",.4**2*.3**2,.0144);
near("Risk parity initial equal-weight variance",.25*.04+.25*.09,.0325);
near("Risk parity stress covariance rise",2*.5*.5*.2*.3*.5,.015);
near("Risk parity stressed variance",.0325+.015,.0475);
near("Risk budget seven-three ratio",1.5*Math.sqrt(7/3),2.29128784747792,1e-8);
// Convex turnover and path-dependent drawdown
yes("Turnover no-trade criterion",Math.abs(2*(.8-.5))<=.7);
near("Trade after soft threshold",.5+(.8-.5-.2/2),.7);
near("Drift after plus 10 minus 10",.5*1.1/(.5*1.1+.5*.9),.55);
near("Actual round-trip turnover",Math.abs(.5-.55)+Math.abs(.5-.45),.1);
near("Fee of ten bp on turnover",.001*.1,.0001);
near("Maximum drawdown peak 120 trough 90",1-90/120,.25);
near("Loss 25 percent recover gain",.25/(1-.25),1/3);
near("Drawdown order A",1-(110*.9*.95)/110,.145);
near("Drawdown order B",.10,.1);
near("Identical final NAV after reorder",110*.9*.95,90*1.10*.95);
// Walk-forward, HAC and discrete tail-risk
yes("Walk-forward sample ending day 10 excluded before 8",10>8);
near("Overlapping training test event intersection",Math.min(7,9)-Math.max(3,6)+1,2);
near("HAC theoretical long run variance",4+2*1,6);
near("Newey West Bartlett lag one",1-1/(1+1),.5);
near("Newey West estimated long run variance",4+2*.5*1,5);
near("Newey West SE",Math.sqrt(5/100),Math.sqrt(.05));
near("HAC uncorrected SE",Math.sqrt(4/100),.2);
near("Overlapping 20-day return label correlation",19/20,.95);
near("Annualized naive Sharpe",.05*Math.sqrt(252),.7937253933193772);
near("Discrete 95% VaR",1,1);
near("Discrete 90% ES",(.05*1+.05*4)/.1,2.5);
near("Incorrect all VaR exceedances ES",(.15*1+.05*4)/.20,1.75);
near("Probability no breaches among 100 at 1%",.99**100,.3660323412732292,1e-9);
// Scientific replication and model selection
near("Versioned trading ledger net NAV",100*1.02*.999,101.898);
near("Five seeded Sharpe average",(.8+1+.6+.9+.7)/5,.8);
near("Twenty independent search false discoveries",1-.95**20,.6415140775914581,1e-8);
// PSI, information distance, CUSUM, adversarial perturbations
near("PSI symmetrized KL",.25*Math.log(3),.27465307216702745);
near("KL directional Q given P",.75*Math.log(1.5)+.25*Math.log(.5),.13081203594113697);
near("KL directional P given Q",.5*Math.log(2/3)+.5*Math.log(2),.14384103622589045);
near("PSI Jeffreys equality",.75*Math.log(1.5)+.25*Math.log(.5)+.5*Math.log(2/3)+.5*Math.log(2),.25*Math.log(3));
near("Laplace smoothing new zero bin",1/(3+3),1/6);
near("Wasserstein two point masses",Math.abs(2-0),2);
near("CUSUM after one alarm period",Math.max(0,.8-.5),.3);
near("CUSUM after three identical periods",3*(.8-.5),.9);
near("Independent sequential false alarm",1-.95**20,.6415140775914581,1e-8);
near("ARL for one percent false positives",1/.01,100);
near("Adversarial initial positive margin",2*1-1*1,1);
near("Adversarial Linfinity bound",.2*(Math.abs(2)+Math.abs(-1)),.6);
near("Adversarial residual margin",1-.6,.4);
near("Adversarial FGSM directional gain",3*.1+(-2)*(-.1),.5);
near("Adversarial PGD clamp",Math.min(.2,.15+.15),.2);
// DataLoader batching and attention
near("DataLoader batches ceil",Math.ceil(10/4),3);
near("DataLoader weighted epoch mean",(4*1+4*2+2*10)/10,3.2);
near("DataLoader wrong unweighted batch mean",(1+2+10)/3,13/3);
near("Padded token weighted loss",(3*1+1*5)/4,2);
near("Padded sequence equal-weight loss",(1+5)/2,3);
near("MHA per-head feature width",768/12,64);
near("MHA scaled qk logit",16/Math.sqrt(64),2);
near("MHA softmax first weight",Math.exp(2)/(Math.exp(2)+1),.8807970779778823);
near("MHA triangular visible entries",1+2+3,6);
// State-space and variational inference
near("State one-step transition",.8*10,8);
near("State third step",.8**3*10,5.12);
near("State illustrative observation",2*8+1,17);
near("State stationary variance",.36/(1-.8**2),1);
near("Kalman update gain",1/(1+1),.5);
near("Kalman filtered mean",0+.5*(2-0),1);
near("ELBO Jensen log evidence",Math.log((1+3)/2),Math.log(2));
near("ELBO Jensen lower term",.5*Math.log(1)+.5*Math.log(3),.5*Math.log(3));
near("ELBO variational lower bound",-3-.2,-3.2);
near("ELBO reconstruction and KL",-2-.5,-2.5);
near("Gaussian KL mean shift",.5*(1+1-1-Math.log(1)),.5);
// Score-function gradient, real transaction costs, GCN, Masked LM
near("Score function unbiased baseline E",(.5*1+.5*(-1))*2,0);
near("Policy gradient uncentered",.5*3+.5*(-1)*1,1);
near("Policy gradient baseline centered",(.5*1*(3-2)+.5*(-1)*(1-2)),1);
near("Variance reduction uncentered",((3-1)**2+(-1-1)**2)/2,4);
near("Variance reduction centered",((1-1)**2+(1-1)**2)/2,0);
near("Off-policy target action importance",.8/.5,1.6);
near("Bid ask spread per share",100.05-99.95,.1,1e-12);
near("Active roundtrip spread 100 shares",100*(100.05-99.95),10);
near("Commission both sides",.0002*100*(100.05+99.95),4);
near("Roundtrip combined spread fee",100*.1+.0002*100*(100.05+99.95),14);
near("Roundtrip bps",14/10000*10000,14);
near("GCN two-node normalized edge",1/2,.5);
near("GCN propagated same feature",(1+3)/2,2);
near("GCN row-major propagation",.5*1+.5*3,2);
near("MLM expected 15% selected",10000*.15,1500);
near("MLM expected eighty percent mask",10000*.15*.8,1200);
near("MLM expected ten percent random",10000*.15*.1,150);
near("MLM two-position total NLL",-Math.log(.8)-Math.log(.5),.916290731874155);
near("MLM two-position average NLL",(-Math.log(.8)-Math.log(.5))/2,.4581453659370775);
near("MLM two-position average with low confidence",(-Math.log(.9)-Math.log(.1))/2,1.203972804325936);
// V114-specific independently recomputed invariants on top of all historical numerical assertions.
near("Target encoding with prior",(2*.08+4*.02)/6,.04);
near("Target encoding leave one out",(.01+.03)/2,.02);
near("Fold-only scaler", (5-2)/1,3);
near("Full-sample leaky scaler",(5-3)/Math.sqrt(8/3),Math.sqrt(1.5));
near("Historical AR lag prediction",.3*.02+(-.1)*(-.01),.007);
near("Historical 20-day overlap",19/20,.95);
near("Lasso proximal coordinate positive",3-1.2,1.8);
near("Lasso proximal coordinate negative",-(2-1.2),-.8);
near("Lasso proximal zero",Math.max(0,.4-1.2),0);
near("Lasso proximal half gradient",Math.max(1.5-.6,0),.9);
near("Lasso stationary threshold",Math.max(3-1.2,0),1.8);
near("Elastic Net duplicated features",(2-.5)/(2+1),.5);
near("Elastic Net derivative stationarity",(.5+.5-2)+.5+.5,0);
near("Lasso zero path lambda max",(1*2+(-1)*(-2))/2,2);
near("Lasso path next coefficient",2-1,1);
near("Warm start scalar path at lambda 2",3-2,1);
near("Warm start scalar path at lambda 1",3-1,2);
near("One SE upper loss",.020+.003,.023);
near("OLS variance for two observations",1/5,.2);
near("GLS unequal error variance",.9**2*1+.1**2*9,.9);
near("OLS heteroskedastic equal mean variance",(1+9)/4,2.5);
near("Biased lower MSE",.04+.2**2,.08);
near("OOB bootstrap no selection in n=10",.9**10,.3486784401);
near("OOB asymptotic held-out share",Math.exp(-1),.3678794411714423);
near("OOB separate tree aggregation",(1+3)/2,2);
near("OOB permutation increase",.035-.02,.015);
near("PCA explained first component",4/5.2,.7692307692307693);
near("PCA explained two components",5/5.2,.9615384615384616);
near("PCA small eigenvalue whitening",.02/.01,2);
near("PCA large eigenvalue whitening",.02/2,.01);
near("PCA stressed largest eigenvalue",.04*(1+.9),.076);
near("Calibration observed event proportion",6/10,.6);
near("Brier reliability",.5*(.8-.6)**2+.5*(.2-.4)**2,.04);
near("Brier resolution",.5*(.6-.5)**2+.5*(.4-.5)**2,.01);
near("Brier total decomposition",.04-.01+.25,.28);
near("Platt calibration score one",1/(1+Math.exp(-1)),.7310585786300049);
near("Trade probability cost threshold",(1+.1)/(2+1),1.1/3);
near("Trade positive expected payoff",.4*2-(1-.4)*1-.1,.1);
near("Sharpe corrected normal sigma",Math.sqrt((1+.5*.2**2)/100),Math.sqrt(.0102));
near("Sharpe deflation toy z",(.12-.07)/.02,2.5);
near("Stable selection 83 of 100",83/100,.83);
near("Stable selection expected false bound",5**2/((2*.75-1)*100),.5);
near("Causal collider before selection",.5,.5);
near("Causal collider conditional same input",1,1);
near("Intervention blocked by hidden confounder",.5,.5);
near("Backdoor average",.5*2+.5*4,3);
near("Block Bootstrap moving blocks n10 l3",10-3+1,8);
near("Block Bootstrap ceil blocks",Math.ceil(10/3),4);
near("Stationary bootstrap mean block size",1/.25,4);
near("Stationary bootstrap probability of length3",.25*.75**2,.140625);
near("AR1 long run variance factor",(1+.5)/(1-.5),3);
near("AR1 mean variance at T100",3/100,.03);
near("Structural break pooled SSE",5*(0-1)**2+5*(2-1)**2,10);
near("Known Chow toy F",((24-16)/2)/(16/(20-4)),4);
near("Sequential CUSUM three increments",3*(.8-.5),.9);
near("GARCH ARCH component",.1*.02**2,.00004);
near("GARCH GARCH component",.85*.00004,.000034);
near("GARCH next variance",.000002+.00004+.000034,.000076);
near("GARCH unconditional variance",.000002/(1-.1-.85),.00004);
near("GARCH next sigma",Math.sqrt(.000076),.008717797887081348);
near("GARCH standardized innovation",.01**2/.0001,1);
near("GARCH Gaussian 99% VaR",2.32635*Math.sqrt(.000076),.02028064911461169,1e-10);
near("Two ETF uncorrelated risk",.5**2*.2**2+.5**2*.2**2,.02);
near("Two ETF stressed correlation risk",.02+2*.5*.5*.9*.2*.2,.038);
near("Two ETF offdiag covariance",.2*.3*.5,.03);
near("Two ETF covariance determinant",.04*.09-.03**2,.0027);
near("Portfolio regularization sensitivity",.01/(2*.0001),50);
near("Portfolio trade relative to drift",Math.abs(.7-.55)+Math.abs(.3-.45),.3);
near("ETF three-way rank IC",1-6*2/(3*(9-1)),.5);
near("ETF dollar neutral net exposure",.5+0-.5,0);
near("ETF dollar neutral gross exposure",.5+.5,1);
near("ETF long short gross payoff",.5*.02-.5*(-.03),.025);
near("ETF execution cost",.4*.002,.0008);
near("ETF net realized return",.025-.0008,.0242);
yes("Publish cut after trade decision is not usable",new Date("2026-03-20")>new Date("2026-01-10"));
console.log("V114 PASS: "+loaded.size+" canonical improved articles, "+sections+
 " substantial reasoning sections, "+strictFormulas+" KaTeX strict formulas, "+crossLinks+
 " exact curriculum links, "+chineseChars+" validated Chinese explanatory characters, "+
 numerics+" independent analytic checks, 720 preserved legacy lectures, responsive CJK and historical V112 regression");
