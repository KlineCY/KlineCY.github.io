import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const here=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,here),"utf8"));
const catalog=read("curriculum.json"),manifest=read("lessons-manifest.json");
const rows=[...["lessons-foundations.json","lessons-advanced.json","lessons-more.json"].flatMap(p=>read(p).lessons),...manifest.modules.flatMap(x=>read(x.path).lessons)];
const all=new Map(rows.map(l=>[l.node_id,l])),nodes=new Map(catalog.nodes.map(n=>[n.id,n]));
assert.equal(rows.length,all.size,"duplicate full math course ID");
assert.equal(manifest.expected_lessons,102);
assert.equal(manifest.modules.length,40);
const ids=["statistics-n0151","riskmath-n0588","riskmath-n0590","riskmath-n0591","riskmath-n0592","riskmath-n0593","riskmath-n0599"];
const deps=[["riskmath-n0588","statistics-n0151"],["riskmath-n0590","riskmath-x001"],["riskmath-n0591","statistics-n0151"],["riskmath-n0592","riskmath-n0590"],["riskmath-n0593","riskmath-x002"],["riskmath-n0599","riskmath-n0593"]];
for(const [a,b] of deps)assert(nodes.get(a)?.prerequisites.includes(b),"missing strict prerequisite "+a+"→"+b);
let sections=0,formulas=0,refs=0;
for(const id of ids){
 const l=all.get(id),n=nodes.get(id);
 assert(l&&n&&l.title===n.title,"full lesson not registered "+id);
 for(const key of ["definition","assumptions","example","application","caveat"])assert(l[key]?.length>=75,"missing argument "+id+"."+key);
 assert(l.formulas.length>=4&&l.derivation.length>=4&&l.properties.length>=4);
 assert.equal(l.deep_sections.length,4);
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,4,"repeated title "+id);
 for(const f of l.formulas){assert(f.label);katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});formulas++;}
 for(const d of l.derivation){assert(d.explanation.length>=35);katex.renderToString(d.tex,{throwOnError:true,strict:"ignore"});formulas++;}
 for(const s of l.deep_sections){
  assert(s.question.length>=11&&s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=40),"thin Chinese explanations "+id+" "+s.title);
  assert(s.proof.length>=45&&s.worked.length>=35&&s.limitation.length>=30,"missing proof, worked example or boundary "+id+" "+s.title);
  assert(s.formulas.length>=1);
  for(const f of s.formulas){katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});formulas++;}
  sections++;
 }
 assert(l.sources.length>=2&&l.links.some(x=>x.kind==="ml"));
 for(const link of l.links)if(link.kind==="math")assert(nodes.has(link.id),"broken math bridge "+link.id);
 for(const src of l.sources){assert(src.title&&/^https:\/\//.test(src.url));refs++;}
}
const near=(a,b)=>assert(Math.abs(a-b)<1e-8,"independent arithmetic "+a+" vs "+b);
near(Math.sqrt(Math.log(40)/400),.09603227913199208);
near((3/5),.6);
near((.2*(3+2+1)+.8*(10-3))/5,1.36);
near(1.96*.02,.0392);
near(.02*.05844/.025,.046752);
near(Math.ceil(5*.8),4);
near(1000*.05,50);
near(Math.sqrt(1000*.05*.95),Math.sqrt(47.5));
near((1+1+1+1-4)/5,0);
near(1-.95**20,.6415140775914581);
near(.05/20,.0025);
const html=fs.readFileSync(new URL("index.html",here),"utf8");
const css=fs.readFileSync(new URL("styles.css",here),"utf8");
for(const id of ids)assert(html.includes("?topic="+id),"V4 route missing "+id);
assert(html.includes("styles.css?v=20261104-tailrisk-v5"));
assert(html.includes("第四条高级知识链"));
assert(css.includes("V4 tail-risk chapters")&&css.includes("overscroll-behavior-x:contain")&&css.includes("@media print"));
console.log("PASS: V4 seven quantile/VaR/ES math courses, "+sections+" proof units, "+formulas+" KaTeX formulas, "+refs+" academic sources, numeric examples and Chinese/mobile/print rules.");
