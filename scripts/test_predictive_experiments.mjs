import fs from 'node:fs';import path from 'node:path';import assert from 'node:assert/strict';import {spawnSync} from 'node:child_process';
import {projects as first,columns as c1} from '../quant/research/predictive_experiment_reference.mjs';
import {projects as second,columns as c2} from '../quant/research/predictive_advanced_experiment_reference.mjs';
const root='quant/research/',read=p=>fs.readFileSync(p,'utf8'),curr=process.argv[2]==='--curriculum';
const decode=s=>s.replaceAll('&lt;','<').replaceAll('&gt;','>').replaceAll('&amp;','&');
let rows=0,fields=0,eq=0,links=0;
function compare(x,y,ctx){assert(Number.isFinite(x)&&Number.isFinite(y),ctx+' nonfinite');assert(Math.abs(x-y)<=2e-5*Math.max(1,Math.abs(y)),ctx+' '+x+' != '+y);}
function check(a,b,id){assert.equal(a.length,b.length,id);for(let j=0;j<a.length;j++){assert.equal(a[j].length,b[j].length,id+' row '+j);for(let k=0;k<a[j].length;k++)compare(a[j][k],b[j][k],id+' row '+j+' col '+k);}}
for(const [n,ref,col,exe] of [['experiment',first,c1,process.argv[2]],['advanced',second,c2,process.argv[3]]]){
 const cat=JSON.parse(read(root+'predictive-'+n+'-catalog.json'));
 for(const c of cat){let lines=read(root+c.csv).trim().split('\n');assert.equal(lines.shift(),col[c.id]);const expected=lines.map(l=>l.split(',').map(Number));assert.equal(expected.length,c.rows);const refRows=ref[c.id]();check(refRows,expected,c.id+' reference');
 if(curr){const katex=(await import('katex')).default,html=read(root+'predictive-project-'+c.id+'.html');assert(html.includes(c.csv)&&html.includes(c.rows+'行'));for(const m of html.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)){katex.renderToString(decode(m[1]),{throwOnError:true,strict:'error',displayMode:true});eq++;}for(const m of html.matchAll(/(?:href|src)="([^"]+)"/g)){if(/^https?:\/\//.test(m[1]))continue;assert(fs.existsSync(path.resolve(root,m[1])),m[1]);links++;}}
 else{assert(exe);const p=spawnSync(exe,[c.id],{encoding:'utf8',maxBuffer:32*1024*1024});assert.equal(p.status,0,c.id+' '+p.stderr);let a=p.stdout.trim().split('\n');assert.equal(a.shift(),col[c.id]);const data=a.map(l=>l.split(',').map(Number));check(data,expected,c.id+' native');for(const r of data){if(c.id==='delayed')assert(Math.abs(r[10])<1e-12);if(c.id==='attention')assert(Math.abs(r[4])+Math.abs(r[7])<1e-12);if(c.id==='decision')assert(r[7]>=0&&r[8]>=-1e-12&&r[9]>=r[8]-1e-10);if(c.id==='utility')assert(Math.abs(r[10]+r[11]+r[12]-1)<1e-10&&Math.abs(r[13])<1e-10);if(c.id==='explanation')assert(Math.abs(r[8])<1e-10);if(c.id==='calibration')assert(r[5]<=r[4]+1e-10&&r[6]<=r[2]+1e-10&&(!r[8]||r[11]<=r[3]+1e-10));if(c.id==='maturity')assert(Math.abs(r[7])+Math.abs(r[8])+Math.abs(r[9])<1e-12);if(c.id==='hypergrad')assert(r[9]<1e-8);if(c.id==='image')assert(r[7]===0&&r[8]<1e-8);}}
 rows+=c.rows;fields+=c.rows*c.fields.length;
 }
}
assert.equal(rows,1026);if(curr)assert.equal(eq,48);
console.log('PASS: 16 integrated projects, '+rows+' scenarios, '+fields+' numeric fields'+(curr?', '+eq+' formulas and '+links+' local links':', C++ and independent reference with structural invariants'));
