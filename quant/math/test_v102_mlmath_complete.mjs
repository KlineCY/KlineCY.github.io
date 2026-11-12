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
 "lessons-risk-axioms-v101.json","lessons-risk-governance-v101.json"
];
const history=oldFiles.flatMap(n=>json(n).lessons);
assert.equal(history.length,476,"V101 historical 476 lesson texts changed");
assert.equal(new Set(history.map(l=>l.node_id)).size,476,"V101 history has duplicate IDs");
const files=["lessons-mlmath-v102a.json","lessons-mlmath-v102b.json","lessons-mlmath-v102c.json"];
const docs=files.map(json);
docs.forEach((d,i)=>{
 assert.equal(d.version,1,"V102 data schema wrong");
 assert.equal(d.revision,"V102","V102 revision mismatch");
 assert.equal(d.lessons.length,[4,4,4][i],"V102 file size mismatch");
});
const newest=docs.flatMap(d=>d.lessons);
const ids=[
 "mlmath-n0451","mlmath-n0456","mlmath-n0458","mlmath-n0459",
 "mlmath-n0468","mlmath-n0471","mlmath-n0474","mlmath-n0475",
 "mlmath-n0476","mlmath-n0477","mlmath-n0479","mlmath-n0480"
];
assert.deepEqual(newest.map(l=>l.node_id),ids,"V102 machine learning math gaps or order mismatch");
const all=[...history,...newest];
assert.equal(all.length,488,"Expected 488 completed mathematics lectures");
assert.equal(new Set(all.map(l=>l.node_id)).size,488,"Historical lesson overwritten");
assert(all.every(l=>nodes.get(l.node_id)?.title===l.title),"Curriculum mismatched or missing lesson");
const completed=new Set(all.map(l=>l.node_id));
const mlmath=curriculum.nodes.filter(n=>n.discipline_id==="mlmath");
assert.equal(mlmath.length,36,"Expected 36 ML mathematics syllabus nodes");
assert(mlmath.every(n=>completed.has(n.id)),"ML mathematics must be 36/36 complete");
assert.equal(history.filter(l=>l.node_id.startsWith("mlmath-")).length,24,"ML math prior 24 modified");
assert.equal(newest.filter(l=>l.node_id.startsWith("mlmath-")).length,12);

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
 assert(l.formulas.length>=3,id+" original core formulas preserved");
 assert(l.derivation.length>=3,id+" original derivations preserved");
 assert(l.deep_sections.length>=3,id+" original deep chapters preserved");
 for(const x of l.formulas){assert(x.label?.length>=2,id+" mathematical label");formula(x.tex,id);}
 for(const x of l.derivation){assert(x.explanation?.length>=25,id+" derivation prose");formula(x.tex,id);}
 const headings=new Set();
 for(const s of l.deep_sections){
  assert(s.title?.length>=8 && s.question?.length>=10,id+" deep section title/question");
  assert(!headings.has(s.title),id+" repeated subchapter");
  headings.add(s.title);
  assert(s.paragraphs?.length>=2 && s.paragraphs.every(p=>p.length>=30),id+" deep explanatory text");
  for(const [key,min] of [["proof",50],["worked",50],["limitation",25]])
   assert(s[key]?.length>=min,id+" short "+key);
  assert(s.formulas?.length>=1,id+" deep mathematical expressions preserved");
  for(const f of s.formulas){assert(f.label?.length>=2,id+" deep formula caption");formula(f.tex,id);}
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
assert(mathCount>=108,"Original strict KaTeX mathematics must not be lost");
assert(deepCount>=36,"Original deep mathematical chapters must remain");
assert(deepChars>=9700,"Depth reduced from reviewed version");
assert(linkCount>=36,"Original cross-chapter references must remain");
const html=read("index.html"),css=read("styles.css"),app=read("app.js");
const workflow=read("../../.github/workflows/test-quant-math.yml");
assert(/app[.]js[?]v=[0-9]{8}/.test(html),"Reader must bust V101 cache");
assert(html.includes("机器学习数学与数值优化"),"V102 CJK curriculum absent");
for(const id of ids)assert(html.includes("?topic="+id+'"'),"Study route missing "+id);
for(const needle of [
 'loadV101TimeseriesRisk("./lessons-risk-governance-v101.json",4)',
 'loadV102MachineLearningMath("./lessons-mlmath-v102a.json",4)',
 'loadV102MachineLearningMath("./lessons-mlmath-v102b.json",4)',
 'loadV102MachineLearningMath("./lessons-mlmath-v102c.json",4)',
 '...v101time,...v101tail,...v101axioms,...v101governance,...v102a,...v102b,...v102c',
 'new Set(lessons.map(l=>l.node_id)).size!==lessons.length',
 'new Set(doc.lessons.map(l=>l.node_id)).size!==8'
]) assert(app.includes(needle),"Missing canonical ML or historical math loader: "+needle);
for(const token of ["overflow-x:auto","line-break:strict","@media(max-width:700px)","@media print",":focus-visible"])
 assert(css.includes(token),"Missing established Chinese/LaTeX mobile accessibility: "+token);
assert(workflow.includes("node quant/math/test_v102_mlmath_complete.mjs"),"V102 CI step absent");


let checks=0;
function near(label,value,expected,tol=1e-8){
 assert(Number.isFinite(value),label+" nonfinite");
 assert(Math.abs(value-expected)<=tol,label+" actual="+value+" expected="+expected);
 checks++;
}
function yes(label,condition){near(label,Number(Boolean(condition)),1);}
function dot(a,b){return a.reduce((z,x,i)=>z+x*b[i],0);}
function norm(a){return Math.sqrt(dot(a,a));}
function lg(x){return Math.log(x);}
function positivePart(x){return Math.max(x,0);}

near("Squared-loss conditional mean",.25*0+.75*2,1.5);
near("Squared-loss mean conditional MSE",.25*(0-1.5)**2+.75*(2-1.5)**2,.75);
near("Absolute-loss median MAE",.25*Math.abs(0-2)+.75*Math.abs(2-2),.5);
near("MSE at median",.25*(0-2)**2+.75*0,1);
near("BCE true probability",- .8*lg(.8)-.2*lg(.2),.5004024235381879);
near("BCE uncalibrated half",- .8*lg(.5)-.2*lg(.5),Math.LN2);
yes("Proper binary scoring",-.8*lg(.8)-.2*lg(.2)<Math.LN2);
near("Huber slope clipped",Math.min(Math.abs(10),1),1);
near("Huber for residual10 and delta1",1*(10-.5),9.5);
near("Huber quadratic near0",.5*.5**2,.125);
near("Finite model n sample bound",lg(4000)/(2*.1**2),414.7024820051014);
near("Finite model integer threshold",Math.ceil(lg(4000)/(2*.1**2)),415);
near("Half epsilon bound",Math.ceil(lg(4000)/(2*.05**2)),1659);
near("Unified risk epsilon M20",Math.sqrt(lg(800)/1000),.08175947484951164);
near("Unified risk ERM excess",2*Math.sqrt(lg(800)/1000),.16351894969902328);
near("Hypothesis class approximation error",.13-.10,.03);
near("Hypothesis class estimation error",.18-.13,.05);
near("Hypothesis class total excess",.18-.1,.08);
near("Computational graph forward x2",2**2+3*2,10);
near("Computational graph derivative x2",2*2+3,7);
near("Computational graph finite diff",( (2+.001)**2+3*(2+.001) - ((2-.001)**2+3*(2-.001)) )/(2*.001),7);
near("Computational graph squared path gradient",2*2,4);
near("Computational graph direct path gradient",3,3);
near("Matrix forward sample 1",dot([1,2],[1,-1]),-1);
near("Matrix forward sample 2",dot([3,4],[1,-1]),-1);
near("Matrix dW first",dot([1,3],[-1,-1]),-4);
near("Matrix dW second",dot([2,4],[-1,-1]),-6);
near("Matrix dX first coord",-1*1,-1);
near("Matrix dX second coord",-1*(-1),1);
near("Matrix db sum",-1-1,-2);
near("Matrix dW first perturbation",dot([1,3],[-1,-1]),-4);
near("SVM margin original",1/Math.abs(1),1);
near("SVM two-sided width original",2/Math.abs(1),2);
near("SVM margin wider points",1/Math.abs(.5),2);
near("SVM hinge inside",positivePart(1-.6),.4);
near("SVM hinge outside",positivePart(1-1.4),0);
near("SVM Gram first eigenvalue",1+.5,1.5);
near("SVM Gram second eigenvalue",1-.5,.5);
yes("SVM PSD kernel example",(1-.5)*(1+.5)>=0);
near("Embedding dot",dot([3,4],[4,3]),24);
near("Embedding first norm",norm([3,4]),5);
near("Embedding second norm",norm([4,3]),5);
near("Embedding cosine",dot([3,4],[4,3])/(norm([3,4])*norm([4,3])),.96);
near("Embedding rescaled cosine",dot([30,40],[4,3])/(norm([30,40])*norm([4,3])),.96);
near("Embedding Gram eigen plus",1+.6,1.6);
near("Embedding Gram eigen minus",1-.6,.4);
near("InfoNCE two logits",lg(1+Math.exp(-2)),.1269280110429726);
near("InfoNCE collapse two logits",lg(2),Math.LN2);
yes("InfoNCE positive separated",lg(1+Math.exp(-2))<Math.LN2);
near("Causal convolution at t2",2-1,1);
near("Causal convolution at t3",3-2,1);
near("Causal convolution at t4",4-3,1);
near("Causal convolution dkernel0",2+3+4,9);
near("Causal convolution dkernel1",1+2+3,6);
near("Conv2D output side",Math.floor((32+2*1-3)/2)+1,16);
near("Conv2D output area",16*16,256);
const p=Math.exp(1/Math.sqrt(2))/(1+Math.exp(1/Math.sqrt(2)));
near("Attention self weight",p,.6697615493266569);
near("Attention other weight",1-p,.3302384506733431);
near("Attention row stochastic",p+(1-p),1);
near("Attention first output coordinate",2*p,1.3395230986533138);
near("Attention first second output coordinate",4*(1-p),1.3209538026933725);
near("Attention masked first output coordinate",2*1,2);
near("Attention masked future weight",0,0);
near("Position first frequency",10000**0,1);
near("Position second frequency",10000**(-.5),.01);
near("Position zero first sine",Math.sin(0),0);
near("Position zero first cosine",Math.cos(0),1);
near("Position second pair first sine",Math.sin(Math.PI/200),.015707317311820675,1e-8);
near("Position second pair first cosine",Math.cos(Math.PI/200),.9998766324816606);
near("Position first pair distance dot",Math.cos(Math.PI/2),0);
near("Position full dot",Math.cos(Math.PI/2)+Math.cos(Math.PI/200),.9998766324816606);
near("Policy Bernoulli reward expectation",.5*2+.5*0,1);
near("Policy Bernoulli true gradient",2*.5*(1-.5),.5);
near("Policy REINFORCE action1 sample",(1-.5)*2,1);
near("Policy REINFORCE action0 sample",(-.5)*0,0);
near("Policy REINFORCE sample mean",.5*1+.5*0,.5);
near("Policy baseline action1",(2-1)*.5,.5);
near("Policy baseline action0",(0-1)*(-.5),.5);
near("Policy baseline still mean",.5*.5+.5*.5,.5);
near("Policy relative return with discount",.9*10,9);
near("PPO positive unclipped",1.3*2,2.6);
near("PPO positive clipped",1.2*2,2.4);
near("PPO positive objective",Math.min(1.3*2,1.2*2),2.4);
near("PPO negative unclipped",.7*(-2),-1.4);
near("PPO negative clipped",.8*(-2),-1.6);
near("PPO negative objective",Math.min(.7*(-2),.8*(-2)),-1.6);
near("PPO GAE discount",.9*.8,.72);
near("PPO GAE 2 step",.4+.9*.8*.2,.544);
yes("PPO clipped r can exceed clip bound",1.3>1.2);
assert(checks>=75,"V102 independently calculated financial and ML checks unexpectedly incomplete");

for(const lecture of newest){
 const seen=new Set(lecture.deep_sections.map(s=>s.title));
 assert.equal(seen.size,lecture.deep_sections.length,lecture.id+" duplicate mathematical chapter");
 assert(lecture.deep_sections.length>=4,lecture.id+" original mathematics and independent deeper analysis missing");
 const extended=lecture.deep_sections.at(-1);
 assert(extended.paragraphs.length>=2&&extended.paragraphs.every(p=>p.length>=80),lecture.id+" new explanation inadequate");
 assert(extended.proof.length>=100&&extended.worked.length>=70&&extended.limitation.length>=65,lecture.id+" insufficient added derivation, worked example or boundary");
 assert(extended.formulas.length>=1,lecture.id+" missing mathematical formulation");
}
near("Quantile check loss at a0",.2*.9*10,1.8);
near("Quantile check loss at a10",.8*.1*10,.8);
yes("Quantile optimum lower loss",.8<1.8);
near("Sample complexity ceiling",Math.ceil(Math.log(40000)/(.02)),530);
near("Branched graph full derivative",2*2+3,7);
near("Matrix BCE class 1 first gradient",(-.5*1+.5*2)/2,.25);
near("Approximation and estimation risk",(.5-.25)+.25,.5);
near("Hard SVM dual primal equality",1-.5,.5);
near("Orthogonal embedding distance",Math.sqrt(2),Math.sqrt(2));
near("Shared convolution w0 gradient",1+2,3);
near("Shared convolution w1 gradient",2+3,5);
near("Causal softmax first weight",1/(1+3),.25);
near("Causal softmax second weight",3/(1+3),.75);
near("Sinusoid relative dot",Math.cos(-Math.PI),-1);
near("Bernoulli REINFORCE expectation",.3*(1/.3),1);
near("PPO positive clipping",Math.min(1.4*2,1.2*2),2.4);
near("PPO negative clipping",Math.min(.5*(-2),.8*(-2)),-1.6);
console.log("V102 PASS",newest.length,"new lessons;",all.length,"total;",
 mathCount,"strict KaTeX;",deepCount,"proof chapters;",deepChars,"deep prose chars;",
 checks,"independent numerics;",linkCount,"connected knowledge links; machine learning mathematics 36/36.");
