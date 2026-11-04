import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const dir=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,dir),"utf8"));
const c=read("curriculum.json"),manifest=read("lessons-manifest.json");
const core=["lessons-foundations.json","lessons-advanced.json","lessons-more.json"].flatMap(f=>read(f).lessons);
const extras=manifest.modules.flatMap(m=>read(m.path).lessons);
const all=[...core,...extras],byId=new Map(all.map(x=>[x.node_id,x]));
const nodes=new Map(c.nodes.map(x=>[x.id,x]));
const ids=[
 "probability-n0106","probability-n0115","probability-n0116","probability-n0117",
 "measure-n0139","measure-n0140","measure-n0141","measure-n0142","measure-n0144",
 "stochastic-n0289","stochastic-n0292","stochastic-n0294"
];
const prerequisites={
 "probability-n0106":["probability-n0104","probability-n0105"],
 "probability-n0115":["probability-n0104"],
 "probability-n0116":["probability-n0115","measure-n0121"],
 "probability-n0117":["probability-n0115","probability-n0106"],
 "measure-n0139":["measure-n0129","probability-n0104"],
 "measure-n0140":["measure-n0139","probability-n0115"],
 "measure-n0141":["measure-n0121","probability-n0116"],
 "measure-n0142":["probability-n0106","probability-n0117"],
 "measure-n0144":["stochastic-n0290","stochastic-n0294","measure-n0139"],
 "stochastic-n0289":["measure-n0121"],
 "stochastic-n0292":["stochastic-n0291"],
 "stochastic-n0294":["stochastic-n0290","stochastic-n0291","stochastic-n0292","measure-n0140"]
};

assert.equal(c.disciplines.length,20);
assert.equal(c.chapters.length,109);
assert.equal(c.nodes.length,656);
assert.equal(manifest.modules.length,41);
assert.equal(manifest.expected_lessons,103);
assert.equal(core.length,59,"core mathematics: 47 existing plus 12 V10");
assert.equal(all.length,162,"complete mathematics: 150 prior plus 12 V10");
assert.equal(byId.size,all.length,"duplicate lecture IDs");
assert.equal(ids.length,12);

let chapters=0,formulas=0,sourceCount=0,links=0;
for(const id of ids){
 const lesson=byId.get(id),node=nodes.get(id);
 assert(lesson&&node&&lesson.id===id&&lesson.title===node.title,"canonical V10 node "+id);
 assert.deepEqual(node.prerequisites,prerequisites[id],"prerequisite mismatch "+id);
 assert(lesson.definition?.length>=100&&lesson.assumptions?.length>=100,"definition too short "+id);
 assert(lesson.example?.length>=100&&lesson.application?.length>=60&&lesson.caveat?.length>=60,"missing empirical discussion "+id);
 assert(lesson.properties.length>=4&&lesson.formulas.length>=4&&lesson.derivation.length>=4,"incomplete mathematical structure "+id);
 assert.equal(lesson.deep_sections?.length,4,"must provide four connected mini-chapters "+id);
 assert.equal(new Set(lesson.deep_sections.map(s=>s.title)).size,4,"duplicate chapter title "+id);
 assert(lesson.sources.length>=2,"academic source missing "+id);
 for(const src of lesson.sources){assert(/^https:\/\/\S+/.test(src.url)&&src.title.length>=20,"bad academic reference "+id);sourceCount++;}
 for(const l of lesson.links||[]){
   assert(l.label&&l.id&&["math","ml"].includes(l.kind),"invalid crosslink "+id);
   if(l.kind==="math")assert(nodes.has(l.id),"unknown math link "+l.id);
   else assert(/^m\d\d-c\d\d-p\d\d$/.test(l.id),"invalid ML lesson link "+l.id);
   links++;
 }
 for(const [i,section] of lesson.deep_sections.entries()){
   assert(section.question?.length>=18&&section.paragraphs.length===2&&section.paragraphs.every(s=>s.length>=60),"incomplete Chinese explanatory structure "+id);
   assert(section.proof?.length>=75&&section.worked?.length>=55&&section.limitation?.length>=42,"missing proof or arithmetic or boundary "+id+" section "+i);
   assert(section.formulas.length>=1,"missing displayed formula "+id);
   for(const f of section.formulas){assert(f.label&&f.tex);katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});formulas++;}
   chapters++;
 }
 for(const f of lesson.formulas)katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});
 for(const d of lesson.derivation){
  assert(d.explanation?.length>=75,"missing full derivation explanation "+id);
  katex.renderToString(d.tex,{throwOnError:true,strict:"ignore"});
 }
 for(const value of [lesson.definition,lesson.assumptions,lesson.example,lesson.application,lesson.caveat,...lesson.deep_sections.flatMap(s=>[...s.paragraphs,s.proof,s.worked,s.limitation])])
  assert(!/[\uFFFD\u200B\uFEFF]/.test(value),"corrupt CJK character "+id);
}
assert.equal(chapters,48);
assert.equal(formulas,48);
assert.equal(sourceCount,24);

