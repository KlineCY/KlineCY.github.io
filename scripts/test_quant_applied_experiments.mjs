import fs from 'node:fs';
import assert from 'node:assert/strict';
import path from 'node:path';
import katex from 'katex';
const root='quant/research/';
const read=p=>fs.readFileSync(p,'utf8');
const cat=JSON.parse(read(root+'applied-experiment-catalog.json'));
const index=read(root+'applied-experiments.html');
const code=read('quant/cpp/applied_experiments.cpp');
const decode=s=>s.replaceAll('&lt;','<').replaceAll('&gt;','>').replaceAll('&quot;','"').replaceAll('&amp;','&');
let equations=0,rows=0,coords=0,sampleCoords=0,links=0;
assert.equal(cat.length,8);
assert(read(root+'applied-systems.html').includes('applied-experiments.html'));
for(const c of cat){
 assert(index.includes(c.file)&&index.includes(c.csv));
 const filename=root+c.file,html=read(filename);
 const csv=read(root+c.csv).trim().split('\n');
 assert.equal(csv[0],c.fields.join(','));
 assert.equal(csv.length-1,c.rows);
 const data=csv.slice(1).map(x=>x.split(',').map(Number));
 for(const x of data){assert.equal(x.length,c.fields.length);assert(x.every(Number.isFinite));}
 rows+=data.length;coords+=data.length*c.fields.length;
 const formulas=[...html.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)];
 assert.equal(formulas.length,4,c.id);
 for(const f of formulas){katex.renderToString(decode(f[1]),{strict:'error',throwOnError:true});equations++;}
 const blocks=[...html.matchAll(/<pre>([\s\S]*?)<\/pre>/g)];
 assert(code.includes(decode(blocks.at(-1)[1])),c.id+' source');
 const sample=[...new Set([0,Math.floor(c.rows/4),Math.floor(c.rows/2),Math.floor(3*c.rows/4),c.rows-1])];
 const table=html.match(/<tbody>([\s\S]*?)<\/tbody>/)[1];
 const trs=[...table.matchAll(/<tr>([\s\S]*?)<\/tr>/g)];
 assert.equal(trs.length,sample.length);
 for(let i=0;i<trs.length;++i){
  const values=[...trs[i][1].matchAll(/<td>(.*?)<\/td>/g)].map(z=>Number(z[1]));
  assert.equal(values.length,c.fields.length);
  values.forEach((v,j)=>{const e=data[sample[i]][j];assert(Number.isFinite(v)&&Math.abs(v-e)<=1e-8*Math.max(1,Math.abs(e)),c.id);sampleCoords++;});
 }
 for(const l of c.links)assert(html.includes(l)&&fs.existsSync(root+l));
 for(const m of html.matchAll(/(?:href|src)="([^"]+)"/g)){
  const h=m[1];if(h.startsWith('https:')||h.startsWith('#'))continue;
  assert(fs.existsSync(path.resolve(path.dirname(filename),h)),filename+' missing '+h);links++;
 }
}
assert.equal(rows,1073);assert.equal(coords,11718);assert.equal(equations,32);
console.log('PASS: 8 projects, '+rows+' scenarios, '+coords+' CSV coordinates, '+equations+' strict KaTeX formula groups, '+sampleCoords+' displayed sample coordinates, '+links+' local links');
