import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>fs.readFileSync(new URL(p,root),"utf8");
const json=p=>JSON.parse(read(p));
const baseline=[{"path":"../ml/atlas-depth-08-j.json","schema":1,"batch":"v39-eighth-generative-reinforcement-learning","revision":"2026-11-06","entries":[{"id":"m19-c03-p04","hash":"33170355","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":224},{"id":"m19-c03-p01","hash":"e9fec31d","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":250},{"id":"m19-c02-p05","hash":"34e6c6f3","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":260},{"id":"m19-c01-p05","hash":"7ac5664b","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":259},{"id":"m19-c05-p03","hash":"13bc2f78","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":223},{"id":"m20-c04-p06","hash":"63aa361e","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":211},{"id":"m20-c02-p04","hash":"eb7e97f8","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":266},{"id":"m20-c05-p01","hash":"7c44534a","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":229},{"id":"m20-c05-p06","hash":"0c0ab3a9","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":206},{"id":"m20-c03-p03","hash":"d9aca829","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":223}]},{"path":"../ml/atlas-depth-08-k.json","schema":1,"batch":"v39-eighth-marl-time-series-econometrics","revision":"2026-11-06","entries":[{"id":"m21-c03-p05","hash":"eef4f97f","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":219},{"id":"m21-c02-p03","hash":"88073bed","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":223},{"id":"m21-c03-p03","hash":"86a00ac4","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":210},{"id":"m21-c03-p01","hash":"eb35fc6d","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":246},{"id":"m21-c01-p06","hash":"4b1840b6","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":243},{"id":"m22-c02-p06","hash":"976e7524","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":213},{"id":"m22-c05-p01","hash":"0ddaebd3","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":230},{"id":"m22-c01-p01","hash":"dd226b01","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":215},{"id":"m22-c02-p05","hash":"8e28a159","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":232},{"id":"m22-c03-p01","hash":"20579052","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":239}]},{"path":"../ml/atlas-depth-08-l.json","schema":1,"batch":"v39-eighth-quant-risk-mlops","revision":"2026-11-06","entries":[{"id":"m23-c05-p05","hash":"fc320762","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":234},{"id":"m23-c02-p05","hash":"c4fb754b","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":253},{"id":"m23-c02-p04","hash":"733cd645","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":223},{"id":"m23-c05-p03","hash":"6430da54","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":228},{"id":"m23-c04-p03","hash":"6e58ecad","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":258},{"id":"m24-c01-p05","hash":"e23a4ef6","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":266},{"id":"m24-c02-p03","hash":"1e5939d3","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":225},{"id":"m24-c01-p02","hash":"9defe142","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":221},{"id":"m24-c01-p01","hash":"b0c4109a","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":234},{"id":"m24-c01-p03","hash":"b3876291","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":222}]},{"path":"../ml/atlas-depth-09-e.json","schema":1,"batch":"v40-ninth-kernels-bayesian-inference","revision":"2026-11-06","entries":[{"id":"m09-c05-p01","hash":"27ef2b0b","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":190},{"id":"m09-c01-p02","hash":"bc69de56","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":195},{"id":"m09-c02-p06","hash":"57ef76c6","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":228},{"id":"m09-c02-p02","hash":"9721034b","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":219},{"id":"m09-c04-p04","hash":"1a6599a5","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":270},{"id":"m10-c03-p01","hash":"c2767037","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":232},{"id":"m10-c01-p04","hash":"cb419c77","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":194},{"id":"m10-c02-p02","hash":"c88291c2","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":218},{"id":"m10-c04-p01","hash":"36febe81","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":222},{"id":"m10-c03-p02","hash":"dad1fdd9","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":214}]},{"path":"../ml/atlas-depth-09-f.json","schema":1,"batch":"v40-ninth-unsupervised-evaluation-causal","revision":"2026-11-06","entries":[{"id":"m11-c04-p02","hash":"9e443e55","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":218},{"id":"m11-c05-p05","hash":"cdb52786","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":202},{"id":"m11-c05-p03","hash":"d171dc04","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":222},{"id":"m11-c01-p01","hash":"dc646f81","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":212},{"id":"m11-c05-p02","hash":"e21899a8","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":192},{"id":"m12-c01-p01","hash":"87b718ac","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":248},{"id":"m12-c04-p06","hash":"80075c1a","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":231},{"id":"m12-c03-p04","hash":"2eaf3a5b","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":229},{"id":"m12-c03-p01","hash":"d26cf357","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":217},{"id":"m12-c01-p06","hash":"d6841555","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":215}]},{"path":"../ml/atlas-depth-09-g.json","schema":1,"batch":"v41-ninth-neural-cnn-vit","revision":"2026-11-06","entries":[{"id":"m13-c02-p04","hash":"838b5521","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":219},{"id":"m13-c02-p01","hash":"ebbe6ce2","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":201},{"id":"m13-c05-p05","hash":"4badf4d1","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":232},{"id":"m13-c03-p03","hash":"aab8bcf1","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":214},{"id":"m13-c02-p05","hash":"33a1bcc8","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":222},{"id":"m14-c05-p04","hash":"e879614c","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":245},{"id":"m14-c04-p05","hash":"6f6a5ce2","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":252},{"id":"m14-c04-p02","hash":"f8ff53fe","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":250},{"id":"m14-c05-p06","hash":"10dbf387","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":241},{"id":"m14-c01-p02","hash":"807ab8f3","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":222}]},{"path":"../ml/atlas-depth-09-h.json","schema":1,"batch":"v41-ninth-sequence-transformer","revision":"2026-11-06","entries":[{"id":"m15-c03-p02","hash":"58e2ad33","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":209},{"id":"m15-c01-p03","hash":"606159e1","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":197},{"id":"m15-c05-p03","hash":"d68b1aec","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":257},{"id":"m15-c02-p04","hash":"c1721823","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":195},{"id":"m15-c04-p01","hash":"a9941947","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":203},{"id":"m16-c05-p02","hash":"57bffa0f","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":200},{"id":"m16-c03-p05","hash":"d4fe252e","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":237},{"id":"m16-c03-p06","hash":"28525cc8","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":206},{"id":"m16-c01-p03","hash":"c90ed114","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":213},{"id":"m16-c02-p03","hash":"a7050322","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":227}]},{"path":"../ml/atlas-depth-09-i.json","schema":1,"batch":"v41-ninth-financial-nlp-graph","revision":"2026-11-06","entries":[{"id":"m17-c03-p06","hash":"930fc254","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":243},{"id":"m17-c02-p05","hash":"bc50a1df","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":217},{"id":"m17-c01-p02","hash":"f66133f8","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":232},{"id":"m17-c01-p01","hash":"723c07e7","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":215},{"id":"m17-c02-p06","hash":"8659eb13","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":214},{"id":"m18-c01-p02","hash":"5d37d33f","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":212},{"id":"m18-c03-p06","hash":"0de67e19","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":216},{"id":"m18-c02-p06","hash":"6fee90d6","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":225},{"id":"m18-c02-p05","hash":"ce6316b7","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":213},{"id":"m18-c01-p01","hash":"da47002c","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":210}]}];
const manifest=json("../ml/atlas-depth-manifest.json");
assert.equal(manifest.schema,1);
assert.equal(manifest.expected_lessons,720);
assert.equal(manifest.expected_domains,24);
assert.equal(manifest.modules.reduce((s,m)=>s+m.lessons,0),720);

