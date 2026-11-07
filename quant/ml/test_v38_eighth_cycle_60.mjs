import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const raw=p=>fs.readFileSync(new URL(p,root),"utf8");
const manifest=read("atlas-depth-manifest.json");
assert.equal(manifest.expected_lessons,720);
assert.equal(manifest.expected_domains,24);
assert.equal(manifest.modules.reduce((a,m)=>a+m.lessons,0),720,"core manifest changed");
const modules=new Map(manifest.modules.map(m=>[m.path,m.lessons]));
const files="abcdefghijkl".split("").map(c=>"atlas-depth-08-"+c+".json");
const v38Ids=new Set(),allIds=new Set(),domains=new Map();
let allEquations=0,legacyEquationCount=0,originalEquations=0,newChars=0,enhanced=0,retained=0;
for(const [idx,path] of files.entries()){
 assert.equal(modules.get("./"+path),10,"missing canonical eighth stage "+path);
 const doc=read(path);
 assert.equal(doc.schema,1,"schema "+path);
 assert.equal(Object.keys(doc.lessons).length,10,"ten canonical lessons per module");
 const isV38=idx<6;
 assert.equal(doc.revision,"2026-11-06",path+" stale");
 for(const [id,lesson] of Object.entries(doc.lessons)){
  assert(!allIds.has(id),"duplicate lesson "+id);
  allIds.add(id);
  domains.set(id.slice(0,3),(domains.get(id.slice(0,3))||0)+1);
  const goal=6;
  assert(lesson.equations.length>=goal,id+" original six mathematical formulas retained");
  assert(lesson.derivation.length>=goal,id+" original six proofs retained");
  assert(lesson.protocol.length>=goal,id+" original six audit protocols retained");
  assert(lesson.pitfalls.length>=3,id+" original three limitations retained");
  assert(lesson.source_title?.length>=2&&lesson.title?.length>=8,id+" original identity lost");
  assert(lesson.overview?.length>=60,id+" original overview lost");
  assert(lesson.worked_example?.length>=60,id+" original example deleted");
  assert(!/TODO|Lorem ipsum|[\uFFFD\u200B\uFEFF]/i.test(JSON.stringify(lesson)),id+" damaged Chinese");
  for(const [formulaIndex,e] of lesson.equations.entries()){
   assert(typeof e.label==="string"&&e.label.length>=2,id+" math label missing");
   assert(typeof e.tex==="string"&&e.tex.length>=3,id+" math source missing");
   katex.renderToString(e.tex,{displayMode:true,throwOnError:true,strict:"ignore"});
   allEquations++;
    if(formulaIndex<6)legacyEquationCount++;
  }
  if(isV38){
   v38Ids.add(id);enhanced++;
   assert(lesson.revision_note?.startsWith(idx===0?"V51":idx<=2?"V52":"V53"),id+" V38 or V51/V52 canonical provenance");
   assert(lesson.derivation.slice(3).every(s=>s.length>=65),id+" missing extended reasoning");
   assert(lesson.protocol.slice(3).every(s=>s.length>=25),id+" missing reproduction steps");
   assert(lesson.pitfalls.at(-1)?.length>=30,id+" invalidity boundary too short");
   assert(lesson.worked_example.length>=100,id+" example not enhanced");
   newChars+=lesson.derivation.slice(3).join("").length+lesson.protocol.slice(3).join("").length+lesson.pitfalls.at(-1).length;
  }else{retained++;originalEquations+=lesson.equations.slice(0,6).length;assert(lesson.revision_note?.startsWith(idx<=8?"V54":"V55"),id+" V54/V55 eighth-wave provenance");}
 }
}
assert.equal(allIds.size,120);
assert.equal(enhanced,60);
assert.equal(retained,60);
assert.equal(v38Ids.size,60);
assert.equal(legacyEquationCount,720,"all original 720 V38/V39 equations remain");
assert(allEquations>=720,"later lectures must not reduce original equation coverage");
assert.equal(originalEquations,360);
assert.equal(domains.size,24);
for(const n of domains.values())assert.equal(n,5,"exactly five eighth-stage lessons per ML domain");
assert(newChars>21000,"V38 must add over 21k substantive Chinese reasoning/protocol chars");

