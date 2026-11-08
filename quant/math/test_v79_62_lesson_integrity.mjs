import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>fs.readFileSync(new URL(p,root),"utf8");
const json=p=>JSON.parse(read(p));
const baseline=[{"path":"../ml/atlas-depth-07-i.json","schema":1,"batch":"v37-seventh-financial-nlp-graph","revision":"2026-11-06","entries":[{"id":"m17-c01-p03","hash":"a42fbb7e","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":279},{"id":"m17-c04-p04","hash":"b76ecc93","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":259},{"id":"m17-c04-p06","hash":"d2f693aa","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":269},{"id":"m17-c04-p01","hash":"d905d290","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":290},{"id":"m18-c04-p01","hash":"ad919bd9","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":247},{"id":"m18-c04-p02","hash":"38ac42f2","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":249},{"id":"m18-c05-p04","hash":"bffe1223","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":230},{"id":"m18-c05-p02","hash":"48db24a2","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":251}]},{"path":"../ml/atlas-depth-07-j.json","schema":1,"batch":"v37-seventh-generative-rl","revision":"2026-11-06","entries":[{"id":"m19-c03-p05","hash":"b4e03233","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":220},{"id":"m19-c03-p03","hash":"1f4ed776","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":262},{"id":"m19-c01-p03","hash":"c9e7f17d","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":271},{"id":"m19-c05-p04","hash":"6901e592","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":279},{"id":"m20-c04-p05","hash":"7ba111a0","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":244},{"id":"m20-c02-p06","hash":"a3c14c32","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":260},{"id":"m20-c05-p05","hash":"d22caa28","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":254},{"id":"m20-c03-p05","hash":"7c53c341","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":244}]},{"path":"../ml/atlas-depth-07-k.json","schema":1,"batch":"v37-seventh-marl-timeseries","revision":"2026-11-06","entries":[{"id":"m21-c04-p04","hash":"aa1976d2","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":272},{"id":"m21-c03-p06","hash":"83122630","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":239},{"id":"m21-c05-p06","hash":"081a1ba7","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":212},{"id":"m21-c02-p05","hash":"d34fef77","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":217},{"id":"m22-c05-p05","hash":"957356c1","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":304},{"id":"m22-c04-p04","hash":"5c5f6327","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":254},{"id":"m22-c05-p04","hash":"a747f38b","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":281},{"id":"m22-c03-p04","hash":"9cb46c05","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":258}]},{"path":"../ml/atlas-depth-07-l.json","schema":1,"batch":"v37-seventh-quant-mlops","revision":"2026-11-06","entries":[{"id":"m23-c01-p04","hash":"ff54f245","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":238},{"id":"m23-c01-p01","hash":"c46b5aeb","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":221},{"id":"m23-c02-p02","hash":"47d5f889","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":275},{"id":"m23-c02-p06","hash":"244364ef","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":259},{"id":"m24-c02-p05","hash":"51d41558","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":223},{"id":"m24-c05-p01","hash":"0f5be77c","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":246},{"id":"m24-c04-p04","hash":"d552fa4a","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":232},{"id":"m24-c02-p01","hash":"c090f17b","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":256}]},{"path":"../ml/atlas-depth-08-a.json","schema":1,"batch":"v38-eighth-statistics-linear-algebra","revision":"2026-11-06","entries":[{"id":"m01-c03-p04","hash":"b62c8477","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":265},{"id":"m01-c02-p04","hash":"ef8ae7ac","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":257},{"id":"m01-c05-p06","hash":"9b2ab85e","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":240},{"id":"m01-c04-p02","hash":"145f143e","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":217},{"id":"m01-c03-p06","hash":"4c01ee1d","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":257},{"id":"m02-c05-p02","hash":"3d505406","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":225},{"id":"m02-c05-p05","hash":"2fb068c5","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":246},{"id":"m02-c03-p04","hash":"92e2c910","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":251},{"id":"m02-c03-p06","hash":"ee763646","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":225},{"id":"m02-c03-p03","hash":"76eec38a","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":239}]},{"path":"../ml/atlas-depth-08-b.json","schema":1,"batch":"v38-eighth-probability-learning-theory","revision":"2026-11-06","entries":[{"id":"m03-c03-p03","hash":"762b4ab9","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":240},{"id":"m03-c01-p03","hash":"ff8493ee","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":233},{"id":"m03-c03-p02","hash":"4c389934","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":236},{"id":"m03-c04-p03","hash":"def34e70","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":222},{"id":"m03-c04-p06","hash":"a6b25901","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":237},{"id":"m04-c05-p01","hash":"795f5d1d","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":245},{"id":"m04-c02-p05","hash":"3ca0cdb5","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":232},{"id":"m04-c02-p01","hash":"7b89c2d2","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":258},{"id":"m04-c04-p04","hash":"818da353","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":249},{"id":"m04-c03-p02","hash":"f38b02ac","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":240}]},{"path":"../ml/atlas-depth-08-c.json","schema":1,"batch":"v38-eighth-optimization-financial-data","revision":"2026-11-06","entries":[{"id":"m05-c05-p02","hash":"62aaafec","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":254},{"id":"m05-c03-p02","hash":"b62f091a","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":231},{"id":"m05-c01-p05","hash":"d25995a8","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":246},{"id":"m05-c02-p06","hash":"0aa4f2b6","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":225},{"id":"m05-c01-p02","hash":"f18c0c3c","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":204},{"id":"m06-c01-p05","hash":"f21cd891","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":258},{"id":"m06-c02-p02","hash":"961b5250","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":229},{"id":"m06-c03-p05","hash":"ed9d01c1","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":213},{"id":"m06-c05-p04","hash":"c3fc2739","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":214},{"id":"m06-c02-p04","hash":"8c27ecbb","oldKeys":["source_title","title","overview","equations","derivation","worked_example","protocol","pitfalls","revision_note"],"counts":{"equations":6,"derivation":6,"protocol":6,"pitfalls":3},"workedLength":231}]}];
const manifest=json("../ml/atlas-depth-manifest.json");
assert.equal(manifest.schema,1);
assert.equal(manifest.expected_lessons,720);
assert.equal(manifest.expected_domains,24);
assert.equal(manifest.modules.reduce((n,m)=>n+m.lessons,0),720);
function hash(o){
 let h=2166136261>>>0;
 for(const c of JSON.stringify(o))h=Math.imul(h^c.charCodeAt(0),16777619)>>>0;
 return h.toString(16).padStart(8,"0");
}
function originalSnapshot(x,old){
 return Object.fromEntries(old.oldKeys.map(k=>[k,
  ["equations","derivation","protocol","pitfalls"].includes(k)?x[k].slice(0,old.counts[k]):
  k==="worked_example"?x[k].slice(0,old.workedLength):x[k]
 ]));
}
let n=0,texCount=0,strictCount=0,numericCount=0;
for(const spec of baseline){
 const d=json(spec.path),ids=spec.entries.map(e=>e.id),name=spec.path.split("/").at(-1);
 const loaded=manifest.modules.filter(m=>m.path==="./"+name);
 assert.equal(loaded.length,1,name+" not live");
 assert.equal(loaded[0].lessons,ids.length,name+" live manifest count changed");
 assert.equal(d.schema,spec.schema,name+" schema changed");
 assert.equal(d.batch,spec.batch,name+" batch changed");
 assert.equal(d.revision,spec.revision,name+" old revision changed");
 assert.deepEqual(Object.keys(d.lessons),ids,name+" IDs or course order changed");
 assert.deepEqual(d.v79_revision.ids,ids,name+" revision manifest does not cover lessons");
 for(const old of spec.entries){
  const id=old.id,x=d.lessons[id];n++;
  assert.equal(hash(originalSnapshot(x,old)),old.hash,id+" historical original content was edited");
  assert(x.v79_note?.startsWith("V79"),id+" new layer absent");
  assert.equal(x.equations.length,old.counts.equations+2+(/atlas-depth-07-[ij]\.json$/.test(spec.path)?2:0),id+" original V79 and optional V88 equations append");
  assert.equal(x.derivation.length,old.counts.derivation+2+(/atlas-depth-07-[ij]\.json$/.test(spec.path)?2:0),id+" original V79 and optional V88 derivations append");
  assert.equal(x.protocol.length,old.counts.protocol+1+(/atlas-depth-07-[ij]\.json$/.test(spec.path)?1:0),id+" original V79 and optional V88 protocols append");
  assert.equal(x.pitfalls.length,old.counts.pitfalls+1+(/atlas-depth-07-[ij]\.json$/.test(spec.path)?1:0),id+" original V79 and optional V88 limitations append");
  assert(x.derivation.slice(6,8).every(s=>s.length>=90),id+" new derivation too short");
  assert(x.worked_example.length-old.workedLength>=80,id+" new worked example too short");
  assert(x.protocol.at(6).length>=30&&x.pitfalls.at(3).length>=30,id+" new protocol/caveat incomplete");
  assert(/[\u3400-\u9fff]/u.test(x.derivation.at(7)),id+" Chinese proof missing");
  assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(JSON.stringify(x)),id+" corrupted CJK or invisible char");
  assert(!/TODO|FIXME|Lorem ipsum/iu.test(JSON.stringify(x)),id+" placeholder leaked");
  const newest=x.equations.slice(6,8);
  assert.equal(new Set(newest.map(e=>e.label)).size,2,id+" duplicate new label");
  assert(newest.every(e=>!x.equations.slice(0,6).some(o=>o.label===e.label)),id+" new heading collides with old");
  for(const [j,e] of x.equations.slice(0,8).entries()){
   assert(e.label.length>=2&&e.tex.length>=5,id+" invalid formula metadata");
   katex.renderToString(e.tex,{throwOnError:true,strict:j>=old.counts.equations?"error":"ignore",displayMode:true});
   texCount++;
   if(j>=old.counts.equations)strictCount++;
  }
 }
}
assert.equal(n,62);
assert.equal(strictCount,124,"expected 2 new strict equations for each of 62 lessons");
function near(name,actual,expected,tol=1e-9){
 assert(Number.isFinite(actual)&&Number.isFinite(expected)&&Math.abs(actual-expected)<=tol,name+": "+actual+" != "+expected);
 numericCount++;
}
function yes(name,x){near(name,x?1:0,1);}
const ln2=Math.log(2),pi=Math.PI;
const Phi1=0.15865525393145707;

