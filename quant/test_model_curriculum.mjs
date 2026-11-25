import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const read=p=>fs.readFileSync(p,"utf8");
const dir="quant/research/model-examples/";
const groups=["optimization","likelihood","dynamics","pricing","portfolio","systems"];
const cli=read("quant/cpp/evidence_cli.hpp");
const hub=read("quant/research/model-laboratories.html");
const esc=s=>s.replaceAll("&","&amp;").replaceAll("<","&lt;").replaceAll(">","&gt;");
const unesc=s=>s.replaceAll("&lt;","<").replaceAll("&gt;",">").replaceAll("&amp;","&");
let formulas=0,rows=0,coords=0;
const all=[];
for(const g of groups){
 const code=read("quant/cpp/model_research_"+g+".hpp");
 const cat=JSON.parse(read(dir+"catalog-"+g+".json"));
 assert.equal(cat.length,6);
 for(const c of cat){
    all.push(c.command);
    const id=c.command.slice(6),file=c.command+".html";
    const h=read("quant/"+c.area+"/"+file);
    assert(h.includes(esc(c.title)));
    assert(h.includes('href="'+c.url+'"'));
    assert(h.includes('href="../cpp/model_research_'+g+'.hpp" download'));
    assert(h.includes('href="../research/model-examples/'+c.file+'" download'));
    assert(h.includes(esc(read(dir+c.file))));
    assert.equal(read(dir+c.file).trim().split("\n")[0],c.columns.join(","));
    assert(h.includes("./build/kline_quant "+c.command+" "+dir+c.file+" "+c.params.join(" ")));
    const fragment=h.match(/<h2>C\+\+17实现<\/h2><pre>([\s\S]*?)<\/pre>/);
    assert(fragment&&code.includes(unesc(fragment[1])),c.command+" code mismatch");
    assert(cli.includes('if(command=="'+c.command+'")fn=model_research::'+id.replaceAll("-","_")+";"));
    assert(hub.includes(file));
    for(const p of ["quant/index.html","quant/cpp/index.html","quant/research/index.html","quant/"+c.area+"/index.html"])assert(read(p).includes(file),p+" missing "+file);
    const eq=[...h.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)];
    assert.equal(eq.length,5,c.command);
    for(const m of eq){katex.renderToString(unesc(m[1]),{throwOnError:true,strict:"error"});formulas++;}
    const heads=[...h.matchAll(/<th>(.*?)<\/th>/g)].map(x=>x[1]);
    assert.deepEqual(heads,c.header.slice(0,c.expected_prefix[0].length));
    const table=h.match(/<tbody>([\s\S]*?)<\/tbody>/);
    const a=[...table[1].matchAll(/<tr>([\s\S]*?)<\/tr>/g)].map(r=>[...r[1].matchAll(/<td>(.*?)<\/td>/g)].map(v=>Number(v[1])));
    assert.equal(a.length,c.expected_prefix.length);
    a.forEach((r,i)=>{rows++;assert.equal(r.length,c.expected_prefix[i].length);r.forEach((v,j)=>{coords++;assert(Number.isFinite(v)&&Math.abs(v-c.expected_prefix[i][j])<=1e-9*Math.max(1,Math.abs(v)),c.command);});});
 }
}
assert.equal(all.length,36);assert.equal(new Set(all).size,36);
assert.equal(formulas,180);assert.equal(rows,104);assert.equal(coords,563);
assert(read("quant/sidebar-v2.js").includes("model-laboratories.html"));
assert(read("quant/cpp/index.html").includes("<strong>1123</strong>"));
const studies=read("quant/research/model-integrated-studies.html"),script=read("quant/research/run_model_studies.py");
assert.equal((studies.match(/<article id="/g)||[]).length,22);
const eq=[...studies.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)];
assert.equal(eq.length,22);
for(const m of eq)katex.renderToString(unesc(m[1]),{throwOnError:true,strict:"error"});
for(const m of studies.matchAll(/model-results\/studies\/([a-z_]+)\.csv/g))assert(script.includes('report("'+m[1]+'"'),m[1]);
assert.equal([...script.matchAll(/^\s+report\("/gm)].length,22);
console.log("PASS: 36 independent model chapters, 202 strict formulas, 104 reference rows, 563 coordinates, embedded code, navigation and 22 studies");
