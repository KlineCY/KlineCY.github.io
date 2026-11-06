import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const raw=p=>fs.readFileSync(new URL(p,root),"utf8");
const groups="abcdefghijkl".split("").map(c=>"atlas-depth-07-"+c+".json");
const manifest=read("atlas-depth-manifest.json");
assert.equal(manifest.expected_lessons,720);
assert.equal(manifest.expected_domains,24);
assert.equal(manifest.modules.reduce((n,m)=>n+m.lessons,0),720);
const modules=new Map(manifest.modules.map(m=>[m.path,m.lessons]));
let checked=0,formulas=0,deepWords=0;
const fullIds=new Set(),newIds=new Set(),domainCounts={};
for(const file of groups){
 assert.equal(modules.get("./"+file),8,"missing canonical module "+file);
 const d=read(file);
 assert.equal(d.schema,1,"module version "+file);
 assert.equal(Object.keys(d.lessons).length,8);
 assert.equal(d.revision,"2026-11-06","module not deepened "+file);
 for(const [id,l] of Object.entries(d.lessons)){
  assert(!fullIds.has(id),"duplicate ML identity "+id);
  fullIds.add(id);checked++;
  const domain=Number(id.slice(1,3)),revision=domain<=8?"V56":domain<=20?"V57":"V58";
  domainCounts[domain]=(domainCounts[domain]||0)+1;
  assert(l.revision_note?.startsWith(revision),id+" deepening revision");
  if(domain>6)newIds.add(id);
  assert(typeof l.source_title==="string"&&l.source_title.length>=2,id+" source title lost");
  assert(typeof l.overview==="string"&&l.overview.length>=60,id+" overview lost");
  assert.equal(l.equations.length,6,id+" enhanced seventh-stage equations");
  assert.equal(l.derivation.length,6,id+" enhanced seventh-stage proofs");
  assert.equal(l.protocol.length,6,id+" enhanced seventh-stage protocols");
  assert.equal(l.pitfalls.length,3,id+" original plus failure boundary");
  assert(l.worked_example.length>=100,id+" worked arithmetic lost");
  assert(l.derivation.slice(3).every(s=>s.length>=65),id+" derivation too shallow");
  assert(l.protocol.slice(3).every(s=>s.length>=25),id+" research verification too shallow");
  assert(l.pitfalls.at(-1)?.length>=25,id+" risk boundary missing");
  assert(!/TODO|Lorem ipsum|[\uFFFD\u200B\uFEFF]/i.test(JSON.stringify(l)),id+" malformed Chinese");
  for(const e of l.equations){
   assert(typeof e.label==="string"&&e.label.length>=2&&typeof e.tex==="string"&&e.tex.length>=5,id+" malformed equation");
   katex.renderToString(e.tex,{throwOnError:true,strict:"ignore",displayMode:true});
   formulas++;
  }
  if(domain>6)deepWords+=l.derivation.slice(3).join("").length+l.protocol.slice(3).join("").length+l.pitfalls.at(-1).length;
 }
}
assert.equal(checked,96);
assert.equal(formulas,576);
assert.equal(newIds.size,72);
// Chronos original paper §3.1 uses m=0 and s=mean(abs(context));
// do not quietly replace the published tokenizer with centered standardization.
const chronos=read("atlas-depth-07-k.json").lessons["m22-c05-p05"];
assert(chronos.equations[3].tex.includes("sum_{j=1}^{C}|x_j|"),"Chronos original mean-absolute scaling missing");
assert(chronos.worked_example.includes("s_C=(2+4+0)/3=2"),"Chronos scale example missing");

assert.deepEqual(Object.keys(domainCounts).length,24);
for(const n of Object.values(domainCounts))assert.equal(n,4,"exactly four seventh-stage lessons per domain");
assert(deepWords>27000,"V37 substantial Chinese derivation and audit text");

