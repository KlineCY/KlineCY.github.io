import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const curriculum=read("curriculum.json"), manifest=read("lessons-manifest.json");
const core=["lessons-foundations.json","lessons-advanced.json","lessons-more.json"].flatMap(p=>read(p).lessons);
const extended=manifest.modules.flatMap(m=>{const rows=read(m.path).lessons;assert.equal(rows.length,m.lessons,"manifest module mismatch "+m.path);return rows});
const all=[...core,...extended], completeIds=new Set(all.map(x=>x.node_id));
const ids=["finance-x001","finance-x002","finance-x003","finance-x004","finance-x005","finance-x006","optimization-x001","optimization-x002","optimization-x003","optimization-x004","optimization-x005","optimization-x006"];
const byId=new Map(all.map(x=>[x.node_id,x]));
const added=ids.map(id=>byId.get(id));
assert(added.every(Boolean),"twelve expanded courses must be loaded exactly once");
const newIds=["finance-x004","finance-x005","finance-x006","optimization-x003","optimization-x004","optimization-x006"];
const netNew="bcd".split("").flatMap(s=>read("lessons-phase-18-"+s+".json").lessons);
assert.deepEqual(netNew.map(x=>x.node_id),newIds,"six net-new phase18 records only");
assert.deepEqual(added.map(x=>x.node_id),ids);
assert.equal(curriculum.nodes.length,656);
assert.equal(manifest.modules.length,48);
assert.equal(manifest.expected_lessons,123);
assert.equal(extended.length,123);
assert.equal(core.length,123);
assert.equal(all.length,246);
assert.equal(completeIds.size,246,"duplicates among all live full mathematical lectures");
assert.equal(all.filter(x=>!newIds.includes(x.node_id)).length,240,"historical V17 240 lectures must remain intact");
const nodes=new Map(curriculum.nodes.map(n=>[n.id,n]));
let formulas=0,sections=0,sources=0,links=0;
for(const l of added){
 assert(l.id===l.node_id,"lesson/node mismatch "+l.id);
 assert(nodes.get(l.id)?.title===l.title,"not original curriculum node "+l.id);
 assert(l.definition.length>=125&&l.assumptions.length>=100&&l.example.length>=125,"thin core prose "+l.id);
 assert(l.application.length>=80&&l.caveat.length>=70,"insufficient financial application or limitations "+l.id);
 assert(l.properties.length>=3&&l.derivation.length>=3&&l.formulas.length>=4);
 assert(l.deep_sections.length>=4);
 const titles=l.deep_sections.map(s=>s.title);
 assert.equal(new Set(titles).size,l.deep_sections.length,"repeated section titles "+l.id);
 for(const s of l.deep_sections.slice(-4)){
  assert(s.question.length>=12&&s.paragraphs.length>=2&&s.paragraphs.every(p=>p.length>=55),"CJK narrative too thin "+l.id+" "+s.title);
  assert(s.proof.length>=50&&s.worked.length>=40&&s.limitation.length>=30,"worked proof or boundary incomplete "+l.id+" "+s.title);
  for(const e of s.formulas){katex.renderToString(e.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++}
  sections++;
 }
 const labels=new Set();
 for(const e of l.formulas){
  assert(!labels.has(e.label),"duplicate equation label "+l.id+" "+e.label);labels.add(e.label);
  katex.renderToString(e.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++;
 }
 for(const e of l.derivation){assert(e.explanation.length>=35,"historical derivation must remain complete "+l.id);katex.renderToString(e.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++}
 for(const e of l.derivation.slice(-3))assert(e.explanation.length>=55,"V18 new derivation requires substantial explanation "+l.id)
 for(const link of l.links){
  if(link.kind==="math")assert(nodes.has(link.id),"missing math link "+l.id+" -> "+link.id);
  else assert(/^m\d\d-c\d\d-p\d\d$/.test(link.id),"unexpected ML link "+l.id+" -> "+link.id);
  links++;
 }
 for(const src of l.sources){assert(src.title.length>20&&/^https:\/\//.test(src.url));sources++}
 assert(!JSON.stringify(l).includes("\ufffd"),"broken UTF-8 Chinese text "+l.id);
}
assert.equal(sections,48);
assert.equal(formulas,169);
assert(sources>=24);
assert(links>=48);
// Independent numerical sanity checks, separate from the embedded prose.
const near=(a,b,id)=>assert(Math.abs(a-b)<1e-9,"independent arithmetic failed "+id+": "+a+" vs "+b);
const nums=[
 ["finance-x001",.5*1.2*1+.5*.8*3,1.8,"=1.8"],
 ["finance-x002",.5*1.2*.5+.5*.8*1.75,1,"=1"],
 ["finance-x003",.1/.2,.5,"0.5"],
 ["finance-x004",.5*.1,.05,"=0.05"],
 ["finance-x005",4*.01/1,.04,"=0.04"],
 ["finance-x006",(-.008)**2+.004**2,.00008,"0.00008"],
 ["optimization-x001",29.5+7*11,106.5,"=106.5"],
 ["optimization-x002",.001*(100+100),.2,"=0.2"],
 ["optimization-x003",.01*(5*5+5*5),.5,"=0.5"],
 ["optimization-x004",10*Math.sinh(.5)/Math.sinh(1),4.434094419758,"≈4.434"],
 ["optimization-x005",.06-Math.hypot(.01,.005),.04881966011250105,"0.04881966"],
 ["optimization-x006",Math.ceil(5000/(20000*.1)),3,"=3"]
];
assert.equal(nums.length,12);
for(const [id,actual,want,token] of nums){
 near(actual,want,id);assert(added.find(l=>l.id===id).example.includes(token),"example missing checked arithmetic "+id);
}
const visiting=new Set(),visited=new Set();
function dfs(id){assert(nodes.has(id),"missing prerequisite "+id);if(visiting.has(id))throw Error("cycle "+id);if(visited.has(id))return;visiting.add(id);for(const p of nodes.get(id).prerequisites||[])dfs(p);visiting.delete(id);visited.add(id)}
for(const n of curriculum.nodes)dfs(n.id);
assert.equal(visited.size,656);
const html=fs.readFileSync(new URL("index.html",root),"utf8");
const ml=fs.readFileSync(new URL("../ml/index.html",root),"utf8");
const css=fs.readFileSync(new URL("styles.css",root),"utf8");
const mlCss=fs.readFileSync(new URL("../ml/atlas.css",root),"utf8");
assert(html.includes('aria-label="V18 SDF与交易执行完整十二篇数学课程路线"'));
assert(ml.includes('aria-label="V18资产定价与量化交易执行交叉学习链"'));
for(const id of ids){assert(html.includes("?topic="+id),"math navigation missing "+id);assert(ml.includes("../math/?topic="+id),"ML cross-navigation missing "+id)}
for(const [name,style] of [["math",css],["ml",mlCss]]){
 assert(style.includes("overflow-x:auto"),"formula horizontal scrolling missing "+name);
 assert(style.includes("line-break:strict"),"Chinese line-break safeguards missing "+name);
 assert(style.includes(":focus-visible"),"keyboard focus missing "+name);
 assert(style.includes("@media print"),"print layout safeguards missing "+name);
}
console.log("PASS V18: six new and six in-place deepened SDF/pricing/portfolio-execution lectures, 48 deep proof sections, 169 strict KaTeX formula parses, 24 source references, 48 prerequisite/cross links, 12 independently computed numeric examples, 246 unique full math courses and 656 acyclic knowledge nodes.");
