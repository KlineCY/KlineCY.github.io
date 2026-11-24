import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const read=p=>fs.readFileSync(p,"utf8");
const dir="quant/research/advanced-examples/";
const groups=["econometrics","learning","pricing","allocation","numerics","microstructure","inference","bayesian"];
const cli=read("quant/cpp/evidence_cli.hpp");
const hub=read("quant/research/advanced-research.html");
const esc=s=>s.replaceAll("&","&amp;").replaceAll("<","&lt;").replaceAll(">","&gt;");
const unesc=s=>s.replaceAll("&lt;","<").replaceAll("&gt;",">").replaceAll("&amp;","&");
let formulas=0,rows=0,coords=0;
const all=[];
for(const g of groups){
 const code=read("quant/cpp/advanced_research_"+g+".hpp");
 const cat=JSON.parse(read(dir+"catalog-"+g+".json"));
 assert.equal(cat.length,6);
 for(const c of cat){
    all.push(c.command);
    const id=c.command.slice(9),file=c.command+".html";
    const h=read("quant/"+c.area+"/"+file);
    assert(h.includes(esc(c.title)));
    assert(h.includes('href="'+c.url+'"'));
    assert(h.includes('href="../cpp/advanced_research_'+g+'.hpp" download'));
    assert(h.includes('href="../research/advanced-examples/'+c.file+'" download'));
    assert(h.includes(esc(read(dir+c.file))));
    assert.equal(read(dir+c.file).trim().split("\n")[0],c.columns.join(","));
    assert(h.includes("./build/kline_quant "+c.command+" "+dir+c.file+(c.params.length?" "+c.params.join(" "):"")));
    const fragment=h.match(/<h2>C\+\+17完整实现<\/h2><pre>([\s\S]*?)<\/pre>/);
    assert(fragment&&code.includes(unesc(fragment[1])),c.command+" code mismatch");
    assert(cli.includes('if(command=="'+c.command+'")fn=advanced_research::'+id.replaceAll("-","_")+";"));
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
assert.equal(formulas,220);assert.equal(rows,124);assert.equal(coords,751);
assert(read("quant/sidebar-v2.js").includes("advanced-research.html"));
assert(read("quant/cpp/index.html").includes("<strong>995</strong>"));
const studies=read("quant/research/advanced-studies.html");
const scripts=["estimation","finance","numerics","inference"].map(g=>read("quant/research/advanced_studies_"+g+".py")).join("\n");
const ids=[...studies.matchAll(/<article id="([a-z_]+)"/g)].map(m=>m[1]);
const reports=[...scripts.matchAll(/^\s+report\("([a-z_]+)"/gm)].map(m=>m[1]);
assert.equal(ids.length,48);assert.equal(new Set(ids).size,48);
assert.deepEqual(ids.slice().sort(),reports.slice().sort());
for(const id of ids)assert(studies.includes("advanced-results/studies/"+id+".csv"));
console.log("PASS: 48 advanced model chapters, 220 strict formulas, 124 reference rows, 751 coordinates and 48 independent studies");
