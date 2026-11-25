import fs from 'node:fs';
import path from 'node:path';
import assert from 'node:assert/strict';
import katex from 'katex';
import {projects,columns} from '../quant/research/multiscale_advanced_experiment_reference.mjs';
const root='quant/research/',read=p=>fs.readFileSync(p,'utf8');
const cat=JSON.parse(read(root+'multiscale-advanced-catalog.json'));
const hub=read(root+'multiscale-advanced-experiments.html');
let rows=0,coords=0,eq=0,links=0;
const decode=s=>s.replaceAll('&lt;','<').replaceAll('&gt;','>').replaceAll('&amp;','&');
assert.equal(cat.length,6);
for(const p of cat){
  const csv=read(root+p.csv).trim().split('\n'),ref=projects[p.id]();
  assert.equal(csv[0],columns[p.id]);assert.equal(csv[0],p.fields.join(','));
  assert.equal(csv.length-1,ref.length);assert.equal(ref.length,p.rows);
  csv.slice(1).forEach((r,i)=>{const a=r.split(',').map(Number);assert.equal(a.length,ref[i].length);a.forEach((v,j)=>{assert(Number.isFinite(v)&&Math.abs(v-ref[i][j])<=1e-10*Math.max(1,Math.abs(ref[i][j])));coords++;});});rows+=ref.length;
  assert(hub.includes(p.file)&&hub.includes(p.csv));
  const filename=root+p.file,html=read(filename);
  for(const a of html.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)){katex.renderToString(decode(a[1]),{throwOnError:true,strict:'error',displayMode:true});eq++;}
  for(const a of html.matchAll(/(?:href|src)="([^"]+)"/g)){if(/^https?:\/\//.test(a[1]))continue;assert(fs.existsSync(path.resolve(path.dirname(filename),a[1])),filename+' -> '+a[1]);links++;}
}
assert.equal(rows,966);assert.equal(coords,10218);assert.equal(eq,18);
for(const f of ['multiscale-research.html','multiscale-experiments.html','multiscale-theory.html'])assert(read(root+f).includes('multiscale-advanced-experiments.html'));
console.log('PASS: '+rows+' regenerated advanced CSV rows, '+coords+' fields, '+eq+' formulas and '+links+' local links');
