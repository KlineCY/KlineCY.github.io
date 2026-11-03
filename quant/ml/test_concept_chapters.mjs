import assert from "node:assert/strict";
import fs from "node:fs";
import vm from "node:vm";
import katex from "katex";
import {gunzipSync} from "node:zlib";

const root=new URL("./",import.meta.url);
const read=path=>JSON.parse(fs.readFileSync(new URL(path,root),"utf8"));
const doc=read("./atlas-concept-chapters.json");
const math=read("../math/curriculum.json");
const mathIds=new Set(math.nodes.map(n=>n.id));
const bytes=Buffer.concat([1,2,3,4].map(i=>fs.readFileSync(new URL("./atlas/data.part"+i,root))));
const archive=JSON.parse(gunzipSync(bytes).toString("utf8"));
assert.equal(doc.schema,1);
assert.equal(Object.keys(doc.lessons).length,4);

let totalSections=0,totalEquations=0;
for(const [id,unit] of Object.entries(doc.lessons)){
 assert.equal(archive.domains[id.slice(0,3)]?.[id]?.title,unit.source_title,"source title mismatch "+id);
 assert(unit.sections.length===4&&unit.goal&&unit.links.length>=2,"incomplete conceptual roadmap "+id);
 for(const link of unit.links){
  if(link.kind==="math")assert(mathIds.has(link.id),"unresolved math reference "+link.id);
  else assert(archive.domains[link.id.slice(0,3)]?.[link.id],"unresolved ML reference "+link.id);
 }
 for(const chapter of unit.sections){
  assert(chapter.title&&chapter.question&&chapter.paragraphs.length>=2&&chapter.worked&&chapter.boundary,"incomplete section "+id);
  totalSections++;
  for(const eq of chapter.equations){
   katex.renderToString(eq.tex,{throwOnError:true,strict:"ignore"});
   totalEquations++;
  }
 }
 for(const ref of unit.sources){
  assert(/^https:\/\//.test(ref.url),"external sources must use HTTPS");
 }
}
assert.equal(totalSections,16);
const code=fs.readFileSync(new URL("./concept-render.js",root),"utf8");
new vm.Script(code,{filename:"quant/ml/concept-render.js"});
const context={window:{},encodeURIComponent};
vm.runInNewContext(code,context);
const ids=context.window.quantMlConcepts.mount(archive,doc);
assert.equal(ids.size,4,"all four learning chains must be mounted");
for(const id of ids){
 const html=archive.domains[id.slice(0,3)][id].html;
 assert(html.includes("atlas-concept-toc")&&html.includes("atlas-concept-related"),"missing navigation "+id);
 assert.equal((html.match(/class="atlas-concept-unit"/g)||[]).length,4,"missing structured subchapters "+id);
}
const cases=[
 [0.5*(4+4)+0.5*((1-2)**2+(3-2)**2),5],
 [1-.95**20,.6415140775914581],
 [0+.9*2-1,.8],
 [.5+.2*2+.3*2+.1*(27/22),1.622727272727273]
];
for(const [actual,expect] of cases)assert(Math.abs(actual-expect)<1e-10,"numerical case check failed");
console.log("PASS: "+ids.size+" advanced lesson chains, "+totalSections+" sections, "+totalEquations+" KaTeX formulas, cross-subject links and case arithmetic");
