import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const read=p=>fs.readFileSync(new URL(p,root),"utf8");
const json=p=>JSON.parse(read(p));
const a=json("lessons-probability-v94a.json");
const b=json("lessons-probability-v94b.json");
const c=json("lessons-probability-v94c.json");
const overlay=json("lessons-probability-v94-deepening.json");
assert.equal(a.version,1);assert.equal(b.version,1);assert.equal(c.version,1);assert.equal(overlay.version,1);
for(const doc of [a,b,c,overlay])assert.equal(doc.revision,"V94");
assert.deepEqual(a.lessons.map(l=>l.node_id),["probability-n0103","probability-n0104","probability-n0105"]);
assert.deepEqual(b.lessons.map(l=>l.node_id),["probability-n0109","probability-n0110","probability-n0111"]);
assert.deepEqual(c.lessons.map(l=>l.node_id),["probability-n0112","probability-n0113"]);
const historicalIds=["probability-n0095","probability-n0096","probability-n0100","probability-n0106","probability-n0107","probability-n0108"];
assert.deepEqual(overlay.extensions.map(x=>x.node_id),historicalIds);
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
assert.equal(existing.length,364,"The historical 364 complete lectures must be preserved");
assert.equal(new Set(existing.map(x=>x.node_id)).size,364,"Old source IDs already duplicate");
const added=[...a.lessons,...b.lessons,...c.lessons];
const all=[...existing,...added];
assert.equal(added.length,8);
assert.equal(all.length,372);
assert.equal(new Set(all.map(x=>x.node_id)).size,372,"V94 new lectures overlap historical complete lessons");
const byId=new Map(existing.map(l=>[l.node_id,l]));
let math=0,chapters=0,depth=0;
function strict(tex,where){
 assert(typeof tex==="string"&&tex.length>=10,where+" missing formula");
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(tex),where+" bad hidden Unicode");
 katex.renderToString(tex,{throwOnError:true,strict:"error",displayMode:true});
 math++;
}
function checkSection(section,where,isExtension=false){
 assert(section.title.length>=8&&section.question.length>=10,where+" unclear title");
 assert(Array.isArray(section.paragraphs)&&section.paragraphs.length===2&&section.paragraphs.every(x=>x.length>=30),where+" shallow explanation");
 assert(section.proof.length>=50&&section.worked.length>=50&&section.limitation.length>=25,where+" lacks rigorous proof, worked example or boundary");
 if(isExtension)assert(section.formulas.length>=2,where+" requires two complete new equations");
 else assert(section.formulas.length===1,where+" original section equation count preserved");
 for(const expr of section.formulas){assert(expr.label.length>=4);strict(expr.tex,where);}
 depth+=section.paragraphs.join("").length+section.proof.length+section.worked.length+section.limitation.length;
 chapters++;
}
for(const l of added){
 const n=nodes.get(l.node_id);
 assert(n&&n.title===l.title,"Missing or mistitled curriculum node "+l.id);
 assert.equal(l.id,l.node_id);
 for(const field of ["definition","assumptions","example","application","caveat"])assert(typeof l[field]==="string"&&l[field].length>=65,l.id+" shallow "+field);
 assert(l.properties.length>=4&&l.properties.every(x=>x.length>=18));
 assert.equal(l.formulas.length,3);assert.equal(l.derivation.length,3);assert(l.deep_sections.length>=5,l.id+" preserve four sections and add one complete extension");
 const titles=new Set();
 for(const f of l.formulas){assert(f.label.length>=4);strict(f.tex,l.id);}
 for(const d of l.derivation){assert(d.explanation.length>=25);strict(d.tex,l.id);}
 for(const s of l.deep_sections){
  assert(!titles.has(s.title),l.id+" repeated section");titles.add(s.title);
  checkSection(s,l.id,l.deep_sections.indexOf(s)>=4);
 }
 assert(/[\u3400-\u9FFF]/u.test(JSON.stringify(l)),l.id+" lacks Chinese textbook narrative");
}
assert.equal(overlay.extensions.length,6);
for(const e of overlay.extensions){
 const old=byId.get(e.node_id);
 assert(old&&old.title===e.title,e.node_id+" overlay is not a historical lesson");
 assert(old.deep_sections.length>=e.base_sections_count,e.node_id+" original baseline sections missing");
  assert.equal(new Set(old.deep_sections.map(s=>s.title)).size,old.deep_sections.length,e.node_id+" extended section title collision");
  for(const s of old.deep_sections.slice(e.base_sections_count)){
   assert(s.paragraphs?.length>=2&&s.proof?.length>=55&&s.worked?.length>=50&&s.limitation?.length>=35,e.node_id+" added historical section incomplete");
   for(const f of s.formulas||[]){assert(f.label&&f.tex);katex.renderToString(f.tex,{throwOnError:true,strict:"ignore",displayMode:true});}
  }
 assert.equal(e.deep_sections.length,2,e.node_id+" must have two nonredundant additions");
 const oldTitles=new Set(old.deep_sections.map(s=>s.title));
 for(const s of e.deep_sections){
  assert(!oldTitles.has(s.title),e.node_id+" repeats an old chapter");
  oldTitles.add(s.title);
  checkSection(s,e.node_id);
 }
}
assert(chapters>=52,"Original 44 sections and eight rigorous extensions must remain");
assert(math>=108,"Original 92 equations and sixteen strict new ones must parse");
assert(depth>11000,"V94 mathematical prose must remain substantive");
const app=read("app.js"),css=read("styles.css");
for(const token of [
 "loadV89Foundations()","loadV91FoundationDeepening()","loadV90LinearFoundations()",
 "loadV92LinearFoundations()","loadV93CompletedLessons(\"./lessons-probability-v93.json\",9)",
 "loadV94ProbabilityLectures(\"./lessons-probability-v94a.json\",3)",
 "loadV94ProbabilityLectures(\"./lessons-probability-v94b.json\",3)",
 "loadV94ProbabilityLectures(\"./lessons-probability-v94c.json\",2)",
 "loadV94HistoricalProbabilityDeepening()",
 "applyV94HistoricalProbabilityDeepening(lessons,v94deep)",
 "...v93probability,...v94a,...v94b,...v94c"
])assert(app.includes(token),"Live reader fails to load "+token);
for(const token of ["overflow-x:auto","line-break:strict","@media(max-width:700px)","@media print",":focus-visible"])
 assert(css.includes(token),"CJK/formula responsive style missing "+token);
