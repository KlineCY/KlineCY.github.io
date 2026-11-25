import fs from 'node:fs';
import assert from 'node:assert/strict';
import path from 'node:path';
import katex from 'katex';
const root='quant/research/';
const groups=['inference','credit','derivatives','policy','exact','queue','optimization','control'];
const read=p=>fs.readFileSync(p,'utf8');
const decode=s=>s.replaceAll('&lt;','<').replaceAll('&gt;','>').replaceAll('&quot;','"').replaceAll('&amp;','&');
const main=read(root+'finite-state-research.html'),cli=read('quant/cpp/finite_state_cli.cpp');
let methods=new Set(),equations=0,links=0,coordinates=0;
for(const g of groups){
 const hub=read(root+'finite-state-'+g+'.html'),header=read('quant/cpp/finite_state_'+g+'.hpp');
 assert(main.includes('finite-state-'+g+'.html'));assert(main.includes('finite_state_'+g+'.hpp'));
 const code=hub.match(/<h2>完整C\+\+17实现<\/h2>[\s\S]*?<pre>([\s\S]*?)<\/pre>/);
 assert(code&&decode(code[1])===header,g+' embedded source');
 const cat=JSON.parse(read(root+'finite-state-'+g+'-cases.json'));assert.equal(cat.length,4);
 for(const c of cat){
  assert(!methods.has(c.id));methods.add(c.id);
  assert(cli.includes('{"'+c.id+'",fsq::'+c.id+'}'));
  assert(header.includes('Vec '+c.id+'('));assert(main.includes(c.file)&&hub.includes(c.file));
  const file=root+c.file,html=read(file);
  assert(html.includes(c.title));assert(html.includes('研究问题与模型假设'));assert(html.includes('数学递推与证明'));
  assert(html.includes('./quant_finite '+c.id+' '+c.input.join(' ')));
  const v=html.match(/解析参考向量：<\/p><pre>([\s\S]*?)<\/pre>/)[1].split(', ').map(Number);
  assert.equal(v.length,c.expected.length);
  v.forEach((z,i)=>{assert(Number.isFinite(z)&&Math.abs(z-c.expected[i])<=1e-10*Math.max(1,Math.abs(c.expected[i])),c.id);coordinates++;});
  const eq=[...html.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)];
  assert.equal(eq.length,3,c.id);
  for(const a of eq){katex.renderToString(decode(a[1]),{throwOnError:true,strict:'error',displayMode:true});equations++;}
  for(const a of html.matchAll(/(?:href|src)="([^"]+)"/g)){
   const u=a[1];if(/^https?:\/\//.test(u))continue;
   assert(fs.existsSync(path.resolve(path.dirname(file),u)),file+' -> '+u);links++;
  }
 }
}
assert.equal(methods.size,32);assert.equal(equations,96);
const projects=JSON.parse(read(root+'finite-state-projects-cases.json'));
const ph=read(root+'finite-state-projects.html'),pc=read('quant/cpp/finite_state_projects.hpp');
assert(main.includes('finite-state-projects.html')&&ph.includes('finite_state_project_reference.py'));
const code=ph.match(/<h2>完整C\+\+17实现<\/h2><pre>([\s\S]*?)<\/pre>/);
assert(code&&decode(code[1])===pc,'project embedded source');
assert.equal(projects.length,8);
for(const c of projects){
 assert(!methods.has(c.id));methods.add(c.id);assert(main.includes(c.file)&&ph.includes(c.file));
 assert(cli.includes('{"'+c.id+'",fsq::'+c.id+'}')&&pc.includes('Vec '+c.id+'('));
 const file=root+c.file,html=read(file);
 assert(html.includes('./quant_finite '+c.id+' '+c.input.join(' ')));
 const v=html.match(/解析参考向量：<\/p><pre>([\s\S]*?)<\/pre>/)[1].split(', ').map(Number);
 assert.equal(v.length,c.expected.length);
 v.forEach((z,i)=>{assert(Number.isFinite(z)&&Math.abs(z-c.expected[i])<=1e-10*Math.max(1,Math.abs(c.expected[i])),c.id);coordinates++;});
 const eq=[...html.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)];assert.equal(eq.length,3,c.id);
 for(const a of eq){katex.renderToString(decode(a[1]),{throwOnError:true,strict:'error',displayMode:true});equations++;}
 for(const a of html.matchAll(/(?:href|src)="([^"]+)"/g)){const u=a[1];if(/^https?:\/\//.test(u))continue;assert(fs.existsSync(path.resolve(path.dirname(file),u)),file+' -> '+u);links++;}
 for(const l of c.links)assert(html.includes(l)&&fs.existsSync(root+l));
}
assert.equal(methods.size,40);assert.equal(equations,120);
for(const p of [root+'index.html',root+'applied-systems.html','quant/cpp/index.html','quant/sidebar-v2.js'])assert(read(p).includes('finite-state-research.html'),p);
console.log('PASS: '+methods.size+' research chapters, '+equations+' strict KaTeX groups, '+coordinates+' displayed reference values, '+links+' local links, nine embedded source files');
