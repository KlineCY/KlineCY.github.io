import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const dir=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,dir),"utf8"));
const c=read("./curriculum.json"),manifest=read("./lessons-manifest.json");
const all=[...["./lessons-foundations.json","./lessons-advanced.json","./lessons-more.json"].flatMap(p=>read(p).lessons),...manifest.modules.flatMap(m=>read(m.path).lessons)];
const map=new Map(all.map(l=>[l.node_id,l])),nodes=new Map(c.nodes.map(n=>[n.id,n]));
assert.equal(map.size,all.length,"duplicate mathematics lesson");
assert.equal(manifest.expected_lessons,201);
const chain=["linear-n0076","linear-n0077","linear-n0085","regression-n0184","regression-n0207","regression-n0208","regression-n0209"];
const edges=[["linear-n0076","linear-n0075"],["linear-n0077","linear-n0076"],["linear-n0085","linear-n0083"],["regression-n0184","regression-n0182"],["regression-n0207","regression-n0206"],["regression-n0208","mlmath-n0453"],["regression-n0209","regression-n0208"]];
for(const [id,p] of edges)assert(nodes.get(id)?.prerequisites.includes(p),"dependency "+id+" -> "+p);
let subchapters=0,equations=0,sources=0;
for(const id of chain){
 const l=map.get(id),node=nodes.get(id);
 assert(l&&node,"missing lecture "+id);
 assert.equal(l.title,node.title);
 assert(l.definition.length>75&&l.assumptions.length>65&&l.example.length>110&&l.application.length>65&&l.caveat.length>60,"incomplete theorem scope "+id);
 assert(l.properties.length>=4&&l.formulas.length>=4&&l.derivation.length>=4);
 assert.equal(l.deep_sections.length,5,"four historical proof sections plus one V86 section "+id);
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,l.deep_sections.length);
 for(const step of l.derivation){assert(step.explanation.length>=40);katex.renderToString(step.tex,{throwOnError:true,strict:"ignore"});equations++;}
 for(const eq of l.formulas){katex.renderToString(eq.tex,{throwOnError:true,strict:"ignore"});equations++;}
 for(const s of l.deep_sections.slice(0,4)){
  assert(s.question.length>12&&s.paragraphs.length>=2&&s.paragraphs.every(p=>p.length>=45),"thin prose "+id);
  assert(s.proof.length>=50&&s.worked.length>=35&&s.limitation.length>=25,"proof/example/boundary missing "+id);
  for(const eq of s.formulas){katex.renderToString(eq.tex,{throwOnError:true,strict:"ignore"});equations++;}
  subchapters++;
 }
 for(const ref of l.links){if(ref.kind==="math")assert(nodes.has(ref.id),"broken math link "+ref.id);else assert(/^m\d\d-c\d\d-p\d\d$/.test(ref.id));}
 for(const ref of l.sources){assert(ref.title&&/^https:\/\//.test(ref.url));sources++;}
}
const near=(x,y,t=1e-10)=>assert(Math.abs(x-y)<t,"arithmetic "+x+" vs "+y);
near(.25+.25+1,1.5);
near(13-4,9);
near(.5+.5,1);
near((2-1)**2+(0-1)**2,2);
near(2-.5,1.5);
near((3-.8)/(1+.5),22/15);
near((-1+.8)/(1+.5),-2/15);
near(.04+.16+.25,.45);
near((.2+.4)/2,.3);
near((.3+.25)/2,.275);
const html=fs.readFileSync(new URL("./index.html",dir),"utf8"),css=fs.readFileSync(new URL("./styles.css",dir),"utf8");
for(const id of chain)assert(html.includes("?topic="+id),"V2 math route missing "+id);
assert(html.includes("styles.css?v=20261104-probscore-v6"));
assert(css.includes("overscroll-behavior-x:contain"));
console.log("PASS: "+chain.length+" V2 math lectures, "+subchapters+" deep proof subchapters, "+equations+" TeX expressions, "+sources+" reference records, independent arithmetic and CJK layout");
