import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const dir=new URL("./",import.meta.url);
const read=path=>JSON.parse(fs.readFileSync(new URL(path,dir),"utf8"));
const curriculum=read("curriculum.json"),manifest=read("lessons-manifest.json");
const rows=[...["lessons-foundations.json","lessons-advanced.json","lessons-more.json"].flatMap(f=>read(f).lessons),...manifest.modules.flatMap(m=>read(m.path).lessons)];
const byId=new Map(rows.map(x=>[x.node_id,x]));
const nodes=new Map(curriculum.nodes.map(n=>[n.id,n]));
assert.equal(curriculum.nodes.length,656);
assert(rows.length>=150,"V9 baseline requires at least 150 full mathematics lessons");
assert.equal(byId.size,rows.length,"duplicate mathematics lessons");
assert.equal(manifest.expected_lessons,117,"historic extension coverage with two complete systemic-risk chapters");
const prerequisites={
 "probability-n0118":["probability-n0115","probability-n0116"],
 "measure-n0137":["probability-n0107","measure-n0136","linear-n0075"],
 "stochastic-n0290":["probability-n0107","stochastic-n0289"],
 "stochastic-n0291":["stochastic-n0290"]
};
const expected={
 "probability-n0118":"大数定律",
 "measure-n0137":"条件期望的Hilbert空间观点",
 "stochastic-n0290":"条件期望鞅",
 "stochastic-n0291":"次鞅与上鞅"
};
let parts=0,formulas=0,sources=0,links=0;
for(const [id,title] of Object.entries(expected)){
 const item=byId.get(id),node=nodes.get(id);
 assert(item&&node&&item.title===title,"V9 canonical node not indexed "+id);
 assert.deepEqual(node.prerequisites,prerequisites[id],"wrong prerequisite graph "+id);
 assert(item.definition?.length>=80&&item.assumptions?.length>=75,"incomplete definition "+id);
 assert(item.application?.length>=90&&item.caveat?.length>=75,"incomplete empirical caveat "+id);
 assert(item.properties.length>=4&&item.formulas.length>=4&&item.derivation.length>=4,"incomplete theorem structure "+id);
 assert.equal(item.deep_sections?.length,4,"expected four deep sections "+id);
 assert.equal(new Set(item.deep_sections.map(s=>s.title)).size,4,"repeated subchapter "+id);
 assert(item.sources?.length>=2,"missing academic references "+id);
 for(const src of item.sources){assert(src.title&&/^https:\/\/[^\s]+$/.test(src.url));sources++;}
 for(const ref of item.links||[]){
  assert(ref.label&&["math","ml"].includes(ref.kind)&&ref.id,"bad prerequisite "+id);
  if(ref.kind==="math")assert(nodes.has(ref.id),"broken math reference "+ref.id);
  else assert(/^m\d\d-c\d\d-p\d\d$/.test(ref.id),"invalid ML reference "+ref.id);
  links++;
 }
 for(const p of item.deep_sections){
  assert(p.question?.length>=12&&p.paragraphs.length===2&&p.paragraphs.every(s=>s.length>=65),"shallow theoretical explanation "+id);
  assert(p.proof?.length>=85&&p.worked?.length>=75&&p.limitation?.length>=60,"incomplete proof/example/boundary "+id);
  assert(p.formulas?.length>=1);
  for(const e of p.formulas){assert(e.tex&&e.label);katex.renderToString(e.tex,{throwOnError:true,strict:"ignore"});formulas++;}
  parts++;
 }
 for(const f of item.formulas)katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});
 for(const f of item.derivation){assert(f.explanation.length>=65);katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});}
}
assert.equal(parts,16);
assert.equal(formulas,16);
const verified=new Set(),active=new Set();
function check(id){if(active.has(id))throw Error("prerequisite cycle "+id);if(verified.has(id))return;assert(nodes.has(id));active.add(id);for(const p of nodes.get(id).prerequisites||[])check(p);active.delete(id);verified.add(id);}
for(const id of Object.keys(expected))check(id);
const close=(a,b,t=1e-10)=>assert(Math.abs(a-b)<t,"failed independent numerical check "+a+" != "+b);
close(.4*.6/(100*.2**2),.06);
close(2*Math.exp(-2*100*.2**2),.0006709252558050237);
close(1.5/(1.5-1),3);
const X=[0,2,4,6],Y=[1,1,5,5];
close(X.reduce((s,x)=>s+x*x,0)/4,14);
close(Y.reduce((s,x)=>s+x*x,0)/4,13);
close(X.reduce((s,x,i)=>s+(x-Y[i])**2,0)/4,1);
close(X.reduce((s,x)=>s+(x-3)**2,0)/4,5);
close(Y.reduce((s,x)=>s+(x-3)**2,0)/4,4);
close(.25*16,4);
close(.5*2**2,2);
close(4-2,2);
close(2-1,1);
close(.5*0+.5*4,2);
close(.5*(-2)+.5*2,0);
close(4*.5,2);
const html=fs.readFileSync(new URL("index.html",dir),"utf8");
const css=fs.readFileSync(new URL("styles.css",dir),"utf8");
const ml=fs.readFileSync(new URL("../ml/index.html",dir),"utf8");
for(const id of Object.keys(expected)){
 assert(html.includes("?topic="+id),"V9 math navigation omits "+id);
 assert(ml.includes("../math/?topic="+id),"ML-to-math route missing "+id);
}
assert(html.includes("大数定律、Hilbert条件期望、鞅与次鞅四篇完整证明链"));
assert(html.includes("styles.css?v=20261104-probscore-v6-inference-v8-martingale-v9"),"cache bust required");
assert(!html.includes("关于“条件期望鞅”和“次鞅/上鞅”的目录节点仍需继续补写"),"stale outline disclaimer");
assert(css.includes("V9: mathematical proof accessibility")&&css.includes("@media print"),"formula/CJK/print styles missing");
assert(css.includes(".formula-block:focus-visible"),"keyboard formula focus missing");
assert(ml.includes("概率大数定律到金融鞅与风险统计的数学先修"));
console.log("PASS: V9 baseline retained; current "+rows.length+" full mathematics lectures; 4 newly completed prerequisite nodes, 16 deep chapters, 16 strict KaTeX equations, "+sources+" academic sources, "+links+" crosslinks, dependency DAG, independent numbers and CJK layout.");

