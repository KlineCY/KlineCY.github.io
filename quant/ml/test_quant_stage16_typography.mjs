import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const all=["a","b","c","d"].flatMap(x=>Object.entries(read("atlas-depth-16-"+x+".json").lessons));
const math=read("../math/lessons-phase-15-a.json").lessons;
assert.equal(all.length,23,"stage16 chapter count must stay stable");
assert.equal(new Set(all.map(([id])=>id)).size,23,"duplicate stage16 lesson");
assert.deepEqual(math.map(x=>x.id),["riskmath-x005","riskmath-x006"]);

let checked=0;
function formula(tex,source){
  assert(typeof tex==="string"&&tex.trim().length>3,"empty math in "+source);
  assert(!tex.includes("\ufffd"),"UTF-8 replacement symbol in "+source);
  assert(!/[\u0000-\u0008\u000B\u000C\u000E-\u001F]/.test(tex),"control character in "+source);
  const html=katex.renderToString(tex,{throwOnError:true,strict:"ignore",displayMode:true});
  assert(html.includes("katex"),"rendered KaTeX missing in "+source);
  assert(!html.includes("katex-error"),"unrenderable TeX: "+source);
  checked++;
}
for(const [id,row] of all){
 assert(row.equations.length>=6,"missing expansion in "+id);
 assert(row.derivation.length>=6&&row.protocol.length>=5&&row.pitfalls.length>=4,"incomplete study notes "+id);
 const labels=new Set();
 for(const item of row.equations){
  assert(!labels.has(item.label),"duplicate equation label "+id+": "+item.label);
  labels.add(item.label);
  formula(item.tex,id+" "+item.label);
 }
 assert(!JSON.stringify(row).includes("\ufffd"),"corrupted Chinese text "+id);
}
for(const row of math){
 assert(row.formulas.length>=7&&row.deep_sections.length>=7,"risk proof path incomplete "+row.id);
 for(const e of row.formulas)formula(e.tex,row.id+" "+e.label);
 for(const s of row.deep_sections){
  assert(s.paragraphs.length>=2&&s.proof.length>35&&s.worked.length>35,"insufficient risk proof "+row.id+" "+s.title);
  for(const e of s.formulas)formula(e.tex,row.id+" "+s.title+" "+e.label);
 }
 for(const e of row.derivation)formula(e.tex,row.id+" derivation");
 assert(!JSON.stringify(row).includes("\ufffd"),"corrupted Chinese math chapter "+row.id);
}
const cssML=fs.readFileSync(new URL("atlas.css",root),"utf8");
const cssMath=fs.readFileSync(new URL("../math/styles.css",root),"utf8");
for(const [name,css] of [["ML",cssML],["Math",cssMath]]){
 assert(css.includes("overflow-x:auto"),"horizontal formula containment missing "+name);
 assert(css.includes("@media print"),"print-specific formula rules missing "+name);
 assert(css.includes("line-break:strict"),"CJK line-breaking rules missing "+name);
}
console.log("Stage16 and systemic-risk KaTeX/CJK checks passed:",all.length+math.length,"lessons;",checked,"mathematical expressions");
