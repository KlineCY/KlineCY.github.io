import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const dir = new URL("./", import.meta.url);
const read = file => JSON.parse(fs.readFileSync(new URL(file, dir), "utf8"));
const curriculum = read("curriculum.json");
const foundation = read("lessons-foundations.json");
const more = read("lessons-more.json");
const existing = new Set(curriculum.nodes.map(node => node.id));
const byId = new Map([...foundation.lessons, ...more.lessons].map(lesson => [lesson.node_id, lesson]));
const ids = ["probability-n0096", "probability-n0107", "probability-n0108"];
const wantedTitles = ["Bayes定理", "条件期望", "全方差公式"];
let sectionCount = 0, formulaCount = 0, referenceCount = 0;

for (const [index, id] of ids.entries()) {
  const lesson = byId.get(id);
  assert(lesson && existing.has(id), "missing canonical mathematics lesson " + id);
  assert.equal(lesson.title, wantedTitles[index]);
  assert.equal(lesson.deep_sections?.length, 4, id + " must contain four genuine deep sections");
  assert.equal(new Set(lesson.deep_sections.map(part => part.title)).size, 4);
  assert(lesson.definition.length >= 65 && lesson.assumptions.length >= 50);
  assert(lesson.example.length >= 80 && lesson.caveat.length >= 60);
  assert(lesson.sources?.length >= 2);
  assert(lesson.links?.length >= 3);
  for (const ref of lesson.links) {
    assert(ref.label && ["ml", "math"].includes(ref.kind));
    if (ref.kind === "math") assert(existing.has(ref.id), "broken mathematical prerequisite " + ref.id);
    if (ref.kind === "ml") assert(/^m\d\d-c\d\d-p\d\d$/.test(ref.id), "invalid ML ID " + ref.id);
  }
  for (const item of lesson.sources) {
    assert(item.title && /^https:\/\/[^ ]+/.test(item.url));
    referenceCount++;
  }
  for (const part of lesson.deep_sections) {
    assert(part.title.length >= 8 && part.question.length >= 12, id + " has an unclear subchapter");
    assert(part.paragraphs?.length === 2 && part.paragraphs.every(p => p.length >= 55), id + " has shallow theory");
    assert(part.proof?.length >= 65 && part.worked?.length >= 55 && part.limitation?.length >= 45, id + " has incomplete argument");
    assert(part.formulas?.length >= 1, id + " missing core formula");
    for (const expression of part.formulas) {
      assert(expression.label && expression.tex);
      katex.renderToString(expression.tex, {throwOnError:true, strict:"ignore"});
      formulaCount++;
    }
    sectionCount++;
  }
}
assert.equal(sectionCount,12);
assert(formulaCount >= 12);
const html = fs.readFileSync(new URL("index.html",dir),"utf8");
for (const id of ids) assert(html.includes("?topic="+id), "new cross-subject pathway not connected: "+id);
assert(html.includes("Bayes、条件期望与全方差的先修证明链"));
const close=(a,b,t=1e-11)=>assert(Math.abs(a-b)<t, String(a)+" != "+String(b));
close(.009/(.009+.0495),2/13);
const x=[.04,-.03,.01,-.01], y=[.04,-.01,-.01,-.01];
close(x.reduce((a,b)=>a+b)/4,.0025);
close(x.reduce((a,b,i)=>a+(b-y[i])**2,0)/4,.0002);
close(y.reduce((a,b)=>a+b*b,0)/4,.000475);
close(x.reduce((a,b)=>a+b*b,0)/4,.000675);
close(.2*.0016+.8*.0001,.0004);
close(.2*(.03-.01)**2+.8*(.005-.01)**2,.0001);
close(.2*.0016+.8*.0001+.2*(.03-.01)**2+.8*(.005-.01)**2,.0005);
console.log("PASS: 3 canonical probability lessons, "+sectionCount+" deep sections, "+formulaCount+" valid KaTeX equations, "+referenceCount+" sources, linked math graph and independent numeric invariants.");
