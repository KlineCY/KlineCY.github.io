import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const read=p=>fs.readFileSync(p,"utf8");
const dir="quant/research/system-examples/";
const groups=["timeseries","portfolio","derivatives","execution","learning","inference","economics","engineering"];
const cli=read("quant/cpp/evidence_cli.hpp");
const hub=read("quant/research/system-research.html");
const esc=s=>s.replaceAll("&","&amp;").replaceAll("<","&lt;").replaceAll(">","&gt;");
const unesc=s=>s.replaceAll("&lt;","<").replaceAll("&gt;",">").replaceAll("&amp;","&");
let formulas=0,rows=0,coords=0;
const all=[];
for(const g of groups){
 const code=read("quant/cpp/system_research_"+g+".hpp");
 const cat=JSON.parse(read(dir+"catalog-"+g+".json"));
 assert.equal(cat.length,6);
 for(const c of cat){
    all.push(c.command);
    const id=c.command.slice(7),file=c.command+".html";
    const h=read("quant/"+c.area+"/"+file);
    assert(h.includes(esc(c.title)));
    assert(h.includes('href="'+c.url+'"'));
    assert(h.includes('href="../cpp/system_research_'+g+'.hpp" download'));
    assert(h.includes('href="../research/system-examples/'+c.file+'" download'));
    assert(h.includes(esc(read(dir+c.file))));
    assert.equal(read(dir+c.file).trim().split("\n")[0],c.columns.join(","));
    assert(h.includes("./build/kline_quant "+c.command+" "+dir+c.file+(c.params.length?" "+c.params.join(" "):"")));
    const fragment=h.match(/<h2>C\+\+17完整实现<\/h2><pre>([\s\S]*?)<\/pre>/);
    assert(fragment&&code.includes(unesc(fragment[1])),c.command+" code mismatch");
    assert(cli.includes('if(command=="'+c.command+'")fn=system_research::'+id.replaceAll("-","_")+";"));
    assert(hub.includes(file));
    for(const p of ["quant/index.html","quant/cpp/index.html","quant/research/index.html","quant/"+c.area+"/index.html"])assert(read(p).includes(file),p+" missing "+file);
    const eq=[...h.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)];
    assert(eq.length>=4,c.command);
    for(const m of eq){katex.renderToString(unesc(m[1]),{throwOnError:true,strict:"error"});formulas++;}
    const heads=[...h.matchAll(/<th>(.*?)<\/th>/g)].map(x=>x[1]);
    assert.deepEqual(heads,c.header.slice(0,c.expected_prefix[0].length));
    const table=h.match(/<tbody>([\s\S]*?)<\/tbody>/);
    const a=[...table[1].matchAll(/<tr>([\s\S]*?)<\/tr>/g)].map(r=>[...r[1].matchAll(/<td>(.*?)<\/td>/g)].map(v=>Number(v[1])));
    assert.equal(a.length,c.expected_prefix.length);
    a.forEach((r,i)=>{rows++;assert.equal(r.length,c.expected_prefix[i].length);r.forEach((v,j)=>{coords++;assert(Number.isFinite(v)&&Math.abs(v-c.expected_prefix[i][j])<=1e-9*Math.max(1,Math.abs(v)),c.command);});});
 }
}
assert.equal(all.length,48);assert.equal(new Set(all).size,48);
assert.equal(formulas,194);assert.equal(rows,130);assert.equal(coords,900);
assert(read("quant/sidebar-v2.js").includes("system-research.html"));
assert(read("quant/cpp/index.html").includes("<strong>1043</strong>"));
console.log("PASS: 48 complete system-model topics, 194 rendered formulas, 130 reference rows and 900 reference coordinates");

const studies=read("quant/research/system-studies.html");
const scripts=["timeseries","finance","learning","economics","linked"].map(g=>read("quant/research/system_studies_"+g+".py")).join("\n");
const ids=[...studies.matchAll(/<article id="([a-z0-9_]+)"/g)].map(m=>m[1]);
const reports=[...scripts.matchAll(/^\s+report\("([a-z0-9_]+)"/gm)].map(m=>m[1]);
assert.equal(ids.length,60);assert.equal(new Set(ids).size,60);
assert.deepEqual(ids.slice().sort(),reports.slice().sort());
for(const id of ids)assert(studies.includes("system-results/studies/"+id+".csv"));
const methodology=read("quant/research/system-methodology.html");
assert.equal([...methodology.matchAll(/<section id="([a-z]+)"/g)].length,8);
const relations=[...methodology.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)];
assert.equal(relations.length,32);
for(const m of relations)katex.renderToString(unesc(m[1]),{throwOnError:true,strict:"error"});
for(const c of all)assert(methodology.includes(c+".html"));
assert(hub.includes("system-studies.html")&&hub.includes("system-methodology.html"));
console.log("PASS: 48 independent studies, 12 cross-model pipelines and 32 mathematical synthesis sections");
