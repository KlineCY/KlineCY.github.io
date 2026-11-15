import fs from "node:fs";import assert from "node:assert/strict";import vm from "node:vm";import katex from "katex";import {gunzipSync} from "node:zlib";
const r=new URL("./",import.meta.url),read=p=>JSON.parse(fs.readFileSync(new URL(p,r),"utf8"));
const bundle=read("atlas-concept-bridge-01.json"),mathIds=new Set(read("../math/curriculum.json").nodes.map(x=>x.id));
const archive=JSON.parse(gunzipSync(Buffer.concat([1,2,3,4].map(i=>fs.readFileSync(new URL("atlas/data.part"+i,r))))).toString("utf8"));
const prev=read("atlas-concept-chapters.json");assert.equal(Object.keys(bundle.lessons).length,3);
let sections=0;for(const [id,l] of Object.entries(bundle.lessons)){
 assert(!(id in prev.lessons));assert.equal(l.source_title,archive.domains[id.slice(0,3)][id].title);
 assert(l.sections.length>=4);assert(l.links.length>=3);
 for(const ref of l.links)assert(ref.kind==="math"?mathIds.has(ref.id):Boolean(archive.domains[ref.id.slice(0,3)]?.[ref.id]));
 for(const s of l.sections){assert(s.paragraphs.length===2&&s.worked.length>50&&s.boundary.length>20);for(const f of s.equations)katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});sections++}
}
const source=fs.readFileSync(new URL("concept-render.js",r),"utf8"),reader=fs.readFileSync(new URL("atlas.js",r),"utf8");
new vm.Script(source);new vm.Script(reader);
assert(reader.includes('fetchAtlasJson("./atlas-concept-bridge-01.json"'));
const ctx={window:{},encodeURIComponent};vm.runInNewContext(source,ctx);
const done=ctx.window.quantMlConcepts.mount(archive,bundle);assert.equal(done.size,3);
for(const id of done)assert.equal((archive.domains[id.slice(0,3)][id].html.match(/class="atlas-concept-unit"/g)||[]).length,bundle.lessons[id].sections.length);
assert(Math.abs(Math.sqrt(Math.log(800)/1000)-0.08175947)<.00001);
assert.equal(2/3+2/3,4/3);
assert(Math.abs(1-.95**20-.6415140775914581)<1e-12);
assert.equal(.05/4,.0125);

const additions={
 "m04-c01-p02":["确定性加权Hoeffding界与有效样本量","有限期连续查看验证结果的同时保证"],
 "m07-c02-p01":["Ridge线性平滑器与留一法残差恒等式","响应扰动下的Ridge稳定性界"],
 "m12-c03-p04":["任意依赖下的Benjamini–Yekutieli修正","共同重抽样的最大统计量与家族错误控制"]
};
for(const [id,required] of Object.entries(additions)){
 const units=bundle.lessons[id].sections;
 assert(units.length>=7,id+" new complete sections missing");
 const titles=units.map(s=>s.title);
 assert.equal(new Set(titles).size,titles.length,id+" duplicate sections");
 for(const title of required)assert(titles.includes(title),id+" lacks "+title);
 for(const sec of units.slice(-2)){
  assert.equal(sec.paragraphs.length,2,id+" two analytic paragraphs required");
  assert.equal(sec.equations.length,2,id+" two formulas required");
  assert(sec.paragraphs.every(p=>p.length>=90),id+" incomplete explanations");
  assert(sec.worked.length>=100&&sec.boundary.length>=70,id+" incomplete example/boundary");
  for(const eq of sec.equations)katex.renderToString(eq.tex,{throwOnError:true,strict:"error",displayMode:true});
 }
}
const close=(actual,expected,tol=1e-5)=>assert(Math.abs(actual-expected)<tol,String(actual)+" differs from "+String(expected));
close(Math.sqrt(0.0125*Math.log(40)/2),0.15184);
close(Math.sqrt(Math.log(4000)/200),0.20364);
close((1/25+1)/2,0.52);
close(3/(9+1),0.3);
close(1/(1+1),0.5);
close(0.1/(4*(25/12)),0.012);
close((1+1)/(5+1),1/3);
console.log("PASS: 3 integrated mathematical learning chains, "+sections+" subchapters, valid formulas and prerequisites");
