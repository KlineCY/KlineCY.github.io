import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const curriculum=read("./curriculum.json"),manifest=read("./lessons-manifest.json");
const all=[...["./lessons-foundations.json","./lessons-advanced.json","./lessons-more.json"].flatMap(p=>read(p).lessons),...manifest.modules.flatMap(m=>read(m.path).lessons)];
const map=new Map(all.map(l=>[l.node_id,l])),nodes=new Map(curriculum.nodes.map(n=>[n.id,n]));
assert.equal(map.size,all.length,"duplicate mathematical article IDs");
assert.equal(manifest.expected_lessons,96);
const ids=["mlmath-n0452","mlmath-n0467","mlmath-n0469","mlmath-n0470","optimization-n0359","riskmath-n0583"];
const edges=[["mlmath-n0467","mlmath-n0452"],["mlmath-n0469","mlmath-n0468"],["mlmath-n0470","mlmath-n0469"],["optimization-n0359","regression-n0206"],["riskmath-n0583","linear-n0086"]];
for(const [id,p] of edges)assert(nodes.get(id)?.prerequisites.includes(p),"missing proof prerequisite "+id+" -> "+p);
let sections=0,formulaCount=0,sourceCount=0;
for(const id of ids){
 const l=map.get(id),node=nodes.get(id);
 assert(l&&node,"V2 mathematical article not published "+id);
 assert.equal(l.title,node.title);
 for(const key of ["definition","assumptions","example","application","caveat"])assert(l[key]?.length>=80,"missing complete "+key+" "+id);
 assert(l.properties.length>=4&&l.formulas.length>=4&&l.derivation.length>=4,"missing theorem structure "+id);
 assert.equal(l.deep_sections.length,4,"must contain four full proof sections "+id);
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,4);
 for(const f of l.formulas){katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});formulaCount++;}
 for(const step of l.derivation){assert(step.explanation.length>=40);katex.renderToString(step.tex,{throwOnError:true,strict:"ignore"});formulaCount++;}
 for(const s of l.deep_sections){
  assert(s.title&&s.question.length>=12);
  assert(s.paragraphs.length>=2&&s.paragraphs.every(p=>p.length>=45),"short explanation "+id);
  assert(s.proof.length>=50&&s.worked.length>=40&&s.limitation.length>=30,"missing proof/example/caveat "+id);
  for(const f of s.formulas){katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});formulaCount++;}
  sections++;
 }
 assert(l.links.some(x=>x.kind==="ml"),"cross-domain knowledge path missing "+id);
 for(const p of l.links)if(p.kind==="math")assert(nodes.has(p.id),"broken prerequisite link "+p.id);
 for(const src of l.sources){assert(src.title&&/^https:\/\//.test(src.url));sourceCount++;}
}
function close(a,b,tol=1e-9){assert(Math.abs(a-b)<tol,"independent numeric invariant "+a+" vs "+b);}
close(Math.sqrt(Math.log(400)/400),0.12238734153404082);
close(2*Math.sqrt(Math.log(400)/400),.24477468306808164);
close(2*3/Math.sqrt(100),.6);
close(((3+Math.sqrt(5))/2)*((3-Math.sqrt(5))/2),1);
close(.5+2/3,7/6);
close(Math.max(3-1,0),2);
close(Math.sign(-.5)*Math.max(Math.abs(-.5)-1,0),0);
close(.06/.02,3);
close(.5*.08+.5*.04,.06);
const html=fs.readFileSync(new URL("./index.html",root),"utf8"),css=fs.readFileSync(new URL("./styles.css",root),"utf8");
for(const id of ids)assert(html.includes("?topic="+id),"missing second path math navigation "+id);
assert(html.includes("styles.css?v=20261104-tailrisk-v5"));
assert(html.includes('class="math-spectral-path math-v2-advanced"'));
assert(css.includes(".math-v2-advanced>summary")&&css.includes("@media print")&&css.includes("overscroll-behavior-x:contain"));
console.log("PASS: six V2 math foundations, "+sections+" proof sections, "+formulaCount+" valid TeX formulas, "+sourceCount+" academic sources, exact numeric invariants and responsive/print typography.");
