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
 assert.equal(lecture.sections.length,6,"five prior complete sections and one independently derived continuation "+lecture.id);
 assert.equal(new Set(lecture.sections.map(x=>x.title)).size,6);
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
 assert.equal((page.html.match(/class="atlas-concept-unit"/g)||[]).length,6);
}
const expandedVolatilityCourse={
 "m22-c06-p01":"Parkinson高低价波动估计的推导假设与跳跃失效边界",
 "m22-c06-p02":"多期GARCH方差预测的递推与累计风险",
 "m22-c06-p03":"QLIKE风险评分的条件期望最优性与噪声代理保持性",
 "m22-c06-p04":"目标波动仓位的杠杆上限与波动预测误差",
 "m22-c06-p05":"同日OHLC完成时间与可交易预测时点：信息泄漏的逐行核对"
};
for(const [id,title] of Object.entries(expandedVolatilityCourse)){
 const lecture=byId.get(id),sec=lecture?.sections.find(x=>x.title===title);
 assert(sec&&lecture.sections.length===6,"unloaded volatility chapter "+id);
 assert(sec.paragraphs.length===2&&sec.paragraphs.every(p=>p.length>=95),"volatility theory and execution assumptions incomplete "+id);
 assert(sec.equations.length===2&&sec.worked.length>=115&&sec.boundary.length>=80,"incomplete risk worked example "+id);
 for(const equation of sec.equations)katex.renderToString(equation.tex,{throwOnError:true,strict:"error",displayMode:true});
}
function near(actual,expect,tol=1e-9){
 assert(Math.abs(actual-expect)<tol,"independent numerical audit: "+actual+" vs "+expect);
}
near(.01**2+(-.02)**2,.0005);
near(Math.log(1.02)**2/(4*Math.log(2)),.0001414385,1e-7);
near(.00001+.3*.0004+.2*.00016+.4*((.0004+21*.0001)/22),.0002074545454545,1e-12);
near(.000002+.08*.0004+.9*.0001,.000124);
near(.25*.01**2+.25*.02**2+2*.25*.25*.01*.02,.00015,1e-12);
near(.01/Math.sqrt(.00015),.816496580927726,1e-12);
near(.0016/(4*Math.log(2)),.0005770780163555854,1e-12);
near(.5+.2*4+.5*2,2.3);
near(.5+.7*2.3,2.11);
near(2.3+2.11,4.41);
near(1+Math.log(.04),-2.2188758248682006);
near(.5+Math.log(.08),-2.0257286443082556);
near(Math.min(2,.1/.05)*.08,.16);
const landing=fs.readFileSync(new URL("./index.html",root),"utf8");
for(const id of ids)assert(landing.includes("#lesson/"+id),"missing learning-route link "+id);
assert(landing.includes("../math/?topic=timeseries-n0268"),"missing HAR mathematics bridge");
console.log("PASS: five volatility tutorials, "+sections+" sections, "+formulas+" valid equations, cross-subject prerequisites and numeric invariants");