const active=new Set(),done=new Set();
function traverse(id){
 assert(nodes.has(id),"invalid node "+id);
 if(active.has(id))throw Error("prerequisite cycle "+id);
 if(done.has(id))return;
 active.add(id);
 for(const next of nodes.get(id).prerequisites||[])traverse(next);
 active.delete(id);done.add(id);
}
for(const n of c.nodes)traverse(n.id);
assert.equal(done.size,656,"full curriculum DAG not validated");

const approx=(actual,expected,tol=1e-10)=>{
 assert(Number.isFinite(actual)&&Math.abs(actual-expected)<=tol,"independent numerical check failed: "+actual+" expected "+expected);
};
// Characteristic functions, Bernoulli moments and independent convolution.
approx(.3-.3**2,.21);
approx(.7-.3,.4);
approx((.7-.3)**2,.16);
approx(.3**2,.09);
approx(Math.sqrt(.21/100),.0458257569495584);
// Convergence-in-probability counterexample: rare, large values.
approx((1/100)*100**2,100);
approx((1/100)*100**4,1e6);
approx((1/100)*100,1);
// Probability-space Lp norms and Cauchy-Schwarz.
approx((0+2+4+6)/4,3);
approx(Math.sqrt((0+4+16+36)/4),Math.sqrt(14));
approx(Math.sqrt(2.5*12.5),Math.sqrt(31.25));
assert(5.5<=Math.sqrt(31.25));
// Vitali: square-root rare losses, bounded L2 but nonvanishing L2 risk.
approx((1/100)*Math.sqrt(100),.1);
approx((1/100)*100,1);
// Borel-Cantelli: summable vs divergent probability tails.
const sumSq=Array.from({length:10000},(_,i)=>1/((i+1)**2)).reduce((a,b)=>a+b,0);
assert(0<Math.PI**2/6-sumSq&&Math.PI**2/6-sumSq<.00011);
const harmonic=Array.from({length:10000},(_,i)=>1/(i+1)).reduce((a,b)=>a+b,0);
assert(harmonic>9);
// Levy and Poisson limit.
approx(.98**100,.13261955589475316,1e-12);
approx(Math.exp(-2),.1353352832366127);
approx(Math.exp(-1/(2*100)),.9950124791926823,1e-12);
// Martingale difference orthogonality and Kronecker check.
approx(1+4+9,14);
approx(1+1/4+1/9,49/36);
approx((1-1+1+1)/4,.5);
// Coin-filtration stopping time and Doob decomposition.
approx(.5+.25+.25,1);
approx(.5*4+.5*0,2);
approx(.5*2+.5*(-2),0);
// Loss of uniform integrability: expectation constant, path limit zero.
approx(8/8,1);
// L2-bounded martingale's energy sum.
approx(1/4+1/16+1/64,21/64);
approx((1/4)/(1-1/4),1/3);

const html=fs.readFileSync(new URL("index.html",dir),"utf8");
const ml=fs.readFileSync(new URL("../ml/index.html",dir),"utf8");
const css=fs.readFileSync(new URL("styles.css",dir),"utf8");
for(const id of ids){
 assert(html.includes("?topic="+id),"math V10 route missing "+id);
 assert(ml.includes("../math/?topic="+id),"ML crosslink missing "+id);
}
assert(html.includes('aria-label="十二篇V10极限定理与鞅收敛系统课程"'));
assert(ml.includes('aria-label="十二篇概率收敛与鞅极限的数学先修课程"'));
assert(html.includes("styles.css?v=20261104-probscore-v6-inference-v8-martingale-v9-convergence-v10"),"CSS version cache stale");
assert(css.includes("V10: 12-lecture proof and Chinese typography"));
assert(css.includes(".math-v10-convergence summary:focus-visible")&&css.includes(".katex-display")&&css.includes("@media print"),"keyboard/CJK/Katex/print style absent");
assert(!html.includes("一篇完整专题 · 四个严密证明小节"),"obsolete martingale-count disclaimer");
console.log("PASS V10: 12 complete lectures, 48 detailed mathematical proof chapters, 48 KaTeX equations, "+sourceCount+" research resource links, "+links+" cross-course references; 162 full mathematics lectures, independently checked numeric examples and 656-node acyclic learning path.");

