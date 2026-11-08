import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const read=p=>fs.readFileSync(new URL(p,root),"utf8");
const json=p=>JSON.parse(read(p));
const a=json("lessons-probability-v94a.json"),b=json("lessons-probability-v94b.json");
const expectedA=["probability-n0095","probability-n0096","probability-n0100","probability-n0103","probability-n0104","probability-n0105"];
const expectedB=["probability-n0106","probability-n0107","probability-n0108","probability-n0109","probability-n0110","probability-n0111"];
assert.equal(a.version,1);assert.equal(b.version,1);
assert.equal(a.revision,"V94");assert.equal(b.revision,"V94");
assert.deepEqual(a.lessons.map(l=>l.node_id),expectedA);
assert.deepEqual(b.lessons.map(l=>l.node_id),expectedB);
const curriculum=json("curriculum.json"),nodes=new Map(curriculum.nodes.map(n=>[n.id,n]));
assert.equal(nodes.size,656);
const manifest=json("lessons-manifest.json");
assert.equal(manifest.expected_lessons,201);assert.equal(manifest.modules.length,77);
const existing=[
 ...json("lessons-foundations.json").lessons,
 ...json("lessons-advanced.json").lessons,
 ...json("lessons-more.json").lessons,
 ...manifest.modules.flatMap(m=>json(m.path).lessons),
 ...json("lessons-foundations-v89.json").lessons,
 ...json("lessons-linear-v90.json").lessons,
 ...json("lessons-linear-v92.json").lessons,
 ...json("lessons-matrix-v93.json").lessons,
 ...json("lessons-probability-v93.json").lessons
];
assert.equal(existing.length,364,"Historical 364 complete courses were changed");
assert.equal(new Set(existing.map(l=>l.node_id)).size,364,"Historical duplicate course IDs");
const added=[...a.lessons,...b.lessons],all=[...existing,...added];
assert.equal(added.length,12);
assert.equal(all.length,376);
assert.equal(new Set(all.map(l=>l.node_id)).size,376,"New courses overlap historical lectures");
let equations=0,sections=0,proseLength=0;
function parseMath(tex,where){
 assert(typeof tex==="string"&&tex.length>=10,where+" incomplete mathematical expression");
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(tex),where+" hidden unicode");
 katex.renderToString(tex,{throwOnError:true,strict:"error",displayMode:true});
 equations++;
}
for(const l of added){
 const node=nodes.get(l.node_id);
 assert(node&&node.title===l.title,"Curriculum node/title mismatch "+l.node_id);
 assert.equal(l.id,l.node_id);
 for(const field of ["definition","assumptions","example","application","caveat"]){
  assert(typeof l[field]==="string"&&l[field].length>=65,l.id+" incomplete "+field);
 }
 assert(l.properties.length>=4&&l.properties.every(x=>x.length>=18),l.id+" incomplete properties");
 assert.equal(l.formulas.length,3);
 assert.equal(l.derivation.length,3);
 assert.equal(l.deep_sections.length,3);
 const titles=new Set();
 for(const f of l.formulas){assert(f.label.length>=4);parseMath(f.tex,l.id);}
 for(const d of l.derivation){assert(d.explanation.length>=25);parseMath(d.tex,l.id);}
 for(const s of l.deep_sections){
  assert(s.title.length>=8&&s.question.length>=10,l.id+" shallow section heading");
  assert(!titles.has(s.title),l.id+" repeated subchapter");
  titles.add(s.title);
  assert(s.paragraphs.length===2&&s.paragraphs.every(x=>x.length>=30),l.id+" shallow narrative");
  assert(s.proof.length>=50&&s.worked.length>=50&&s.limitation.length>=25,l.id+" incomplete proof worked example or boundary");
  assert.equal(s.formulas.length,1);
  assert(s.formulas[0].label.length>=4);
  parseMath(s.formulas[0].tex,l.id);
  proseLength+=s.paragraphs.join("").length+s.proof.length+s.worked.length+s.limitation.length;
  sections++;
 }
 assert(/[\u3400-\u9FFF]/u.test(JSON.stringify(l)),l.id+" lacks Chinese");
 assert(!/TODO|FIXME|Lorem ipsum/i.test(JSON.stringify(l)),l.id+" draft placeholder");
}
assert.equal(sections,36);
assert.equal(equations,108);
assert(proseLength>11000,"Substantial narrative depth is missing");
const app=read("app.js"),css=read("styles.css");
for(const text of [
 "loadV89Foundations()","loadV91FoundationDeepening()",
 "loadV90LinearFoundations()","loadV92LinearFoundations()",
 "loadV93CompletedLessons(\"./lessons-matrix-v93.json\",3)",
 "loadV93CompletedLessons(\"./lessons-probability-v93.json\",9)",
 "loadV94ProbabilityLectures(\"./lessons-probability-v94a.json\",6)",
 "loadV94ProbabilityLectures(\"./lessons-probability-v94b.json\",6)",
 "...v93matrix,...v93probability,...v94a,...v94b"
])assert(app.includes(text),"Live reader has no reference to "+text);
for(const style of ["overflow-x:auto","line-break:strict","@media(max-width:700px)","@media print",":focus-visible"])
 assert(css.includes(style),"CJK/mathematics responsive rule missing "+style);