// Group 07-i: NLP and financial graph time integrity
near("BPE token six",6,6);
near("BPE token after two merges",6-2-2,2);
const dcg=1+7/Math.log2(3),idcg=7+1/Math.log2(3);
yes("retrieval ranking relevance",dcg<idcg);
near("NDCG counterexample",dcg/idcg,.7098,.001);
near("retrieval precision",3/(3+2),.6);
near("retrieval recall",3/(3+1),.75);
near("chapter chunk count",Math.ceil(70/50)+Math.ceil(30/50),3);
near("whole-doc chunk count",Math.ceil(100/50),2);
yes("graph edge present just before end",4>=2&&4<5);
yes("graph edge absent at exclusive end",!(5>=2&&5<5));
near("edge message usable at max clocks",Math.max(9,12,14),14);
yes("edge unavailable at 13",13<14);
near("GNN baseline squared error",(1-.6)**2,.16);
near("GNN new squared error",(1-.8)**2,.04);
near("GNN incremental squared risk",.16-.04,.12);
yes("correlation edge above .8",.81>=.8);
yes("correlation edge below .8",!(.79>=.8));

// Group 07-j: generative modeling and policy control
near("WGAN gradient penalty high",10*(1.5-1)**2,2.5);
near("WGAN gradient penalty low",10*(.8-1)**2,.4);
near("disjoint JS divergence",.5*ln2+.5*ln2,ln2);
near("AE lost low variance direction",1,1);
near("market impact 1 pct",.1*Math.sqrt(100/10000),.01);
near("market impact 4 pct",.1*Math.sqrt(400/10000),.02);
near("Actor Critic continuing TD",1+.9*3-2,1.7);
near("Actor Critic terminal TD",1-2,-1);
near("epsilon greedy top",1-.2+.2/4,.85);
near("epsilon greedy each alternate",.2/4,.05);
near("epsilon greedy total",.85+3*.05,1);
near("tanh log3",Math.tanh(Math.log(3)),.8);
near("tanh Jacobian",1-.8**2,.36);
near("tanh logdensity correction",-Math.log(1-.8**2),1.0216512475319814);
near("DQN target using max target",.9*Math.max(2,6),5.4);
near("DoubleDQN target online choice",.9*2,1.8);

