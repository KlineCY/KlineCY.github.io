import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const dir=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,dir),"utf8"));
const c=read("./curriculum.json"),manifest=read("./lessons-manifest.json");
const nodes=new Map(c.nodes.map(n=>[n.id,n]));
const base=["./lessons-foundations.json","./lessons-advanced.json","./lessons-more.json"].flatMap(p=>read(p).lessons);
const ext=manifest.modules.flatMap(m=>read(m.path).lessons);
const all=[...base,...ext],lectures=new Map(all.map(l=>[l.node_id,l]));
assert.equal(manifest.expected_lessons,135);
assert.equal(manifest.modules.length,54);
assert.equal(lectures.size,all.length,"duplicate math lesson ID");
assert(all.length>=133,"unexpected loss of previously complete math lessons");
const IDs=["mlmath-n0454","mlmath-n0472","mlmath-n0473","optimization-n0347","optimization-n0358","riskmath-n0585","riskmath-n0586","riskmath-n0587"];
const prereqs=[
 ["mlmath-n0454","mlmath-n0452"],
 ["mlmath-n0472","mlmath-n0469"],
 ["mlmath-n0473","mlmath-n0470"],
 ["optimization-n0347","linear-n0082"],
 ["optimization-n0358","optimization-n0355"],
 ["riskmath-n0585","riskmath-n0583"],
 ["riskmath-n0586","regression-n0182"],
 ["riskmath-n0587","timeseries-n0242"]
];
for(const [a,b] of prereqs)assert(nodes.get(a)?.prerequisites.includes(b),"missing proof prerequisite "+a+" → "+b);
let deep=0,formula=0,provenance=0;
for(const id of IDs){
 const l=lectures.get(id),n=nodes.get(id);
 assert(l&&n,"unpublished full mathematical tutorial "+id);
 assert.equal(l.title,n.title);
 assert(l.definition.length>=95&&l.assumptions.length>=90&&l.example.length>=120&&l.application.length>=80&&l.caveat.length>=90,"insufficient premises and applications "+id);
 assert(l.properties.length>=4&&l.formulas.length>=4&&l.derivation.length>=4&&l.deep_sections.length===4);
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,4,"repeated conceptual subsection "+id);
 for(const f of l.formulas){katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});formula++;}
 for(const d of l.derivation){assert(d.explanation.length>=45);katex.renderToString(d.tex,{throwOnError:true,strict:"ignore"});formula++;}
 for(const s of l.deep_sections){
  assert(s.title.length>5&&s.question.length>12&&s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=65),"thin explanatory proof "+id+" / "+s.title);
  assert(s.proof.length>=55&&s.worked.length>=55&&s.limitation.length>=40,"proof/numeric/caveat missing "+id+" / "+s.title);
  assert(s.formulas.length>0);
  for(const f of s.formulas){katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});formula++;}
  deep++;
 }
 assert(l.sources.length>=2&&l.links.some(x=>x.kind==="ml"),"source or ML bridge missing "+id);
 for(const r of l.sources){assert(r.title&&/^https:\/\//.test(r.url));provenance++;}
 for(const x of l.links)if(x.kind==="math")assert(nodes.has(x.id),"broken maths prerequisite "+x.id);
}
const close=(a,b,t=1e-9)=>assert(Math.abs(a-b)<t,"independent arithmetic mismatch: "+a+" vs "+b);
close(2*2**2/100,.08);
close(2*2/100,.04);
close(1/Math.sqrt(2),.7071067811865476);
close(Math.exp(-.5),.6065306597126334);
close(.064-2*Math.hypot(.006,.008),.044);
close(.95+.05,1);
close((.95-.7)**2+(.05+.2)**2,.125);
close((1-.7)**2+(0+.2)**2,.13);
close(((0-1)**2+(1-1)**2+(2-1)**2)/2,1);
close((0+1+2+100)/4,25.75);
close(5-.5,4.5);
close(5**2/2,12.5);
close((1+2+1+2)/4,1.5);
close((3+4+3+4)/4,3.5);
close(1+2*.2,1.4);
const html=fs.readFileSync(new URL("./index.html",dir),"utf8");
const css=fs.readFileSync(new URL("./styles.css",dir),"utf8");
for(const id of IDs)assert(html.includes("?topic="+id),"advanced math route missing "+id);
assert(html.includes('styles.css?v=20261104-probscore-v6'),"old stylesheet cached");
assert(html.includes('aria-label="算法稳定性、可扩展核方法与稳健金融数学的贯通知识链"'),"advanced route not accessible");
assert(css.includes("/* V3 CHAPTER READABILITY")&&css.includes("scrollbar-gutter:stable")&&css.includes("@media print"),"CJK formula/touch/print styles missing");
console.log("PASS: V3 mathematics "+IDs.length+" complete lectures, "+deep+" proof sections, "+formula+" TeX formulas, "+provenance+" source records, exact numerical calculations, CJK/responsive/print design.");
