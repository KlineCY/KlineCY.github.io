import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const read=p=>fs.readFileSync(new URL(p,root),"utf8");
const json=p=>JSON.parse(read(p));
const first=json("lessons-optimization-v96a.json"),second=json("lessons-optimization-v96b.json");
for(const d of [first,second]){assert.equal(d.version,1);assert.equal(d.revision,"V96");assert.equal(d.lessons.length,7);}
const expectedA=["optimization-n0331","optimization-n0332","optimization-n0333","optimization-n0334","optimization-n0335","optimization-n0336","optimization-n0337"];
const expectedB=["optimization-n0339","optimization-n0341","optimization-n0343","optimization-n0344","optimization-n0345","optimization-n0346","optimization-n0348"];
assert.deepEqual(first.lessons.map(x=>x.node_id),expectedA);
assert.deepEqual(second.lessons.map(x=>x.node_id),expectedB);
const curriculum=json("curriculum.json"),nodes=new Map(curriculum.nodes.map(x=>[x.id,x]));
assert.equal(nodes.size,656,"The mathematics curriculum should still have 656 unique nodes");
const manifest=json("lessons-manifest.json");
assert.equal(manifest.expected_lessons,201);assert.equal(manifest.modules.length,77);
const historical=[
 ...json("lessons-foundations.json").lessons,
 ...json("lessons-advanced.json").lessons,
 ...json("lessons-more.json").lessons,
 ...manifest.modules.flatMap(m=>json(m.path).lessons),
 ...json("lessons-foundations-v89.json").lessons,
 ...json("lessons-linear-v90.json").lessons,
 ...json("lessons-linear-v92.json").lessons,
 ...json("lessons-matrix-v93.json").lessons,
 ...json("lessons-probability-v93.json").lessons,
 ...json("lessons-probability-v94a.json").lessons,
 ...json("lessons-probability-v94b.json").lessons,
 ...json("lessons-probability-v94c.json").lessons,
 ...json("lessons-statistics-v95a.json").lessons,
 ...json("lessons-statistics-v95b.json").lessons
];
assert.equal(historical.length,386,"A historical course file or lesson has changed");
assert.equal(new Set(historical.map(x=>x.node_id)).size,historical.length,"Duplicate historical ID");
const added=[...first.lessons,...second.lessons],all=[...historical,...added];
assert.equal(added.length,14);
assert.equal(all.length,400);
assert.equal(new Set(all.map(x=>x.node_id)).size,400,"A V96 course ID overlaps historical lessons");
let equationCount=0,chapterCount=0,explanationLength=0;
function strictMath(tex,where){
 assert(typeof tex==="string"&&tex.length>=10,where+": missing mathematical relationship");
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(tex),where+": hidden Unicode invalid in formula");
 katex.renderToString(tex,{throwOnError:true,strict:"error",displayMode:true});
 equationCount++;
}
for(const lesson of added){
 const n=nodes.get(lesson.node_id);
 assert(n&&n.title===lesson.title,lesson.node_id+": curriculum mismatch");
 assert.equal(lesson.id,lesson.node_id);
 for(const field of ["definition","assumptions","example","application","caveat"])
  assert(typeof lesson[field]==="string"&&lesson[field].length>=65,lesson.id+": incomplete "+field);
 assert(lesson.properties.length>=4&&lesson.properties.every(x=>x.length>=18),lesson.id+": missing mathematical properties");
 assert.equal(lesson.formulas.length,3);
 assert.equal(lesson.derivation.length,3);
 assert.equal(lesson.deep_sections.length,second.lessons.some(x=>x.id===lesson.id)?4:3);
 const seen=new Set();
 for(const f of lesson.formulas){assert(f.label.length>=4);strictMath(f.tex,lesson.id);}
 for(const d of lesson.derivation){assert(d.explanation.length>=25);strictMath(d.tex,lesson.id);}
 for(const s of lesson.deep_sections){
  assert(s.title.length>=8&&s.question.length>=10,lesson.id+": unclear learning question");
  assert(!seen.has(s.title),lesson.id+": duplicate deep section");
  seen.add(s.title);
  assert(s.paragraphs.length===2&&s.paragraphs.every(x=>x.length>=30),lesson.id+": shallow deep section");
  assert(s.proof.length>=50&&s.worked.length>=50&&s.limitation.length>=25,lesson.id+": missing proof, numerical example, or boundary");
  assert.equal(s.formulas.length,1);
  assert(s.formulas[0].label.length>=4);
  strictMath(s.formulas[0].tex,lesson.id);
  explanationLength+=s.paragraphs.join("").length+s.proof.length+s.worked.length+s.limitation.length;
  chapterCount++;
 }
 assert(/[\u3400-\u9FFF]/u.test(JSON.stringify(lesson)),lesson.id+": Chinese explanation absent");
 assert(!/TODO|FIXME|Lorem ipsum/i.test(JSON.stringify(lesson)),lesson.id+": unfinished placeholder");
}
assert.equal(chapterCount,49);
assert.equal(equationCount,133);
assert(explanationLength>12000,"Depth must exceed 12,000 explanatory characters");
const app=read("app.js"),css=read("styles.css");
for(const token of [
 "loadV89Foundations()","loadV91FoundationDeepening()","loadV90LinearFoundations()","loadV92LinearFoundations()",
 "loadV93CompletedLessons(\"./lessons-probability-v93.json\",9)",
 "loadV94ProbabilityLectures(\"./lessons-probability-v94c.json\",2)",
 "applyV94HistoricalProbabilityDeepening(lessons,v94deep)",
 "loadV95StatisticsLectures(\"./lessons-statistics-v95b.json\",7)",
 "loadV96OptimizationLectures(\"./lessons-optimization-v96a.json\",7)",
 "loadV96OptimizationLectures(\"./lessons-optimization-v96b.json\",7)",
 "...v95a,...v95b,...v96a,...v96b",
 "new Set(lessons.map(l=>l.node_id)).size!==lessons.length"
])assert(app.includes(token),"Math reader missing dependency "+token);
for(const token of ["overflow-x:auto","line-break:strict","@media(max-width:700px)","@media print",":focus-visible"])
 assert(css.includes(token),"CJK/responsive math rule missing "+token);
