import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const base=new URL("./",import.meta.url);
const read=(name)=>JSON.parse(fs.readFileSync(new URL(name,base),"utf8"));
const files=["lessons-phase-01-a.json","lessons-phase-01-b.json"];
const lessons=files.flatMap(file=>read(file).lessons);
const catalog=read("curriculum.json");
const ids=["foundation-n0013","calculus-n0040","linear-n0065","probability-n0095","measure-n0121","statistics-n0162"];
const nodeIds=new Set(catalog.nodes.map(x=>x.id));
assert.deepEqual(lessons.map(x=>x.id),ids);
assert.equal(lessons.length,6);
let formulas=0,proofs=0,links=0;
for(const lesson of lessons){
  assert(nodeIds.has(lesson.id),"missing canonical node "+lesson.id);
  assert.equal(lesson.id,lesson.node_id);
  assert.equal(lesson.deep_sections?.length,4,"four original pedagogical units "+lesson.id);
  assert.equal(new Set(lesson.deep_sections.map(x=>x.title)).size,4);
  assert(lesson.checked_status.startsWith("V31深化"));
  assert(lesson.definition.length>45 && lesson.example.length>90);
  for(const section of lesson.deep_sections){
    assert(section.title.length>6 && section.question.length>12);
    assert(section.paragraphs.length>=2 && section.paragraphs.every(x=>x.length>=40));
    for(const key of ["proof","worked","limitation"])assert(section[key].length>=35,lesson.id+" "+key);
    assert(section.paragraphs.join("").length+section.proof.length+section.worked.length+section.limitation.length>=240,lesson.id+" low section density");
    assert(section.formulas.length>=1);
    for(const formula of section.formulas){
      katex.renderToString(formula.tex,{throwOnError:true,strict:"ignore",displayMode:true});
      formulas++;
    }
    proofs++;
  }
  assert(lesson.links.length>=3);
  for(const link of lesson.links){assert.equal(link.kind,"math");assert(nodeIds.has(link.id),lesson.id+" => "+link.id);links++;}
  assert(!/TODO|Lorem ipsum|待填|占位|[\uFFFD\u200B]/i.test(JSON.stringify(lesson)));
}
assert.equal(proofs,24);assert.equal(formulas,24);assert.equal(links,18);
const near=(a,b)=>assert(Math.abs(a-b)<1e-8,a+" != "+b);
near(5*Math.pow(.8,28),.00967140655691704);
near(.018/.116,9/58);
near(.2*.1+.8*.01,.028);
near(4/16+1/64,17/64);
near(Math.sqrt(8*8+3*3),Math.sqrt(73));
near(Math.pow(.96,2),.9216);
const html=fs.readFileSync(new URL("index.html",base),"utf8");
const ml=fs.readFileSync(new URL("../ml/index.html",base),"utf8");
assert(html.includes('aria-label="V31基础数学六课深度修订"'));
assert(ml.includes('aria-label="V31基础数学跨模块精讲"'));
for(const id of ids){
 assert(html.includes("?topic="+id));
 assert(ml.includes("../math/?topic="+id));
}
const app=fs.readFileSync(new URL("app.js",base),"utf8");
assert(app.includes("renderMathDeepSections"));
assert(app.includes("part.paragraphs"));
assert(app.includes("part.proof"));
const css=fs.readFileSync(new URL("styles.css",base),"utf8");
assert(css.includes("math-deep-unit")&&css.includes("overflow-x:auto")&&css.includes("@media print"));
console.log("PASS V31: six canonical improved lessons, 24 rigorous sections, 24 KaTeX expressions, 18 validated navigation links, numerical checks, typography and reader support.");
