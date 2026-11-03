import fs from "node:fs";import assert from "node:assert/strict";import vm from "node:vm";import katex from "katex";import {gunzipSync} from "node:zlib";
const r=new URL("./",import.meta.url),read=p=>JSON.parse(fs.readFileSync(new URL(p,r),"utf8"));
const bundle=read("atlas-concept-bridge-01.json"),mathIds=new Set(read("../math/curriculum.json").nodes.map(x=>x.id));
const archive=JSON.parse(gunzipSync(Buffer.concat([1,2,3,4].map(i=>fs.readFileSync(new URL("atlas/data.part"+i,r))))).toString("utf8"));
const prev=read("atlas-concept-chapters.json");assert.equal(Object.keys(bundle.lessons).length,3);
let sections=0;for(const [id,l] of Object.entries(bundle.lessons)){
 assert(!(id in prev.lessons));assert.equal(l.source_title,archive.domains[id.slice(0,3)][id].title);
 assert.equal(l.sections.length,4);assert(l.links.length>=3);
 for(const ref of l.links)assert(ref.kind==="math"?mathIds.has(ref.id):Boolean(archive.domains[ref.id.slice(0,3)]?.[ref.id]));
 for(const s of l.sections){assert(s.paragraphs.length===2&&s.worked.length>50&&s.boundary.length>20);for(const f of s.equations)katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});sections++}
}
const source=fs.readFileSync(new URL("concept-render.js",r),"utf8"),reader=fs.readFileSync(new URL("atlas.js",r),"utf8");
new vm.Script(source);new vm.Script(reader);
assert(reader.includes('fetchAtlasJson("./atlas-concept-bridge-01.json"'));
const ctx={window:{},encodeURIComponent};vm.runInNewContext(source,ctx);
const done=ctx.window.quantMlConcepts.mount(archive,bundle);assert.equal(done.size,3);
for(const id of done)assert.equal((archive.domains[id.slice(0,3)][id].html.match(/class="atlas-concept-unit"/g)||[]).length,4);
assert(Math.abs(Math.sqrt(Math.log(800)/1000)-0.08175947)<.00001);
assert.equal(2/3+2/3,4/3);
assert(Math.abs(1-.95**20-.6415140775914581)<1e-12);
assert.equal(.05/4,.0125);
console.log("PASS: 3 integrated mathematical learning chains, 12 subchapters, valid formulas and prerequisites");
