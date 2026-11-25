import fs from 'node:fs';
import path from 'node:path';
import assert from 'node:assert/strict';
import katex from 'katex';
import {projects,columns} from '../quant/research/multiscale_experiment_reference.mjs';
const root='quant/research/';
const read=p=>fs.readFileSync(p,'utf8');
const cat=JSON.parse(read(root+'multiscale-experiment-catalog.json'));
const hub=read(root+'multiscale-experiments.html'),code=read('quant/cpp/multiscale_experiments.cpp');
const decode=s=>s.replaceAll('&lt;','<').replaceAll('&gt;','>').replaceAll('&quot;','"').replaceAll('&amp;','&');
let rows=0,coordinates=0,equations=0,links=0,sample=0;
assert.equal(cat.length,8);
assert(read(root+'multiscale-research.html').includes('multiscale-experiments.html'));
for(const c of cat){
  assert(hub.includes(c.file)&&hub.includes(c.csv));
  const data=read(root+c.csv).trim().split('\n');
  assert.equal(data[0],c.fields.join(','));
  assert.equal(data[0],columns[c.id]);
  const actual=projects[c.id]();
  assert.equal(actual.length,data.length-1);assert.equal(actual.length,c.rows);
  const expected=data.slice(1).map(x=>x.split(',').map(Number));
  for(let i=0;i<actual.length;i++){
    assert.equal(actual[i].length,c.fields.length);
    for(let j=0;j<actual[i].length;j++){
      assert(Number.isFinite(expected[i][j])&&Math.abs(expected[i][j]-actual[i][j])<=1e-10*Math.max(1,Math.abs(actual[i][j])),c.id+' CSV '+i+','+j);coordinates++;
    }
  }rows+=actual.length;
  const name=root+c.file,html=read(name);
  const blocks=[...html.matchAll(/<pre>([\s\S]*?)<\/pre>/g)];
  assert(code.includes(decode(blocks.at(-1)[1]))&&decode(blocks.at(-1)[1]).startsWith('void '+c.source+'(){'),c.id+' source');
  const eq=[...html.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)];
  assert.equal(eq.length,4);
  for(const m of eq){katex.renderToString(decode(m[1]),{throwOnError:true,strict:'error',displayMode:true});equations++;}
  const indices=[...new Set([0,Math.floor(c.rows/4),Math.floor(c.rows/2),c.rows-1])];
  const cells=[...html.matchAll(/<tbody>([\s\S]*?)<\/tbody>/g)];
  assert.equal(cells.length,1);
  const trs=[...cells[0][1].matchAll(/<tr>([\s\S]*?)<\/tr>/g)];
  assert.equal(trs.length,indices.length);
  for(let i=0;i<trs.length;i++){
    const vals=[...trs[i][1].matchAll(/<td>([^<]+)<\/td>/g)].map(z=>Number(z[1]));
    assert.equal(vals.length,c.fields.length);
    vals.forEach((v,j)=>{assert(Number.isFinite(v)&&Math.abs(v-actual[indices[i]][j])<=1e-8*Math.max(1,Math.abs(actual[indices[i]][j])),c.id+' display');sample++;});
  }
  for(const l of c.links)assert(html.includes(l)&&fs.existsSync(root+l));
  for(const m of html.matchAll(/(?:href|src)="([^"]+)"/g)){
    const u=m[1];if(/^https?:\/\//.test(u))continue;
    assert(fs.existsSync(path.resolve(path.dirname(name),u)),name+' -> '+u);links++;
  }
}
assert.equal(rows,491);assert.equal(coordinates,5587);assert.equal(equations,32);
console.log('PASS: 8 complete research projects, '+rows+' regenerated CSV rows, '+coordinates+' coordinates, '+equations+' strict KaTeX groups, '+sample+' displayed sample values, '+links+' local links');