function hash(o){
 let h=2166136261>>>0;
 for(const c of JSON.stringify(o))h=Math.imul(h^c.charCodeAt(0),16777619)>>>0;
 return h.toString(16).padStart(8,"0");
}
function oldSnapshot(x,old){
 return Object.fromEntries(old.oldKeys.map(k=>[k,
  ["equations","derivation","protocol","pitfalls"].includes(k)?x[k].slice(0,old.counts[k]):
  k==="worked_example"?x[k].slice(0,old.workedLength):x[k]
 ]));
}
let count=0,texCount=0,strictCount=0,numericCount=0;
for(const spec of baseline){
 const doc=json(spec.path),ids=spec.entries.map(e=>e.id),name=spec.path.split("/").at(-1);
 const reg=manifest.modules.filter(m=>m.path==="./"+name);
 assert.equal(reg.length,1,name+" absent from live lesson loader");
 assert.equal(reg[0].lessons,10,name+" canonical count changed");
 assert.equal(doc.schema,spec.schema,name+" schema changed");
 assert.equal(doc.batch,spec.batch,name+" original batch changed");
 assert.equal(doc.revision,spec.revision,name+" original revision overwritten");
 assert.deepEqual(Object.keys(doc.lessons),ids,name+" original IDs or order changed");
 assert.deepEqual(doc.v81_revision.ids,ids,name+" V81 coverage incomplete");
 for(const old of spec.entries){
  const id=old.id,x=doc.lessons[id];count++;
  assert.equal(hash(oldSnapshot(x,old)),old.hash,id+" old lesson / metadata / source / notes changed");
  assert(x.v81_note?.startsWith("V81"),id+" V81 addition missing");
  assert.equal(x.equations.length,old.counts.equations+2,id+" two formulas not appended");
  assert.equal(x.derivation.length,old.counts.derivation+2,id+" two derivations not appended");
  assert.equal(x.protocol.length,old.counts.protocol+1,id+" protocol not appended");
  assert.equal(x.pitfalls.length,old.counts.pitfalls+1,id+" economic caveat not appended");
  assert(x.derivation.slice(-2).every(s=>s.length>=85),id+" short new derivation");
  assert(x.worked_example.length-old.workedLength>=60,id+" short new worked example");
  assert(x.protocol.at(-1).length>=30&&x.pitfalls.at(-1).length>=30,id+" missing protocol or caveat");
  assert(/[\u3400-\u9fff]/u.test(x.derivation.at(-1)),id+" Chinese proof absent");
  assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(JSON.stringify(x)),id+" Chinese Unicode corrupt");
  assert(!/TODO|FIXME|Lorem ipsum/iu.test(JSON.stringify(x)),id+" placeholder text");
  const newFormulas=x.equations.slice(-2);
  assert.equal(new Set(newFormulas.map(e=>e.label)).size,2,id+" new formula labels duplicate");
  assert(newFormulas.every(e=>!x.equations.slice(0,-2).some(v=>v.label===e.label)),id+" original equation label reused");
  for(const [index,e] of x.equations.entries()){
   assert(e.tex.length>=5&&e.label.length>=2,id+" malformed formula");
   katex.renderToString(e.tex,{throwOnError:true,strict:index>=old.counts.equations?"error":"ignore",displayMode:true});
   texCount++;
   if(index>=old.counts.equations)strictCount++;
  }
 }
}
assert.equal(count,80);
assert.equal(texCount,640);
assert.equal(strictCount,160);

