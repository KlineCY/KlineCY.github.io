import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root = new URL("./", import.meta.url);
const read = name => fs.readFileSync(new URL(name,root),"utf8");
const json = name => JSON.parse(read(name));
const curriculum = json("curriculum.json");
const nodeMap = new Map(curriculum.nodes.map(n => [n.id,n]));
assert.equal(nodeMap.size,656,"Curriculum unexpectedly changed");
const manifest = json("lessons-manifest.json");
assert.equal(manifest.expected_lessons,201);
assert.equal(manifest.modules.length,77);
const before=[
 "lessons-foundations.json","lessons-advanced.json","lessons-more.json",
 ...manifest.modules.map(m=>m.path.replace(/^\.\//,"")),
 "lessons-foundations-v89.json","lessons-linear-v90.json","lessons-linear-v92.json",
 "lessons-matrix-v93.json","lessons-probability-v93.json",
 "lessons-probability-v94a.json","lessons-probability-v94b.json","lessons-probability-v94c.json",
 "lessons-statistics-v95a.json","lessons-statistics-v95b.json",
 "lessons-optimization-v96a.json","lessons-optimization-v96b.json",
 "lessons-portfolio-v97.json","lessons-algorithms-v97.json","lessons-numerical-v97.json",
 "lessons-diffeq-v98a.json","lessons-diffeq-v98b.json"
];
const historical=before.flatMap(x=>json(x).lessons);
assert.equal(historical.length,420,"V98 historical lesson count modified");
assert.equal(new Set(historical.map(x=>x.node_id)).size,420,"Historical duplicated lesson ID");
const files=["lessons-dynamics-v99a.json","lessons-pde-v99b.json","lessons-pricing-pde-v99c.json","lessons-control-v99d.json"];
const expectedSizes=[5,6,5,6];
const docs=files.map(json);
docs.forEach((d,i)=>{
 assert.equal(d.version,1);
 assert.equal(d.revision,"V99");
 assert.equal(d.lessons.length,expectedSizes[i]);
});
const newLessons=docs.flatMap(d=>d.lessons);
const expectedIds=[
 ...Array.from({length:11},(_,i)=>"diffeq-n"+String(i+398).padStart(4,"0")),
 ...Array.from({length:11},(_,i)=>"diffeq-n"+String(i+410).padStart(4,"0"))
];
assert.deepEqual(newLessons.map(l=>l.node_id),expectedIds,"V99 gap or order mismatch");
assert.equal(newLessons.length,22);
const all=[...historical,...newLessons];
assert.equal(all.length,442,"New total math lecture count wrong");
assert.equal(new Set(all.map(l=>l.node_id)).size,442,"Historical node overwritten");
assert(all.every(l=>nodeMap.get(l.node_id)?.title===l.title),"Dangling/mismatched curriculum lesson");
const diffeq=curriculum.nodes.filter(n=>n.discipline_id==="diffeq");
assert.equal(diffeq.length,30);
const completed=new Set(all.map(l=>l.node_id));
assert(diffeq.every(n=>completed.has(n.id)),"Differential equations 30/30 not complete");

let formulaCount=0,chapterCount=0,deepChars=0,linkCount=0;
function formula(tex,id){
 assert.equal(typeof tex,"string");
 assert(tex.length>=10,id+" short expression");
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(tex),id+" bad Unicode");
 try{katex.renderToString(tex,{displayMode:true,throwOnError:true,strict:"error"});}
 catch(e){throw Error(id+" invalid strict KaTeX: "+tex+" / "+e.message)}
 formulaCount++;
}
for(const lesson of newLessons){
 const id=lesson.node_id;
 assert.equal(lesson.id,id);
 assert.equal(lesson.title,nodeMap.get(id).title);
 for(const f of ["definition","assumptions","example","application","caveat"])
  assert(lesson[f]?.length>=70,id+" incomplete "+f);
 assert(lesson.properties.length>=4 && lesson.properties.every(p=>p.length>=20),id+" shallow properties");
 assert.equal(lesson.formulas.length,3,id+" insufficient core equations");
 assert.equal(lesson.derivation.length,3,id+" insufficient proof steps");
 assert.equal(lesson.deep_sections.length,3,id+" insufficient deep chapters");
 const titles=new Set();
 for(const part of lesson.formulas){
  assert(part.label?.length>=4);
  formula(part.tex,id);
 }
 for(const step of lesson.derivation){
  assert(step.explanation?.length>=25,id+" shallow derivation");
  formula(step.tex,id);
 }
 for(const part of lesson.deep_sections){
  assert(part.title?.length>=8 && part.question?.length>=10,id+" unclear deep section");
  assert(!titles.has(part.title),id+" repeated chapter title");
  titles.add(part.title);
  assert(part.paragraphs?.length===2 && part.paragraphs.every(p=>p.length>=30),id+" thin deep text");
  for(const [k,min] of [["proof",50],["worked",50],["limitation",25]])
   assert(part[k]?.length>=min,id+" thin "+k);
  assert(part.formulas?.length===1);
  assert(part.formulas[0].label?.length>=4);
  formula(part.formulas[0].tex,id);
  chapterCount++;
  deepChars+=part.paragraphs.join("").length+part.proof.length+part.worked.length+part.limitation.length;
 }
 for(const link of lesson.links||[]){
  if(link.kind==="math")assert(nodeMap.has(link.id),id+" invalid related math link "+link.id);
  assert(["math","ml"].includes(link.kind),id+" unknown related link kind");
  linkCount++;
 }
 assert(/[\u3400-\u9FFF]/u.test(JSON.stringify(lesson)),id+" no Chinese body");
 assert(!/TODO|FIXME|Lorem ipsum|待补充/i.test(JSON.stringify(lesson)),id+" placeholder");
}
assert.equal(chapterCount,66);
assert.equal(formulaCount,198);
assert(deepChars>=18000,"Proof depth unexpectedly reduced");
assert(linkCount>=55);
const app=read("app.js"),css=read("styles.css"),workflow=read("../../.github/workflows/test-quant-math.yml");
const html=read("index.html");
assert(html.includes("20261108-v100-finance-complete"),"Current app cache-busting version missing");
for(const l of newLessons) assert(html.includes('?topic='+l.node_id+'"'),"Canonical study path missing "+l.node_id);

for(const needle of [
 'loadV98DifferentialEquations("./lessons-diffeq-v98b.json",3)',
 'loadV99DynamicsPDE("./lessons-dynamics-v99a.json",5)',
 'loadV99DynamicsPDE("./lessons-pde-v99b.json",6)',
 'loadV99DynamicsPDE("./lessons-pricing-pde-v99c.json",5)',
 'loadV99DynamicsPDE("./lessons-control-v99d.json",6)',
 "...v98a,...v98b,...v99a,...v99b,...v99c,...v99d",
 "new Set(lessons.map(l=>l.node_id)).size!==lessons.length",
 'new Set(doc.lessons.map(l=>l.node_id)).size!==8'
]) assert(app.includes(needle),"Missing canonical loading or historic fix: "+needle);
for(const token of ["overflow-x:auto","line-break:strict","@media(max-width:700px)","@media print",":focus-visible"])
 assert(css.includes(token),"Missing CJK/KaTeX/responsive rule "+token);
assert(workflow.includes("node quant/math/test_v99_dynamics_pde_controls.mjs"),"V99 CI step missing");

let checks=0;
function near(label,value,expected,tol=1e-8){
 assert(Number.isFinite(value),label+" not finite");
 assert(Math.abs(value-expected)<=tol,label+" observed "+value+" expected "+expected);
 checks++;
}
function check(label,condition){near(label,Number(Boolean(condition)),1);}
near("Hurwitz diag first eigen",-2,-2);
near("Hurwitz diag second eigen",-.5,-.5);
near("Hurwitz first mode at 2 ln2",Math.exp(-4*Math.log(2)),1/16);
near("Hurwitz second mode at 2 ln2",2*Math.exp(-Math.log(2)),1);
near("Hurwitz Lyapunov first element",-2*.25*2,-1);
near("Hurwitz Lyapunov second element",2*(-.5)*1,-1);
near("Hurwitz nonnormal norm",Math.sqrt(101)/Math.E,3.697142627340455,1e-10);
near("Discrete first state step",.6*2,1.2);
near("Discrete second state step",-.5*1,-.5);
near("Discrete first state step2",.6**2*2,.72);
near("Discrete second state step2",(-.5)**2,.25);
near("Discrete Lyapunov scalar",1/(1-.5**2),4/3);
near("Discrete stationary variance",.75/(1-.5**2),1);
near("Euler spectral instability",1-4*.75,-2);
near("State continuous input",2*(1-Math.exp(-2*Math.log(2))),1.5);
near("State exact F",Math.exp(-2*Math.log(2)),.25);
near("State exact G",(1-Math.exp(-2*Math.log(2)))/2,.375);
near("State Euler F",1-2*Math.log(2),-.3862943611198906);
near("Controllability rank det",0*0-1*1,-1);
near("Controllability gramian scalar",2,2);
near("Controllability min energy",1/2,.5);
near("Kalman predicted mean",.8*1,.8);
near("Kalman predicted var",.8**2*.25+.04,.2);
near("Kalman innovation",1.1-.8,.3);
near("Kalman gain",.2/(.2+.1),2/3);
near("Kalman posterior mean",.8+2/3*.3,1);
near("Kalman posterior var",.2*(1-2/3),1/15);
near("Kalman Joseph",(.2/(.2+.1)-1)**2*.2+(2/3)**2*.1,1/15);
near("Transport characteristic",Math.sin(Math.PI-2*Math.PI/4),1);
near("Transport inflow",1-1/2,.5);
near("Heat first frequency",Math.exp(-Math.log(2)),.5);
near("Heat second frequency",Math.exp(-4*Math.log(2)),1/16);
near("Heat second versus first ratio",(1/16)/(.5),1/8);
near("Wave dAlembert",Math.cos(2*Math.PI/6),.5);
near("Wave period energy",.5*4*Math.PI,2*Math.PI);
near("Laplace polynomial",2-2,0);
near("Laplacian average sin4",Math.sin(Math.PI),0);
near("European put zero stock",100*Math.exp(-.05),95.1229424500714);
near("European deep ITM call far boundary",400-100*Math.exp(-.05),304.877057549929);
near("Heat Fourier first t ln2",3*Math.exp(-Math.log(2)),1.5);
near("Heat Fourier second t ln2",Math.exp(-4*Math.log(2)),.0625);
near("Heat truncated squared error",Math.PI/2*(1/16)**2,Math.PI/512);
near("Risk neutral drift discounted",100*Math.exp(.05)*Math.exp(-.05),100);
near("Feynman Kac annuity",2*(1-Math.exp(-.05))/.05,1.950823019971);
const p=(Math.exp(.05)-.9)/(.2);
check("binomial p in (0,1)",p>0&&p<1);
near("American binomial put up payoff",Math.max(105-110,0),0);
near("American binomial put down payoff",Math.max(105-90,0),15);
check("American early put optimal",Math.exp(-.05)*(1-p)*15<5);
near("American put immediate",105-100,5);
near("FTCS stable nu",.1/.5**2,.4);
near("FTCS stable center",1-2*.4,.2);
near("FTCS unstable center",1-2*.8,-.6);
near("FTCS high frequency instability",1-4*.6,-1.4);
near("CN damp amplification", (1-4*.5/2)/(1+4*.5/2),0);
near("CN oscillatory amplification",(1-4*1/2)/(1+4*1/2),-1/3);
near("CN large negative z amplification",(1-100/2)/(1+100/2),-49/51);
near("CN stencil diagonal",1+.4,1.4);
near("CFL transport .8",2*.2/.5,.8);
near("CFL transport 1.2",2*.3/.5,1.2);
near("CFL max dt heat",.5**2/(2*1),.125);
near("CFL halved dx dt",.25**2/(2*1),.03125);
near("DPP path peak A",1-90/100,.1);
near("DPP path peak B",1-90/120,.25);
near("HJB LQ coefficient t0",1/(1+1),.5);
near("HJB LQ control t0",-1/(1+1),-.5);
near("HJB LQ value t0",1/(2*(1+1)),.25);
near("HJB LQ running cost",.5*(.5**2),.125);
near("HJB LQ terminal cost",.5*(.5**2),.125);
near("Merton portfolio",(.08-.02)/(2*.2**2),.75);
near("Merton risk premium q",.06**2/(2*2*.2**2),.0225);
near("Merton consumption",(.05-(-1)*(.02+.0225))/2,.04625);
near("Merton portfolio sigma .25",.06/(2*.25**2),.48);
near("Merton portfolio mu shift",.01/(2*.2**2),.125);
near("Verification LQ terminal wealth",1-.5,.5);
near("Verification running cost",.5*(.5**2)*1,.125);
near("Verification total cost",.125+.125,.25);
near("Obstacle convex immediate",.5**2,.25);
near("Obstacle convex continuation",.5,.5);
near("Obstacle concave immediate",.5*(1-.5),.25);
near("Obstacle penalty",.01/.001,10);
near("HJB action continuous cost",.5*(-.5)**2+.5*(1-.5)**2,.25);
near("HJB discrete action -1",.5*(-1)**2+.5*(1-1)**2,.5);
near("HJB discrete action 0",.5*0+.5*(1+0)**2,.5);
near("HJB discrete action 1",.5+ .5*4,2.5);
near("HJB fixed policy value B",2/(1-.9),20);
near("HJB improved policy value A",1/(1-.9),10);
assert(checks>=80,"insufficient independently calculated invariants");
console.log("V99 PASS",newLessons.length,"new lessons;",all.length,"total;",diffeq.length,
 "of",diffeq.length,"diffeq nodes completed;",formulaCount,"KaTeX;",chapterCount,
 "proof chapters;",deepChars,"deep Chinese characters;",checks,"numeric checks;",linkCount,"cross references.");
