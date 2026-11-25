import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const read=p=>fs.readFileSync(p,"utf8");
const dir="quant/research/comprehensive-examples/";
const groups=["numerics","learning","dynamics","pricing","portfolio","microstructure","inference","bayesian","economics","data"];
const cli=read("quant/cpp/evidence_cli.hpp");
const hub=read("quant/research/comprehensive-research.html");
const esc=s=>s.replaceAll("&","&amp;").replaceAll("<","&lt;").replaceAll(">","&gt;");
const unesc=s=>s.replaceAll("&lt;","<").replaceAll("&gt;",">").replaceAll("&amp;","&");
let formulas=0,rows=0,coords=0;
const all=[];
for(const g of groups){
 const code=read("quant/cpp/comprehensive_"+g+".hpp");
 const cat=JSON.parse(read(dir+"catalog-"+g+".json"));
 assert.equal(cat.length,8);
 for(const c of cat){
    all.push(c.command);
    const id=c.command.slice(14),file=c.command+".html";
    const h=read("quant/"+c.area+"/"+file);
    assert(h.includes(esc(c.title)));
    assert(h.includes('href="'+c.url+'"'));
    assert(h.includes('href="../cpp/comprehensive_'+g+'.hpp" download'));
    assert(h.includes('href="../research/comprehensive-examples/'+c.file+'" download'));
    assert(h.includes(esc(read(dir+c.file))));
    assert.equal(read(dir+c.file).trim().split("\n")[0],c.columns.join(","));
    assert(h.includes("./build/kline_quant "+c.command+" "+dir+c.file+(c.params.length?" "+c.params.join(" "):"")));
    const fragment=h.match(/<h2>C\+\+17完整实现<\/h2><pre>([\s\S]*?)<\/pre>/);
    assert(fragment&&code.includes(unesc(fragment[1])),c.command+" code mismatch");
    assert(cli.includes('if(command=="'+c.command+'")fn=comprehensive::'+id.replaceAll("-","_")+";"));
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
assert.equal(all.length,80);assert.equal(new Set(all).size,80);
assert(formulas>=320&&rows>=80&&coords>=400);
assert(read("quant/sidebar-v2.js").includes("comprehensive-research.html"));
assert(read("quant/cpp/index.html").includes("<strong>1123</strong>"));
console.log(`PASS: ${all.length} complete research topics, ${formulas} rendered formulas, ${rows} reference rows and ${coords} coordinates`);


const studies=read("quant/research/comprehensive-studies.html");
const scripts=[...groups,"linked"].map(g=>read("quant/research/comprehensive_studies_"+g+".py")).join("\n");
const ids=[...studies.matchAll(/<article id="([a-z0-9_]+)"/g)].map(m=>m[1]);
const reports=[...scripts.matchAll(/^\s+report\("([a-z0-9_]+)"/gm)].map(m=>m[1]);
assert.equal(ids.length,92);assert.equal(new Set(ids).size,92);assert.deepEqual(ids.slice().sort(),reports.slice().sort());
for(const id of ids)assert(studies.includes("comprehensive-results/studies/"+id+".csv"));
let synthesis=0;for(const g of groups){const h=read("quant/research/comprehensive-course-"+g+".html");for(const m of h.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)){katex.renderToString(unesc(m[1]),{throwOnError:true,strict:"error"});synthesis++;}}
assert.equal(synthesis,40);console.log("PASS: 80 independent study descriptions, 12 linked pipelines and 40 mathematical synthesis formulas");