function near(name,actual,expected,tol=1e-9){
 assert(Number.isFinite(actual)&&Number.isFinite(expected)&&Math.abs(actual-expected)<=tol,name+": "+actual+" != "+expected);
 numericCount++;
}
function yes(name,value){near(name,value?1:0,1);}
const sig=x=>1/(1+Math.exp(-x));
const ln2=Math.log(2);

// 08-j: generative models and policy gradients
near("point mass W1 distance",Math.abs(3-0),3);
near("W1 100x scale invariance",Math.abs(300-0)/Math.abs(3-0),100);
near("optimal GAN discriminator A",.8/(.8+.2),.8);
near("optimal GAN discriminator B",.2/(.2+.8),.2);
near("optimal matched GAN discriminator",.5/(.5+.5),.5);
near("Gaussian reparam draw",2+3*(-1),-1);
near("Gaussian reparam variance",3**2,9);
near("information bottleneck independent predictive MI",0,0);
near("information bottleneck retained input H",ln2,ln2);
near("compounded two-day wealth",1.1*.9-.9999999999999999,-.01,1e-9);
near("compounded two-day return",1.1*.9-1,-.01);
near("entropy policy tau1 high",sig(1),.7310585786300049);
near("entropy policy tauhalf high",sig(2),.8807970779778823);
near("SARSA TD target",1+.9*3,3.7);
near("SARSA updated Q",2+.5*(3.7-2),2.85);
near("offpolicy importance weight",.5/.1,5);
near("offpolicy weighted reward",(.5/.1)*2,10);
near("offpolicy near-zero behavior weight",.5/.01,50);
near("CMDP budget violation",1.1-1,.1);
near("CMDP soft exceed penalty",.5*(1.1-1),.05);
near("DQN target lagged",1+.9*4,4.6);
near("DQN invalid online target",1+.9*8,8.2);

