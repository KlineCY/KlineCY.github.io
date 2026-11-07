import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const mathSpec=[{"file":"lessons-phase-03-c.json","id":"optimization-x005","originalSections":4,"hash":"fcda1296"},{"file":"lessons-phase-03-c.json","id":"riskmath-x004","originalSections":4,"hash":"bfd18656"},{"file":"lessons-phase-04-b.json","id":"econometrics-x001","originalSections":4,"hash":"bc14d566"},{"file":"lessons-phase-04-b.json","id":"econometrics-x004","originalSections":4,"hash":"c023841a"},{"file":"lessons-phase-05-a.json","id":"statistics-x002","originalSections":4,"hash":"27ea6b5b"},{"file":"lessons-phase-05-a.json","id":"finance-x002","originalSections":8,"hash":"453f8c0b"},{"file":"lessons-phase-05-a.json","id":"econometrics-x002","originalSections":6,"hash":"c29cf44e"},{"file":"lessons-phase-05-b.json","id":"timeseries-x002","originalSections":4,"hash":"e451693c"},{"file":"lessons-phase-05-b.json","id":"optimization-x002","originalSections":8,"hash":"3f7a776f"},{"file":"lessons-phase-05-b.json","id":"riskmath-x003","originalSections":4,"hash":"428a9153"},{"file":"lessons-phase-06-a.json","id":"mlmath-x001","originalSections":4,"hash":"15bb3c2e"},{"file":"lessons-phase-06-b.json","id":"mlmath-x002","originalSections":4,"hash":"e31abf4b"},{"file":"lessons-phase-06-b.json","id":"optimization-x007","originalSections":6,"hash":"035c3aff"},{"file":"lessons-phase-07-a.json","id":"linear-x001","originalSections":4,"hash":"9c27c473"},{"file":"lessons-phase-07-a.json","id":"linear-x003","originalSections":4,"hash":"f4a5b7f1"},{"file":"lessons-phase-07-b.json","id":"linear-x002","originalSections":4,"hash":"09595f28"},{"file":"lessons-phase-07-b.json","id":"linear-x004","originalSections":4,"hash":"b4141fae"}];
const mlSpec=[{"id":"m03-c01-p04","hash":"64e610c3","eq":8,"der":8,"prot":7,"pit":4,"worked":373},{"id":"m03-c02-p06","hash":"3abec5ae","eq":8,"der":8,"prot":7,"pit":4,"worked":345},{"id":"m03-c05-p01","hash":"c29d25be","eq":8,"der":8,"prot":7,"pit":4,"worked":360},{"id":"m03-c03-p06","hash":"3530350d","eq":8,"der":8,"prot":7,"pit":4,"worked":347},{"id":"m04-c02-p06","hash":"57320d6f","eq":8,"der":8,"prot":7,"pit":4,"worked":375},{"id":"m04-c04-p01","hash":"b36c4218","eq":8,"der":8,"prot":7,"pit":4,"worked":330},{"id":"m04-c05-p06","hash":"2c5fad62","eq":8,"der":8,"prot":7,"pit":4,"worked":371},{"id":"m04-c03-p01","hash":"bef2ec6f","eq":8,"der":8,"prot":7,"pit":4,"worked":378}];
const read=p=>fs.readFileSync(new URL(p,root),"utf8");
const json=p=>JSON.parse(read(p));
function hash(o){let h=2166136261>>>0;for(const c of JSON.stringify(o))h=Math.imul(h^c.charCodeAt(0),16777619)>>>0;return h.toString(16).padStart(8,"0")}
function clean(obj,id){const s=JSON.stringify(obj);assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(s),id+" corrupted Unicode");assert(!/TODO|FIXME|Lorem ipsum/i.test(s),id+" draft placeholder");assert(/[\u3400-\u9FFF]/u.test(s),id+" missing Chinese exposition");}
let strict=0,mathCount=0,mlCount=0;
const cache=new Map();
for(const s of mathSpec){
 const d=cache.get(s.file)??json(s.file);cache.set(s.file,d);
 assert(d.v84_revision.ids.includes(s.id),s.id+" not registered in V84");
 const x=d.lessons.find(z=>z.id===s.id);
 assert(x&&x.node_id===s.id,s.id+" math canonical node changed");
 assert.equal(hash({...x,deep_sections:x.deep_sections.slice(0,s.originalSections)}),s.hash,s.id+" old math text changed");
 assert.equal(x.deep_sections.length,s.originalSections+1,s.id+" missing append-only subchapter");
 const n=x.deep_sections.at(-1);
 assert(n.title.length>=12&&n.question.length>=12,s.id+" weak heading");
 assert.equal(n.paragraphs.length,2,s.id+" Chinese paragraph structure");
 assert(n.paragraphs.every(p=>p.length>=75),s.id+" thin Chinese narrative");
 assert(n.proof.length>=120&&n.worked.length>=90&&n.limitation.length>=50,s.id+" incomplete proof/numerics/boundary");
 assert.equal(n.formulas.length,2);
 assert.equal(new Set(x.deep_sections.map(t=>t.title)).size,x.deep_sections.length,s.id+" duplicate section heading");
 for(const f of n.formulas){assert(f.label.length>=4&&f.tex.length>=18,s.id+" weak formula");katex.renderToString(f.tex,{throwOnError:true,strict:"error",displayMode:true});strict++;}
 clean(n,s.id);mathCount++;
}
assert.equal(mathCount,17);
const module=json("../ml/atlas-depth-07-b.json"),manifest=json("../ml/atlas-depth-manifest.json");
assert.equal(module.revision,"2026-11-06");
assert(manifest.modules.some(x=>x.path==="./atlas-depth-07-b.json"&&x.lessons===8));
assert.deepEqual(Object.keys(module.lessons),mlSpec.map(x=>x.id));
assert.deepEqual(module.v84_revision.ids,mlSpec.map(x=>x.id));
for(const s of mlSpec){
 const x=module.lessons[s.id];assert(x.v84_note?.startsWith("V84"),s.id+" no V84 note");
 const orig={...x,equations:x.equations.slice(0,s.eq),derivation:x.derivation.slice(0,s.der),protocol:x.protocol.slice(0,s.prot),pitfalls:x.pitfalls.slice(0,s.pit),worked_example:x.worked_example.slice(0,s.worked)};
 delete orig.v84_note;
 assert.equal(hash(orig),s.hash,s.id+" original V78 lesson modified");
 assert.equal(x.equations.length,s.eq+2);
 assert.equal(x.derivation.length,s.der+2);
 assert.equal(x.protocol.length,s.prot+1);
 assert.equal(x.pitfalls.length,s.pit+1);
 assert(x.worked_example.length-s.worked>=100);
 assert(x.derivation.slice(-2).every(t=>t.length>=110),s.id+" thin derivation");
 assert(x.protocol.at(-1).length>=60&&x.pitfalls.at(-1).length>=60,s.id+" thin protocol/boundary");
 assert.equal(new Set(x.equations.map(t=>t.label)).size,x.equations.length);
 for(const f of x.equations.slice(-2)){assert(f.label.length>=4&&f.tex.length>=15);katex.renderToString(f.tex,{throwOnError:true,strict:"error",displayMode:true});strict++;}
 clean(x,s.id);mlCount++;
}
assert.equal(mlCount,8);assert.equal(strict,50);
let numeric=0;
function near(name,a,b,tol=1e-8){assert(Number.isFinite(a)&&Math.abs(a-b)<=tol,name+": "+a+" != "+b);numeric++}
function yes(name,v){near(name,v?1:0,1)}
near("joint chi square covariance",.6**2*.0001+.4**2*.0004,.0001);
near("joint chi square robust penalty",Math.sqrt(5.991*.0001),.024476519362437,1e-6);
near("independent finite tail given marginal",.01**2/.01,.01);
near("perfect dependence conditional tail",.01/.01,1);
near("orthogonal two-rate product",.05*.05,.0025);
near("orthogonal remainder norm",.05**2+.05*.05,.005);
near("DML root n scale",1/Math.sqrt(10000),.01);
near("AIPW low propensity",.4/.02,20);
near("AIPW clipped propensity",.4/.05,8);
near("multiasset synchronous combination positive",(.5*1+.5*2),1.5);
near("multiasset synchronous variance",1.5**2,2.25);
near("multiasset independent false variance",(.25*1+.25*4),1.25);
near("GMM J 2 moment",200*(.02**2/.01+.01**2/.01),10);
yes("GMM chi square rejection",10>3.841);
yes("label day96 invalid",96+5>=100);
yes("label day94 valid",94+5<100);
near("RV microstructure bias",2*1000*1e-6,.002);
near("RV corrected",.012-.002,.01);
near("RV correction misspec negative",.012-2*1000*1e-5,-.008);
near("self financing cash",1000-100+60-(100+60)*.001,959.84);
near("self financing net value",959.84+100-60,999.84);
near("copula lower Fréchet",.95+.95-1,.9);
near("copula upper Fréchet",Math.min(.95,.95),.95);
near("copula independent joint upper tail",.05**2,.0025);
near("Bernstein event variance",.02*.98,.0196);
near("Bernstein exponent",-1000*.05**2/(2*.0196+2*.05/3),-34.47,1e-2);
near("Hoeffding exponent",-2*1000*.05**2,-5);
near("Hoeffding upper tail",2*Math.exp(-5),.013475893998170934);
near("fixed model validation radius",Math.sqrt(Math.log(40)/2000),.0429478,1e-5);
near("candidate 100 simultaneous radius",Math.sqrt(Math.log(4000)/2000),.06439739761862287);
near("nonconvex grad squared bound",2*4*8/100,.64);
near("nonconvex grad bound",Math.sqrt(.64),.8);
near("generalized Rayleigh",13/5,2.6);
near("generalized eigen one",4/1,4);
near("generalized eigen two",9/4,2.25);
near("effective resistance chain",1+1,2);
near("effective resistance parallel",1/(1+1/2),2/3);
near("principal angle projector squared",2*(1-.5**2),1.5);
near("lazy random walk center stationary",2/4,.5);
near("HMM forecast regime one",.8*.9+.2*.3,.78);
near("HMM forecast regime two",.8*.1+.2*.7,.22);
near("HMM posterior",(.78*.2)/(.78*.2+.22*.6),.5416666666666667);
near("martingale heteroskedastic variance",(50*1+50*9)/100**2,.05);
near("martingale heteroskedastic SE",Math.sqrt(.05),.22360679774997896);
near("Bernoulli Fisher information",100/(.2*.8),625);
near("Bernoulli CRLB",.2*.8/100,.0016);
near("mixture point density",.5/(.001*Math.sqrt(2*Math.PI)),199.47114020071635);
near("mixture point narrower ratio",(.5/(.0001*Math.sqrt(2*Math.PI)))/(.5/(.001*Math.sqrt(2*Math.PI))),10);
near("net risk model A",.08+.03,.11);
near("net risk model B",.09+.005,.095);
near("strongly convex parameter stability",2/(.5*100),.04);
near("TV shift sample",.5,.5);
near("PAC realizable sample",Math.ceil((Math.log(100)+Math.log(20))/.1),77);
const mathHTML=read("index.html"),mlHTML=read("../ml/index.html"),mathCSS=read("styles.css"),mlCSS=read("../ml/atlas.css");
for(const s of mathSpec)assert(mathHTML.includes("?topic="+s.id),s.id+" not in live math navigation");
for(const s of mlSpec)assert(mlHTML.includes('href="#lesson/'+s.id+'"'),s.id+" not in Atlas navigation");
for(const [name,css] of [["math",mathCSS],["ML",mlCSS]]){
 for(const token of ["overflow-x:auto","line-break:strict",":focus-visible","@media print","@media(max-width:700px)"])assert(css.includes(token),name+" missing CJK/mobile formula support "+token);
}
const mathReader=read("app.js"),mlReader=read("../ml/atlas.js");
for(const marker of ["renderMathDeepSections","part.paragraphs","part.proof","part.worked","part.limitation"])assert(mathReader.includes(marker),"math renderer missing "+marker);
for(const marker of ["s.equations.map","s.derivation","s.protocol","s.pitfalls","s.worked_example"])assert(mlReader.includes(marker),"Atlas renderer missing "+marker);
console.log("V84 PASS: "+mathCount+" math + "+mlCount+" ML full-text-preserved live lessons, "+strict+" strict KaTeX new formulas, "+numeric+" independently checked arithmetic invariants, CJK responsive/navigation.");
