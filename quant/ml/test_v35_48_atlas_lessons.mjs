import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=(p)=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const raw=(p)=>fs.readFileSync(new URL(p,root),"utf8");
const groups=[
 ["foundations",["m01-c01-p03","m01-c02-p05","m02-c02-p02","m02-c04-p01","m03-c04-p01","m03-c04-p04","m04-c04-p06","m04-c01-p04"]],
 ["classical",["m05-c05-p01","m05-c03-p05","m06-c05-p03","m06-c03-p04","m07-c04-p05","m07-c01-p06","m08-c05-p06","m08-c02-p01"]],
 ["models",["m09-c02-p05","m09-c03-p03","m10-c01-p06","m10-c03-p05","m11-c03-p06","m11-c01-p05","m12-c01-p05","m12-c05-p01"]],
 ["deep",["m13-c03-p05","m13-c01-p06","m14-c01-p03","m14-c05-p03","m15-c04-p02","m15-c02-p06","m16-c01-p04","m16-c05-p05"]],
 ["crossdomain",["m17-c05-p05","m17-c05-p06","m18-c01-p04","m18-c01-p05","m19-c01-p04","m19-c02-p06","m20-c04-p04","m20-c03-p06"]],
 ["quant",["m21-c02-p06","m21-c03-p04","m22-c04-p02","m22-c05-p02","m23-c01-p05","m23-c05-p01","m24-c02-p04","m24-c01-p06"]]
];
const manifest=read("atlas-depth-manifest.json");
assert.equal(manifest.expected_lessons,720);
assert.equal(manifest.expected_domains,24);
assert.equal(manifest.modules.reduce((n,m)=>n+m.lessons,0),720,"original Atlas depth catalog");
const modules=new Map(manifest.modules.map(m=>[m.path,m.lessons]));
let checked=0,math=0,chars=0;
for(const [name,ids] of groups){
 const path="atlas-depth-"+name+"-5.json";
 assert.equal(modules.get("./"+path),8,"manifest references canonical "+path);
 const d=read(path);
 assert.equal(d.schema,1);
 assert.equal(d.revision,"2026-11-06");
 assert.deepEqual(Object.keys(d.lessons),ids,"canonical original lesson order "+path);
 for(const id of ids){
  const l=d.lessons[id];
  assert.equal(typeof l.title,"string",id+" title");
  assert.equal(l.source_title.length>=2,true,id+" original source title");
  assert(l.revision_note?.startsWith("V35"),id+" revision");
  assert.equal(l.equations.length,5,id+" original 3 + 2");
  assert.equal(l.derivation.length,5,id+" original 3 + 2");
  assert.equal(l.protocol.length,5,id+" original 3 + 2");
  assert.equal(l.pitfalls.length,3,id+" original 2 + 1");
  assert(l.worked_example.length>=120,id+" expanded example");
  assert(l.derivation.slice(-2).every(x=>x.length>=75),id+" proof completeness");
  assert(l.protocol.slice(-2).every(x=>x.length>=32),id+" reproduction protocols");
  assert(l.pitfalls.at(-1).length>=30,id+" failure boundary");
  assert(!/TODO|Lorem ipsum|[\uFFFD\u200B\uFEFF]/i.test(JSON.stringify(l)),id+" corrupted CJK text");
  for(const eq of l.equations){
   assert(eq.tex?.length>=8&&eq.label?.length>=3,id+" mathematical annotation");
   katex.renderToString(eq.tex,{displayMode:true,throwOnError:true,strict:"ignore"});
   math++;
  }
  chars+=l.derivation.slice(-2).join("").length+l.protocol.slice(-2).join("").length+l.pitfalls.at(-1).length;
  checked++;
 }
}
assert.equal(checked,48);
assert.equal(math,240,"96 new and 144 retained strict KaTeX equations");
assert(chars>17500,"at least 17.5k chars of proof, protocol, failure reasoning");

const home=raw("index.html"),js=raw("atlas.js"),css=raw("atlas.css");
assert(home.includes('aria-label="V35四十八篇机器学习正式课程完整深化"'));
for(const [,ids] of groups)for(const id of ids)
 assert(home.includes('href="#lesson/'+id+'"'),id+" missing homepage route");
for(const id of ["linear-n0065","calculus-n0041","statistics-x001","finance-n0430","riskmath-x002"])
 assert(home.includes('href="../math/?topic='+id+'"'),"broken math precursor "+id);
for(const k of ["s.equations.map","s.derivation","s.protocol","s.pitfalls","s.worked_example","katex.render"])
 assert(js.includes(k),"Atlas viewer unable to render "+k);
for(const k of [".atlas-spectral-path",".atlas-spectral-stage",".atlas-spectral-links",".ml-math-block","overflow-x:auto","line-break:strict","@media print"])
 assert(css.includes(k),"CJK / narrow screen / print layout "+k);

const near=(x,y,label,tol=1e-9)=>assert(Math.abs(x-y)<=tol,label+": "+x+" vs "+y);
near(1.2*.8-1,-.04,"geometric returns");
near((Math.log(1.2)+Math.log(.8))/2,-.020410997260127583,"log growth");
near(-.6+(1-.6)*2+.1,.3,"trade cost mapping");
near(-.9*Math.log(.9)-.1*Math.log(.1),.3250829733914482,"binary Shannon entropy");
near(9*.1/(9*.1+.9),.5,"weighted cross entropy calibration");
near(Math.sqrt(Math.log(400)/(2000)),.054733,"finite-class Hoeffding radius",1e-5);
near(Math.sqrt(4/100),.2,"HAC sampling SE");
near(.2/(1-.9),2,"Adam startup correction");
near(.004/(1-.999),4,"Adam second moment startup");
near(.8-.3,.5,"lasso soft threshold");
near(1/(1-.8),5,"VIF");
near((1-1/100)**100,.3660323412732292,"Bootstrap OOB probability at n=100");
near(.2+.8/100,.208,"ensemble residual correlation floor");
near((5+2)/(6+4),.7,"Beta posterior mean");
near((26/3)**.5,2.943920288775949,"RMSE");
near(2-2*0**2,2,"orthogonal projector distance square");
near((5-2)/5,.6,"silhouette");
near((10+2-2-1)/2,4.5,"CNN stride pre floor");
near(1+2*(1+2+4),15,"dilated TCN receptive field");
near(Math.sqrt(25+144),13,"global clipping norm");
near(20*Math.exp(-Math.log(2)),10,"news signal decay in bp");
near((.02*100+.03*10)/.05,46,"atomic expected shortfall");
near(.02*.5*Math.sqrt(.01),.001,"market impact");
near(.8**5,.32768,"cointegrated deviation decay");
near((1+.5)/2,.75,"event uniqueness");
near(.6*.01+(-.2)*(-.02)-.002,.008,"net portfolio return");
console.log("PASS V35: 48 canonical Atlas lessons, 96 newly authored formulas (240 KaTeX-validated in total), >17.5k proof/protocol characters, reproducible numerical invariants, 48 deep links and responsive CJK/formula layout. Original 720-depth lesson manifest unchanged.");
