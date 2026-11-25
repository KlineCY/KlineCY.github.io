import fs from 'node:fs';
import assert from 'node:assert/strict';
import path from 'node:path';
import katex from 'katex';
const root='quant/research/';
const read=p=>fs.readFileSync(p,'utf8');
const decode=s=>s.replaceAll('&lt;','<').replaceAll('&gt;','>').replaceAll('&quot;','"').replaceAll('&amp;','&');
const main=read(root+'continuous-research.html'),cli=read('quant/cpp/continuous_research_cli.cpp');
let methods=new Set(),equations=0,links=0,coordinates=0;
for(const g of ['filter','pricing']){
 const hub=read(root+'continuous-'+g+'.html'),header=read('quant/cpp/continuous_research_'+g+'.hpp');
 assert(main.includes('continuous-'+g+'.html'));
 const code=hub.match(/<h2>完整原生C\+\+17实现<\/h2><pre>([\s\S]*?)<\/pre>/);
 assert(code&&decode(code[1])===header,g+' embedded source');
 const cat=JSON.parse(read(root+'continuous-'+g+'-cases.json'));assert.equal(cat.length,4);
 for(const c of cat){
  assert(!methods.has(c.id));methods.add(c.id);
  assert(cli.includes('{"'+c.id+'",crq::'+c.id+'}')&&header.includes('Vec '+c.id+'('));
  assert(hub.includes(c.file));
  const file=root+c.file,html=read(file);assert(html.includes(c.title));
  assert(html.includes('./quant_continuous '+c.id+' '+c.input.join(' ')));
  const v=[...html.matchAll(/<tr><td>(\d+)<\/td><td>([^<]+)<\/td><\/tr>/g)];
  assert.equal(v.length,Object.keys(c.expected).length);
  for(const row of v){const j=Number(row[1]),z=Number(row[2]);assert(Number.isFinite(z)&&Math.abs(z-c.expected[j])<=1e-10*Math.max(1,Math.abs(c.expected[j])),c.id);coordinates++;}
  const eq=[...html.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)];assert.equal(eq.length,4,c.id);
  for(const a of eq){katex.renderToString(decode(a[1]),{throwOnError:true,strict:'error',displayMode:true});equations++;}
  for(const a of html.matchAll(/(?:href|src)="([^"]+)"/g)){const u=a[1];if(/^https?:\/\//.test(u))continue;assert(fs.existsSync(path.resolve(path.dirname(file),u)),file+' -> '+u);links++;}
 }
}
assert.equal(methods.size,8);assert.equal(equations,32);
console.log('PASS:',methods.size,'chapters,',equations,'strict KaTeX groups,',coordinates,'reference coordinates,',links,'local links, two full embedded headers');