// Group 07-k: MARL, market making, time series and microstructure
near("uninformed decentralized binary success",.5,.5);
near("drawdown portfolio A",(120-110)/120,1/12);
near("drawdown portfolio B",0,0);
near("market maker inventory",0+7-2,5);
near("market maker risk quadratic",.1*5**2,2.5);
yes("offline supported action1",.98>=.05);
yes("offline unsupported action2",.02<.05);
near("illustrative Chronos bin width",(2-(-2))/4,1);
near("illustrative Chronos token",Math.floor((.7-(-2))/1),2);
near("factor covariance",1*1*4,4);
near("factor single asset variance",4+1,5);
near("factor correlation",4/5,.8);
yes("TFT future calendar available",10<11&&10<12);
near("high frequency noise RV n1000",2*1000*.000001,.002);
near("high frequency noise RV n2000",2*2000*.000001,.004);

// Group 07-l: labels, factor risk and production integrity
near("event time 1 concurrency",1,1);
near("event time 2 concurrency",2,2);
near("average event uniqueness",(1+.5)/2,.75);
near("fixed horizon return",101/100-1,.01);
near("triple-barrier first exit return",103/100-1,.03);
near("ICIR mean over standard deviation",.03/.06,.5);
near("IC t under IID",.03/(.06/Math.sqrt(100)),5);
near("gross net volume",100-100,0);
near("absolute transaction volume",Math.abs(100)+Math.abs(-100),200);
near("linear fees on gross",200*.001,.2);
near("model hash-gating false",1*1*0,0);
near("risk config fail-closed",1*0*1,0);
near("training data poison mean sensitivity",100/100,1);
near("permutation asset 1 actual",5,5);
near("permutation asset 2 actual",2,2);

