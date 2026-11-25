import assert from 'node:assert/strict';
import fs from 'node:fs';
import path from 'node:path';
import katex from 'katex';

const root = 'quant/research';
const read = name => fs.readFileSync(path.join(root,name),'utf8');
const decode = s => s.replaceAll('&lt;','<').replaceAll('&gt;','>').replaceAll('&quot;','"').replaceAll('&amp;','&');
const catalog = JSON.parse(read('applied-theory-catalog.json'));
assert.equal(catalog.length,8);
assert.equal(new Set(catalog.map(c=>c.id)).size,8);
const hub = read('applied-theory.html');
assert(read('applied-systems.html').includes('href="applied-theory.html"'));
let units=0, formulas=0, localLinks=0;
for(const c of catalog){
  const html=read(c.file);
  assert(hub.includes('href="'+c.file+'"'),c.id+' missing from theory hub');
  const group=read(c.group);
  assert(group.includes('href="'+c.file+'"') && group.includes('href="'+c.project+'"'),c.id+' missing group navigation');
  assert(html.includes('href="'+c.group+'"') && html.includes('href="'+c.project+'"'));
  assert(html.includes(c.title));
  assert.equal(c.sections,7);
  const parts=[...html.matchAll(/<section>\s*<h2>(\d+)．[\s\S]*?<\/section>/g)];
  assert.equal(parts.length,c.sections,c.id+' units');
  for(const [i,part] of parts.entries()){
    assert.equal(Number(part[1]),i+1);
    assert((part[0].match(/<p>/g)||[]).length>=2);
    assert.equal((part[0].match(/<div class="equation">/g)||[]).length,2);
    const prose=part[0].replace(/<div class="equation">[\s\S]*?<\/div>/g,'').replace(/<[^>]+>/g,'');
    assert(prose.length>100,c.id+' insufficient explanation '+(i+1));
    units++;
  }
  const eq=[...html.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)];
  assert.equal(eq.length,c.formulaCount,c.id);
  for(const m of eq){
    katex.renderToString(decode(m[1]),{displayMode:true,throwOnError:true,strict:'error'});
    formulas++;
  }
  for(const link of c.links)assert(html.includes('href="'+link+'"'),c.id+' missing '+link);
  assert(/<a href="https?:\/\//.test(html),c.id+' missing bibliography');
  for(const m of html.matchAll(/(?:href|src)="([^"]+)"/g)){
    const u=m[1].split('#')[0];
    if(!u||/^https?:\/\//.test(u))continue;
    assert(fs.existsSync(path.resolve(root,u)),c.file+' broken link: '+u);
    localLinks++;
  }
}
assert.equal(units,56);
assert.equal(formulas,112);
console.log('PASS: '+catalog.length+' theory courses, '+units+' derivation units, '+formulas+' strict KaTeX formula groups, '+localLinks+' local links');