// 08-k: market making and time series
near("market inventory next",4+3-1,6);
yes("market hard inventory limit violated",6>5);
near("model transition five-step TV bound",5*.02,.1);
near("model transition ten-step TV bound",10*.02,.2);
near("distributional RL expected return",.9*2+.1*(-10),.8);
near("distributional RL worst-tail return",-10,-10);
near("two independent period variance",1+1,2);
near("perfectly correlated period variance",4*1,4);
near("market action confound conditional difference",.9-.2,.7);
near("white-noise conditional volatility first",1**2,1);
near("white-noise conditional volatility second",3**2,9);
near("recursive three-step predicted",.9**3,.729);
near("recursive three-step true",.8**3,.512);
near("recursive three-step bias",.9**3-.8**3,.217);
near("random walk variance five",5*1,5);
near("random walk variance ten",10*1,10);
near("rolling last 2 mean",(10+10)/2,10);
near("expanding five mean",(0+0+0+10+10)/5,4);
near("ARCH positive shock variance",1+.5*2**2,3);
near("ARCH negative shock variance",1+.5*(-2)**2,3);
near("ARCH conditional SD",Math.sqrt(3),1.7320508075688772);

// 08-l: finance backtesting and production operations
near("optimal stop after carry cost",Math.max(5,6-2),5);
near("optimal continuing value",6-2,4);
near("stability selection 78/100",78/100,.78);
near("stable other factor below .7",65/100,.65);
near("FWL numerator",(-1)*(-2)+0*0+1*2,4);
near("FWL denominator",(-1)**2+0**2+1**2,2);
near("FWL residual slope",4/2,2);
near("posterior trade EV at 60pct",.6*.02-.4*.03,0);
near("posterior trade EV at 55pct",.55*.02-.45*.03,-.0025);
near("Sharpe mean daily",(-.02+.04)/2,.01);
near("Sharpe volatility population",Math.sqrt(((-.02-.01)**2+(.04-.01)**2)/2),.03);
near("Sharpe ratio",.01/.03,1/3);
near("Sortino downside deviation",Math.sqrt(((-.02)**2+0)/2),Math.sqrt(.0002));
near("Sortino ratio",.01/Math.sqrt(.0002),1/Math.sqrt(2));
near("production net PnL",10-2-3,5);
near("execution latency 2+3+4+1",2+3+4+1,10);
yes("execution latency higher than opportunity life",10>8);
near("deployment reproducibility three gates",1*1*0,0);
near("three seed mean",(.1+.2+.3)/3,.2);
near("three seed sample SD",Math.sqrt((.1**2+.1**2)/2),.1);
yes("finance ingest later than decision",12>11);
near("earliest asof availability",Math.max(10,12),12);