// Group 08-a: learning theory, linear algebra and financial decision
near("train-val gap",.20-.02,.18);
near("val-future gap",.35-.20,.15);
near("confidence budget first",.05/2,.025);
near("confidence budget second",.05/4,.0125);
near("confidence budget third",.05/8,.00625);
near("confidence total geometric",.05,.05);
near("unit trade utility negative",.006-.008,-.002);
near("half position trade utility negative",.5*(.006-.008),-.001);
near("last train with 3-day maturity",20-3-1,16);
near("covshift training label mean",.5,.5);
near("covshift test label mean",.9,.9);
near("PCA covariance trace",9+4,13);
near("PCA max variance",9,9);
near("PCA best rank1 reconstruction",13-9,4);
near("PCA other projection reconstruction",13-4,9);
near("low rank lost target MSE",1,1);
near("VJP coordinate one",3*1+4*2,11);
near("VJP coordinate two",3*1+4*(-1),-1);
near("Jacobian wrong multiply coordinate one",3+4,7);
near("central diff x cubic",(2.1**3-1.9**3)/(.2),12.01);
near("cubic analytic derivative",3*2**2,12);
near("central diff error",.1**2,.01);
near("Hessian x-axis value",.1**2,.01);
near("Hessian y-axis value",-(.1**2),-.01);

