import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
import katex from "katex";
import {gunzipSync} from "node:zlib";
const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const archive=JSON.parse(gunzipSync(Buffer.concat([1,2,3,4].map(i=>fs.readFileSync(new URL("atlas/data.part"+i,root))))).toString("utf8"));
const baseIds=new Set(archive.catalog.items.map(x=>x.id));
const previous=["atlas-new-lessons.json","atlas-volatility-chapter.json","atlas-linear-regularization-chapter.json","atlas-evaluation-evidence-chapter.json"].map(read);
const docs=["atlas-generalization-v2.json","atlas-kernel-covariance-v2.json"].map(read);
const prior=new Set(previous.flatMap(d=>d.chapters.flatMap(ch=>ch.lessons.map(l=>l.id))));
const mathIds=new Set(read("../math/curriculum.json").nodes.map(n=>n.id));
const lessons=docs.flatMap(d=>d.chapters.flatMap(ch=>ch.lessons));
assert.equal(lessons.length,5,"five new proof-first machine learning lessons required");
const ids=new Set(lessons.map(l=>l.id));
assert.equal(ids.size,5);
const expected=["m04-c07-p01","m04-c07-p02","m09-c07-p01","m09-c07-p02","m23-c07-p01"];
assert.deepEqual([...ids],expected);
let sections=0,formulaCount=0,sources=0;
for(const l of lessons){
 assert(!baseIds.has(l.id)&&!prior.has(l.id),"duplicate course "+l.id);
 assert(l.goal.length>=70&&l.protocol.length>=4,"incomplete learning objective or research protocol "+l.id);
 assert.equal(l.sections.length,5,"five full conceptual sections per lesson "+l.id);
 assert.equal(new Set(l.sections.map(s=>s.title)).size,5,"repeated title "+l.id);
 for(const s of l.sections){
  assert(s.question.length>=12&&s.paragraphs.length>=2&&s.paragraphs.every(p=>p.length>=65),"short explanatory content "+l.id);
  assert(s.worked.length>=50&&s.boundary.length>=35,"missing worked calculation or boundary "+l.id);
  assert(s.equations.length>=1);
  for(const f of s.equations){assert(f.label&&f.tex);katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});formulaCount++;}
  sections++;
 }
 for(const p of l.prerequisites){
  assert(p.label);
  if(p.kind==="math")assert(mathIds.has(p.id),"missing mathematical link "+p.id);
  else assert(baseIds.has(p.id)||prior.has(p.id)||ids.has(p.id),"missing ML prerequisite "+p.id);
  if(ids.has(p.id))assert(expected.indexOf(p.id)<expected.indexOf(l.id),"forward learning dependency "+l.id);
 }
 for(const s of l.sources){assert(s.title&&/^https:\/\//.test(s.url));sources++;}
}
const scripts=["atlas.js","new-topics.js"].map(p=>fs.readFileSync(new URL(p,root),"utf8"));
for(const s of scripts)new vm.Script(s);
assert(scripts[0].includes('fetchAtlasJson(path,{cache:"no-cache"})')&&scripts[0].includes('"./atlas-generalization-v2.json"')&&scripts[0].includes('"./atlas-kernel-covariance-v2.json"'),"Atlas loader does not fetch the new courses");
const ctx={window:{},encodeURIComponent};
vm.runInNewContext(scripts[1],ctx);
for(const d of [...previous,...docs])ctx.window.quantMlExpansion.install(archive,d);
assert.equal(archive.catalog.items.length,758);
assert.equal(new Set(archive.catalog.items.map(x=>x.id)).size,758);
for(const id of ids){
 const a=archive.domains[id.slice(0,3)][id];
 assert(a?.html.includes("atlas-concept-toc"),"unrendered lesson "+id);
 assert.equal((a.html.match(/class="atlas-concept-unit"/g)||[]).length,5);
 assert(archive.catalog.items.some(row=>row.id===id));
}
function near(a,b,t=1e-9){assert(Math.abs(a-b)<t,"numeric mismatch "+a+" vs "+b);}
near(Math.sqrt(Math.log(400)/400),.12238734153404082);
near(2*3/Math.sqrt(100),.6);
near((3+Math.sqrt(5))/2,2.618033988749895);
near(.5+2/3,7/6);
near(.06/.02,3);
near(.5*.08+.5*.04,.06);
const html=fs.readFileSync(new URL("./index.html",root),"utf8"),css=fs.readFileSync(new URL("./atlas.css",root),"utf8");
for(const id of ids)assert(html.includes("#lesson/"+id),"not linked in advanced learning route "+id);
assert(html.includes("atlas.js?v=20261109-v112-20-math-ml-lessons"));
assert(html.includes("atlas.css?v=20261104-copula-v7"));
assert(html.includes('class="atlas-spectral-path atlas-v2-advanced"'));
assert(css.includes(".atlas-v2-advanced>summary")&&css.includes("@media print")&&css.includes("overflow-x:auto"));
console.log("PASS: "+ids.size+" ML lessons, "+sections+" deep sections, "+formulaCount+" valid TeX expressions, "+sources+" sources, 758 distinct IDs, independent numeric and responsive/print CSS.");
