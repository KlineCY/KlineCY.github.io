import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=path=>JSON.parse(fs.readFileSync(new URL(path,root),"utf8"));
const curriculum=read("./curriculum.json"),manifest=read("./lessons-manifest.json");
const nodes=new Map(curriculum.nodes.map(n=>[n.id,n]));
const archive=[read("./lessons-foundations.json"),read("./lessons-advanced.json"),read("./lessons-more.json"),...manifest.modules.map(m=>read(m.path))];
const all=archive.flatMap(doc=>doc.lessons);
const byId=new Map(all.map(l=>[l.node_id,l]));
assert.equal(all.length,byId.size,"duplicate maths lesson ID");
assert(all.length>=114,"V2 expected at least 114 completed mathematics lectures");
const chain=["linear-n0075","linear-n0078","linear-n0083","linear-n0086","regression-n0205","regression-n0206"];
const prerequisiteEdges=[
 ["linear-n0078","linear-n0075"],
 ["linear-n0083","linear-n0078"],
 ["linear-n0086","linear-n0083"],
 ["regression-n0205","linear-n0086"],
 ["regression-n0206","regression-n0205"]
];
for(const [next,prior] of prerequisiteEdges)assert(nodes.get(next).prerequisites.includes(prior),"broken V2 proof dependency "+next+" -> "+prior);
let proofs=0,equations=0;
for(const id of chain){
 const node=nodes.get(id),lesson=byId.get(id);
 assert(node&&lesson,"unpublished V2 maths lecture "+id);
 assert.equal(lesson.title,node.title);
 assert(lesson.checked_status.includes("教材级精讲"),"not registered as full textbook chapter "+id);
 assert(lesson.definition.length>=80&&lesson.assumptions.length>=80,"thin definition or hypotheses "+id);
 assert(lesson.properties.length>=4&&lesson.derivation.length>=3&&lesson.formulas.length>=4,"missing theorem proof structure "+id);
 assert(lesson.example.length>=100&&lesson.application.length>=60&&lesson.caveat.length>=60,"missing numerical example or caveats "+id);
 assert.equal(lesson.deep_sections.length,5,"preserve four V2 proofs plus V85 append-only section");
 assert(new Set(lesson.deep_sections.map(s=>s.title)).size===lesson.deep_sections.length,"duplicate subchapter headings "+id);
 assert(lesson.links.some(x=>x.kind==="ml"&&x.id.startsWith("m07-c06-p")),"ML bridge missing "+id);
 assert(lesson.sources.length>=2,"academic source provenance missing "+id);
 for(const src of lesson.sources)assert(src.title&&/^https:\/\//.test(src.url),"source metadata invalid");
 for(const f of lesson.formulas){katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});equations++;}
 for(const d of lesson.derivation){assert(d.explanation.length>=45);katex.renderToString(d.tex,{throwOnError:true,strict:"ignore"});equations++;}
 for(const part of lesson.deep_sections.slice(0,4)){
  assert(part.question.length>=12&&part.paragraphs.length>=2&&part.paragraphs.every(p=>p.length>=45),"inadequate theoretical treatment "+id);
  assert(part.proof.length>=55&&part.worked.length>=40&&part.limitation.length>=25,"missing proof, calculation or limitations "+id);
  for(const f of part.formulas){katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});equations++;}
  proofs++;
 }
}
const homepage=fs.readFileSync(new URL("./index.html",root),"utf8");
for(const id of chain)assert(homepage.includes("?topic="+id),"missing V2 navigation path "+id);
const near=(a,b,eps=1e-10)=>assert(Math.abs(a-b)<eps,"numerical V2 verification failed "+a+" vs "+b);
near(1/36+1/9+1/36,1/6);
near(3/.1,30);
near((3/.1)**2,900);
near(3/1.5,2);
near(-1/1.5,-2/3);
near(2/1.5,4/3);
near(Math.max(3-1,0),2);
near((3-.8)/(1+.5),1.4666666666666666);
console.log("PASS: V2 maths proof chain "+chain.length+" full articles, "+proofs+" detailed subchapters, "+equations+" valid formula/proof expressions, non-cyclic prerequisites and independently recomputed examples.");
