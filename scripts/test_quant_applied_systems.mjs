import fs from 'node:fs';
import assert from 'node:assert/strict';
import path from 'node:path';
import katex from 'katex';
const root='quant/research/';
const groups=['forecast','inference','liquidity','rates','options','credit','accounts','allocation','synthesis'];
const read=p=>fs.readFileSync(p,'utf8');
const decode=s=>s.replaceAll('&lt;','<').replaceAll('&gt;','>').replaceAll('&quot;','"').replaceAll('&amp;','&');
const all=[],main=read(root+'applied-systems.html');
let eq=0,numbers=0,links=0;
for(const g of groups){
 const cat=JSON.parse(read(root+'applied-'+g+'-cases.json'));
 const hub=g==='allocation'?'applied-allocation-systems.html':'applied-'+g+'.html';
 const header=['allocation','credit'].includes(g)?'applied_lab_'+g+'.hpp':'applied_'+g+'.hpp';
 const code=read('quant/cpp/'+header),index=read(root+hub);
 assert.equal(cat.length,g==='synthesis'?16:7);
 assert(main.includes(hub));assert(main.includes(header));
 for(const c of cat){
  all.push(c.id);assert(index.includes(c.file));assert(main.includes(c.file));
  const fn=root+c.file,html=read(fn);
  const formula=[...html.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)];
  assert.equal(formula.length,2,fn);
  for(const x of formula){katex.renderToString(decode(x[1]),{throwOnError:true,strict:'error'});eq++;}
  const method=c.id.slice(g.length+1);
  assert(code.includes('Vec '+method+'('),c.id);
  const embedded=html.match(/<h2>(?:完整计算函数|原生C\+\+17组合实现)<\/h2><pre>([\s\S]*?)<\/pre>/);
  assert(embedded&&code.includes(decode(embedded[1])),c.id+' source mismatch');
  assert.equal(c.cases.length,2);
  const cells=[...html.matchAll(/<tbody><tr>([\s\S]*?)<\/tr><\/tbody>/g)];
  assert.equal(cells.length,2);
  cells.forEach((m,i)=>{let values=[...m[1].matchAll(/<td>(.*?)<\/td>/g)].map(z=>Number(z[1]));assert.equal(values.length,c.cases[i].expected.length);values.forEach((v,j)=>{let e=c.cases[i].expected[j];assert(Number.isFinite(v)&&Math.abs(v-e)<=1e-10*Math.max(1,Math.abs(e)),c.id+' table '+j);numbers++;});});
  for(const match of html.matchAll(/(?:href|src)="([^"]+)"/g)){
   let h=match[1];if(h.startsWith('https:')||h.startsWith('#'))continue;
   assert(fs.existsSync(path.resolve(path.dirname(fn),h)),fn+' missing '+h);links++;
  }
  for(const l of c.links||[])assert(html.includes(l)&&fs.existsSync(root+l));
 }
}
assert.equal(new Set(all).size,72);assert.equal(eq,144);
for(const p of [root+'index.html','quant/cpp/index.html','quant/sidebar-v2.js',root+'integrated-research.html'])assert(read(p).includes('applied-systems.html'));
console.log('PASS: 72 topics, '+eq+' strict KaTeX formula groups, '+numbers+' published numerical coordinates, '+links+' local links; source fidelity and navigation');
