import fs from "node:fs";
import path from "node:path";
import assert from "node:assert/strict";
import katex from "katex";
const read=f=>fs.readFileSync(path.join(process.cwd(),f),"utf8");
const esc=s=>s.replaceAll("&","&amp;").replaceAll("<","&lt;").replaceAll(">","&gt;");
const unesc=s=>s.replaceAll("&lt;","<").replaceAll("&gt;",">").replaceAll("&amp;","&");
const cat=JSON.parse(read("quant/research/evidence-examples/catalog.json"));
const root=read("quant/index.html"),cpp=read("quant/cpp/index.html"),nav=read("quant/sidebar-v2.js");
const hub=read("quant/research/evidence-laboratories.html"),cli=read("quant/cpp/evidence_cli.hpp");
const groups=[...new Set(cat.map(e=>e.group))];
let chapters=0,formulas=0,rows=0,coordinates=0;
const names=new Set();
for(const group of groups){
 const items=cat.filter(e=>e.group===group),area=items[0].area;
 const file="evidence-"+group+".html",html=read("quant/"+area+"/"+file);
 const header=read("quant/cpp/evidence_"+group+".hpp");
 assert(html.includes(esc(header)),"Source fidelity: "+group);
 assert(html.includes('"Times New Roman","SimSun"')&&html.includes("background:white"));
 for(const source of [root,cpp,nav,hub,read("quant/"+area+"/index.html")])assert(source.includes(file),"Missing navigation: "+file);
 assert(!/(学习进度|修改规则|待补充|TODO|本轮修改)/.test(html));
 const sections=[...html.matchAll(/<article class="lesson" id="(lesson\d+)">([\s\S]*?)<\/article>/g)];
 assert.equal(sections.length,4);
 for(let j=0;j<items.length;j++){
  const e=items[j],section=sections[j][2];
  assert.equal(sections[j][1],"lesson"+(j+1));
  assert(section.includes(esc(e.title)));
  const csv=read("quant/research/evidence-examples/"+e.file);
  assert(section.includes(esc(csv)),"Published input differs: "+e.file);
  const data=csv.trim().split(/\r?\n/);
  assert.deepEqual(data[0].split(","),e.columns);
  for(const r of data.slice(1)){
   const a=r.split(",");assert.equal(a.length,e.columns.length);
   assert(a.every(v=>v.trim()!==""&&Number.isFinite(Number(v))));
  }
  const cmd="./build/kline_quant "+e.command+" quant/research/evidence-examples/"+e.file+(e.params.length?" "+e.params.join(" "):"");
  assert(section.includes(esc(cmd)));
  assert(section.includes('href="../research/evidence-examples/'+e.file+'" download'));
  assert(hub.includes('href="evidence-examples/'+e.file+'" download'));
  assert(cli.includes('"'+e.command+'"'));assert(!names.has(e.command));names.add(e.command);
  const f=[...section.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)];
  assert.equal(f.length,2);
  for(const m of f){katex.renderToString(unesc(m[1]),{throwOnError:true,strict:"error"});formulas++;}
  const table=section.match(/<tbody>([\s\S]*?)<\/tbody>/);
  assert(table,"Missing analytic table: "+e.command);
  const observed=[...table[1].matchAll(/<tr>([\s\S]*?)<\/tr>/g)].map(r=>[...r[1].matchAll(/<td>(.*?)<\/td>/g)].map(c=>Number(c[1])));
  assert.equal(observed.length,e.expected.length);
  observed.forEach((r,i)=>{
   assert.equal(r.length,e.expected[i].length);
   r.forEach((v,k)=>{assert(Number.isFinite(v)&&Math.abs(v-e.expected[i][k])<=1e-8*Math.max(1,Math.abs(e.expected[i][k])),"Analytic table mismatch "+e.command);coordinates++;});
   rows++;
  });
  chapters++;
 }
}
assert.equal(groups.length,8);assert.equal(chapters,32);assert.equal(formulas,64);assert.equal(rows,71);
for(const s of [root,cpp,nav,read("quant/research/index.html")])assert(s.includes("evidence-laboratories.html"));
assert(hub.includes("run_evidence_labs.py")&&hub.includes("catalog.json"));
assert(cpp.includes("<strong>321</strong>")&&cpp.includes("<strong>995</strong>"));
console.log("PASS: "+chapters+" evidence chapters, "+formulas+" strict KaTeX equations, "+rows+" analytic output rows, "+coordinates+" verified coordinates, 32 exact data downloads and eight source-linked topics");
