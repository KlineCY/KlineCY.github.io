import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const c=read("curriculum.json"),m=read("lessons-manifest.json");
const allModules=m.modules.flatMap(x=>read(x.path).lessons);
const core=["lessons-foundations.json","lessons-advanced.json","lessons-more.json"].flatMap(readable=>read(readable).lessons);
const active=[...core,...allModules],unique=new Set(active.map(l=>l.node_id));
const additions=["a","b","c"].flatMap(x=>read("lessons-phase-17-"+x+".json").lessons);
const ids=["econometrics-n0211","econometrics-n0213","econometrics-n0214","econometrics-n0215",
"econometrics-n0216","econometrics-n0218","econometrics-n0219","econometrics-n0220",
"econometrics-n0221","econometrics-n0222","econometrics-n0228","econometrics-n0231"];
assert.equal(m.modules.length,49);
assert.equal(m.expected_lessons,129);
assert.equal(allModules.length,129);
assert.equal(core.length,123);
assert.equal(active.length,252);
assert.equal(unique.size,252,"duplicate complete lesson IDs");
const v18Ids=new Set(["finance-x001","finance-x002","finance-x003","finance-x004","finance-x005","finance-x006","optimization-x001","optimization-x002","optimization-x003","optimization-x004","optimization-x005","optimization-x006"]);
assert.equal(active.filter(l=>!v18Ids.has(l.node_id)).length,240,"historical V17 240 complete lectures lost");
assert.deepEqual(additions.map(x=>x.id),ids,"stage V17 learning order changed");
assert.equal(c.nodes.length,656);
const nodes=new Map(c.nodes.map(x=>[x.id,x]));
let formulas=0,sections=0,sources=0,links=0;
for(const l of additions){
 assert(l.id===l.node_id&&nodes.get(l.id)?.title===l.title,"curriculum mapping changed "+l.id);
 assert(l.definition.length>=95&&l.assumptions.length>=75&&l.example.length>=95,"thin core lecture "+l.id);
 assert(l.properties.length>=3&&l.derivation.length>=3&&l.formulas.length>=4);
 assert.equal(l.deep_sections.length,4);
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,4);
 assert(l.application.length>=80&&l.caveat.length>=55,"missing financial application or boundary "+l.id);
 for(const s of l.deep_sections){
  assert(s.question.length>=12&&s.paragraphs.length>=2&&s.paragraphs.every(p=>p.length>=44),"incomplete CJK explanation "+l.id+" "+s.title);
  assert(s.proof.length>=40&&s.worked.length>=30&&s.limitation.length>=20,"proof, arithmetic or limitation too thin "+l.id+" "+s.title);
  for(const eq of s.formulas){katex.renderToString(eq.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++;}
  sections++;
 }
 for(const e of [...l.formulas,...l.derivation]){katex.renderToString(e.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++;}
 for(const link of l.links){
  if(link.kind==="math")assert(nodes.has(link.id),"broken math cross-reference "+l.id+" -> "+link.id);
  else assert(/^m\d\d-c\d\d-p\d\d$/.test(link.id),"invalid ML cross-reference "+l.id+" -> "+link.id);
  links++;
 }
 for(const src of l.sources){assert(src.title.length>18&&/^https:\/\//.test(src.url));sources++;}
 assert(!JSON.stringify(l).includes("\ufffd"),"corrupted Chinese characters "+l.id);
}
assert.equal(sections,48);
assert.equal(formulas,132);
const visiting=new Set(),done=new Set();
function visit(id){assert(nodes.has(id));if(visiting.has(id))throw Error("prerequisite cycle "+id);if(done.has(id))return;visiting.add(id);for(const p of nodes.get(id).prerequisites||[])visit(p);visiting.delete(id);done.add(id)}
for(const n of c.nodes)visit(n.id);
assert.equal(done.size,656,"all mathematics prerequisites must be acyclic");
assert.deepEqual(nodes.get("econometrics-n0218").prerequisites,["econometrics-n0215","econometrics-n0216"]);
assert.deepEqual(nodes.get("econometrics-n0222").prerequisites,["econometrics-n0221","econometrics-n0219"]);
const near=(x,y,id)=>assert(Math.abs(x-y)<1e-9,"numerical case failed "+id+" "+x+" != "+y);
const arithmetic=[
["econometrics-n0211",2+1/4,2.25,"2.25"],
["econometrics-n0213",2*9/(9+3),1.5,"1.5"],
["econometrics-n0214",5/2,2.5,"2.5"],
["econometrics-n0215",(6+1)/2,3.5,"3.5"],
["econometrics-n0216",98*.01/.99,.9898989898989898,"0.9899"],
["econometrics-n0218",9/4.5,2,"9/4.5=2"],
["econometrics-n0219",10*(.4**2+.2**2),2,"J=2"],
["econometrics-n0220",3-0,3,"β=3"],
["econometrics-n0221",9/17,.5294117647058824,"9/17"],
["econometrics-n0222",100*(.2**2+.2**2),8,"=8"],
["econometrics-n0228",8/(4**2),.5,"0.5"],
["econometrics-n0231",.5*(4-1)+.5*(5-3),2.5,"2.5"]
];
for(const [id,value,expected,token] of arithmetic){near(value,expected,id);assert(additions.find(l=>l.id===id).example.includes(token),"missing independently checked arithmetic "+id);}
assert.equal(arithmetic.length,12);
const index=fs.readFileSync(new URL("index.html",root),"utf8");
const ml=fs.readFileSync(new URL("../ml/index.html",root),"utf8");
const css=fs.readFileSync(new URL("styles.css",root),"utf8");
const mlcss=fs.readFileSync(new URL("../ml/atlas.css",root),"utf8");
assert(index.includes('aria-label="V17十二篇计量识别、工具变量与GMM深入学习路线"'));
assert(ml.includes('aria-label="V17计量经济学与金融机器学习识别先修"'));
for(const id of ids){
 assert(index.includes("?topic="+id),"mathematics index missing "+id);
 assert(ml.includes("../math/?topic="+id),"ML-to-mathematics link missing "+id);
}
for(const [name,style] of [["math",css],["ml",mlcss]]){
 assert(style.includes("overflow-x:auto"),"formula overflow rule missing "+name);
 assert(style.includes("line-break:strict"),"Chinese typography rule missing "+name);
 assert(style.includes("@media print"),"print layout safeguards missing "+name);
 assert(style.includes(":focus-visible"),"keyboard focus missing "+name);
}
console.log("V17 PASS: 12 deepened complete econometrics lectures, 48 coherent sections, "+formulas+" strict KaTeX parses, "+sources+" source refs, "+links+" crosslinks, 252 loaded complete maths courses (V17 historical 240 retained), 656-node acyclic prerequisite graph, 12 independent arithmetic cases, CJK/print rules.");
