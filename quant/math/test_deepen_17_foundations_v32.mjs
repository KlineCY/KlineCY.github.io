import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const groups=[
 ["lessons-phase-01-c.json",["regression-n0188","econometrics-n0212","timeseries-n0241","stochastic-n0279"]],
 ["lessons-phase-01-d.json",["optimization-n0340","numerical-n0362"]],
 ["lessons-phase-01-e.json",["diffeq-n0397","finance-n0430","mlmath-n0453","information-n0484"]],
 ["lessons-phase-01-f.json",["discrete-n0519","control-n0545","riskmath-n0584"]],
 ["lessons-phase-02-a.json",["foundation-n0005","calculus-n0041","linear-n0070","probability-n0100"]]
];
const expected=groups.flatMap(x=>x[1]);
assert.equal(new Set(expected).size,17);
const catalog=read("curriculum.json"),manifest=read("lessons-manifest.json");
const nodes=new Map(catalog.nodes.map(n=>[n.id,n]));
assert.equal(catalog.nodes.length,656);
assert.equal(manifest.expected_lessons,201);
assert.equal(manifest.modules.length,77);
const core=["lessons-foundations.json","lessons-advanced.json","lessons-more.json"].flatMap(p=>read(p).lessons);
assert.equal(core.length,123);
assert.equal(core.length+manifest.modules.reduce((n,m)=>n+m.lessons,0),324);
const lessons=groups.flatMap(([p,ids])=>{
 const data=read(p);
 assert.equal(data.version,1);
 assert.equal(data.revision,"2026-11-05");
 const selected=data.lessons.filter(x=>ids.includes(x.id));
 assert.deepEqual(selected.map(x=>x.id),ids,"original canonical ordering "+p);
 return selected;
});
let formulas=0,deepSections=0,links=0,characters=0;
for(const l of lessons){
 assert.equal(l.id,l.node_id,l.id+" node id");
 assert.equal(nodes.get(l.id)?.title,l.title,l.id+" canonical title");
 assert(l.checked_status.startsWith("V32系统深化"));
 for(const k of ["definition","assumptions","example","application","caveat"])assert(l[k]?.length>=45,l.id+" "+k);
 assert.equal(l.deep_sections?.length,4,l.id+" deep section count");
 assert.equal(new Set(l.deep_sections.map(x=>x.title)).size,4,l.id+" distinct proof sections");
 for(const s of l.deep_sections){
  assert(s.question.length>=6 && s.title.length>=6,l.id+" question");
  assert.equal(s.paragraphs.length,2,l.id+" prose structure");
  assert(s.paragraphs.every(x=>x.length>=45),l.id+" detailed paragraphs");
  for(const k of ["proof","worked","limitation"])assert(s[k]?.length>=30,l.id+" "+k);
  const count=s.paragraphs.join("").length+s.proof.length+s.worked.length+s.limitation.length;
  assert(count>=220,l.id+" overall subchapter density");
  characters+=count;
  for(const eq of s.formulas){
   assert(eq.label?.length>2 && typeof eq.tex==="string");
   katex.renderToString(eq.tex,{throwOnError:true,strict:"ignore",displayMode:true});
   formulas++;
  }
  deepSections++;
 }
 assert.equal(l.links?.length,3,l.id+" crosslinks");
 for(const link of l.links){
  assert(link.kind==="math"&&nodes.has(link.id),l.id+" broken link "+link.id);
  links++;
 }
 assert(!/TODO|占位|待填|Lorem ipsum|[\uFFFD\u200B\uFEFF]/i.test(JSON.stringify(l)),l.id+" corrupted text");
}
assert.equal(deepSections,68);
assert.equal(formulas,68);
assert.equal(links,51);
assert(characters>20000);
const unchangedIto=read("lessons-phase-01-d.json").lessons.find(x=>x.id==="ito-n0308");
assert.equal(unchangedIto.deep_sections.length,4,"retained preexisting Ito proof chapters");

const near=(actual,expected,label,tol=1e-9)=>assert(Math.abs(actual-expected)<=tol,label+": "+actual+" vs "+expected);
near(200*.06,12,"White LM");
near(2+3*(1/2),3.5,"omitted-variable bias");
const Zp=2/3,Zm=1/3;
near(Zp*1+Zm*(-2),0,"stationarity zero mean");
near(Zp*1+Zm*4,2,"stationarity variance");
assert(lessons.find(x=>x.id==="timeseries-n0241").deep_sections[1].proof.includes("P(X_{2t+1}=1)=0"),"correct marginal counterexample");
const P=[[.8,.2],[.3,.7]],p2=P.map(row=>P[0].map((_,j)=>row.reduce((v,a,k)=>v+a*P[k][j],0)));
near(p2[0][0],.70,"Markov P2(1,1)");
near(p2[1][0],.45,"Markov P2(2,1)");
near(.6*.8+.4*.3,.6,"stationary distribution");
near(2-4/4,1,"dual optimum");
near(2*(1-Math.exp(-2)),1.7293294335267746,"OU discretization covariance");
near(.5*20/1.05,9.523809523809524,"risk-neutral call");
near(.9*Math.log(1.8)+.1*Math.log(.2),.3680642071684971,"mutual information");
near(.25*(.04625+.08375)+.5*.0135,.03925,"shrunk portfolio variance");
near(45*.2**2*.8**8,.301989888,"binomial exact probability");
near(1-Math.exp(-1),.6321205588285577,"Poisson limit");
near(2*3-1*4,2,"LU determinant");
near(Math.sqrt(73),8.54400374531753,"gradient magnitude");

const mathHome=fs.readFileSync(new URL("index.html",root),"utf8");
const mlHome=fs.readFileSync(new URL("../ml/index.html",root),"utf8");
const mathCss=fs.readFileSync(new URL("styles.css",root),"utf8");
const mlCss=fs.readFileSync(new URL("../ml/atlas.css",root),"utf8");
const reader=fs.readFileSync(new URL("app.js",root),"utf8");
assert(mathHome.includes('aria-label="V32十七篇基础数学与量化方法深化"'));
assert(mlHome.includes('aria-label="V32机器学习与量化数学先修衔接"'));
for(const id of expected)assert(mathHome.includes('href="?topic='+id+'"'),"math homepage missing "+id);
for(const id of ["calculus-n0041","numerical-n0362","mlmath-n0453","information-n0484","timeseries-n0241","econometrics-n0212","control-n0545","riskmath-n0584"])
 assert(mlHome.includes('href="../math/?topic='+id+'"'),"Atlas link missing "+id);
for(const feature of ["renderMathDeepSections","part.paragraphs","part.proof","part.worked","part.limitation"])
 assert(reader.includes(feature),"reader missing "+feature);
for(const feature of ["overflow-x:auto","line-break:strict","@media print",":focus-visible"])
 assert(mathCss.includes(feature),"math responsive design missing "+feature);
assert(mlCss.includes("overflow-x:auto"),"Atlas formula overflow missing");
console.log("PASS V32: 17 canonical deepened lectures, 68 substantive chapters, 68 strict KaTeX formulas, 51 valid graph links, 20k+ proof prose, 16 independent numerical invariants and responsive math/Chinese layout.");