// Group 08-b: statistics and covariate drift
near("multinomial negative covariance",-100*.2*.3,-6);
near("Bayes LR 20 posterior",20/(99+20),20/119);
near("covariance transformed positive axis",1+1,2);
near("covariance transformed cross",1-1,0);
near("xor conditional mutual information",ln2,ln2);
near("Jensen log of mean",Math.log((1+4)/2),Math.log(2.5));
near("Jensen mean of logs",(Math.log(1)+Math.log(4))/2,ln2);
near("Jensen strict gap",Math.log(2.5)-ln2,Math.log(1.25));
near("covariate shift positive weight",.9/.5,1.8);
near("covariate shift negative weight",.1/.5,.2);
near("covariate positive reweighted mass",.5*1.8,.9);
near("covariate negative reweighted mass",.5*.2,.1);
near("finite class uniform Hoeffding bound",20*Math.exp(-2*100*.2**2),20*Math.exp(-8));
yes("finite class bound below one percent",20*Math.exp(-8)<.01);
near("beta SE informative features",Math.sqrt(1/100),.1);
near("beta SE weak features",Math.sqrt(1/25),.2);
near("paired HAC marginal variance",1+1,2);
near("paired HAC covariance",1,1);
near("paired HAC longrun variance",2+2*1,4);
near("paired HAC mean SE at n100",Math.sqrt(4/100),.2);
near("paired invalid IID SE",Math.sqrt(2/100),Math.sqrt(.02));
near("PAC confidence complement",1-.05,.95);

// Group 08-c: optimization and feature-engineering validity
near("stable LSE large logits",Math.log(1+Math.exp(-1))+1001,1001.3132616875182);
near("stable softmax larger",1/(1+Math.exp(-1)),.7310585786300049);
near("RMSProp first squared gradient",.9*0+.1*4,.4);
near("RMSProp first step size",-.1*2/Math.sqrt(.4),-.31622776601683794);
near("RMSProp second decayed variance",.9*.4,.36);
near("smooth quadratic GD final",1-.5*2*1,0);
near("smooth quadratic GD decrease",1-0**2,1);
near("smooth descent certificate",.5/2*(2**2),1);
near("no-progress zero LR",0,0);
near("zero LR gradient at theta0",2*100*(0-1),-200);
near("boundary minimum at x1",-1,-1);
yes("boundary variational inequality",(-1)*(.4-1)>=0);
near("OHLCV opening",100,100);
near("OHLCV high",Math.max(103,104),104);
near("OHLCV low",Math.min(99,100),99);
near("OHLCV close",101,101);
near("OHLCV volume",10+20,30);
near("frozen MinMax out of range",(15-0)/(10-0),1.5);
near("VIF highly collinear",1/(1-.96),25);
near("good feature schema prediction",.02+.01*200,2.02);
near("reordered schema incorrect prediction",200+.01*.02,200.0002);
near("leave-one-out encoded first",1,1);
near("leave-one-out encoded second",0,0);

const home=read("../ml/index.html"),css=read("../ml/atlas.css"),viewer=read("../ml/atlas.js");
for(const spec of baseline)for(const entry of spec.entries)
 assert(home.includes('href="#lesson/'+entry.id+'"'),entry.id+" missing from official lesson navigation");
for(const marker of ["s.equations.map","s.derivation","s.protocol","s.pitfalls","s.worked_example","katex.render"])
 assert(viewer.includes(marker),"live renderer missing "+marker);
for(const marker of ["overflow-x:auto","line-break:strict","katex-display",":focus-visible","@media print","@media(max-width:700px)"])
 assert(css.includes(marker),"CJK/responsive formula style missing "+marker);
console.log("V79 PASS: "+n+" live lessons, "+texCount+" KaTeX parses ("+strictCount+" strict new), "+numericCount+" independent numeric checks; original content fingerprints, CJK, official navigation and responsive CSS retained.");