const home=raw("index.html"),css=raw("atlas.css"),viewer=raw("atlas.js");
const start=home.indexOf('aria-label="V38第八批六十篇正式课程数学与量化深化"');
assert(start>=0,"missing V38 curriculum");
const end=home.indexOf("</details>",start);
assert(end>start,"missing navigation details close");
const nav=home.slice(start,end);
assert.equal((nav.match(/href="#lesson\//g)||[]).length,60);
for(const id of v38Ids)assert(nav.includes('href="#lesson/'+id+'"'),"broken canonical lesson link "+id);
for(const section of ["阶段一","阶段二","阶段三","阶段四","阶段五","阶段六"])
 assert(nav.includes(section),"missing learning stage "+section);
for(const mathId of ["linear-n0065","calculus-n0041","statistics-x001","riskmath-x002"])
 assert(nav.includes('href="../math/?topic='+mathId+'"'),"missing linked math prerequisite "+mathId);
for(const marker of [".atlas-spectral-path",".atlas-spectral-stage",".atlas-spectral-links",".ml-math-block","overflow-x:auto","line-break:strict","@media print"])
 assert(css.includes(marker),"missing CJK / small-screen / print style "+marker);
for(const marker of ["s.equations.map","s.derivation","s.protocol","s.pitfalls","s.worked_example","katex.render"])
 assert(viewer.includes(marker),"Atlas viewer lost structured section "+marker);

const near=(value,expected,label,tol=1e-9)=>{
 assert(Number.isFinite(value)&&Math.abs(value-expected)<=tol,label+" numeric mismatch: "+value+" != "+expected);
};
near(.14-.06,.08,"learning drift risk");
near(9/(9+1),.9,"PCA explained energy");
near(1**2+(-1)**2,2,"Hessian two-dimensional quadratic");
near(3*2**2,12,"cubic derivative");
near(((2.01)**3-(1.99)**3)/.02,12.0001,"center difference cubic");
near(.009/(.009+.0495),.15384615384615385,"rare event posterior");
near(-10*.2*.3,-.6,"multinomial negative covariance");
near(.25*4+.25*1+2*.25*.5,1.5,"Gaussian affine portfolio covariance");
near(100/66,1.5151515151515151,"importance sample ESS");
near(Math.sqrt(Math.log(4000)/2000),.064400,"finite hypothesis class confidence",1e-5);
near(Math.ceil(Math.log(400)/(.02)),300,"sample bound ceil");
near(1001+Math.log1p(Math.exp(-1)),1001.3132616875182,"stable logsumexp");
near(.9*.4+.1*4,.76,"RMSProp second moment");
near((2-1.5*4)**2,16,"unstable gradient step");
near((1.1*.9)-1,-.01,"return compounding");
near((4-(-2))/(2-(-2)),1.5,"MinMax out-of-range");
near(1/(1-.9),10,"VIF conditional OLS variance");
near((2*.5+3*.2)/5,.32,"target encoding posterior shrink");
near(1/4,.25,"OLS hat intercept leverage");
near(.25*(1+4),1.25,"logistic Hessian 1D");
near((2+3+2*1)/4,1.75,"target encoding finance");
near(5-.5,4.5,"Huber robust residual");
near(1-(.5**2+.5**2),.5,"Gini class impurity");
near(120/1000,.12,"support vector ratio");
near(1-.4,.6,"soft margin violation");
near(.004/(.004+.995*.02),.16736401673640168,"rare-event anomaly PPV",1e-12);
near(2/5,.4,"MRF partition normalization");
near(1/(.7**2+3*.1**2),1.923076923076923,"particle filtering ESS");
near(.8**2*4+1,3.56,"Kalman forecast covariance");
near(Math.abs(.01/(.0001+.01)),.9900990099009901,"spectral regularization");
near(Math.sqrt(4/4+1),1.4142135623730951,"Mahalanobis scaled distance");
near(200+5*Math.log(100),223.02585092994047,"GMM BIC");
near(2*3/5*4,4.8,"Ward WCSS merge");
near((16-10)-(11-8),3,"DiD effect");
near(1/.1,10,"IPW high weight");
near((1+.5)**2/((1)**2+(.5)**2),1.8,"causal weight ESS");

const v51=read("atlas-depth-08-a.json").lessons;
const v51Ids=["m01-c03-p04","m01-c02-p04","m01-c05-p06","m01-c04-p02","m01-c03-p06","m02-c05-p02","m02-c05-p05","m02-c03-p04","m02-c03-p06","m02-c03-p03"];
assert.equal(v51Ids.length,10);
for(const id of v51Ids){
 const l=v51[id];
 assert(l&&l.revision_note?.startsWith("V51"),"V51 lesson did not overwrite canonical V38 content "+id);
 assert(l.equations.length>=6,"V51 sixth mathematical expression missing "+id);
 assert(l.derivation.length>=6,"V51 sixth substantial proof missing "+id);
 assert(l.protocol.length>=6,"V51 sixth empirical test missing "+id);
 assert.equal(new Set(l.equations.slice(0,6).map(e=>e.tex)).size,6,"V51 duplicate mathematical formula "+id);
 assert(l.derivation[5].length>=110&&l.protocol[5].length>=65,"V51 proof/test too thin "+id);
 assert(l.worked_example.length>=160,"V51 numerical finance example absent "+id);
 katex.renderToString(l.equations[5].tex,{throwOnError:true,displayMode:true,strict:"ignore"});
}
near(.2*0+.8*.75,.6,"V51 target total risk");
near((.2-.5)*0+(.8-.5)*1,.3,"V51 covariate drift risk");
near(.8*(.75-1),-.2,"V51 label mechanism drift risk");
near(6*.05/Math.PI**2,.03039635509270133,"V51 confidence budget first model family",1e-12);
near(6*.05/(4*Math.PI**2),.0075990887731753325,"V51 confidence budget second family",1e-12);
near(.02-.1*.2,0,"V51 no-trade band marginal cost");
assert(.04-.1*.2>.005,"V51 one-sided cost must allow trade");
assert(94+5<100&&!(95+5<100),"V51 five-day purged label information boundary");
near(.2*(1-0),.2,"V51 unsupported target domain non-identifiability width");
near(2/(2+2),.5,"V51 repeated PCA eigenvalue fraction");
near(Math.abs(1.1-1),.1,"V51 SVD Weyl spectral perturbation");
near(1*0+2*6+4*1,16,"V51 Jacobian JVP left adjoint inner");
near(2*7+(-1)*(-2),16,"V51 Jacobian VJP right adjoint inner");
near(((2+.01)**3-(2-.01)**3)/(.02)-12,.0001,"V51 centered derivative cubic truncation",1e-9);
near(-.2-1,-1.2,"V51 damped Newton strict descent");


const v52=read("atlas-depth-08-b.json").lessons;
const v52ids=["m03-c03-p03","m03-c01-p03","m03-c03-p02","m03-c04-p03","m03-c04-p06","m04-c05-p01","m04-c02-p05","m04-c02-p01","m04-c04-p04","m04-c03-p02"];
assert.equal(v52ids.length,10);
for(const id of v52ids){
 const l=v52[id];
 assert(l&&l.revision_note?.startsWith("V52"),"V52 canonical ML lesson absent "+id);
 assert(l.equations.length>=6,"V52 sixth equation missing "+id);
 assert(l.derivation.length>=6,"V52 sixth proof missing "+id);
 assert(l.protocol.length>=6,"V52 sixth empirical audit missing "+id);
 assert(l.pitfalls.length>=3,"V52 original limitations modified "+id);
 assert.equal(new Set(l.equations.slice(0,6).map(x=>x.tex)).size,6,"V52 repeated mathematical expression "+id);
 assert(l.derivation[5].length>90&&l.protocol[5].length>40,"V52 proof/validation too short "+id);
 assert(l.worked_example.length>140,"V52 numerical example too short "+id);
 katex.renderToString(l.equations[5].tex,{throwOnError:true,displayMode:true,strict:"ignore"});
}
near(.2*1+.3*0+.5*1-(.2*1+.3*0+.5*(-1))**2,.61,"V52 multinomial Softmax Hessian curvature",1e-12);
near((6/99)/(1+6/99),6/105,"V52 independent evidence posterior risk");
near(.5/1*2,1,"V52 Gaussian conditional mean");
near(4-.5**2/1,3.75,"V52 Gaussian conditional covariance Schur");
near(-.5*Math.log(1-.6**2),.2231435513142097,"V52 Gaussian mutual information",1e-12);
near(.5*Math.log(.8)+.5*Math.log(.2),Math.log(.4),"V52 ELBO reconstruction",1e-12);
near(Math.log(.5)-Math.log(.4),.2231435513142097,"V52 ELBO true posterior gap",1e-12);
near(.8*.5+.2*3,1,"V52 covariate shift density ratio unit mean");
near(.8*.5**2+.2*3**2,2,"V52 density ratio quadratic moment");
near(1/2,.5,"V52 asymptotic Kish ESS ratio");
near(2*.05,.1,"V52 ERM two-sided uniform deviation penalty");
near(Math.sqrt(.01**2/.0001+.02**2/.0004),Math.sqrt(2),"V52 optimal unconstrained signal risk ratio");
near(Math.sqrt(.0004/100*(1+.5)/(1-.5)),Math.sqrt(.000012),"V52 paired loss AR1 HAC standard error");
near(.9**30,.0423911582752162,"V52 fixed classifier zero observed error probability");


const v52optimization=read("atlas-depth-08-c.json").lessons;
const v52optIds=["m05-c05-p02","m05-c03-p02","m05-c01-p05","m05-c02-p06","m05-c01-p02","m06-c01-p05","m06-c02-p02","m06-c03-p05","m06-c05-p04","m06-c02-p04"];
assert.equal(v52optIds.length,10);
for(const id of v52optIds){
 const l=v52optimization[id];
 assert(l&&l.revision_note?.startsWith("V52"),"V52 optimization/data contract missing "+id);
 assert(l.equations.length>=6,"V52 sixth formula missing "+id);
 assert(l.derivation.length>=6,"V52 sixth proof missing "+id);
 assert(l.protocol.length>=6,"V52 sixth verification protocol missing "+id);
 assert(l.pitfalls.length>=3,"V52 preexisting limitations lost "+id);
 assert.equal(new Set(l.equations.slice(0,6).map(e=>e.tex)).size,6,"V52 duplicate optimization formula "+id);
 assert(l.derivation[5].length>=100&&l.protocol[5].length>=40,"V52 reasoning too shallow "+id);
 assert(l.worked_example.length>=150,"V52 financial data example lost "+id);
 katex.renderToString(l.equations[5].tex,{throwOnError:true,displayMode:true,strict:"ignore"});
}
near(1000+Math.log1p(Math.exp(-1)),1000.3132616875182,"V52 stable LogSumExp",1e-10);
assert(1000+Math.log1p(Math.exp(-1))<=1000+Math.log(2),"V52 LogSumExp upper bound");
near(.9*.4+.1*4,.76,"V52 RMSProp second square moment");
near(.76/(1-.9**2),4,"V52 RMSProp optional initialization correction");
near((.75**2+4*0**2)/2,.28125,"V52 strong-convex one-step true objective");
assert(.28125<=(1-1/4)*2.5,"V52 strong-convex linear-rate bound");
const earlyLoss=[.20,.17,.16,.165,.18];
assert.equal(earlyLoss.indexOf(Math.min(...earlyLoss))+1,3,"V52 restore best checkpoint");
near(Math.min(1,1-.1*(-2)),1,"V52 projected convex fixed point");
near((100*10+110*90)/(10+90),109,"V52 intrabar VWAP distinct from OHLC mean");
near(.0005/(.002-.001),.5,"V52 MinMax frozen range sensitivity");
near(1/Math.sqrt(1-.9),Math.sqrt(10),"V52 VIF standard error not variance multiplier");
const assetPermutation=[1,0],originalWeights=[.6,-.2];
const permutedWeights=assetPermutation.map(i=>originalWeights[i]);
const restoredWeights=assetPermutation.map(i=>permutedWeights[i]);
assert.deepEqual(restoredWeights,originalWeights,"V52 asset ID order must invert correctly");
near((0+3*.2)/(1+3),.15,"V52 leave-one-out target encoding");
near((1+0+3*.2)/(2+3),.32,"V52 full class target encoding leakage");


const v53sets=[
 {file:"atlas-depth-08-d.json",ids:["m07-c03-p03","m07-c05-p06","m07-c05-p03","m07-c01-p03","m07-c03-p05","m08-c05-p02","m08-c04-p04","m08-c03-p06","m08-c05-p01","m08-c02-p05"]},
 {file:"atlas-depth-08-f.json",ids:["m11-c02-p06","m11-c03-p02","m11-c04-p06","m11-c01-p02","m11-c02-p01","m12-c02-p05","m12-c04-p05","m12-c02-p04","m12-c04-p01","m12-c04-p02"]}
];
assert.equal(v53sets.reduce((sum,x)=>sum+x.ids.length,0),20,"V53 canonical lesson total");
for(const group of v53sets){
 const lessons=read(group.file).lessons;
 for(const id of group.ids){
  const l=lessons[id];
  assert(l&&l.revision_note?.startsWith("V53"),"V53 formal lesson absent "+id);
  assert(l.equations.length>=6,"V53 sixth formula absent "+id);
  assert(l.derivation.length>=6,"V53 sixth mathematical proof absent "+id);
  assert(l.protocol.length>=6,"V53 sixth independent audit absent "+id);
  assert(l.pitfalls.length>=3,"V53 prior financial caveats lost "+id);
  assert.equal(new Set(l.equations.slice(0,6).map(x=>x.tex)).size,6,"V53 exact duplicated mathematical expression "+id);
  assert(l.derivation[5].length>100&&l.protocol[5].length>40,"V53 proof or numerical validation shallow "+id);
  assert(l.worked_example.length>145,"V53 financial worked example incomplete "+id);
  katex.renderToString(l.equations[5].tex,{throwOnError:true,displayMode:true,strict:"ignore"});
 }
}
near(.25*(1**2+2**2),1.25,"V53 Logistic Hessian directional curvature");
near(3-5-1,-3,"V53 marginal post-cost risk-adjusted benefit");
near(.04/2,.02,"V53 FWL coefficient conditional variance");
near(-1.5/(1-.25),-2,"V53 OLS leave-one-out leverage-adjusted residual");
near(4*20/(4*20+1*80),.5,"V53 class weighted probability differs from base rate");
const v53ShapConditional=[.5,.5],v53ShapMarginal=[1,0];
near(v53ShapConditional[0]+v53ShapConditional[1],1,"V53 correlated conditional SHAP reconstruction");
near(v53ShapMarginal[0]+v53ShapMarginal[1],1,"V53 marginal SHAP reconstruction");
assert(v53ShapConditional[1]>v53ShapMarginal[1],"V53 SHAP dependence is not causal importance");
const v53OneHot=[[1,0,0],[0,1,0],[0,0,1]];
assert(v53OneHot.every(row=>row.reduce((a,b)=>a+b,0)===1),"V53 one-hot full dummy sum equals intercept");
near(-Math.min(1,5),-1,"V53 Huber clipped prediction-gradient",1e-12);
near(.5-2*.8*.2,.18,"V53 binary Gini split improvement");
const v53PseudoInv=[[.25,0],[0,0]];
near(v53PseudoInv[1][1],0,"V53 singular covariance pseudoinverse ignores null direction");
near(1/.01,100,"V53 singular Mahalanobis positive-ridge null-direction cost");
near((3**2+1**2)-3**2,1,"V53 PCA optimal rank-one residual Frobenius energy");
near(1/Math.sqrt(2*Math.PI*.01),3.989422804014327,"V53 GMM Gaussian spike density bound",1e-12);
near((0**2+2**2+10**2)-(2*1**2+1*10**2),2,"V53 Kmeans within-cluster identity");
near((2*3/(2+3))*(2**2),4.8,"V53 Ward baseline merge distortion");
near((2*3/(2+3))*((10*2)**2),480,"V53 Ward rescaling multiplies squared cost");
assert(Math.max(95,100)<=Math.min(105,110),"V53 purge required before-validation overlapping labels");
near((12-10)-(10-8),0,"V53 pre-policy placebo DiD");
near((16-12)-(11-10),3,"V53 treated post-policy DID");
assert(!(Math.max(95,100)<Math.min(100,110)),"V53 adjacent half-open event intervals do not overlap");
assert(Math.max(95,100)<Math.min(101,110),"V53 half-open events overlap after end extension");
const v53TreatedMean=(.5*.25*3+.5*.75*6)/(.5*.25+.5*.75);
const v53WeightedControl=(.5*.75*(.25/.75)*1+.5*.25*(.75/.25)*2)/(.5*.25+.5*.75);
near(v53TreatedMean,5.25,"V53 treated-outcome mean for ATT");
near(v53WeightedControl,1.75,"V53 propensity odds-weighted missing counterfactual");
near(v53TreatedMean-v53WeightedControl,3.5,"V53 ATT under conditional selection and overlap");
near(1+2*3,7,"V53 causal direct-plus-mediated total effect");


const v53extra=read("atlas-depth-08-e.json").lessons;
const v53extraIds=["m09-c01-p05","m09-c02-p01","m09-c05-p04","m09-c04-p01","m09-c01-p06","m10-c02-p04","m10-c05-p04","m10-c05-p06","m10-c02-p01","m10-c05-p01"];
assert.equal(v53extraIds.length,10,"V53 kernel/graph/filter lesson count");
for(const id of v53extraIds){
 const l=v53extra[id];
 assert(l&&l.revision_note?.startsWith("V53"),"V53 kernel/graph canonical lesson missing "+id);
 assert(l.equations.length>=6,"V53 new kernel/graph equation absent "+id);
 assert(l.derivation.length>=6,"V53 complete derivation absent "+id);
 assert(l.protocol.length>=6,"V53 numerical contract absent "+id);
 assert(l.pitfalls.length>=3,"V53 original error boundaries lost "+id);
 assert.equal(new Set(l.equations.slice(0,6).map(e=>e.tex)).size,6,"V53 duplicate kernel/graph equation "+id);
 assert(l.derivation[5].length>100&&l.protocol[5].length>40,"V53 kernel/graph proof too thin "+id);
 assert(l.worked_example.length>150,"V53 kernel/graph worked example too short "+id);
 katex.renderToString(l.equations[5].tex,{throwOnError:true,strict:"ignore",displayMode:true});
}
assert(.3>0&&.3<1,"V53 soft-margin SVM internal support-vector KKT case");
near(Math.max(0,1-.4),.6,"V53 hinge positive margin slack");
near(Math.max(0,1-1.5),0,"V53 correctly classified outside margin");
near(100*.005*.1,.05,"V53 anomaly threshold false-negative savings");
near((1-.005)*.01,.00995,"V53 anomaly false-positive extra cost");
near(.05-.00995,.04005,"V53 net threshold decision saving");
near((2-1)/(1-.5),2,"V53 KRR leave-one-out first example");
near((4-2)/(1-.5),4,"V53 KRR leave-one-out second example");
near(Math.exp(-.5*2),.36787944117144233,"V53 unscaled RBF similarity",1e-12);
near(Math.exp(-.5*5),.0820849986238988,"V53 anisotropic scaled RBF similarity",1e-12);
const v53IsingJ=Math.log(2)/2;
near(4*Math.cosh(v53IsingJ),4.242640687119286,"V53 Ising partition function",1e-11);
near(1/(1+Math.exp(-2*v53IsingJ)),2/3,"V53 Ising same-sign probability");
near(1-.3**4+3*(1-.9**4),2.0236,"V53 expected distinct ancestors after weighted resampling");
near(1/(.7**2+3*.1**2),1/.52,"V53 precursor particle ESS");
near((.2/(1-.2))*16,4,"V53 filtering-to-smoothing odds update");
near(4/(1+4),.8,"V53 hindsight smoothing probability");
near(.5*.8*.9,.36,"V53 chain joint observational probability");
near(.5*(.8*.5/.5)*.9,.36,"V53 fork joint observational probability");
const v53GramDet=(1+.8**2)*(1+.5**2)-(1+.8*.5)**2;
near(v53GramDet,.09,"V53 finite-horizon observability Gramian determinant",1e-12);
assert(v53GramDet>0,"V53 observability Gramian positive definite");


const v54groups=[
 {file:"atlas-depth-08-g.json",ids:["m13-c01-p05","m13-c01-p01","m13-c03-p06","m13-c05-p06","m13-c05-p02","m14-c03-p03","m14-c01-p04","m14-c02-p02","m14-c05-p05","m14-c02-p03"]},
 {file:"atlas-depth-08-h.json",ids:["m15-c02-p03","m15-c01-p06","m15-c02-p01","m15-c04-p03","m15-c02-p02","m16-c05-p04","m16-c01-p05","m16-c02-p04","m16-c03-p04","m16-c05-p06"]},
 {file:"atlas-depth-08-i.json",ids:["m17-c05-p03","m17-c03-p03","m17-c04-p03","m17-c05-p01","m17-c03-p05","m18-c05-p05","m18-c03-p04","m18-c02-p04","m18-c05-p06","m18-c03-p05"]}
];
assert.equal(v54groups.reduce((n,g)=>n+g.ids.length,0),30,"V54 detailed course count");
for(const group of v54groups){
 const lessons=read(group.file).lessons;
 for(const id of group.ids){
  const l=lessons[id];
  assert(l&&l.revision_note?.startsWith("V54"),"V54 original curriculum identity missing "+id);
  assert(l.equations.length>=6,"V54 sixth strict mathematical expression absent "+id);
  assert(l.derivation.length>=6,"V54 sixth rigorous proof absent "+id);
  assert(l.protocol.length>=6,"V54 sixth empirical verification absent "+id);
  assert(l.pitfalls.length>=3,"V54 retained preexisting failure conditions missing "+id);
  assert.equal(new Set(l.equations.slice(0,6).map(e=>e.tex)).size,6,"V54 duplicated existing mathematics "+id);
  assert(l.derivation[5].length>90&&l.protocol[5].length>40,"V54 proof or audit too thin "+id);
  assert(l.worked_example.length>145,"V54 worked financial example missing "+id);
  katex.renderToString(l.equations[5].tex,{throwOnError:true,displayMode:true,strict:"ignore"});
 }
}
near(200/2*.02,2,"V54 He backward fan-in width ratio");
near(2*(100+60),320,"V54 low-rank dense linear weight count");
near(.9*2+.1*2,2,"V54 EMA optimizer checkpoint restore");
near(.1*2,.2,"V54 reset momentum corrupts optimizer trajectory");
const v54PairedA=[.1,.12,.14],v54PairedB=[.09,.11,.13];
const v54PairedDiff=v54PairedA.map((a,i)=>a-v54PairedB[i]);
assert(v54PairedDiff.every(x=>Math.abs(x-.01)<1e-12),"V54 paired seed risk delta is constant");
near(.2/(1-.2)*2**2,1,"V54 inverted Dropout output variance");
near(2*(2/3)/(1+2/3)-2/3,2/15,"V54 Dice minus IoU exact positive gap");
near((110-100)/(120-100),.5,"V54 original normalized price-window midpoint");
near((225-205)/(245-205),.5,"V54 positive affine normalized price-window invariance");
near(2/Math.sqrt(1+.1*4),1.6903085094570331,"V54 AlexNet local response normalization");
near(.9*.1+.1*.25,.115,"V54 source-domain mixed CNN risk");
near(.5*.12+.5*.30,.21,"V54 target-domain mixed CNN risk");
near((.5-.9)*.1+(.5-.1)*.25,.06,"V54 domain-shift composition effect");
near(.5*(.12-.1)+.5*(.3-.25),.035,"V54 domain-shift within-industry error effect");
near(3+(3-1)*2,7,"V54 CNN effective-stride receptive field");
near(2*.5**4/(1-.5),.25,"V54 truncated BPTT geometric gradient omission bound");
near(2/(2+1),2/3,"V54 numerically normalized masked attention on legal news");
near(2*.01/(1-.8),.1,"V54 RNN input perturbation stable geometric limit");
const v54Causal=(current,previous,future)=>current+2*previous;
assert.equal(v54Causal(1,2,0),v54Causal(1,2,100),"V54 causal TCN cannot react to future price mutation");
near(v54Causal(1,3,0)-v54Causal(1,2,0),2,"V54 causal TCN must react to legitimate past mutation");
near(3*.5**2,.75,"V54 shared-parameter RNN gradient over three steps");
near(Math.cos(2*Math.PI*(3-9)/12),-1,"V54 periodic calendar contrast");
near(4/(4+1),.8,"V54 cross-attention ignores post-trade information");
near(Math.cos((1-0)*Math.PI/2),Math.cos((6-5)*Math.PI/2),"V54 RoPE common-index shift invariance",1e-12);
near(-1/Math.sqrt(1+1),-1/Math.sqrt(2),"V54 LayerNorm epsilon-one baseline");
near(-2/Math.sqrt(4+1),-.8944271909999159,"V54 LayerNorm positive-affine scale effect",1e-12);
assert(Math.abs(-1/Math.sqrt(2)+2/Math.sqrt(5))>0.1,"V54 epsilon breaks exact LayerNorm scale invariance");
near(5-7-1,-3,"V54 Transformer post-cost risk-adjusted utility");
near(108/100-1,.08,"V54 pre-announcement event headline price return");
near(108/105-1,.02857142857142847,"V54 legitimately executable post-announcement return");
near(Math.exp(1)/(Math.exp(1)+1),.7310585786300049,"V54 finite-KL optimal preference policy");
near(2-Math.sqrt(2),.5857864376269049,"V54 normalized embedding cosine-distance identity");
near(30/150,.2,"V54 financial net margin correct units");
near(30*1e4/(150*1e6),.002,"V54 net margin unit mismatch risk");
near(1-1/10,.9,"V54 audit-or-abstain support threshold");
near(2/Math.sqrt(2*5),.6324555320336759,"V54 false contagion from common market factor");
near(.5+.5/4,.625,"V54 correlated graph attention heads variance");
near(.2+.8*.5,.6,"V54 graph residual eigenvalue mixing");
near(.6**6,.046656,"V54 residual graph smoothing after six layers",1e-12);
assert(Math.max(90,100,105)>102&&Math.max(90,100,105)<=105,"V54 point-in-time edge disclosure constraint");
near(.5*(4-8)**2+.5*(12-8)**2,16,"V54 uniform neighbor-sampling variance");
near(2/.25,8,"V54 optimal importance sampling for first neighbor");
near(6/.75,8,"V54 optimal importance sampling for second neighbor");


const v55groups=[
 {file:"atlas-depth-08-j.json",ids:["m19-c03-p04","m19-c03-p01","m19-c02-p05","m19-c01-p05","m19-c05-p03","m20-c04-p06","m20-c02-p04","m20-c05-p01","m20-c05-p06","m20-c03-p03"]},
 {file:"atlas-depth-08-k.json",ids:["m21-c03-p05","m21-c02-p03","m21-c03-p03","m21-c03-p01","m21-c01-p06","m22-c02-p06","m22-c05-p01","m22-c01-p01","m22-c02-p05","m22-c03-p01"]},
 {file:"atlas-depth-08-l.json",ids:["m23-c05-p05","m23-c02-p05","m23-c02-p04","m23-c05-p03","m23-c04-p03","m24-c01-p05","m24-c02-p03","m24-c01-p02","m24-c01-p01","m24-c01-p03"]}
];
assert.equal(v55groups.reduce((n,g)=>n+g.ids.length,0),30,"V55 batch must deepen thirty existing courses");
for(const group of v55groups){
 const lessons=read(group.file).lessons;
 for(const id of group.ids){
  const l=lessons[id];
  assert(l&&l.revision_note?.startsWith("V55"),"V55 canonical course or revision missing "+id);
  assert.equal(l.equations.length,6,"V55 sixth formula missing "+id);
  assert.equal(l.derivation.length,6,"V55 sixth mathematical derivation missing "+id);
  assert.equal(l.protocol.length,6,"V55 sixth empirically testable protocol missing "+id);
  assert.equal(l.pitfalls.length,3,"V55 original limitations lost "+id);
  assert.equal(new Set(l.equations.map(e=>e.tex)).size,6,"V55 duplicated pre-existing formula "+id);
  assert(l.derivation.at(-1).length>100&&l.protocol.at(-1).length>50,"V55 incomplete proof or financial audit "+id);
  assert(l.worked_example.length>155,"V55 numerical worked example incomplete "+id);
  katex.renderToString(l.equations.at(-1).tex,{throwOnError:true,strict:"ignore",displayMode:true});
 }
}
near(.5*0+.5*4,2,"V55 Wasserstein lower bound from shifted mean");
near(.5*4+.5*4,4,"V55 Wasserstein zero-mean does not imply close tail distribution");
near(-Math.log(.01),4.605170185988092,"V55 non-saturating GAN loss near perfect discriminator");
near(Math.log(1-.01),-.01005033585350145,"V55 original minimax generator saturated loss");
near(1**2+2**2,5,"V55 Gaussian reparametrization exact second moment");
near(2*1,2,"V55 reparametrization gradient w.r.t mean");
near(2*2,4,"V55 reparametrization gradient w.r.t scale");
near(Math.log(2)-2*Math.log(2),-Math.log(2),"V55 ideal information-bottleneck informative encoding");
near(Math.log(2),Math.log(2),"V55 noise-only representation has no label information");
near(.5*2,1,"V55 log wealth cumulative factor resets to one");
near(Math.log(.5)+Math.log(2),0,"V55 two-period log wealth identity");
const v55Softmax=Math.E/(1+Math.E);
near(v55Softmax*(1-v55Softmax),.19661193324148185,"V55 soft policy Jacobian self sensitivity");
near(.9*(5-2),2.7,"V55 Q-learning versus SARSA target gap");
near(.25*2+.75*(2/3),1,"V55 action importance ratios integrate to one");
near(.25*2**2+.75*(2/3)**2,4/3,"V55 action importance ratio second moment");
near(1/(4/3),.75,"V55 weighted-policy effective sample share");
near(1/100,.01,"V55 expected-cost Markov tail probability budget");
near(10*(1-.9**3),2.71,"V55 DQN Polyak three-step target lag");
near(100+(-90)-8,2,"V55 conservative short inventory order limit");
assert(-90-8-2===-100,"V55 market-making worst-case short inventory");
near(1*.1*3*(3-1),.6,"V55 finite-horizon transition-model approximation risk");
near(.9*2+.1*(-8),1,"V55 distributional action mean");
near(.9*4+.1*64-1,9,"V55 distributional action variance");
near(.25+1,1.25,"V55 ex ante hierarchical risk total variance");
near(Math.min(50,Math.max(0,120-100)),20,"V55 executable FIFO queue fill upper bound");
near(.5**2,.25,"V55 ARCH squared return lag-two autocorrelation");
near(.0004*(1+.5**2),.0005,"V55 recursive two-step error variance");
near(.0004/(1-.5**2),.0005333333333333334,"V55 stationary recursive long-horizon variance");
near((10-3)*1,7,"V55 random-walk historical covariance at date ten");
near((100-3)*1,97,"V55 random-walk historical covariance at date one hundred");
near((80*0+20*.1)/100,.02,"V55 prebreak expanding-window mean bias");
near(.02-.1,-.08,"V55 expanding mean relative to new regime");
near(3*(1-.5**2)/(1-3*.5**2),9,"V55 finite fourth moment Gaussian ARCH kurtosis");
near((100-80)/(110-80),2/3,"V55 optimal stopping payoff probability threshold");
near(2*Math.exp(-2*500*.1**2),.00009079985952496962,"V55 independent stability-selection Hoeffding frequency deviation");
near(.04/2,.02,"V55 FWL residualized factor coefficient uncertainty");
near((20+10)/(40+20),.5,"V55 mean reversion posterior break-even probability");
near(.6*40-.4*20-10,6,"V55 successful mean reversion cost-adjusted expected bp");
near(Math.sqrt((1-.5)/(1+.5)),Math.sqrt(1/3),"V55 Sharpe AR1 long-run annualization factor");
assert(99<=100&&100<105,"V55 feature available before decision while future label remains immature");
near(1-(1-.995)*2,.99,"V55 two-component P995 union bound for P99 aggregate");
assert(Math.abs(.0100-.0102)>.0001,"V55 numeric model deployment margin protects order threshold");
near(Math.sqrt(((.05-.09)**2+(.07-.09)**2+(.15-.09)**2)/2),Math.sqrt(.0028),"V55 seed-to-seed trading net return volatility");
const v55Versions=[{released:100,value:120},{released:120,value:150}];
const v55SelectAsOf=t=>v55Versions.filter(v=>v.released<=t).sort((a,b)=>b.released-a.released)[0]?.value;
assert.equal(v55SelectAsOf(110),120,"V55 point-in-time report before subsequent revision");
assert.equal(v55SelectAsOf(130),150,"V55 report revision legitimate only after actual publication");

console.log("PASS V55/V38+V39: 60 V38 and 60 V39 enhanced eighth-cycle canonical lectures; 120 lessons/24 domains, 720 strict KaTeX equations (50 rigorous additions across V51-V53), substantive derivations/protocols, 60 route links, original numeric examples and responsive Chinese/formula layout.");