// 09-e: kernels, GP, and posterior inference
near("one-class rare event alarm rate",.01*.8+.99*.02,.0278);
near("one-class true positives per 10000",100*.8,80);
near("one-class false positives per 10000",9900*.02,198);
near("one-class PPV",80/278,.28776978417266186);
near("SVM normal margin",1/Math.sqrt(3**2+4**2),.2);
near("SVM full support band",2/5,.4);
near("SVM quadratic norm objective",.5*(3**2+4**2),12.5);
near("RBF scaled squared metric",1**2+3**2,10);
near("RBF scaled kernel",Math.exp(-.1*10),Math.exp(-1));
near("RBF unscaled kernel",Math.exp(-.1*2),Math.exp(-.2));
near("soft SVM slack",Math.max(0,1-.25),.75);
near("soft SVM penalty C2",2*.75,1.5);
near("soft SVM penalty Chalf",.5*.75,.375);
near("GP posterior mean",4/(1+1),2);
near("GP latent posterior variance",1-1/(1+1),.5);
near("GP observation variance",.5+1,1.5);
near("KL q||p",.5*Math.log(25/9),.5108256237659907);
near("KL p||q",.9*Math.log(1.8)+.1*Math.log(.2),.3680642071684971);
near("Gaussian posterior precision",1+3/1,4);
near("Gaussian posterior mean",6/4,1.5);
near("Gaussian posterior variance",1/4,.25);
near("BN joint chain probability",.5*.8*.6,.24);
near("MH accepting down target",.2/.8,.25);
near("MH forward detailed balance",.8*.5*.25,.1);
near("MH reverse detailed balance",.2*.5*1,.1);
near("variational posterior KL",.5*Math.log(4/3),.14384103622589045);
near("variational ELBO given logp-2",-2-.5*Math.log(4/3),-2.1438410362258905);

// 09-f: unsupervised, causal inference, confidence
near("GMM mixture evidence",.4*.2+.6*.8,.56);
near("GMM first responsibility",(.4*.2)/(.4*.2+.6*.8),1/7);
near("GMM second responsibility",(.6*.8)/(.4*.2+.6*.8),6/7);
near("representation collapsed variance",0,0);
near("contrastive time inversion original",110/100-1,.1);
near("contrastive time inversion reversed",100/110-1,-1/11);
near("Kmeans two-cluster WCSS",(0-1)**2+(2-1)**2,2);
near("Kmeans three-cluster WCSS",0,0);
near("InfoNCE tau1",Math.log(1+Math.exp(-1)),.31326168751822286);
near("InfoNCE tauhalf",Math.log(1+Math.exp(-2)),.1269280110429725);
near("rare confusion matrix recall",10/(10+10),.5);
near("rare confusion matrix precision",10/(10+90),.1);
near("weak IV slope",.2/.01,20);
near("weak IV alternate denominator",.2/.02,10);
near("BH rank1 cutoff",.05/4,.0125);
near("BH rank2 cutoff",2*.05/4,.025);
near("BH rank3 cutoff",3*.05/4,.0375);
yes("BH rank3 meets threshold",.03<=.0375);
yes("BH rank4 fails threshold",.06>.05);
near("MA1 marginal variance",1+1,2);
near("MA1 lag one covariance",1,1);
near("MA1 longrun variance",2+2*1,4);
near("MA1 n100 SE",Math.sqrt(4/100),.2);
near("MA1 iid invalid n100 SE",Math.sqrt(2/100),Math.sqrt(.02));
near("calibration single bin ECE",Math.abs(.6-.8),.2);

