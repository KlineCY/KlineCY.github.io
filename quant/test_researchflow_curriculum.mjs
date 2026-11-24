import fs from "node:fs";
import path from "node:path";
import assert from "node:assert/strict";
import katex from "katex";
const read=f=>fs.readFileSync(path.join(process.cwd(),f),"utf8");
const groups=[
 ["portfolio","research","portfolio-workflows"],
 ["ml","ml","predictive-workflows"],
 ["timeseries","math","timeseries-workflows"],
 ["risk","audit","risk-workflows"],
 ["execution","feeds","execution-workflows"]
];
const catalog=JSON.parse(read("quant/research/examples/catalog.json"));
const root=read("quant/index.html"), cpp=read("quant/cpp/index.html"), sidebar=read("quant/sidebar-v2.js");
const hub=read("quant/research/reproducible-workflows.html");
const cli=read("quant/cpp/researchflow_cli.hpp");
const esc=s=>s.replaceAll("&","&amp;").replaceAll("<","&lt;").replaceAll(">","&gt;");
const unesc=s=>s.replaceAll("&lt;","<").replaceAll("&gt;",">").replaceAll("&amp;","&");
let chapters=0,formulas=0,observations=0;
const fixtures=new Set(),commands=new Set();
assert.equal(catalog.length,25);
for(const [group,area,slug] of groups){
 const file=slug+".html",html=read("quant/"+area+"/"+file),header=read("quant/cpp/researchflow_"+group+".hpp");
 assert(html.includes(esc(header)),"Published source must match repository: "+file);
 assert(html.includes('"Times New Roman","SimSun"')&&html.includes("background:white"));
 assert(html.includes('data-page="'+area+'"')&&html.includes("参考文献"));
 assert(!/(学习进度|修订规则|本轮修改|待补充|TODO)/.test(html));
 for(const directory of [root,cpp,sidebar,hub,read("quant/"+area+"/index.html")])assert(directory.includes(file),"Navigation missing: "+file);
 const lessons=[...html.matchAll(/<article class="lesson" id="(lesson\d+)">([\s\S]*?)<\/article>/g)];
 const entries=catalog.filter(e=>e.group===group);
 assert.equal(entries.length,5);
 assert.equal(lessons.length,5);
 for(let i=0;i<5;i++){
  const [_,id,body]=lessons[i],e=entries[i];
  assert.equal(id,"lesson"+(i+1));
  assert(body.includes(e.title)&&body.includes("数学定义与推导")&&body.includes("模型假设与适用边界"));
  const cmd="./build/kline_quant "+e.command+" quant/research/examples/"+e.fixture+(e.params.length?" "+e.params.join(" "):"");
  assert(body.includes(esc(cmd)),"Incorrect reproduction command: "+e.command);
  assert(body.includes('href="../research/examples/'+e.fixture+'" download'),"Dataset link missing");
  assert(hub.includes('href="examples/'+e.fixture+'" download'));
  assert(!commands.has(e.command));commands.add(e.command);
  assert(cli.includes('"'+e.command+'"'),"Executable command missing");
  fixtures.add(e.fixture);chapters++;
  const f=[...body.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)];
  assert.equal(f.length,3);
 }
 for(const m of html.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)){
  katex.renderToString(unesc(m[1]),{throwOnError:true,strict:"error"});formulas++;
 }
}
for(const file of fixtures){
 const rows=read("quant/research/examples/"+file).trim().split(/\r?\n/);
 assert(rows.length>1,file);
 const columns=rows[0].split(",").length;
 for(const row of rows.slice(1)){
  const v=row.split(",");
  assert.equal(v.length,columns,file);
  assert(v.every(x=>x.trim()!==""&&Number.isFinite(Number(x))),file);
  observations++;
 }
}
for(const directory of [root,cpp,sidebar,read("quant/research/index.html")])assert(directory.includes("reproducible-workflows.html"));
assert(hub.includes("run_researchflow.py")&&hub.includes("catalog.json"));
assert.equal(chapters,25);
assert.equal(commands.size,25);
assert.equal(fixtures.size,16);
assert.equal(formulas,86);
assert(cpp.includes("<strong>321</strong>")&&cpp.includes("<strong>995</strong>"));
console.log("PASS: "+chapters+" research chapters, "+formulas+" strict KaTeX equations, "+fixtures.size+" real downloadable synthetic CSV files, "+observations+" input rows and exact workflow recipes");