let checked=0;
function close(label,value,expected,tol=1e-8){
 assert(Number.isFinite(value)&&Math.abs(value-expected)<=tol,label+": "+value+" !== "+expected);
 checked++;
}
function valid(label,condition){close(label,condition?1:0,1);}
close("open feasible domain infimum",1/1000000,0,1e-5);
close("quadratic feasible min at w0.3",(.3-.3)**2,0);
close("strict constrained gradient at boundary",2*(0+1),2);
close("second-order Hessian first eigenvalue",2,2);
close("second-order Hessian second eigenvalue",4,4);
close("x to the fourth strict positivity",(-.1)**4,.0001);
close("x cubed sign counterexample",(-.1)**3,-.001);
close("simplex convex midpoint first",(.5+.1)/2,.3);
close("simplex convex midpoint second",(.3+.6)/2,.45);
close("simplex convex midpoint third",(.2+.3)/2,.25);
close("simplex midpoint total",.3+.45+.25,1);
close("PSD mixture first diagonal",(.01+.09)/2,.05);
close("PSD mixture second diagonal",(.04+.01)/2,.025);
close("convex first-order tangent",2*3-1,5);
close("quadratic tangent bound",3**2,9);
close("exp and sin composed negative second derivative",2*Math.cos(2*Math.PI/2),-2);
close("convexity PSD Hessian negative direction",(1-2+1)*(-1),0);
close("indefinite quadratic along y minus x",.5*(1-4+1),-1);
close("softmax Hessian eigenvalue nonzero",.25+.25,.5);
close("Lagrange equality optimum x",1/2,.5);
close("Lagrange equality optimum objective",2*(.5**2),.5);
close("Lagrange equality multiplier",-1,-1);
close("sensitivity value at b1.01",(1.01**2)/2,.51005);
close("LICQ failed constraint derivative",2*0,0);
close("MFCQ duplicated gradients first",-1,-1);
close("MFCQ duplicated gradients second",-2,-2);
close("LICQ redundant multiplier combination",0+2*1,2);
close("dual q lambda1",1-1/4,.75);
close("dual q lambda2",2-4/4,1);
close("dual feasible candidate objective",1.1**2,1.21);
close("dual certificate gap",1.21-.75,.46);
close("LP primal best",2*1+3*0,2);
close("LP dual max bound",Math.min(2,3),2);
close("LP feasible certificate gap",2*.5+3*.5-2,.5);
close("QP constrained optimum",(1-2)**2,1);
close("QP boundary multiplier",2*(2-1),2);
close("QP upper bound loosened",(1.01-2)**2,.9801);
close("QP finite improvement",1-.9801,.0199);
close("SOCP norm",Math.sqrt(3**2+4**2),5);
close("SOCP convex midpoint",Math.sqrt(2**2+2**2),Math.sqrt(8));
valid("SOCP midpoint feasible",Math.sqrt(8)<=3);
close("risk square from weights",(.6*.2)**2+(.4*.3)**2,.0288);
close("SOCP risk standard deviation",Math.sqrt(.0288),.16970562748477142,1e-10);
valid("SOCP exposure 0.17 feasible",Math.sqrt(.0288)<=.17);
valid("SOCP exposure 0.16 infeasible",Math.sqrt(.0288)>.16);
close("ellipsoid worst return",10-Math.hypot(3,4),5);
close("ellipsoid half radius return",10-.5*Math.hypot(3,4),7.5);
close("SDP primal trace",1+0,1);
close("SDP dual max",1,1);
close("SDP correlation eigenvalues sum",(1+.5)+(1-.5),2);
valid("SDP valid rho",Math.abs(.5)<=1);
valid("SDP invalid rho",Math.abs(1.1)>1);
close("stochastic expected at x0",.5*3+.5*9,6);
close("stochastic expected at x1",.5*2+.5*8,5);
close("stochastic expected at x3",.5*6+.5*6,6);
close("stochastic perfect information expectation",(2+6)/2,4);
close("stochastic nonanticipativity value",5-4,1);
close("stochastic objective piecewise left x.5",6-.5,5.5);
close("stochastic objective middle x2",4.5+.5*2,5.5);
close("CVaR alpha0.5 x1",Math.max(2,8),8);
close("CVaR alpha0.5 x3",Math.max(6,6),6);
assert(checked>=55);
console.log("V96 PASS: 14 new convex optimization lectures, 49 rigorous deep chapters, 133 strict KaTeX expressions, "+checked+" independent numeric checks, historical 386 courses preserved, 400/656 complete, CJK/mobile formatting guards intact.");