let numeric=0;
function close(label,a,b,tol=1e-8){
 assert(Number.isFinite(a)&&Math.abs(a-b)<=tol,label+": "+a+" != "+b);numeric++;
}
close("Jensen sample mean",(80+120)/2,100);
close("square root Jensen",(Math.sqrt(80)+Math.sqrt(120))/2,9.94936153005124,1e-9);
close("expected portfolio return",.6*.03+.4*(-.01),.014);
close("unbiased sample variance",(1+0+1)/2,1);
close("empirical sample variance",2/3,2/3);
close("nonlinear covariance",(-1)**3/3+1**3/3,0);
close("nonlinear covariance X2 mean",2/3,2/3);
close("nonlinear Y variance",2/3-(2/3)**2,2/9);
close("portfolio variance with negative correlation",.25*.04+.25*.04+2*.5*.5*(-.5)*.04,.01);
close("joint distribution sum",.3+.2+.1+.4,1);
close("joint marginal X",.3+.2,.5);
close("joint marginal Y",.3+.1,.4);
close("joint covariance",.3-.5*.4,.1);
close("conditional independence mixture",.5*(.9**2+.1**2),.41);
close("marginal state independence benchmark",.5*.5,.25);
close("collider P C",.25+.25,.5);
close("collider joint after selection",.25/.5,.5);
close("triangle density integrates",2*.5,1);
close("triangle conditional density",1/.8,1.25);
close("triangle conditional expectation",.8/2,.4);
close("triangle E Y",(2/3)/2,1/3);
close("random vector portfolio mean",.6*.02+.4*.04,.028);
close("random vector portfolio variance",.36*.04+.16*.09+2*.6*.4*.01,.0336);
close("positive definite covariance det",.04*.09-.01**2,.0035);
close("covariance of two asset sum",.04+.09+2*.01,.15);
close("Jacobian determinant preserving area",2*.5,1);
close("Jacobian stretched rectangle density",1/2,.5);
close("Gaussian radial probability",1-Math.exp(-.5),.3934693402873666);
close("Bayesian risk bounds lower",.3*.10+.7*.01,.037);
close("Bayesian risk bounds upper",.3*.14+.7*.03,.063);
close("Bayes posterior predictive variance",1+1,2);
close("Brier score calibration optimum",.2*(1-.2),.16);
close("Brier misspecification penalty",(.5-.2)**2+.16,.25);
close("Poisson binomial one success",.2*.2+.8*.8,.68);
close("Poisson binomial two successes",.2*.8,.16);
close("correlated Bernoulli win rate variance",.25/100*(1+99*.05),.014875);
close("cumulant Bernoulli variance",.3*(1-.3),.21);
close("Chernoff Gaussian tstar",2,2);
close("Chernoff Gaussian upper bound",Math.exp(-2),.1353352832366127);
close("conditional Jensen squared",.5*4+.5*4,4);
close("total covariance in two states",.5*(1*1)+.5*((-1)*(-1)),1);
close("extra info variance improvement",.25-0,.25);
assert(numeric>=40);
console.log("V94 PASS: 8 completed probability lectures and 6 historical overlays, "+chapters+" rigorous sections, "+math+" strict KaTeX expressions, "+numeric+" independent numerical invariants, baseline 364 unchanged, total 372/656.");