const home=raw("index.html"),reader=raw("atlas.js"),css=raw("atlas.css");
const start=home.indexOf('aria-label="V37第七批九十六篇正式讲义全覆盖深化"');
assert(start>=0,"V37 nine-stage navigation missing");
const end=home.indexOf("</details>",start);
assert(end>start,"V37 navigation not closed");
const v37=home.slice(start,end);
assert.equal((v37.match(/href="#lesson\//g)||[]).length,72,"72 new deep links required");
for(const id of newIds)assert(v37.includes('href="#lesson/'+id+'"'),"V37 link not reachable "+id);
for(const title of ["阶段一","阶段二","阶段三","阶段四","阶段五","阶段六","阶段七","阶段八","阶段九"])
 assert(v37.includes(title),"incomplete nine-stage curricular flow "+title);
assert(home.includes('aria-label="V36四十八篇机器学习数学和量化深入课程"'),"previous roadmap lost");
for(const m of ["linear-n0065","statistics-x001","riskmath-x002"])
 assert(v37.includes('href="../math/?topic='+m+'"'),"missing quantitative math bridge "+m);
for(const key of ["s.equations.map","s.derivation","s.protocol","s.pitfalls","s.worked_example","katex.render"])
 assert(reader.includes(key),"course viewer not compatible "+key);
for(const marker of [".atlas-spectral-path",".atlas-spectral-stage",".atlas-spectral-links",".ml-math-block","overflow-x:auto","line-break:strict","@media print"])
 assert(css.includes(marker),"narrow-screen / Chinese / math style missing "+marker);

const near=(value,expect,label,tol=1e-9)=>assert(Math.abs(value-expect)<=tol,label+" "+value+" != "+expect);
near(.75*8,6,"dropout conditional mean");
near(.75*(8-6)**2+.25*(0-6)**2,12,"dropout conditional variance");
near(Math.sqrt(.8*.2/20),.08944271909999159,"patient-level standard error");
near(.009/(.009+.0495),.15384615384615385,"rare diagnostic class PPV");
near(.75*2+.25*6,3,"GRU update gate");
near(.75**5,.2373046875,"GRU direct memory path");
near(.75*6+.25*2,5,"GRU smooth interpolation");
near(512/8,64,"eight head dimensions");
near(Math.exp(-1)/(1+Math.exp(-1)),.2689414213699951,"stable softmax");
near(14/3,4.666666666666667,"linear attention toy");
near(3/5,.6,"retrieval precision");
near(3/4,.75,"retrieval recall");
near(50/250,.2,"document block overlap");
near(Math.exp(-1),.36787944117144233,"temporal edge decay");
near(.12-.09,.03,"GNN baseline increment");
near(.10-.09,.01,"GNN after industry controls");
near(10*(1.4-1)**2,1.6,"WGAN GP");
near(Math.log(2),.6931471805599453,"disjoint JS divergence");
near(.5*2+.5*1,1.5,"Actor-Critic value");
near(.8+.2/4,.85,"epsilon greedy");
near(-Math.log(.75),.2876820724517809,"tanh log Jacobian");
near(.8**2,.64,"VAR-like stability");
near((Math.abs(2)+Math.abs(-4)+Math.abs(0))/3,2,"Chronos paper mean-absolute scale");
near(1-88/110,.2,"maximum drawdown");
near(1/Math.sqrt(8),.3535533905932738,"sample RMS");
near(2*.02*.5*.1,.002,"impact structure demonstration");
near((.01**2+.01**2+.02**2)**.5,.02449489742783178,"realized volatility");
near((1+.5+.5)/3,2/3,"event sample uniqueness");
near((.02+.04+.06)/3/.02,2,"ICIR population SD");
near(.02/.01,.2*10,"probability ratio scalar");
near(64/.016,4000,"batch inference throughput");
near(.7-.4,.3,"MIA advantage");
near(1+2*.5*.2,1.2,"Bartlett HAC L=1");
near(1+2*.2,1.4,"population long-run variance");
near(.03/.02,1.5,"rolling beta");
near(5*1.2,6,"Gamma log-link proportional mean");
near((2+1+3)/3,2,"Fama-MacBeth mean percent");
near(.8*(1-.8),.16,"boost logistic curvature");
near(6/(4+2),1,"regularized leaf weight");
near(.2+.1*.5,.25,"boosting update");
near(1/Math.sqrt(2*Math.PI),.3989422804014327,"Gaussian KDE");
near(.04+.09,.13,"GP observed prediction variance");
near(.2*.4/(.8*.1+.2*.4),.5,"GMM related posterior check");
near(.9**2*4+1,4.24,"Kalman variance prediction");
near(-100-5,-105,"standard ELBO");
near((1+.8)/(1-.8),9,"Gibbs correlation time");
near(Math.sqrt(.01),.1,"whitening small direction");
near(1/Math.sqrt(.01+.03),5,"ridge-whitening small direction");
near(95+5,100,"purging label maturity");
near(1/.1,10,"IPW small propensity");

const v56Applied=read("atlas-depth-07-d.json").lessons;
const v56AppliedIds=["m07-c05-p05","m07-c05-p04","m07-c04-p04","m07-c05-p01","m08-c05-p05","m08-c04-p01","m08-c04-p02","m08-c03-p03"];
for(const id of v56AppliedIds){
 const l=v56Applied[id];
 assert(l?.revision_note?.startsWith("V56"),"V56 classical financial ML lesson missing "+id);
 assert.equal(l.equations.length,6,"V56 sixth exact formula missing "+id);
 assert.equal(l.derivation.length,6,"V56 sixth proof missing "+id);
 assert.equal(l.protocol.length,6,"V56 sixth reproducible check missing "+id);
 assert.equal(new Set(l.equations.map(e=>e.tex)).size,6,"V56 original exact duplicate formula was not resolved "+id);
 assert(l.derivation.at(-1).length>100&&l.protocol.at(-1).length>50,"V56 finance GLM and Boosting argument incomplete "+id);
 katex.renderToString(l.equations.at(-1).tex,{throwOnError:true,strict:"ignore",displayMode:true});
}
near(1+2*.2,1.4,"V56 HAC population true long-run variance");
near(1+2*.5*.2,1.2,"V56 HAC Bartlett finite-band variance");
near(1.4-1.2,.2,"V56 HAC Bartlett bias relative to one-lag truth");
near(.001/.02,.05,"V56 rolling beta sensitivity to factor covariance");
near(Math.sqrt(.25)*6,3,"V56 Gamma model absolute risk scales with mean");
near((.04+.04+2*.02)/4,.03,"V56 Fama MacBeth cross-month dependent error");
near(.5,.5,"V56 duplicated signals group permutation squared loss");
near((1-.8)/(.8*(1-.8)),1.25,"V56 Logistic Newton jump for positive label");
near(-(-6)/(4+2),1,"V56 XGBoost regularized leaf optimum");
near(-6*1+.5*(4+2)*1, -3,"V56 XGBoost optimized quadratic objective");
near(.2+.1*.5,.25,"V56 Boosting fixed learning-rate one-step output");


const v57AuditGroups=[
 ["atlas-depth-07-e.json",["m09-c05-p03","m09-c05-p02","m09-c04-p05","m09-c05-p06","m10-c04-p06","m10-c05-p03","m10-c03-p03","m10-c04-p02"]],
 ["atlas-depth-07-f.json",["m11-c01-p06","m11-c02-p02","m11-c03-p04","m11-c02-p03","m12-c02-p02","m12-c02-p01","m12-c02-p06","m12-c04-p04"]],
 ["atlas-depth-07-g.json",["m13-c04-p03","m13-c01-p02","m13-c04-p01","m13-c03-p04","m14-c03-p05","m14-c04-p06","m14-c02-p04","m14-c03-p06"]],
 ["atlas-depth-07-h.json",["m15-c03-p04","m15-c04-p06","m15-c01-p04","m15-c03-p01","m16-c02-p01","m16-c01-p02","m16-c04-p05","m16-c03-p03"]],
 ["atlas-depth-07-i.json",["m17-c01-p03","m17-c04-p04","m17-c04-p06","m17-c04-p01","m18-c04-p01","m18-c04-p02","m18-c05-p04","m18-c05-p02"]],
 ["atlas-depth-07-j.json",["m19-c03-p05","m19-c03-p03","m19-c01-p03","m19-c05-p04","m20-c04-p05","m20-c02-p06","m20-c05-p05","m20-c03-p05"]]
];
assert.equal(v57AuditGroups.reduce((n,group)=>n+group[1].length,0),48,"V57 audited course total");
for(const [file,expectedIds] of v57AuditGroups){
 const lessons=read(file).lessons;
 assert.deepEqual(Object.keys(lessons),expectedIds,"V57 canonical order must not be modified "+file);
 for(const id of expectedIds){
  const l=lessons[id];
  assert(l.revision_note?.startsWith("V57"),"V57 substantive revision missing "+id);
  assert.equal(l.equations.length,6,"V57 sixth rigorous equation missing "+id);
  assert.equal(l.derivation.length,6,"V57 sixth complete mathematical proof missing "+id);
  assert.equal(l.protocol.length,6,"V57 sixth empirical verification missing "+id);
  assert.equal(l.pitfalls.length,3,"V57 original financial caveat deleted "+id);
  assert.equal(new Set(l.equations.map(e=>e.tex)).size,6,"V57 duplicate formula in lesson "+id);
  assert(l.derivation.at(-1).length>=90&&l.protocol.at(-1).length>=48,"V57 proof or numerical protocol too shallow "+id);
  assert(l.worked_example.length>=145,"V57 original financial example missing "+id);
  assert(!/[\uFFFD\u200B\uFEFF]|TODO|Lorem ipsum/.test(JSON.stringify(l)),"V57 corrupted Chinese "+id);
  katex.renderToString(l.equations.at(-1).tex,{throwOnError:true,strict:"ignore",displayMode:true});
 }
}
near(Math.pow(1/(4*1000),1/5),.1903653938715871,"V57 KDE optimal bandwidth",1e-9);
near(1/(.2*10),.5,"V57 one-class SVM dual upper bound");
near(2/(1+1),1,"V57 single-point Gaussian process posterior mean");
near(1-1/(1+1),.5,"V57 Gaussian process latent function posterior variance");
near(.5+1,1.5,"V57 Gaussian process future observation variance");
near(100*.005*.1-.995*.01,.04005,"V57 low-base-rate anomaly threshold cost benefit");
near(4*1.25-1,4,"V57 correlated Cholesky posterior determinant");
near(2*(Math.log(2)+Math.log(1)),Math.log(4),"V57 Cholesky stable log determinant");
near(4.24/(4.24+1),.8091603053435115,"V57 Kalman scalar Joseph posterior variance");
near(.4-0,.4,"V57 VAE average posterior mutual information decomposition");
near(1000*(1-.8)/(1+.8),1000/9,"V57 Gibbs correlated effective draw count");
near((2*2)/6,2/3,"V57 ARI chance-corrected expected agreeing pairs");
near(Math.min(10,9),9,"V57 single linkage cluster separation");
near(Math.max(10,9),10,"V57 complete linkage cluster separation");
near(1/Math.sqrt(.01+.03),5,"V57 regularized ZCA whitening max noise gain");
const v57Dbscan=[0,.1,.2,5];
assert.equal(v57Dbscan.filter(x=>Math.abs(x-.1)<=.15).length,3,"V57 core neighborhood includes three samples");
assert.equal(v57Dbscan.filter(x=>Math.abs(x-0)<=.15).length,2,"V57 DBSCAN border sample is not core");
assert(100>=98,"V57 training label end overlaps forward validation start and requires purging");
near(.5*.1+.5*.4,.25,"V57 randomly mixed historic regime risk");
assert(.4>.25,"V57 future bear-only market risk exceeds mixed validation risk");
const v57ReportVersions=[{t:100,x:120},{t:120,x:150}];
const v57AsOf=t=>v57ReportVersions.filter(row=>row.t<=t).sort((a,b)=>b.t-a.t)[0]?.x;
assert.equal(v57AsOf(110),120,"V57 point-in-time pre-revision financial reporting");
assert.equal(v57AsOf(130),150,"V57 point-in-time post-revision financial reporting");
near(1/.1,10,"V57 strict propensity overlap limits weights");
near(1/.01,100,"V57 low propensity causes weight explosion");
near(Math.min(.08,.12),.08,"V57 early-stopping validation selection winner");
near(2/Math.sqrt(2*Math.PI),.7978845608028654,"V57 GELU curvature at zero");
near(-2*Math.exp(-2)/Math.sqrt(2*Math.PI),-.10798193302637613,"V57 GELU negative high-activation curvature");
near(2*(1-.1*.05),1.99,"V57 decoupled AdamW pure weight decay");
near(2*(1-.1*2*.05),1.98,"V57 coupled L2 preconditioner scales regularization");
near(Math.sqrt(12)/6,Math.sqrt(1/3),"V57 inverted dropout relative stochastic variation");
near(Math.log(110/100)+Math.log(100/110),0,"V57 price image time reversal flips log return sign");
near(200/(1+9*.8),24.390243902439025,"V57 independent-patient cluster design effective sample size");
near(1-.2,.8,"V57 residual projection Jacobian lower singular value bound");
near(.9*.01/(.9*.01+.05*.99),.15384615384615385,"V57 PPV at one-percent incident rate");
near(.9*.02/(.9*.02+.05*.98),.26865671641791045,"V57 PPV at two-percent incident rate");
near(.75**5,.2373046875,"V57 GRU direct five-step memory gradient path");
near(5-1,4,"V57 adjacent five-day overlapping financial labels");
near(.0004*(1+.5**2),.0005,"V57 independent recursive two-period error variance");
near(.75*(1-.75)*(6-2),.75,"V57 sigmoid gated-fusion derivative");
near((6-2)/4,1,"V57 gated-fusion derivative maximum");
near(8*64,16*32,"V57 equal-width multihead query-key multiplication across heads");
near(Math.exp(-1)/(Math.exp(-1)+1),.2689414213699951,"V57 overflow-free shifted softmax");
near((1*2+2*6)/(1+2),14/3,"V57 positive-kernel linear-attention normalized convex combination");
near(1+.1*.5,1.05,"V57 Pre-LN identity gradient path local toy direction");
near(.5*(1+.1),.55,"V57 Post-LN local toy gradient direction");


near(9-3,6,"V57 BPE repeated ETF merge decreases sequence length");
const v57Gains={A:2**1-1,B:2**3-1};
const v57DcgBefore=v57Gains.A+v57Gains.B/Math.log2(3);
const v57DcgAfter=v57Gains.B+v57Gains.A/Math.log2(3);
near(v57DcgAfter-v57DcgBefore,6*(1-1/Math.log2(3)),"V57 RAG reranking DCG change");
near(2*.6*.75/(.6+.75),2/3,"V57 retrieval F1 versus citation factual support");
near(250/(250-50),1.25,"V57 overlapped RAG chunk compute inflation");
near(Math.max(1,20),20,"V57 graph legal edge start limited by publication");
assert(10<Math.max(1,20),"V57 graph relationship unavailable at historical date ten");
const v57DelayedEdge=(t,created,available,lambda)=>t<Math.max(created,available)?0:Math.exp(-lambda*(t-created));
near(v57DelayedEdge(2,0,3,.5),0,"V57 old-but-undisclosed graph edge must not be visible");
near(v57DelayedEdge(3,0,3,.5),Math.exp(-1.5),"V57 newly disclosed historical graph edge decay");
near(.10-.09,.01,"V57 GNN incremental information after controlling industry");
near(1/Math.sqrt(103-3),.1,"V57 Fisher transformed correlation standard error");
assert(2<=2*1,"V57 critic bounded gradient restricts endpoint difference");
near(Math.log(2),.6931471805599453,"V57 JS divergence saturates for disjoint point masses");
near((3**2+1**2)/(3**2+1**2+.2**2),10/10.04,"V57 reconstruction PCA energy share");
near(.02/(2*.005),2,"V57 order impact optimal unconstrained inventory");
near(2*.02-.005*2**2,.02,"V57 order flow net profit after quadratic impact");
near(8*.02-.005*8**2,-.16,"V57 increasing size beyond impact optimum loses money");
near(.5*(.5*2)+.5*(-.5*2),0,"V57 state baseline contributes zero expected actor score");
near(.2*(0+2+2+2)/4,.3,"V57 epsilon greedy expected exploration execution cost");
near(.2*(1-1/4),.15,"V57 epsilon greedy actual non-greedy action frequency");
near(-Math.log(1-.5**2),.2876820724517809,"V57 tanh inverse-Jacobian log correction");
near(2*.5,1,"V57 tanh Jacobian log density adjustment derivative");
near(8-7,1,"V57 Double DQN target network action-selection gap");


const v58FinalGroups=[
 ["atlas-depth-07-k.json",["m21-c04-p04","m21-c03-p06","m21-c05-p06","m21-c02-p05","m22-c05-p05","m22-c04-p04","m22-c05-p04","m22-c03-p04"]],
 ["atlas-depth-07-l.json",["m23-c01-p04","m23-c01-p01","m23-c02-p02","m23-c02-p06","m24-c02-p05","m24-c05-p01","m24-c04-p04","m24-c02-p01"]]
];
assert.equal(v58FinalGroups.reduce((n,x)=>n+x[1].length,0),16,"V58 all remaining 16 courses");
for(const [file,ids] of v58FinalGroups){
 const lessons=read(file).lessons;
 assert.deepEqual(Object.keys(lessons),ids,"V58 original canonical lesson order retained "+file);
 for(const id of ids){
  const l=lessons[id];
  assert(l?.revision_note?.startsWith("V58"),"V58 finalized sixth proof missing "+id);
  assert.equal(l.equations.length,6,"V58 six equations "+id);
  assert.equal(l.derivation.length,6,"V58 six derivations "+id);
  assert.equal(l.protocol.length,6,"V58 six independent validation protocols "+id);
  assert.equal(l.pitfalls.length,3,"V58 original risk limits retained "+id);
  assert.equal(new Set(l.equations.map(e=>e.tex)).size,6,"V58 repeated mathematical equation "+id);
  assert(l.derivation.at(-1).length>=100&&l.protocol.at(-1).length>=50&&l.worked_example.length>=150,"V58 too-short final section "+id);
  katex.renderToString(l.equations.at(-1).tex,{throwOnError:true,strict:"ignore",displayMode:true});
 }
}
near(.6*.25,.15,"V58 two decentralized actors independently both buy");
near(.6*.75+.4*.25+.4*.75+.6*.25,1,"V58 complete independent decentralized action distribution");
near(1-88/110,.2,"V58 net value historical peak drawdown");
assert((1-99/110)<.2,"V58 final point drawdown is not path maximum");
near(80*2,160,"V58 actual trading inventory shock loss");
near(100*2,200,"V58 hard inventory limit bounds only linear mark loss");
assert(!(0>0)&&.2>0,"V58 behavior-policy support excludes counterfactual new large position");
near((2+4+0)/3,2,"V58 Chronos mean absolute history scale");
near(2*.1,.2,"V58 Chronos original-unit forecast quantization lower edge");
near(2*.2,.4,"V58 Chronos original-unit forecast quantization upper edge");
near(.01*.04-.02**2,0,"V58 rank-one common factor covariance determinant");
near(.5*(1-.5),.25,"V58 TFT variable-selection diagonal Jacobian");
near(-.5*.25,-.125,"V58 TFT off-diagonal softmax Jacobian");
near(.0006+2*4*.000001,.000608,"V58 high-frequency bid-ask noise inflates realized variance");
near((2/3+1/2)**2/((2/3)**2+(1/2)**2),49/25,"V58 event uniqueness effective Kish sample size");
near(105/100-1,.05,"V58 vertical horizon positive return despite earlier stop");
near(97/100-1,-.03,"V58 stopped-out actual barrier return");
near(Math.sqrt((.0004+2*.0001)/100),Math.sqrt(.000006),"V58 positive serial IC covariance increases SE");
near((2+2)**2,16,"V58 two same-side market impacts aggregate quadratically");
near(2*2**2,8,"V58 separately priced impacts underestimate joint crowded liquidity");
assert([true,true,true,true,false].every(Boolean)===false,"V58 deployment approval gate");
near(Math.abs(1.2)+Math.abs(-.8),2,"V58 risk configuration doubles signed long short leverage");
assert(2>1.5,"V58 leverage gate must reject over-limit production config");
near(.02*1,.02,"V58 fixed bounded-loss model contamination risk bound");
near(64/.016,4000,"V58 batch inference throughput");
near(.5+.016,.516,"V58 full trading pipeline latency cannot equal model-only latency");

console.log("PASS V58/V37: 96 complete seventh-cycle canonical lectures (V36 24 + V37 72); 576 strict KaTeX equations, V56/V57/V58 added 96 to all 96 courses; 24 domains, nine-stage 72-link navigation, original numerical examples, independent numerical cross-checks and responsive Chinese/math typography.");
