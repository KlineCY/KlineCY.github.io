import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
import katex from "katex";
import {gunzipSync} from "node:zlib";

const root=new URL("./",import.meta.url);
const read=path=>JSON.parse(fs.readFileSync(new URL(path,root),"utf8"));
const catalog=JSON.parse(gunzipSync(Buffer.concat([1,2,3,4].map(i=>fs.readFileSync(new URL("atlas/data.part"+i,root))))).toString("utf8"));
const baseIds=new Set(catalog.catalog.items.map(x=>x.id));
const existing=read("./atlas-new-lessons.json"),vol=read("./atlas-volatility-chapter.json"),bundle=read("./atlas-linear-regularization-chapter.json");
const previousIds=new Set([...existing.chapters,...vol.chapters].flatMap(ch=>ch.lessons.map(x=>x.id)));
const mathIds=new Set(read("../math/curriculum.json").nodes.map(x=>x.id));
assert.equal(bundle.schema,1);
assert.equal(bundle.chapters.length,1);
const chapter=bundle.chapters[0];
assert.equal(chapter.domain_id,"m07");
assert.equal(chapter.lessons.length,5);
const ids=new Set(chapter.lessons.map(l=>l.id));
assert.equal(ids.size,5,"stable learning-chain IDs must be unique");
const byId=new Map(chapter.lessons.map(l=>[l.id,l]));
let sections=0,formulas=0,sourceCount=0;
for(let i=0;i<chapter.lessons.length;i++){
 const lesson=chapter.lessons[i];
 assert.equal(lesson.id,"m07-c06-p0"+(i+1),"course must follow proof dependency order");
 assert(!baseIds.has(lesson.id)&&!previousIds.has(lesson.id),"new course must not overwrite existing course");
 assert(lesson.goal.length>=45&&lesson.protocol.length>=4&&lesson.sources.length>=2,"missing long-form tutorial "+lesson.id);
 assert.equal(lesson.sections.length,5,"full tutorial must have five connected subchapters");
 assert.equal(new Set(lesson.sections.map(s=>s.title)).size,5,"repeated subchapter heading "+lesson.id);
 for(const section of lesson.sections){
  assert(section.title.length>=5&&section.question.length>=10,"subchapter must have explicit research question");
  assert(section.paragraphs.length>=2&&section.paragraphs.every(x=>x.length>=48),"thin reasoning chain "+lesson.id);
  assert(section.worked.length>=60&&section.boundary.length>=30,"worked calculation or model boundary missing "+lesson.id);
  assert(section.equations.length>0,"quantitative proof must include exact expression");
  sections++;
  for(const equation of section.equations){
   assert(equation.label&&equation.tex);
   katex.renderToString(equation.tex,{throwOnError:true,strict:"ignore"});
   formulas++;
  }
 }
 for(const p of lesson.prerequisites){
  assert(p.label&&["math","ml"].includes(p.kind));
  if(p.kind==="math")assert(mathIds.has(p.id),"broken mathematical prerequisite "+p.id);
  else assert(baseIds.has(p.id)||previousIds.has(p.id)||ids.has(p.id),"broken ML prerequisite "+p.id);
  if(p.kind==="ml"&&ids.has(p.id))assert(p.id<lesson.id,"learning prerequisites must point backward "+lesson.id);
 }
 for(const src of lesson.sources){assert(/^https:\/\//.test(src.url)&&src.title,"broken source metadata");sourceCount++;}
}
const visiting=new Set(),visited=new Set();
function visit(id){
 if(visiting.has(id))throw Error("cyclic chain "+id);
 if(visited.has(id))return;
 visiting.add(id);
 for(const link of byId.get(id).prerequisites)if(link.kind==="ml"&&ids.has(link.id))visit(link.id);
 visiting.delete(id);visited.add(id);
}
for(const id of ids)visit(id);

const scripts=["atlas.js","new-topics.js"].map(path=>fs.readFileSync(new URL(path,root),"utf8"));
for(const source of scripts)new vm.Script(source);
assert(scripts[0].includes('fetchAtlasJson("./atlas-linear-regularization-chapter.json"'),"Atlas reader does not load new chapter");
const ctx={window:{},encodeURIComponent};
vm.runInNewContext(scripts[1],ctx);
const original=ctx.window.quantMlExpansion.install(catalog,existing);
const volatility=ctx.window.quantMlExpansion.install(catalog,vol);
const regularization=ctx.window.quantMlExpansion.install(catalog,bundle);
assert.equal(original.size,20);
assert.equal(volatility.size,5);
assert.equal(regularization.size,5);
assert.equal(catalog.catalog.items.length,750);
assert.equal(new Set(catalog.catalog.items.map(x=>x.id)).size,750);
for(const id of regularization){
 const article=catalog.domains.m07[id];
 assert(article?.html.includes("atlas-concept-toc"),"navigation missing "+id);
 assert.equal((article.html.match(/class="atlas-concept-unit"/g)||[]).length,5,"five full subchapters must render "+id);
 assert(catalog.catalog.items.find(x=>x.id===id).chapter===chapter.chapter);
}
const mathHome=fs.readFileSync(new URL("../math/index.html",root),"utf8");
const mlHome=fs.readFileSync(new URL("./index.html",root),"utf8");
for(const id of ids)assert(mlHome.includes("#lesson/"+id),"ML study path missing "+id);
for(const id of ["linear-n0075","linear-n0078","linear-n0083","linear-n0086","regression-n0205","regression-n0206"]){
 assert(mlHome.includes("../math/?topic="+id),"cross-module study path missing "+id);
 assert(mathHome.includes("?topic="+id),"mathematical lesson navigation missing "+id);
}
assert(mathHome.includes("../ml/#lesson/m07-c06-p05"),"reverse path to ETF research missing");
function near(a,b,delta=1e-11){assert(Math.abs(a-b)<=delta,"independent arithmetic check failed "+a+" vs "+b);}
near(1/36+1/9+1/36,1/6);
near(3/0.1,30);
near((3/0.1)**2,900);
near(3/(1+.5),2);
near(-1/(1+.5),-2/3);
near(2/(1+.5),4/3);
near(Math.max(3-1,0),2);
near(Math.max(1-1,0),0);
near((3-.8)/(1+.5),2.2/1.5);
near((-1+.8)/(1+.5),-.2/1.5);
near(.6*.02+(-.2)*(-.01)-.001,.013);
console.log("PASS: V2 linear-model chapter; "+regularization.size+" lessons, "+sections+" substantial subchapters, "+formulas+" valid KaTeX formulas, "+sourceCount+" source records, linked math graph and independent arithmetic.");