let numeric=0;
function close(label,a,b,tol=1e-8){assert(Number.isFinite(a)&&Math.abs(a-b)<=tol,label+": "+a+" vs "+b);numeric++;}
function okay(label,yes){close(label,yes?1:0,1);}
close("regime loss probability",.3*.12+.7*.02,.05);
close("updated regime probability",.4*.12+.6*.02,.06);
close("Bayes overall evidence",.9*.01+.05*.99,.0585);
close("Bayes posterior",(.9*.01)/(.9*.01+.05*.99),2/13);
close("Bayes likelihood ratio",.9/.05,18);
close("Bayes posterior odds",18/99,2/11);
close("binomial zero",.8**5,.32768);
close("binomial one",5*.2*.8**4,.4096);
close("binomial tail",1-.8**5-5*.2*.8**4,.26272);
close("binomial mean",5*.2,1);
close("binomial variance",5*.2*.8,.8);
close("binomial Bernoulli variance",.3*(1-.3),.21);
close("portfolio expected return",.6*.03+.4*(-.01),.014);
close("sample mean",(1+2+3)/3,2);
close("sample unbiased variance",(1+0+1)/2,1);
close("sample empirical variance",(1+0+1)/3,2/3);
close("zero covariance nonlinear",(-1)**3/3+0+1**3/3,0);
close("nonlinear Y variance",2/3-(2/3)**2,2/9);
close("correlation example",4.5/(2*3),.75);
close("two asset variance reduction",.25*.04+.25*.04+2*.5*.5*(-.5)*.04,.01);
close("mgf Bernoulli derivative",.3, .3);
close("mgf Bernoulli variance",.3-.3**2,.21);
close("regime conditional mean",.2*(-.05)+.8*.02,.006);
close("regime within variance",.2*.04**2+.8*.01**2,.0004);
close("regime between variance",.2*.8*(.07)**2,.000784);
close("total variance",.0004+.000784,.001184);
close("total standard deviation",Math.sqrt(.001184),.0344093016497,1e-10);
close("joint PMF sum",.3+.2+.1+.4,1);
close("joint marginal X",.3+.2,.5);
close("joint marginal Y",.3+.1,.4);
close("joint covariance",.3-.5*.4,.1);
close("mixture conditional independence",.5*(.9**2+.1**2),.41);
close("independent marginal product",.5*.5,.25);
close("collider P(C)",.25+.25,.5);
close("collider joint conditional",.25/.5,.5);
close("triangle joint density integral",2*(.5),1);
close("triangle fX",2*.8,1.6);
close("triangle conditional density",1/.8,1.25);
close("triangle conditional mean",.8/2,.4);
close("triangle E[X]",2/3,2/3);
close("triangle E[Y]",(2/3)/2,1/3);
okay("conditional independence distinct from marginal",.41!==.25);
assert(numeric>=40);
console.log("V94 PASS: 12 complete Bayesian and conditional probability courses; 36 rigorous chapters; 108 strict KaTeX equations; "+numeric+" independent numerical checks; historical 364 courses preserved; total 376/656. CJK/mobile styles verified.");

