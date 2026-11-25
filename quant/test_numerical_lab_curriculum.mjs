import fs from "node:fs";
import path from "node:path";
import assert from "node:assert/strict";
import katex from "katex";
const read=f=>fs.readFileSync(path.join(process.cwd(),f),"utf8");
const groups=[["options","assets"],["rates","assets"],["optim","research"],["inference","audit"],["learning","ml"],["simulation","math"],["factors","research"],["market","feeds"]];
const catalog=JSON.parse(read("quant/research/numerical-examples/catalog.json"));
const root=read("quant/index.html"),cpp=read("quant/cpp/index.html"),sidebar=read("quant/sidebar-v2.js"),hub=read("quant/research/numerical-laboratories.html");
const cli=read("quant/cpp/numerical_lab_cli.hpp");
const esc=s=>s.replaceAll("&","&amp;").replaceAll("<","&lt;").replaceAll(">","&gt;");
const unesc=s=>s.replaceAll("&lt;","<").replaceAll("&gt;",">").replaceAll("&amp;","&");
let lessons=0,formulas=0,rows=0;
const commands=new Set(),fixtures=new Set();
assert.equal(catalog.length,40);
for(const [group,area] of groups){
 const file="numerical-"+group+".html";
 const html=read("quant/"+area+"/"+file),header=read("quant/cpp/numerical_lab_"+group+".hpp");
 assert(html.includes(esc(header)),"Published source mismatch: "+file);
 assert(html.includes('"Times New Roman","SimSun"')&&html.includes("background:white"));
 assert(html.includes('data-page="'+area+'"')&&html.includes("参考文献"));
 assert(!/(学习进度|修订规则|本轮修改|待补充|TODO)/.test(html));
 for(const directory of [root,cpp,sidebar,hub,read("quant/"+area+"/index.html")])assert(directory.includes(file),"Navigation missing: "+file);
 const entries=catalog.filter(e=>e.group===group);
 const articles=[...html.matchAll(/<article class="lesson" id="(lesson\d+)">([\s\S]*?)<\/article>/g)];
 assert.equal(articles.length,5,file);assert.equal(entries.length,5);
 for(let j=0;j<5;++j){
  const [_,id,body]=articles[j],e=entries[j];
  assert.equal(id,"lesson"+(j+1));
  assert(body.includes(e.title)&&body.includes("数学推导与求解结构")&&body.includes("估计假设、实证解释与适用边界"));
  const cmd="./build/kline_quant "+e.command+" quant/research/numerical-examples/"+e.fixture+(e.params.length?" "+e.params.join(" "):"");
  assert(body.includes(esc(cmd)),"Incorrect reproduction command: "+e.command);
  assert(body.includes('href="../research/numerical-examples/'+e.fixture+'" download'));
  assert(hub.includes('href="numerical-examples/'+e.fixture+'" download'));
  assert(cli.includes('"'+e.command+'"')&&!commands.has(e.command));
  commands.add(e.command);fixtures.add(e.fixture);lessons++;
  const equations=[...body.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)];
  assert.equal(equations.length,3,e.command);
  for(const equation of equations){katex.renderToString(unesc(equation[1]),{throwOnError:true,strict:"error"});formulas++;}
 }
}
for(const file of fixtures){
 const lines=read("quant/research/numerical-examples/"+file).trim().split(/\r?\n/);
 assert(lines.length>1,file);const d=lines[0].split(",").length;
 for(const line of lines.slice(1)){const a=line.split(",");assert.equal(a.length,d,file);assert(a.every(v=>v.trim()!==""&&Number.isFinite(Number(v))),file);rows++;}
}
for(const directory of [root,cpp,sidebar,read("quant/research/index.html")])assert(directory.includes("numerical-laboratories.html"));
assert(hub.includes("run_numerical_labs.py")&&hub.includes("catalog.json"));
assert.equal(lessons,40);assert.equal(formulas,120);assert.equal(commands.size,40);assert.equal(fixtures.size,37);assert.equal(rows,245);
assert(cpp.includes("<strong>321</strong>")&&cpp.includes("<strong>1123</strong>"));
console.log("PASS: "+lessons+" complete numerical chapters, "+formulas+" strict KaTeX equations, "+fixtures.size+" downloadable datasets, "+rows+" input rows and exact C++ source-linked recipes");
