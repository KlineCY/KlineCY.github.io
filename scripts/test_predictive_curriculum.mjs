import fs from 'node:fs';import path from 'node:path';import assert from 'node:assert/strict';import katex from 'katex';
const root='quant/research/',read=p=>fs.readFileSync(p,'utf8'),main=read(root+'predictive-decision.html'),cli=read('quant/cpp/predictive_cli.cpp');
const decode=s=>s.replaceAll('&lt;','<').replaceAll('&gt;','>').replaceAll('&amp;','&');
let count=0,eq=0,links=0,display=0;
for(const g of ['kernel','online','neural','portfolio']){
 const cat=JSON.parse(read(root+'predictive-'+g+'-catalog.json')),hub=read(root+'predictive-'+g+'.html'),header=read('quant/cpp/predictive_'+g+'.hpp');
 assert.equal(cat.length,6);assert(main.includes('predictive-'+g+'.html'));assert.equal(decode(hub.match(/<pre>([\s\S]*?)<\/pre>/)[1]),header);
 for(const c of cat){
  count++;assert(cli.includes('{"'+c.id+'",pdq::'+c.id+'}'));assert(hub.includes(c.file)&&main.includes(c.file));
  const name=root+c.file,html=read(name),data=JSON.parse(read(root+c.cases));assert.equal(data.length,c.rows);
  assert(html.includes('./quant_predictive '+c.id+' '+data[0].input.join(' ')));
  const a=[...html.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)];assert.equal(a.length,3);
  for(const m of a){katex.renderToString(decode(m[1]),{throwOnError:true,strict:'error',displayMode:true});eq++;}
  for(const m of html.matchAll(/<tr><td>(\d+)<\/td><td>([^<]+)<\/td><\/tr>/g)){const v=Number(m[2]),e=data[0].expected[Number(m[1])];assert(Number.isFinite(v)&&Math.abs(v-e)<=1e-10*Math.max(1,Math.abs(e)));display++;}
  for(const m of html.matchAll(/(?:href|src)="([^"]+)"/g)){if(/^https?:\/\//.test(m[1]))continue;assert(fs.existsSync(path.resolve(path.dirname(name),m[1])),name+' -> '+m[1]);links++;}
 }
}
assert.equal(count,24);assert.equal(eq,72);
for(const p of ['quant/ml/index.html','quant/research/index.html','quant/cpp/index.html','quant/research/multiscale-research.html','quant/sidebar-v2.js'])assert(read(p).includes('predictive-decision.html'));
console.log('PASS: '+count+' chapters, '+eq+' strict KaTeX groups, '+display+' published sample values, '+links+' local links and four embedded source files');
