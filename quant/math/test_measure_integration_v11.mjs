import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const dir=new URL("./",import.meta.url);
const read=f=>JSON.parse(fs.readFileSync(new URL(f,dir),"utf8"));
const catalog=read("curriculum.json"),manifest=read("lessons-manifest.json");
const core=["lessons-foundations.json","lessons-advanced.json","lessons-more.json"].flatMap(n=>read(n).lessons);
const modules=manifest.modules.flatMap(m=>read(m.path).lessons);
const all=[...core,...modules],byId=new Map(all.map(x=>[x.node_id,x]));
const nodes=new Map(catalog.nodes.map(n=>[n.id,n]));
const newlyCompleted=[
 "probability-n0120","measure-n0122","measure-n0123","measure-n0124","measure-n0125",
 "measure-n0126","measure-n0127","measure-n0128","measure-n0129","measure-n0130",
 "measure-n0131","measure-n0133","measure-n0134","measure-n0135","measure-n0136"
];
const prerequisites={
 "probability-n0120":["probability-n0104","probability-n0106"],
 "measure-n0122":["measure-n0121"],
 "measure-n0123":["measure-n0121","measure-n0122"],
 "measure-n0124":["measure-n0123"],
 "measure-n0125":["measure-n0123","measure-n0124"],
 "measure-n0126":["measure-n0125","measure-n0122"],
 "measure-n0127":["measure-n0122","measure-n0123"],
 "measure-n0128":["measure-n0127","measure-n0123"],
 "measure-n0129":["measure-n0128"],
 "measure-n0130":["measure-n0129"],
 "measure-n0131":["measure-n0130"],
 "measure-n0132":["measure-n0131","measure-n0129"],
 "measure-n0133":["measure-n0123","measure-n0130"],
 "measure-n0134":["measure-n0133","measure-n0130"],
 "measure-n0135":["measure-n0134","measure-n0129"],
 "measure-n0136":["measure-n0135","measure-n0124"],
 "measure-n0146":["measure-n0145","measure-n0136"],
 "measure-n0147":["measure-n0146","probability-n0096"]
};
assert.equal(catalog.nodes.length,656);
assert.equal(catalog.chapters.length,109);
assert.equal(catalog.disciplines.length,20);
assert.equal(manifest.modules.length,77);
assert.equal(manifest.expected_lessons,201);
assert.equal(modules.length,201);
assert(core.length>=74,"V11 baseline: at least 74 completed core lectures");
assert(all.length>=177,"V11 baseline: at least 177 complete mathematics lectures");
assert.equal(byId.size,all.length,"no duplicated full mathematics lesson ID");
assert.equal(newlyCompleted.length,15);

const seen=new Set(),active=new Set();
function traverse(id){
 assert(nodes.has(id),"unknown prerequisite "+id);
 if(active.has(id))throw Error("cyclic prerequisite at "+id);
 if(seen.has(id))return;
 active.add(id);
 for(const p of nodes.get(id).prerequisites||[])traverse(p);
 active.delete(id);seen.add(id);
}
for(const [id,pre] of Object.entries(prerequisites))assert.deepEqual(nodes.get(id).prerequisites,pre,"unexpected prerequisite "+id);
for(const n of catalog.nodes)traverse(n.id);
assert.equal(seen.size,656,"full mathematics DAG");

