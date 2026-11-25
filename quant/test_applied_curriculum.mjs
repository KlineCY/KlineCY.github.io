import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const read=p=>fs.readFileSync(p,"utf8");
const cat=JSON.parse(read("quant/research/applied-examples/catalog.json"));
const cli=read("quant/cpp/evidence_cli.hpp");
const hub=read("quant/research/applied-laboratories.html");
const map={linear:"math",panel:"audit",learning:"ml",pricing:"assets",allocation:"research",credit:"assets",execution:"feeds",stochastic:"math"};
const escape=s=>s.replaceAll("&","&amp;").replaceAll("<","&lt;").replaceAll(">","&gt;");
const unescape=s=>s.replaceAll("&lt;","<").replaceAll("&gt;",">").replaceAll("&amp;","&");
assert.equal(cat.length,32);
assert.equal(new Set(cat.map(c=>c.command)).size,32);
let equations=0,coordinates=0;
for(const [g,area] of Object.entries(map)){
  const file="applied-"+g+".html";
  const html=read("quant/"+area+"/"+file);
  assert(html.includes(escape(read("quant/cpp/applied_"+g+".hpp"))));
  for(const p of ["quant/index.html","quant/cpp/index.html","quant/research/index.html","quant/"+area+"/index.html"])assert(read(p).includes(file),p+" missing "+file);
  assert(hub.includes(file));
  const cases=cat.filter(c=>c.group===g);
  assert.equal(cases.length,4);
  for(const c of cases){
    const id=c.command.slice("applied-".length);
    const start=html.indexOf('<article id="'+id+'">');
    assert(start>=0);
    const section=html.slice(start,html.indexOf("</article>",start));
    assert(section.includes(escape(c.title)));
    const csv=read("quant/research/applied-examples/"+c.file);
    assert(section.includes(escape(csv)));
    assert.equal(csv.trim().split("\n")[0],c.columns.join(","));
    assert(section.includes('href="../research/applied-examples/'+c.file+'" download'));
    assert(cli.includes('"'+c.command+'"'));
    const cmd="./build/kline_quant "+c.command+" quant/research/applied-examples/"+c.file+(c.params.length?" "+c.params.join(" "):"");
    assert(section.includes(cmd));
    const fs=[...section.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)];
    assert.equal(fs.length,2);
    for(const f of fs){katex.renderToString(unescape(f[1]),{throwOnError:true,strict:"error"});equations++;}
    const table=section.match(/<tbody>([\s\S]*?)<\/tbody>/);
    const rows=[...table[1].matchAll(/<tr>([\s\S]*?)<\/tr>/g)].map(r=>[...r[1].matchAll(/<td>(.*?)<\/td>/g)].map(x=>Number(x[1])));
    assert.equal(rows.length,c.expected.length);
    rows.forEach((r,i)=>{assert.equal(r.length,c.expected[i].length);r.forEach((v,j)=>{assert(Number.isFinite(v)&&Math.abs(v-c.expected[i][j])<=1e-9*Math.max(1,Math.abs(v)));coordinates++;});});
  }
}
assert.equal(equations,64);
assert.equal(coordinates,252);
assert(read("quant/sidebar-v2.js").includes("applied-laboratories.html"));
assert(read("quant/cpp/index.html").includes("<strong>1123</strong>"));
console.log("PASS: 32 research lessons, "+equations+" strict KaTeX equations, "+coordinates+" numeric coordinates, source and navigation consistency");
