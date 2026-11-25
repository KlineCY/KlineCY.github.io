import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const read=p=>fs.readFileSync(p,"utf8");
const root="quant/research/";
const cat=JSON.parse(read(root+"integrated-catalog.json"));
const native=JSON.parse(read(root+"integrated-examples/catalog.json"));
const code=read("quant/cpp/integrated_projects.hpp"),cli=read("quant/cpp/integrated_cli.cpp");
const hub=read(root+"integrated-research.html"),nativeHub=read(root+"integrated-native-projects.html");
const unesc=s=>s.replaceAll("&lt;","<").replaceAll("&gt;",">").replaceAll("&amp;","&");
let count=0;
function equations(h){
    for(const m of h.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)){
        katex.renderToString(unesc(m[1]),{throwOnError:true,strict:"error"});count++;
    }
}
assert.equal(cat.length,60);assert.equal(new Set(cat.map(c=>c.id)).size,60);
for(const c of cat){
    const path="integrated-"+c.id.replaceAll("_","-")+".html",h=read(root+path),s=read(root+c.script);
    assert(hub.includes(path));assert(h.includes("integrated-results/"+c.id+".csv"));
    const block=h.match(/<h2>可执行实验与证据<\/h2><pre>([\s\S]*?)<\/pre>/);
    assert(block&&s.includes(unesc(block[1])),c.id);
    assert(s.includes('report("'+c.id+'"'));
    for(const model of c.models)assert(h.includes("comprehensive-"+model+".html"),model);
    equations(h);
}
assert.equal(new Set(cat.flatMap(c=>c.models)).size,80);
assert.equal(native.length,4);
for(const c of native){
    const path="integrated-native-"+c.id+".html",h=read(root+path);
    assert(nativeHub.includes(path)&&cli.includes('name=="'+c.id+'"'));
    const block=h.match(/<h2>完整C\+\+17实现<\/h2><pre>([\s\S]*?)<\/pre>/);
    assert(block&&code.includes(unesc(block[1])),c.id);
    assert.equal(read(root+"integrated-examples/"+c.file).split("\n")[0],c.columns.join(","));
    assert(h.includes("integrated-examples/"+c.file));
    equations(h);
}
assert(count>=76);assert(read("quant/sidebar-v2.js").includes("integrated-research.html"));
assert(read("quant/index.html").includes("integrated-research.html"));
console.log("PASS: 60 cross-model chapters, four native project chapters, "+count+" rendered equations, source and navigation fidelity");
