import fs from 'node:fs';
import assert from 'node:assert/strict';
import path from 'node:path';
import katex from 'katex';
const root='quant/research/';
const groups=['simulation','microstructure','engineering','empirical'];
const read=p=>fs.readFileSync(p,'utf8');
const decode=s=>s.replaceAll('&lt;','<').replaceAll('&gt;','>').replaceAll('&quot;','"').replaceAll('&amp;','&');
const main=read(root+'multiscale-research.html'),cli=read('quant/cpp/multiscale_cli.cpp');
let methods=new Set(),equations=0,links=0,coordinates=0;
for(const g of groups){
 const hub=read(root+'multiscale-'+g+'.html'),header=read('quant/cpp/multiscale_'+g+'.hpp');
 assert(main.includes('multiscale-'+g+'.html'));assert(main.includes('multiscale_'+g+'.hpp'));
 const code=hub.match(/<h2>原生计算实现<\/h2>[\s\S]*?<pre>([\s\S]*?)<\/pre>/);
 assert(code&&decode(code[1])===header,g+' embedded source');
 const cat=JSON.parse(read(root+'multiscale-'+g+'-cases.json'));assert.equal(cat.length,8);
 for(const c of cat){
  assert(!methods.has(c.id));methods.add(c.id);
  assert(cli.includes('{"'+c.id+'",msq::'+c.id+'}'));assert(header.includes('Vec '+c.id+'('));
  assert(main.includes(c.file)&&hub.includes(c.file));
  const filename=root+c.file,html=read(filename);
  assert(html.includes(c.title));assert(html.includes('研究对象与假设'));assert(html.includes('数学推导与计算结构'));
  assert(html.includes('./quant_multiscale '+c.id+' '+c.input.join(' ')));
  const values=[...html.matchAll(/<tr><td>(\d+)<\/td><td>([^<]+)<\/td><\/tr>/g)];
  assert.equal(values.length,Object.keys(c.expected).length);
  for(const m of values){const v=Number(m[2]),e=c.expected[m[1]];assert(Number.isFinite(v)&&Math.abs(v-e)<=1e-10*Math.max(1,Math.abs(e)),c.id);coordinates++;}
  const eq=[...html.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)];
  assert.equal(eq.length,3,c.id);
  for(const a of eq){katex.renderToString(decode(a[1]),{throwOnError:true,strict:'error',displayMode:true});equations++;}
  for(const a of html.matchAll(/(?:href|src)="([^"]+)"/g)){
   const u=a[1];if(/^https?:\/\//.test(u))continue;
   assert(fs.existsSync(path.resolve(path.dirname(filename),u)),filename+' -> '+u);links++;
  }
 }
}
assert.equal(methods.size,32);assert.equal(equations,96);
for(const p of [root+'index.html',root+'finite-state-research.html',root+'continuous-research.html','quant/cpp/index.html','quant/sidebar-v2.js'])assert(read(p).includes('multiscale-research.html'),p);
console.log('PASS: '+methods.size+' research chapters, '+equations+' strict KaTeX groups, '+coordinates+' displayed reference values, '+links+' local links, four embedded source files');
