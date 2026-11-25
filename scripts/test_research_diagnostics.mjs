import fs from 'node:fs';
import assert from 'node:assert/strict';
import katex from 'katex';
const root='quant/research/';
const refs=JSON.parse(fs.readFileSync(root+'diagnostic-cases.json','utf8'));
const ids=[...new Set(refs.map(x=>x.id))];assert.equal(ids.length,16);assert.equal(refs.length,32);
const idx=fs.readFileSync(root+'research-diagnostics.html','utf8');
const main=fs.readFileSync(root+'index.html','utf8');assert(main.includes('research-diagnostics.html'));
const code=fs.readFileSync('quant/cpp/research_diagnostics.cpp','utf8');
for(const id of ids){const name='diagnostic-'+id.replaceAll('_','-')+'.html';assert(idx.includes(name));const html=fs.readFileSync(root+name,'utf8');const equations=[...html.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)];assert(equations.length>=1);for(const m of equations)katex.renderToString(m[1],{throwOnError:true,strict:'error'});assert(code.includes('Vec '+id+'('));assert(html.includes('../cpp/research_diagnostics.cpp'));}
console.log('16 linked topics; strict formula validation and source discovery passed');
