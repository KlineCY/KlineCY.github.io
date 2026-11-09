import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
import katex from "katex";
import {gunzipSync} from "node:zlib";

const root=new URL("./",import.meta.url);
const read=name=>fs.readFileSync(new URL(name,root),"utf8");
const json=name=>JSON.parse(read(name));
const files=["atlas-concept-bridge-v112a.json","atlas-concept-bridge-v112b.json","atlas-concept-bridge-v112c.json",
             "atlas-concept-bridge-v112d.json","atlas-concept-bridge-v112e.json"];
const docs=files.map(json);
const manifest=json("atlas-depth-manifest.json");
const math=json("../math/curriculum.json");
const mathIDs=new Map(math.nodes.map(node=>[node.id,node.title]));
const archive=JSON.parse(gunzipSync(Buffer.concat([1,2,3,4].map(i=>fs.readFileSync(new URL("atlas/data.part"+i,root))))).toString("utf8"));
const previous=[
 json("atlas-concept-chapters.json"),json("atlas-concept-bridge-01.json"),
 json("atlas-concept-bridge-v111a.json"),json("atlas-concept-bridge-v111b.json"),json("atlas-concept-bridge-v111c.json")
];
const priorIDs=new Set(previous.flatMap(b=>Object.keys(b.lessons)));
const expected=[
 "m03-c03-p01","m04-c02-p02","m08-c01-p05","m10-c01-p02",
 "m05-c05-p05","m11-c04-p03","m12-c01-p04","m13-c02-p01",
 "m14-c04-p02","m15-c02-p03","m16-c02-p04","m17-c03-p01",
 "m18-c02-p02","m19-c04-p01","m20-c03-p05","m20-c05-p05",
 "m21-c05-p05","m22-c05-p06","m23-c03-p06","m24-c03-p06"
];
const entries=docs.flatMap(d=>Object.entries(d.lessons));
assert.equal(manifest.expected_lessons,720,"Original ML core must stay 720");
assert.equal(manifest.expected_domains,24,"Original ML domains must stay 24");
assert.equal(archive.catalog.items.length,720,"Original archive course count changed");
assert.equal(mathIDs.size,656,"All 656 math prerequisite nodes must remain");
assert.equal(entries.length,20,"20 canonical deep lessons required");
assert.deepEqual(entries.map(([id])=>id),expected,"V112 exact topics and order changed");
assert.equal(new Set(expected).size,20,"Duplicate topic identity");
assert(expected.every(id=>!priorIDs.has(id)),"Do not duplicate any earlier deep concept chapter");
for(const d of docs){
 assert.equal(d.schema,1,"Chapter schema changed");
 assert.equal(d.revision,"V112","V112 revision not verified");
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
assert.equal(sections,80,"V112 requires 80 full proof sections");
assert.equal(strictFormulas,80,"V112 requires 80 strict KaTeX equations");
assert.equal(crossLinks,80,"V112 requires 80 valid deep prerequisite links");
assert(chineseChars>=21000,"Explanation content was shortened too much");

const source=read("concept-render.js"),reader=read("atlas.js"),html=read("index.html"),css=read("atlas.css");
new vm.Script(source);new vm.Script(reader);
assert(reader.includes('for(const path of ["./atlas-concept-bridge-v112a.json","./atlas-concept-bridge-v112b.json"'),"New V112 reader loader missing");
assert(reader.includes('state.conceptChapters.has(id)||newIds.has(id)'),"V112 duplicate/extension collision guard missing");
const ctx={window:{},encodeURIComponent};vm.runInNewContext(source,ctx);
const loaded=new Set();
for(const d of docs){
 const original=new Map(Object.keys(d.lessons).map(id=>[id,archive.domains[id.slice(0,3)][id].html]));
 const mounted=ctx.window.quantMlConcepts.mount(archive,d);
 assert.equal(mounted.size,4,"One of five V112 canonical installations failed");
 for(const id of mounted){
  assert(!loaded.has(id),"Duplicate V112 installed course");loaded.add(id);
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
assert(html.includes("概率模型、神经网络与强化学习"),"V112 title or Chinese course map absent");
assert(html.includes('atlas.js?v=20261109-v115-reader-layout-v116-ml-bridges-v117-no-learning-progress'),"V112 runtime cache version missing");
for(const selector of ["line-break:strict","overflow-x:auto","@media(max-width:700px)",".atlas-concept-jump:focus-visible",".atlas-concept-chain","@media print"])
 assert(css.includes(selector),"Responsive CJK, KaTeX scroll, keyboard or print style missing "+selector);
const workflow=read("../../.github/workflows/test-quant-ml.yml");
assert(workflow.includes("node quant/ml/test_concept_bridges_v112.mjs"),"V112 gate omitted from CI");
const readV111=read("test_concept_bridges_v111.mjs");
assert(readV111.includes("new vm.Script(source)")&&readV111.includes("canonical 720 Atlas"),"V111 historical test disabled");

// Arithmetic independent of lesson source; use literal known analytical values and deliberately small examples.
let numerics=0;
function near(name,value,expected,tol=1e-9){
 assert(Number.isFinite(value)&&Math.abs(value-expected)<=tol,name+" computed "+value+", expected "+expected);numerics++;
}
function yes(name,value){assert(value,name);numerics++;}
near("Bernoulli variance",.2*(1-.2),.16);
near("Binomial mass 2/3",(3*.5**2*.5),.375);
near("Bernoulli MLE",12/20,.6);
near("Binary loss predicted .9",-Math.log(.9),.10536051565782628);
near("Binary loss predicted .1",-Math.log(.1),2.302585092994046);
near("Threshold growth n3",3+1,4);
near("Interval growth n3",1+3*4/2,7);
near("VC finite bound",Math.sqrt(Math.log(800)/1000),.08175947432281454,1e-8);
near("Tree one-leaf alpha .05",.28+.05,.33);
near("Tree two-leaf alpha .05",.18+2*.05,.28);
near("Tree critical alpha",(.28-.18)/(2-1),.1);
near("Prune subtree alpha",(.30-.18)/(3-1),.06);
near("Beta-Bernoulli MLE",7/10,.7);
near("Beta-Bernoulli MAP",8/12,2/3);
near("Beta-Bernoulli posterior mean",9/14,.6428571428571429);
near("FP16 unit roundoff",2**-11,.00048828125);
near("BF16 unit roundoff",2**-8,.00390625);
near("FP16 loss scale rescue",1024*1e-8,.00001024);
near("FP16 example stable softmax",1/(1+Math.exp(-1)),.7310585786300049);
near("GMM numerator first",.6*.2,.12);
near("GMM numerator second",.4*.1,.04);
near("GMM responsibility first",.12/(.12+.04),.75);
near("GMM weighted mean",(.75*0+.25*10)/(.75+.25),2.5);
near("ROC AUC with one tie",2.5/4,.625);
near("ROC precision rare-event",.01*.8/(.01*.8+.99*.05),.1391304347826087);
near("AD scalar graph f",2*3+2**2,10);
near("AD d/da",3+2*2,7);
near("AD d/db",2,2);
near("AD finite derivative",((2.01*3+2.01**2)-(1.99*3+1.99**2))/.02,7);
near("AD broadcast bias grad",2*(2+3+4),18);
near("ViT patches",224/16*224/16,196);
near("ViT patch tokens with CLS",14*14+1,197);
near("ViT revised patch tokens",16*16+1,257);
near("BPTT h3",.5**3,.125);
near("BPTT d h3 /d w",3*.5**2,.75);
near("BPTT last step derivative",.5**2,.25);
near("BPTT exploding 10",1.5**10,57.6650390625);
near("RoPE 90 degree x coordinate",Math.cos(Math.PI/2),0,1e-14);
near("RoPE relative displacement invariance",3-2,1);
near("SFT response NLL",-Math.log(.8)-Math.log(.5),.916290731874155);
near("SFT response mean NLL",(-Math.log(.8)-Math.log(.5))/2,.4581453659370775);
near("GCN degree 1",1+1,2);
near("GCN degree 2",2+1,3);
near("GCN edge norm",1/Math.sqrt(2*3),.4082482904638631);
near("GCN complete average",(1+2+3)/3,2);
near("DDPM accumulated alpha",.9*.8,.72);
near("DDPM q mean at t2",2*Math.sqrt(.72),1.697056274847714,1e-8);
near("DDPM q std at t2",Math.sqrt(.28),.5291502622129182,1e-8);
near("DDPM SNR at t2",.72/.28,2.5714285714285716);
near("DDPM correlated portfolio variance",.005+2*.5*.5*.9*.01,.0095);
near("Double DQN classic target",1+.9*8,8.2);
near("Double DQN new target",1+.9*7,7.3);
near("Double DQN soft target update",.9*2+.1*10,2.8);
near("Double DQN net action 1",1-.5,.5);
near("Double DQN net action 2",.8-.1,.7);
near("Squashed action sigmoid shape",Math.tanh(2),.9640275800758169);
near("Tanh inverse at 0.6",Math.atanh(.6),.6931471805599453);
near("Tanh Jacobian at .6",1-.6**2,.64);
near("Tanh log prob correction",-Math.log(.64),.44628710262841953);
near("Scaled action log Jacobian",Math.log(2*3),1.791759469228055);
near("Squashed 3 asset leverage",3*.9,2.7);
near("Simulator gross adverse-selection returns",.5*.02+.5*(-.03),-.005);
near("Simulator executed adverse-selection loss",-.03,-.03);
near("Simulator impact cost",.01*100+.0001*100**2,2);
near("Simulator doubled cost",.01*200+.0001*200**2,6);
near("Covariate shift high weight",.6/.2,3);
near("Covariate shift weighted mean",.2*3,.6);
near("Importance-weight ESS",100/52,1.9230769230769231);
near("Constrained optimum f at .6",.5*(.6-1)**2,.08);
near("Constrained optimum lambda",1-.6,.4);
near("Relaxed optimum at .61",.5*(.61-1)**2,.07605);
near("Shadow first-order forecast",-.4*.01,-.004);
near("ES top quartile",3+(8-3)/(4*.25),8);
near("Constrained net exposure",.8+.8-.3,1.3);
near("Constrained gross leverage",.8+.8+.3,1.9);
near("Drift false alarm 20",1-.95**20,.6415140775914581,1e-8);
near("Challenger vs champion error",.025-.020,.005);
yes("MLOps gate rejects excess leverage",2>1.5);
console.log("V112 PASS: "+loaded.size+" canonical advanced lectures, "+sections+" coherent proof sections, "+strictFormulas+
 " strictly parsed KaTeX expressions, "+crossLinks+" exact math/ML prerequisite links, "+chineseChars+
 " source CJK explanation characters, "+numerics+" independently computed arithmetic invariants, unchanged original 720 pages, responsive formulas and historical V111 coverage");