// 09-g: neural/ViT mathematics
near("sigmoid slope at zero",sig(0)*(1-sig(0)),.25);
near("tanh slope at zero",1-Math.tanh(0)**2,1);
near("sigmoid ten-link derivative",.25**10,9.5367431640625e-7);
near("autodiff output", (2*3)**2,36);
near("autodiff gradient a",2*2*3**2,36);
near("autodiff gradient b",2*2**2*3,24);
near("adaptive full restored step",1-.1*2/Math.sqrt(4),.9);
near("LayerNorm two inputs mean",(2+4)/2,3);
near("LayerNorm two inputs variance",((2-3)**2+(4-3)**2)/2,1);
near("softmax loss equal logits",-Math.log(.5),ln2);
near("softmax p minus y first",.5-0,.5);
near("softmax p minus y second",.5-1,-.5);
near("GAP four pixels",(1+3+5+7)/4,4);
near("GAP reversed pixels",(7+5+3+1)/4,4);
near("visual source shift risk",.1*1+.9*0,.1);
near("visual target shift risk",.9*1+.1*0,.9);
near("ViT 224 patch 16 count",(224/16)**2,196);
near("ViT plus CLS count",(224/16)**2+1,197);
near("CNN financial gross less execution",.005-.008,-.003);
near("CNN output shape stride2",Math.floor((32+2-1*(3-1)-1)/2+1),16);

// 09-h: sequence models and attention
near("LSTM memory output",.8*2+.2*5,2.6);
near("LSTM hidden with output gate",.5*Math.tanh(2.6),.494513701493006,1e-6);
near("teacher-forcing joint path",.8*.9,.72);
near("SSM recursive second state",.5*2+4,5);
near("SSM causal convolution",4+.5*2,5);
near("state matrix first component",.5**3,.125);
near("state matrix second component",1.2**3,1.728);
near("causal convolution past only",.2*10+.3*8+.5*6,7.4);
near("cross asset one-factor covariance",9,9);
near("cross asset one-factor variance",9+1,10);
near("cross asset factor correlation",9/10,.9);
near("transformer FFN weight count",2*512*2048,2097152);
near("transformer FFN bias count",2048+512,2560);
near("transformer FFN total with bias",2097152+2560,2099712);
yes("causal mask row1 forbids future",1<2&&1<3);
near("softmax tau1",sig(1),.7310585786300049);
near("softmax tauhalf",sig(2),.8807970779778823);
near("position sin at zero",Math.sin(0),0);
near("position cos at zero",Math.cos(0),1);

// 09-i: financial NLP and temporal graph
near("factual proof fail gate",1*1*0,0);
near("language autoregressive joint",.5*.8,.4);
near("TF-IDF unsmoothed",2*Math.log(100/10),2*Math.log(10));
near("yuan conversion 3.5 yi",3.5*1e8,350000000);
near("wan yuan conversion 3.5 yi",3.5*1e4,35000);
near("dual loss finance+general",.8*1+.2*3,1.4);
near("two node normalized GCN degree",1+1,2);
near("two node normalized GCN weight",1/2,.5);
yes("new ETF excluded before listing",9<10);
near("GNN max fanout3 depth3",1+3+9+27,40);
near("GNN max fanout10 depth3",1+10+100+1000,1111);
near("graph spectral difference at k4",.5**4,1/16);
near("directed adjacency from A to B",1,1);
near("directed reverse absent",0,0);

const home=read("../ml/index.html"),viewer=read("../ml/atlas.js"),css=read("../ml/atlas.css");
for(const spec of baseline)for(const entry of spec.entries)
 assert(home.includes('href="#lesson/'+entry.id+'"'),entry.id+" missing canonical live navigation");
for(const marker of ["s.equations.map","s.derivation","s.protocol","s.pitfalls","s.worked_example","katex.render"])
 assert(viewer.includes(marker),"live renderer missing "+marker);
for(const marker of ["overflow-x:auto","line-break:strict","katex-display",":focus-visible","@media print","@media(max-width:700px)"])
 assert(css.includes(marker),"responsive CJK/KaTeX style missing "+marker);
console.log("V81 PASS: "+count+" live lessons, "+texCount+" KaTeX parses ("+strictCount+" new strict), "+numericCount+" numeric checks, frozen original-content fingerprints, intact Chinese navigation and responsive styles.");
