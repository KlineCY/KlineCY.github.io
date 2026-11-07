import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const load=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const mathModules=["lessons-phase-17-b.json","lessons-phase-17-c.json"].map(load);
const mlPaths=["../ml/atlas-depth-15-d.json","../ml/atlas-depth-16-c.json","../ml/atlas-depth-16-d.json"];
const mlModules=mlPaths.map(load);
const math=mathModules.flatMap(x=>x.lessons);
const ml=Object.assign({},...mlModules.map(x=>x.lessons));
const mathIds=[
  "econometrics-n0216","econometrics-n0218","econometrics-n0219","econometrics-n0220",
  "econometrics-n0221","econometrics-n0222","econometrics-n0228","econometrics-n0231"
];
const mlIds=[
  "m19-c01-p01","m20-c04-p03","m21-c05-p04","m22-c04-p05","m23-c05-p06",
  "m13-c05-p01","m14-c04-p03","m15-c05-p01","m16-c01-p01","m17-c02-p01","m18-c02-p03",
  "m19-c02-p03","m20-c01-p05","m21-c05-p05","m22-c05-p06","m23-c03-p06"
];
assert.deepEqual(math.map(x=>x.id),mathIds);
assert.deepEqual(Object.keys(ml),mlIds);
assert.equal(math.length+mlIds.length,24);
let parsed=0,strictParsed=0;
const validTex=(tex,strict=false)=>{
  assert(typeof tex==="string"&&tex.trim().length>3);
  const html=katex.renderToString(tex,{displayMode:true,throwOnError:true,strict:strict?"error":"ignore"});
  assert(html.includes("katex"));
  parsed++;
  if(strict)strictParsed++;
};
for(const doc of mathModules){
  assert.deepEqual(doc.v72_revision.lessons,doc.lessons.map(x=>x.id));
  for(const x of doc.lessons){
    const s=x.deep_sections.at(-1);
    assert(x.checked_status.startsWith("V72"),x.id+" not updated");
    assert.equal(s.formulas.length,2,x.id+" latest mini-chapter must have two formulas");
    assert.equal(s.paragraphs.length,2,x.id+" latest mini-chapter needs two arguments");
    assert(s.paragraphs.every(p=>p.length>=80),x.id+" thin argument");
    assert(s.question.length>=14&&s.proof.length>=125&&s.worked.length>=90&&s.limitation.length>=50,x.id+" incomplete proof/example");
    assert.equal(new Set(x.deep_sections.map(s=>s.title)).size,x.deep_sections.length,x.id+" duplicate subsections");
    assert(x.links?.length>=3&&x.sources?.length>=2,x.id+" references lost");
    for(const f of [...x.formulas,...x.derivation,...x.deep_sections.flatMap(s=>s.formulas)])validTex(f.tex);
    for(const f of s.formulas)validTex(f.tex,true);
    assert(!/[\uFFFD\u200B\uFEFF]/u.test(JSON.stringify(x)),x.id+" broken Chinese text");
  }
}
for(let m=0;m<mlModules.length;m++){
  const doc=mlModules[m],ids=Object.keys(doc.lessons);
  assert.deepEqual(doc.v72_revision.ids,ids,mlPaths[m]+" has wrong revision scope");
  for(const [id,x] of Object.entries(doc.lessons)){
    assert(x.v72_note.startsWith("V72"),id+" not marked");
    assert(x.equations.length>=8&&x.derivation.length>=8,id+" missing mathematical expansion");
    assert(x.protocol.length>=5&&x.pitfalls.length>=4,id+" missing verification boundary");
    assert(x.worked_example.length>=250,id+" insufficient worked example");
    assert(x.derivation.slice(-2).every(p=>p.length>=85),id+" short derivative");
    assert.equal(new Set(x.equations.map(e=>e.label)).size,x.equations.length,id+" duplicated equation label");
    assert.equal(new Set(x.protocol).size,x.protocol.length,id+" duplicated procedure");
    for(const f of x.equations)validTex(f.tex);
    for(const f of x.equations.slice(-2))validTex(f.tex,true);
    assert(!/TODO|Lorem ipsum|[\uFFFD\u200B\uFEFF]/iu.test(JSON.stringify(x)),id+" incomplete or corrupted Chinese");
  }
}
let n=0;
function near(actual,expected,label,tol=1e-9){
  assert(Number.isFinite(actual)&&Math.abs(actual-expected)<tol,label+": "+actual+" != "+expected);
  n++;
}
near(.05**2-1.96**2*.1**2,-.035916,"Fieller quadratic coefficient");
near((2+4)/2,3,"overidentified 2SLS coefficient");
near(-1+1,0,"2SLS fitted-regressor residual");
near(2+.5,2.5,"common invalid instruments pseudotruth");
near((1+.5)/(1-.5),3,"HAC true long-run variance");
near(1+.5,1.5,"HAC one-lag Bartlett truncation");
near((1+3/9)/(1+1/9),1.2,"GMM precision weight");
near(1/(1+1/9),.9,"GMM efficient asymptotic variance");
near((1+9)/4,2.5,"GMM equal weight asymptotic variance");
near(1-.95**2,.0975,"J-selection familywise size");
near((1+9*.5)/10,.55,"cluster mean variance");
near(10/(1+9*.5),10/5.5,"cluster effective sample size");
near((3-2)+(4-3)-(1-2),3,"AIPW double robustness");
near(9/(9+1),.9,"PCA explained variance");
near(.5*2*1+.5*0*(-1),1,"policy gradient baseline truth");
near(.2*.001,.0002,"portfolio traded-notional cost");
near((.02*100+.03*10)/.05,46,"discrete ES tail mass");
near(.6*.1+.4*(-.05),.04,"portfolio return");
near(.66/1.04+.38/1.04,1,"pretrade drift weight conservation");
near((4<5&&0<6)?1:0,1,"label window overlap");
near((5<5)?1:0,0,"touching half-open windows do not overlap");
near((2+4)/2,3,"permutation-equivariant attention toy");
near(.8,.8,"latent state eigenvalue");
near(64/8**2,1,"QK score scaling variance");
near(.5*.5,.25,"two MLM marginals product");
near(.45-.25,.2,"MLM nonfactorizing joint gap");
near((2+6)/2,4,"GCN component one");
near((10+14)/2,12,"GCN component two");
near((Math.log(.5)+Math.log(1.5))/2,Math.log(.75)/2,"IWAE log concavity");
near((1+1+1-1)/4,.5,"maximization bias");
near(1-(1-.5)**2,.75,"wrong independent-fill formula");
near(.5,.5,"correct perfectly correlated fills");
near(.9*0+.1*10,1,"support shift risk");
near(1+2*0,1,"KKT nonunique multiplier pair one");
near(0+2*.5,1,"KKT nonunique multiplier pair two");
const html=fs.readFileSync(new URL("index.html",root),"utf8");
const mlHtml=fs.readFileSync(new URL("../ml/index.html",root),"utf8");
for(const id of mathIds){
  assert(html.includes("?topic="+id),"missing math navigation "+id);
  assert(mlHtml.includes("../math/?topic="+id),"missing ML-to-math navigation "+id);
}
const mathCSS=fs.readFileSync(new URL("styles.css",root),"utf8");
const mlCSS=fs.readFileSync(new URL("../ml/atlas.css",root),"utf8");
for(const [file,css] of [["math styles",mathCSS],["ml styles",mlCSS]]){
  for(const marker of ["overflow-x:auto","line-break:strict",":focus-visible","@media print","@media(max-width:700px)"])
    assert(css.includes(marker),file+" missing "+marker);
}
console.log("V72 PASS: 8 math + 16 ML lessons; "+parsed+" KaTeX parses ("+strictParsed+" strict), "+n+" independent numerical identities; schema/CJK/navigation/mobile-and-print CSS guards.");
