import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const read=p=>fs.readFileSync(p,"utf8");
const dir="quant/research/depth-examples/";
const catalog=["foundations","learning","finance","research"].flatMap(p=>JSON.parse(read(dir+"catalog-"+p+".json")));
const map={numerics:"math",proximal:"math",distribution:"math",sampling:"math",temporal:"math",inference:"audit",kernel:"ml",loss:"ml",differentiation:"ml",validation:"audit",uncertainty:"ml",features:"ml",optiongreeks:"assets",termstructure:"assets",dynamicpricing:"assets",creditmodels:"assets",robustallocation:"research",volatility:"assets",executiontheory:"feeds",engineering:"feeds",identification:"audit",economics:"research",games:"research",factorconstruction:"research"};
const esc=s=>s.replaceAll("&","&amp;").replaceAll("<","&lt;").replaceAll(">","&gt;");
const unesc=s=>s.replaceAll("&lt;","<").replaceAll("&gt;",">").replaceAll("&amp;","&");
assert.equal(catalog.length,96);
assert.equal(new Set(catalog.map(c=>c.command)).size,96);
assert.equal(new Set(catalog.map(c=>c.file)).size,96);
assert.equal(Object.keys(map).length,24);
const cli=read("quant/cpp/evidence_cli.hpp"),hub=read("quant/research/depth-laboratories.html");
let eq=0,coords=0,rows=0,sections=0;
for(const [g,area] of Object.entries(map)){
 const file="depth-"+g+".html",html=read("quant/"+area+"/"+file),code=read("quant/cpp/depth_"+g+".hpp");
 assert(html.includes(esc(code)),g+" embedded code differs");
 assert(hub.includes(file),g+" absent from hub");
 assert(cli.includes('#include "depth_'+g+'.hpp"'));
 for(const p of ["quant/index.html","quant/cpp/index.html","quant/research/index.html","quant/"+area+"/index.html"])assert(read(p).includes(file),p+" missing "+file);
 const items=catalog.filter(c=>c.group===g);
 assert.equal(items.length,4);
 for(const c of items){
  const id=c.command.slice(6),i=html.indexOf('<article id="'+id+'">');
  assert(i>=0,c.command);
  assert(hub.includes(file+"#"+id),c.command+" absent from hub");
  const s=html.slice(i,html.indexOf("</article>",i));
  const csv=read(dir+c.file);
  assert(s.includes(esc(csv))&&s.includes(esc(c.title)));
  assert.equal(csv.trim().split("\n")[0],c.columns.join(","));
  assert(s.includes('href="../research/depth-examples/'+c.file+'" download'));
  assert(s.includes("./build/kline_quant "+c.command+" "+dir+c.file+(c.params.length?" "+c.params.join(" "):"")));
  assert(cli.includes('"'+c.command+'"'));
  assert(code.includes("inline Report "+id.replaceAll("-","_")+"("));
  for(const title of ["数学定义与推导","数值演算与金融应用","算法复杂度、输入契约与边界","独立样本与输出基准"]){
    assert(s.includes("<h3>"+title+"</h3>"),c.command+" missing "+title);
    sections++;
  }
  assert(s.includes("参考文献："));
  const formulas=[...s.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)];
  assert.equal(formulas.length,3,c.command);
  for(const m of formulas){katex.renderToString(unesc(m[1]),{throwOnError:true,strict:"error"});eq++;}
  const headers=[...s.matchAll(/<th>(.*?)<\/th>/g)].map(x=>x[1]);
  assert.deepEqual(headers,c.header,c.command+" column names");
  const table=s.match(/<tbody>([\s\S]*?)<\/tbody>/);
  const result=[...table[1].matchAll(/<tr>([\s\S]*?)<\/tr>/g)].map(r=>[...r[1].matchAll(/<td>(.*?)<\/td>/g)].map(v=>Number(v[1])));
  assert.equal(result.length,c.expected.length);
  result.forEach((r,i)=>{rows++;assert.equal(r.length,c.expected[i].length);r.forEach((v,j)=>{assert(Number.isFinite(v)&&Math.abs(v-c.expected[i][j])<=1e-9*Math.max(1,Math.abs(v)),c.command);coords++;});});
 }
}
assert.equal(eq,288);assert.equal(rows,201);assert.equal(coords,911);assert.equal(sections,384);
assert(read("quant/sidebar-v2.js").includes("depth-laboratories.html"));
assert(read("quant/cpp/index.html").includes("<strong>947</strong>"));
const studies=read("quant/research/depth-integrated-studies.html"),script=read("quant/research/run_depth_labs.py");
assert.equal((studies.match(/<article id="study-/g)||[]).length,16);
assert.equal([...studies.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)].length,16);
for(const m of studies.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g))katex.renderToString(unesc(m[1]),{throwOnError:true,strict:"error"});
for(const m of studies.matchAll(/depth-results\/studies\/([a-z_]+)\.csv/g))assert(script.includes('report("'+m[1]+'"'),m[1]);
assert.equal([...script.matchAll(/^\s+report\("/gm)].length,16);
console.log("PASS: 24 topics, 96 research courses, 304 strict KaTeX formulas, 201 reference rows, 911 numerical coordinates, 384 content sections and 16 linked studies");
