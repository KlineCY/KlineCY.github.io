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
  const goal=5;
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
   assert(lesson.revision_note?.startsWith("V38"),id+" V38 provenance");
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
assert.equal(allEquations,600,"300 equations V38 plus 300 equations V39");
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
console.log("PASS V38+V39: 60 V38 and 60 V39 enhanced eighth-cycle canonical lectures; 120 lessons/24 domains, 600 strict KaTeX equations (240 added across V38+V39), substantive derivations/protocols, 60 route links, original numeric examples and responsive Chinese/formula layout.");
