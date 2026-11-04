import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
import katex from "katex";
import {gunzipSync} from "node:zlib";

const root=new URL("./",import.meta.url);
const read=path=>JSON.parse(fs.readFileSync(new URL(path,root),"utf8"));
const archive=JSON.parse(gunzipSync(Buffer.concat([1,2,3,4].map(i=>fs.readFileSync(new URL("atlas/data.part"+i,root))))).toString("utf8"));
const originalIds=new Set(archive.catalog.items.map(x=>x.id));
const mathIds=new Set(read("../math/curriculum.json").nodes.map(x=>x.id));
const older=read("./atlas-new-lessons.json");
const doc=read("./atlas-volatility-chapter.json");
const olderIds=new Set(older.chapters.flatMap(c=>c.lessons.map(x=>x.id)));
assert.equal(doc.schema,1);
assert.equal(doc.chapters.length,1);
assert.equal(doc.chapters[0].domain_id,"m22");
const all=doc.chapters[0].lessons;
assert.equal(all.length,5);
const ids=new Set(all.map(x=>x.id));
assert.equal(ids.size,5);
const byId=new Map(all.map(x=>[x.id,x]));
let sections=0,formulas=0;
for(const lecture of all){
 assert(/^m22-c06-p0[1-5]$/.test(lecture.id));
 assert(!originalIds.has(lecture.id)&&!olderIds.has(lecture.id));
 assert(lecture.goal.length>=40&&lecture.protocol.length>=4,"insufficient practical protocol "+lecture.id);
 assert.equal(lecture.sections.length,5,"five logically connected sections "+lecture.id);
 assert.equal(new Set(lecture.sections.map(x=>x.title)).size,5);
 for(const section of lecture.sections){
  assert(section.question.length>=10&&section.paragraphs.length>=2);
  assert(section.paragraphs.every(p=>p.length>=45),"thin explanation "+lecture.id);
  assert(section.worked.length>=45&&section.boundary.length>=20);
  for(const formula of section.equations){
   assert(formula.label&&formula.tex);
   katex.renderToString(formula.tex,{throwOnError:true,strict:"ignore"});
   formulas++;
  }
  sections++;
 }
 for(const link of lecture.prerequisites||[]){
  if(link.kind==="math")assert(mathIds.has(link.id),"missing math prerequisite "+link.id);
  else assert(originalIds.has(link.id)||olderIds.has(link.id)||ids.has(link.id),"missing ML prerequisite "+link.id);
  assert(link.label);
 }
 for(const src of lecture.sources)assert(/^https:\/\//.test(src.url)&&src.title);
}
const visited=new Set(),stack=new Set();
function check(id){
 if(stack.has(id))throw Error("cyclic prerequisite "+id);
 if(visited.has(id))return;
 stack.add(id);
 for(const link of byId.get(id).prerequisites)if(link.kind==="ml"&&byId.has(link.id))check(link.id);
 stack.delete(id);visited.add(id);
}
for(const id of ids)check(id);

const reader=fs.readFileSync(new URL("./atlas.js",root),"utf8");
assert(reader.includes('fetchAtlasJson("./atlas-volatility-chapter.json"'),"missing ML reader integration");
const script=fs.readFileSync(new URL("./new-topics.js",root),"utf8");
new vm.Script(reader);new vm.Script(script);
const context={window:{},encodeURIComponent};
vm.runInNewContext(script,context);
const originalAdditions=context.window.quantMlExpansion.install(archive,older);
assert.equal(originalAdditions.size,olderIds.size);
const injected=context.window.quantMlExpansion.install(archive,doc);
assert.equal(injected.size,5);
assert.equal(archive.catalog.items.length,720+olderIds.size+5);
for(const id of injected){
 const page=archive.domains.m22[id];
 assert(page&&page.html.includes("atlas-concept-toc"));
 assert.equal((page.html.match(/class="atlas-concept-unit"/g)||[]).length,5);
}
function near(actual,expect,tol=1e-9){
 assert(Math.abs(actual-expect)<tol,"independent numerical audit: "+actual+" vs "+expect);
}
near(.01**2+(-.02)**2,.0005);
near(Math.log(1.02)**2/(4*Math.log(2)),.0001414385,1e-7);
near(.00001+.3*.0004+.2*.00016+.4*((.0004+21*.0001)/22),.0002074545454545,1e-12);
near(.000002+.08*.0004+.9*.0001,.000124);
near(.25*.01**2+.25*.02**2+2*.25*.25*.25*.01*.02,.00015,1e-12);
near(.01/Math.sqrt(.00015),.816496580927726,1e-12);
const landing=fs.readFileSync(new URL("./index.html",root),"utf8");
for(const id of ids)assert(landing.includes("#lesson/"+id),"missing learning-route link "+id);
assert(landing.includes("../math/?topic=timeseries-n0268"),"missing HAR mathematics bridge");
console.log("PASS: five volatility tutorials, "+sections+" sections, "+formulas+" valid equations, cross-subject prerequisites and numeric invariants");