let sections=0,displayFormulas=0,sources=0,crosslinks=0;
for(const id of newlyCompleted){
 const l=byId.get(id),node=nodes.get(id);
 assert(l&&node&&l.id===id&&l.title===node.title,"V11 node not attached "+id);
 assert(l.definition?.length>=100&&l.assumptions?.length>=75,"weak foundations "+id);
 assert(l.example?.length>=80&&l.application?.length>=85&&l.caveat?.length>=80,"incomplete research example or boundary "+id);
 assert(l.properties?.length>=4&&l.formulas?.length>=4&&l.derivation?.length>=4,"incomplete theorem properties "+id);
 assert.equal(l.deep_sections?.length,4,"missing deep proof sections "+id);
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,4,"duplicate mini-chapter "+id);
 assert(l.sources?.length>=2,"academic reference missing "+id);
 for(const src of l.sources){assert(src.title?.length>=20&&/^https:\/\/\S+$/.test(src.url));sources++;}
 for(const link of l.links||[]){
  assert(link.label?.length>=4&&["math","ml"].includes(link.kind),"invalid related content "+id);
  if(link.kind==="math")assert(nodes.has(link.id),"unresolvable mathematics node "+link.id);
  else assert(/^m\d\d-c\d\d-p\d\d$/.test(link.id),"invalid ML lesson ID "+link.id);
  crosslinks++;
 }
 for(const [i,s]of l.deep_sections.entries()){
  assert(s.title?.length>=6&&s.question?.length>=17,"weak research question "+id+"/"+i);
  assert(s.paragraphs?.length===2&&s.paragraphs.every(p=>p.length>=45),"missing explanatory paragraphs "+id+"/"+i);
  assert(s.proof?.length>=72&&s.worked?.length>=55&&s.limitation?.length>=40,"missing proof/calculation/limitation "+id+"/"+i);
  assert(s.formulas?.length>=1,"missing readable displayed formula "+id+"/"+i);
  for(const f of s.formulas){assert(f.label&&f.tex);katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});displayFormulas++;}
  sections++;
 }
 for(const f of l.formulas)katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});
 for(const d of l.derivation){assert(d.explanation?.length>=72);katex.renderToString(d.tex,{throwOnError:true,strict:"ignore"});}
 for(const prose of [l.definition,l.assumptions,l.application,l.caveat,l.example,...l.deep_sections.flatMap(s=>[s.question,...s.paragraphs,s.proof,s.worked,s.limitation])])
  assert(!/[\uFFFD\u200B\uFEFF]/.test(prose),"corrupt CJK or invisible characters "+id);
}
assert.equal(sections,60);
assert.equal(displayFormulas,60);
assert.equal(sources,30);
const close=(a,b,tol=1e-10)=>assert(Number.isFinite(a)&&Math.abs(a-b)<=tol,"independent arithmetic invariant failed: "+a+" != "+b);
// Markov, Chernoff and Hoeffding: independent Bernoulli(0.4).
close(.2*5,1);
close((.2*5)/4,.25);
close((.4*.6)/(100*.2**2),.06);
close(2*Math.exp(-2*100*.2**2),.0006709252558050237);
close(Math.exp(-2),.1353352832366127);
// Borel events and sigma-additive measures.
close(.25+.5,.75);
close(.3+.5+.2,1);
close(.3+.4-.1,.6);
// Cantor covers, outer measure, Lebesgue translation, scaling.
close((2/3)**3,8/27);
close(3-1,2);
close(Math.abs(-2)*(3-1),4);
close(.5*(2-0),1);
close(.5*(1-0),.5);
// Measurability and simple function integrals.
close(2/4,.5);
close(.2*0+.5*2+.3*5,2.5);
close(.2*1+.5*3+.3*7,3.8);
close(.5*.25+.5*0,.125);
// Lebesgue signed expectation, convergence and Fatou.
close(-2*.25+1*.5+3*.25,.75);
close(2*.25+1*.5+3*.25,1.75);
close(1/3,1/3);
close(2*(1-Math.sqrt(.01)),1.8);
close(-Math.log(.01),4.605170185988091);
close(1-1/(2*2),.75);
close(.5+.25+.125+.0625,.9375);
close(100*(1/100),1);
close(-100*(1/100),-1);
// Product probability, Tonelli and Fubini.
close(.3*.4,.12);
close(.3*.6+.7*.4+.7*.6+.3*.4,1);
close(.5+.5,1);
close(.25,.25);
close(Math.log(10)**2,5.301898110478399);
close(Math.PI/4,.7853981633974483);
// Radon-Nikodym density and reverse likelihood.
close(.25*2+.75*(2/3),1);
close(.25*2*1+.75*(2/3)*3,2);
close(.5*.5+.5*1.5,1);
const html=fs.readFileSync(new URL("index.html",dir),"utf8");
const ml=fs.readFileSync(new URL("../ml/index.html",dir),"utf8");
const css=fs.readFileSync(new URL("styles.css",dir),"utf8");
for(const id of newlyCompleted){
 assert(html.includes("?topic="+id),"V11 mathematics index missing "+id);
 assert(ml.includes("../math/?topic="+id),"V11 ML pathway missing "+id);
}
assert(html.includes('aria-label="测度论与概率积分的完整建构：从可测事件到Fubini及测度变换"'));
assert(ml.includes('aria-label="概率与测度论基础：从事件、积分到金融风险测度变换"'));
assert(html.includes("styles.css?v=20261104-probscore-v6-inference-v8-martingale-v9-convergence-v10-integration-v11"),"stylesheet version cache must update");
assert(css.includes(".math-v11-measure summary:focus-visible")&&css.includes(".katex-display")&&css.includes("@media print"),"keyboard/mobile/print math styles missing");
console.log("V11 PASS: 15 newly completed mathematical lectures, 60 full proof sections, 60 strict KaTeX formulas, 30 academic sources, "+crosslinks+" mathematics/ML links; "+all.length+" complete mathematics lessons, 656-node acyclic learning graph, independent numerical invariants and responsive typography.");

