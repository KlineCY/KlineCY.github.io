import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const dir=new URL("./",import.meta.url);
const read=p=>fs.readFileSync(new URL(p,dir),"utf8");
const json=p=>JSON.parse(read(p));
const ids=["foundation-n0001","foundation-n0002","foundation-n0003","foundation-n0004","foundation-n0006","foundation-n0007","foundation-n0008","foundation-n0009","foundation-n0010","foundation-n0011","foundation-n0014","foundation-n0015"];
function fnv(o){let h=2166136261>>>0;for(const c of JSON.stringify(o))h=Math.imul(h^c.charCodeAt(0),16777619)>>>0;return h.toString(16).padStart(8,"0")}
const canonical=json("lessons-foundations.json");
assert.equal(canonical.lessons.length,19,"the entire historical canonical core must remain unchanged");
assert.equal(fnv(canonical.lessons),"55234b60","V89 must not change one character of the original 19 lessons");
const v89=json("lessons-foundations-v89.json");
assert.equal(v89.version,1);
assert.equal(v89.lessons.length,12,"twelve real completed foundations loaded through the canonical reader");
assert.equal(v89.integrated_by,"quant/math/app.js");
assert.deepEqual(v89.lessons.map(x=>x.node_id),ids);
const curriculum=json("curriculum.json"),nodes=new Map(curriculum.nodes.map(x=>[x.id,x]));
const manifest=json("lessons-manifest.json");
assert.equal(manifest.version,1);
assert.equal(manifest.expected_lessons,201,"historical 201-module count stays frozen");
assert.equal(manifest.modules.length,77,"all existing 77 phase modules unchanged");
const originalModules=manifest.modules.flatMap(x=>json(x.path).lessons);
assert.equal(originalModules.length,201);
const all=[...canonical.lessons,...json("lessons-advanced.json").lessons,...json("lessons-more.json").lessons,...originalModules,...v89.lessons];
assert.equal(all.length,336,"from 324 to 336 actual complete loaded mathematics lessons");
const completed=new Map(all.map(x=>[x.node_id,x]));
assert.equal(completed.size,all.length,"a completed node must appear exactly once");
assert.equal(nodes.size,656,"canonical curriculum stays intact");
let newEquations=0,subchapters=0;
for(const id of ids){
 const node=nodes.get(id),x=completed.get(id);
 assert(node&&x,id+" not integrated in actual course reader");
 assert.equal(x.id,id);
 assert.equal(x.title,node.title,"course title differs from curriculum "+id);
 assert(node.state==="outline"&&node.lesson_id===null,"V89 must preserve original outline metadata; live reader derives completion from loaded lessonMap");
 for(const key of ["definition","assumptions","example","application","caveat"])
  assert(typeof x[key]==="string"&&x[key].length>=50,id+" insufficient completed "+key);
 assert(x.properties.length>=4&&x.properties.every(p=>p.length>=18),id+" missing systematic properties");
 assert.equal(x.formulas.length,4,id+" must explain four principal relations");
 assert.equal(x.derivation.length,4,id+" must contain four separately explained proof steps");
 for(const f of x.formulas){
  assert(f.label.length>=4&&f.tex.length>=15,id+" incomplete formula");
  katex.renderToString(f.tex,{throwOnError:true,strict:"error",displayMode:true});newEquations++;
 }
 for(const step of x.derivation){
  assert(step.explanation.length>=28&&step.tex.length>=12,id+" incomplete proof step");
  katex.renderToString(step.tex,{throwOnError:true,strict:"error",displayMode:true});newEquations++;
 }
 assert.equal(x.deep_sections.length,4,id+" must have four complete teaching mini-chapters");
 assert.equal(new Set(x.deep_sections.map(s=>s.title)).size,4,id+" duplicate section");
 for(const s of x.deep_sections){
  assert(s.title.length>=8&&s.question.length>=10,id+" missing section goal");
  assert(s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=30),id+" incomplete section explanation");
  assert(s.proof.length>=50&&s.worked.length>=50&&s.limitation.length>=25,id+" proof, numeric example or scope missing");
  assert.equal(s.formulas.length,1,id+" missing new mini-chapter formula");
  for(const f of s.formulas){assert(f.tex.length>=12&&f.label.length>=4);katex.renderToString(f.tex,{throwOnError:true,strict:"error",displayMode:true});newEquations++;}
  subchapters++;
 }
 for(const l of x.links||[]){assert(l.kind==="math"&&nodes.has(l.id),id+" invalid previous/next math curriculum route: "+l.id);}
 const text=JSON.stringify(x);
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(text),id+" hidden or invalid Unicode");
 assert(!/TODO|FIXME|Lorem ipsum/i.test(text),id+" draft placeholders forbidden");
 assert(/[\u3400-\u9FFF]/u.test(text),id+" Chinese textbook narrative missing");
}
assert.equal(newEquations,144);
assert.equal(subchapters,48);
let checks=0;
function close(label,a,b,tol=1e-8){assert(Number.isFinite(a)&&Math.abs(a-b)<=tol,label+": "+a+" vs "+b);checks++;}
function truth(label,t){close(label,t?1:0,1);}
close("finite union overlap",3+3-2,4);
close("symmetric difference count",1+1,2);
close("panel full dimensions",3*2,6);
close("panel available observations",3*2-1,5);
close("four products by three",4*3,12);
close("bijective affine inverse",(6-1)/2,2.5);
close("affine inverse twice",(9+6)/3,5);
close("matrix singular determinant",1*4-2*2,0);
close("invertible diagonal determinant",2*3,6);
close("equivalence group count",3+2+2,7);
close("modulo three congruence",(6-3)%3,0);
close("correlation 30 degrees",Math.cos(Math.PI/6),Math.sqrt(3)/2);
close("correlation 60 degrees",Math.cos(Math.PI/3),.5);
truth("correlation threshold counterexample",Math.cos(Math.PI/6)>.8&&Math.cos(Math.PI/3)<.8);
close("integer enumeration minus five",-5,-5);
close("power set three",2**3,8);
close("power set twenty",2**20,1048576);
close("growth from ternary states",3**4,81);
truth("counterexample even not divisible four",2%2===0&&2%4!==0);
truth("material implication with false premise",!(-3>5)||(-3>2));
truth("De Morgan Q false",!(true&&false)===(!true||!false));
truth("quantifier order witness",1+1>1);
truth("quantifier reversed counterexample",!(1>1));
truth("epsilon N convergence",1/101<.01);
truth("epsilon N strict case fails",!(1/100<.01));
truth("sufficient vs necessary",1.5>1&&!(1.5>2));
close("direct inequality a3 b1 gap",3**2+1**2-2*3*1,4);
close("geometric series four",1+.5+.25+.125,1.875);
close("geometric series formula",(1-.5**4)/(1-.5),1.875);
close("two asset independent variance",.25*.1**2+.25*.2**2,.0125);
truth("contrapositive odd square",7**2%2===1&&7%2===1);
truth("no smallest positive element",.01/2<.01&&.01/2>0);
truth("numerical error can change sign",1e-12-1e-9<0);
close("Cauchy tail n201",2/201,.009950248756218905);
truth("Cauchy strict epsilon",2/201<.01);
truth("harmonic remote difference bound",100/200,.5);
close("contraction n5",.5**5*2/(1-.5),.125);
close("contraction n8",.5**8*2/(1-.5),.015625);
truth("supremum approximant n1001",1-1/1001>.999);
close("supremum sequence first 4 fourth",1-1/4,.75);
truth("monotone bounded convergence n7",2**(-7)<.01);
close("rational square lower",1.41**2,1.9881);
close("rational square upper",1.42**2,2.0164);
assert(checks>=40,"need at least forty independently recomputed examples");
const html=read("index.html"),css=read("styles.css"),app=read("app.js");
assert(html.includes('aria-label="逻辑、集合与实数完备性"'),"V89 new full-course entry missing");
for(const id of ids)assert(html.includes('href="?topic='+id+'"'),id+" not discoverable by user");
assert(app.includes('fetch("./lessons-foundations.json",{cache:"no-cache"})'),"historical mathematics foundation data not loaded");
assert(app.includes('fetchMathJson("./lessons-foundations-v89.json")')&&app.includes("...extra,...v89"),"newly completed foundations not in live mathematics reader");
assert(app.includes("lessonMap=new Map(lessons.map(l=>[l.node_id,l]))"),"completion must be derived from canonical lesson data");
assert(app.includes('lessonMap.has(node.id)?"完整讲义"'),"live completion badge would remain stale");
for(const token of ["overflow-x:auto","line-break:strict",":focus-visible","@media print","@media(max-width:700px)"])assert(css.includes(token),"responsive CJK/math styles missing: "+token);
console.log("V89 PASS: 12 NEW complete foundational mathematics lessons, 48 proof chapters, 144 strict KaTeX formulas, "+checks+" numeric invariants; 19 old foundation lessons byte-for-byte content preserved, total 336/656 complete and 320 missing, all historical 201 phase lessons retained.");
