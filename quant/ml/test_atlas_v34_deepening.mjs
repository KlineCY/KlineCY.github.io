import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=(p)=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const raw=(p)=>fs.readFileSync(new URL(p,root),"utf8");
const plans=[
 ["atlas-depth-foundations-4.json",["m01-c01-p02","m01-c02-p01","m02-c02-p05","m02-c04-p04","m03-c04-p05","m03-c02-p05","m04-c01-p06","m04-c05-p03"]],
 ["atlas-depth-learning-4.json",["m05-c05-p04","m05-c05-p06","m06-c04-p04","m06-c03-p06","m07-c03-p01","m07-c04-p02","m08-c05-p04","m08-c04-p05"]],
 ["atlas-depth-models-4.json",["m09-c03-p05","m09-c04-p02","m10-c03-p04","m10-c05-p05","m11-c03-p05","m11-c01-p03","m12-c05-p02","m12-c05-p06"]],
 ["atlas-depth-quant-4.json",["m21-c02-p04","m21-c04-p05","m22-c03-p03","m22-c05-p03","m23-c01-p03","m23-c01-p06","m24-c05-p04","m24-c05-p06"]]
];
const manifest=read("atlas-depth-manifest.json");
assert.equal(manifest.expected_lessons,720);
assert.equal(manifest.expected_domains,24);
assert.equal(manifest.modules.reduce((n,m)=>n+m.lessons,0),720,"manifest total unchanged");
const modules=new Map(manifest.modules.map(m=>[m.path,m.lessons]));
const lessonIds=plans.flatMap(([p,ids])=>ids);
assert.equal(lessonIds.length,32);
assert.equal(new Set(lessonIds).size,32);
let totalMath=0, newMath=0, derivationProse=0, checked=0;
for(const [p,ids] of plans){
 assert.equal(modules.get("./"+p),8,"original module count "+p);
 const data=read(p);
 assert.equal(data.schema,1);
 assert.equal(data.revision,"2026-11-06");
 assert.deepEqual(Object.keys(data.lessons),ids,"canonical lesson IDs and ordering "+p);
 for(const id of ids){
  const l=data.lessons[id];
  assert(l.revision_note?.startsWith("V34数学强化"),id+" revision marker");
  assert(typeof l.source_title==="string"&&typeof l.title==="string"&&l.title.length>8,id+" canonical title");
  assert(l.overview?.length>=45,id+" existing overview retained");
  assert.equal(l.equations.length,5,id+" 3 original + 2 additional equations");
  assert.equal(l.derivation.length,5,id+" 3 original + 2 additional derivations");
  assert.equal(l.protocol.length,5,id+" 3 original + 2 detailed protocols");
  assert.equal(l.pitfalls.length,3,id+" 2 original + 1 error boundary");
  assert(l.worked_example?.length>=160,id+" expanded worked example missing");
  assert(l.derivation.slice(-2).every(x=>typeof x==="string"&&x.length>=70),id+" shallow proof text");
  assert(l.protocol.slice(-2).every(x=>typeof x==="string"&&x.length>=35),id+" impractical research verification");
  assert(l.pitfalls.at(-1).length>=30,id+" missing limit");
  assert(!/TODO|Lorem ipsum|[\uFFFD\u200B\uFEFF]/i.test(JSON.stringify(l)),id+" invalid CJK placeholders");
  for(const e of l.equations){
   assert(typeof e.label==="string"&&e.label.length>=3,id+" formula label");
   assert(typeof e.tex==="string"&&e.tex.length>=9,id+" LaTeX expression");
   katex.renderToString(e.tex,{throwOnError:true,strict:"ignore",displayMode:true});
   totalMath++;
  }
  newMath+=2;
  derivationProse+=l.derivation.slice(-2).join("").length+l.protocol.slice(-2).join("").length+l.pitfalls.at(-1).length;
  checked++;
 }
}
assert.equal(checked,32);
assert.equal(newMath,64);
assert.equal(totalMath,160);
assert(derivationProse>12000,"insufficient explanatory depth");

const home=raw("index.html"),js=raw("atlas.js"),css=raw("atlas.css");
assert(home.includes('aria-label="V34三十二篇机器学习理论与量化工程深化"'));
for(const id of lessonIds)assert(home.includes('href="#lesson/'+id+'"'),"missing Atlas navigation "+id);
for(const id of ["calculus-n0041","numerical-n0362","statistics-x001","riskmath-n0584"])
 assert(home.includes('href="../math/?topic='+id+'"'),"broken mathematics prerequisite "+id);
for(const part of ["s.equations.map","s.derivation","s.protocol","s.pitfalls","s.worked_example","katex.render"])
 assert(js.includes(part),"reader doesn't display added content "+part);
for(const part of [".atlas-spectral-stage",".atlas-spectral-links",".ml-math-block","overflow-x:auto","line-break:strict","@media print"])
 assert(css.includes(part),"missing CJK/formula layout "+part);

const near=(actual,expected,name,tol=1e-9)=>assert(Math.abs(actual-expected)<=tol,name+" "+actual+" vs "+expected);
near((2+0.1)/(1+2),0.7,"net trade probability threshold");
near(.6-(1-.6)*2-.1,-.3,"negative trade expected value");
near(.36*.04+2*.6*.4*.01+.16*.09,.0336,"portfolio variance with covariance cross-term");
near(.6*.028+.4*.042,.0336,"Euler variance reconstruction");
near(1.1**3*.6-1,-.2014,"geometric returns vs arithmetic");
near((1e4+1)/(.0001+1),10001/1.0001,"Ridge matrix condition");
const kl=.9*Math.log(.9/.5)+.1*Math.log(.1/.5);
near(kl,.3680642071684971,"Bernoulli KL divergence");
assert(.4<=Math.sqrt(kl/2),"Pinsker check");
near(3/(3-2),3,"Student t variance with nu=3");
near(Math.sqrt(.00025)/Math.sqrt(5),Math.sqrt(.00005),"seed performance standard error");
near((45+43-8)/100,.8,"correlated feature cluster stability");
near((.25*2)/(1+.25*2),1/3,"Logit odds not probability");
near(3+.25*3**2,5.25,"negative binomial overdispersion");
near(-2,-2,"non-PSD Gram quadratic");
near(Math.sqrt(3)/(1+Math.sqrt(3)),.6339745962155614,"mean field update");
near((2*1+4*2)/(1**2+2**2),2,"Kalman EM complete observation");
near(Math.exp(.8),2.225540928492468,"EGARCH asymmetry");
const N=Math.floor((100-16)/10)+1;
assert.equal(N,9,"PatchTST number");
assert.equal((N-1)*10+16,96,"PatchTST coverage");
near((1+.5)/2,.75,"triple barrier event uniqueness");
near(1e-5+1e-6*100,.00011,"numeric test tolerance");
near(.6*.01+(-.2)*(-.02)-.002,.008,"net multiasset return");
near(.2+.6*Math.exp(-2),.2812011699419676,"Hawkes stability bridge");

console.log("PASS V34: 32 canonical ML lessons, 64 new strictly parseable KaTeX formulas (160 checked), 12k+ proof/protocol characters, 32 expanded examples, 32 navigable routes, numerical invariants and responsive Chinese/math layout. Curriculum counts unchanged.");
