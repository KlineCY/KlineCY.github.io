import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const read=p=>fs.readFileSync(p,"utf8");
const dir="quant/research/methods-examples/";
const catalog=["catalog-foundations.json","catalog-finance.json"].flatMap(p=>JSON.parse(read(dir+p)));
const map={geometry:"math",regularization:"ml",optimization:"math",dynamics:"math",series:"math",sampling:"math",classification:"ml",decision:"ml",derivatives:"assets",rates:"assets",counterparty:"assets",microstructure:"feeds",portfolio:"research",empirical:"audit",macroeconomics:"research",dataengineering:"feeds"};
const esc=s=>s.replaceAll("&","&amp;").replaceAll("<","&lt;").replaceAll(">","&gt;");
const unesc=s=>s.replaceAll("&lt;","<").replaceAll("&gt;",">").replaceAll("&amp;","&");
assert.equal(catalog.length,64);
assert.equal(new Set(catalog.map(c=>c.command)).size,64);
const cli=read("quant/cpp/evidence_cli.hpp"),hub=read("quant/research/methods-laboratories.html");
let eq=0,coords=0,rows=0;
for(const [g,area] of Object.entries(map)){
 const file="methods-"+g+".html",html=read("quant/"+area+"/"+file),code=read("quant/cpp/methods_"+g+".hpp");
 assert(html.includes(esc(code)));
 assert(hub.includes(file));
 for(const p of ["quant/index.html","quant/cpp/index.html","quant/research/index.html","quant/"+area+"/index.html"])assert(read(p).includes(file),p+" missing "+file);
 const items=catalog.filter(c=>c.group===g);
 assert.equal(items.length,4);
 for(const c of items){
  const id=c.command.slice(8),i=html.indexOf('<article id="'+id+'">');
  assert(i>=0);
  const s=html.slice(i,html.indexOf("</article>",i));
  const csv=read(dir+c.file);
  assert(s.includes(esc(csv))&&s.includes(esc(c.title)));
  assert.equal(csv.trim().split("\n")[0],c.columns.join(","));
  assert(s.includes('href="../research/methods-examples/'+c.file+'" download'));
  assert(s.includes("./build/kline_quant "+c.command+" "+dir+c.file+(c.params.length?" "+c.params.join(" "):"")));
  assert(cli.includes('"'+c.command+'"'));
  const formulas=[...s.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)];
  assert.equal(formulas.length,2);
  for(const m of formulas){katex.renderToString(unesc(m[1]),{throwOnError:true,strict:"error"});eq++;}
  const table=s.match(/<tbody>([\s\S]*?)<\/tbody>/);
  const result=[...table[1].matchAll(/<tr>([\s\S]*?)<\/tr>/g)].map(r=>[...r[1].matchAll(/<td>(.*?)<\/td>/g)].map(v=>Number(v[1])));
  assert.equal(result.length,c.expected.length);
  result.forEach((r,i)=>{rows++;assert.equal(r.length,c.expected[i].length);r.forEach((v,j)=>{assert(Number.isFinite(v)&&Math.abs(v-c.expected[i][j])<=1e-9*Math.max(1,Math.abs(v)),c.command);coords++;});});
 }
}
assert.equal(eq,128);assert.equal(rows,122);assert.equal(coords,450);
assert(read("quant/sidebar-v2.js").includes("methods-laboratories.html"));
assert(read("quant/cpp/index.html").includes("<strong>941</strong>"));
const studies=read("quant/research/methods-integrated-studies.html");
assert.equal((studies.match(/<article id="study-/g)||[]).length,8);
for(const m of studies.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g))katex.renderToString(unesc(m[1]),{throwOnError:true,strict:"error"});
console.log("PASS: 64 research lessons, 128 strict KaTeX equations, 122 reference rows, 450 numerical coordinates and eight linked studies");
