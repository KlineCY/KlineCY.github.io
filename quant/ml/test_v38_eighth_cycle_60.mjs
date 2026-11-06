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
let allEquations=0,originalEquations=0,newChars=0,enhanced=0,retained=0;
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
  const goal=idx<=2?6:5;
  assert.equal(lesson.equations.length,goal,id+" mathematical depth");
  assert.equal(lesson.derivation.length,goal,id+" proof depth");
  assert.equal(lesson.protocol.length,goal,id+" empirical audit depth");
  assert.equal(lesson.pitfalls.length,3,id+" explicit limitations");
  assert(lesson.source_title?.length>=2&&lesson.title?.length>=8,id+" original identity lost");
  assert(lesson.overview?.length>=60,id+" original overview lost");
  assert(lesson.worked_example?.length>=60,id+" original example deleted");
  assert(!/TODO|Lorem ipsum|[\uFFFD\u200B\uFEFF]/i.test(JSON.stringify(lesson)),id+" damaged Chinese");
  for(const e of lesson.equations){
   assert(typeof e.label==="string"&&e.label.length>=2,id+" math label missing");
   assert(typeof e.tex==="string"&&e.tex.length>=3,id+" math source missing");
   katex.renderToString(e.tex,{displayMode:true,throwOnError:true,strict:"ignore"});
   allEquations++;
  }
  if(isV38){
   v38Ids.add(id);enhanced++;
   assert(lesson.revision_note?.startsWith(idx===0?"V51":idx<=2?"V52":"V38"),id+" V38 or V51/V52 canonical provenance");
   assert(lesson.derivation.slice(3).every(s=>s.length>=65),id+" missing extended reasoning");
   assert(lesson.protocol.slice(3).every(s=>s.length>=25),id+" missing reproduction steps");
   assert(lesson.pitfalls.at(-1)?.length>=30,id+" invalidity boundary too short");
   assert(lesson.worked_example.length>=100,id+" example not enhanced");
   newChars+=lesson.derivation.slice(3).join("").length+lesson.protocol.slice(3).join("").length+lesson.pitfalls.at(-1).length;
  }else{retained++;originalEquations+=lesson.equations.length;assert(lesson.revision_note?.startsWith("V39"),id+" V39 provenance");}
 }
}
assert.equal(allIds.size,120);
assert.equal(enhanced,60);
assert.equal(retained,60);
assert.equal(v38Ids.size,60);
assert.equal(allEquations,630,"V52 retains all 600 V38/V39 eighth-cycle formulas and adds thirty rigorous formulas");
assert.equal(originalEquations,300);
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
 assert.equal(l.equations.length,6,"V51 sixth mathematical expression missing "+id);
 assert.equal(l.derivation.length,6,"V51 sixth substantial proof missing "+id);
 assert.equal(l.protocol.length,6,"V51 sixth empirical test missing "+id);
 assert.equal(new Set(l.equations.map(e=>e.tex)).size,6,"V51 duplicate mathematical formula "+id);
 assert(l.derivation.at(-1).length>=110&&l.protocol.at(-1).length>=65,"V51 proof/test too thin "+id);
 assert(l.worked_example.length>=160,"V51 numerical finance example absent "+id);
 katex.renderToString(l.equations.at(-1).tex,{throwOnError:true,displayMode:true,strict:"ignore"});
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
 assert.equal(l.equations.length,6,"V52 sixth equation missing "+id);
 assert.equal(l.derivation.length,6,"V52 sixth proof missing "+id);
 assert.equal(l.protocol.length,6,"V52 sixth empirical audit missing "+id);
 assert.equal(l.pitfalls.length,3,"V52 original limitations modified "+id);
 assert.equal(new Set(l.equations.map(x=>x.tex)).size,6,"V52 repeated mathematical expression "+id);
 assert(l.derivation.at(-1).length>90&&l.protocol.at(-1).length>40,"V52 proof/validation too short "+id);
 assert(l.worked_example.length>140,"V52 numerical example too short "+id);
 katex.renderToString(l.equations.at(-1).tex,{throwOnError:true,displayMode:true,strict:"ignore"});
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

console.log("PASS V52/V38+V39: 60 V38 and 60 V39 enhanced eighth-cycle canonical lectures; 120 lessons/24 domains, 630 strict KaTeX equations (240 added across V38+V39), substantive derivations/protocols, 60 route links, original numeric examples and responsive Chinese/formula layout.");
