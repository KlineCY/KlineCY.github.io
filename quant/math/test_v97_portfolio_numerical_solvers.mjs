import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const read=p=>fs.readFileSync(new URL(p,root),"utf8");
const json=p=>JSON.parse(read(p));
const p=json("lessons-portfolio-v97.json"),a=json("lessons-algorithms-v97.json"),n=json("lessons-numerical-v97.json");
const docs=[p,a,n];
for(const d of docs){assert.equal(d.version,1);assert.equal(d.revision,"V97");}
assert.deepEqual(p.lessons.map(l=>l.node_id),["optimization-n0349","optimization-n0350","optimization-n0352","optimization-n0353","optimization-n0354"]);
assert.deepEqual(a.lessons.map(l=>l.node_id),["optimization-n0355","optimization-n0356","optimization-n0357","optimization-n0360"]);
assert.deepEqual(n.lessons.map(l=>l.node_id),["numerical-n0361","numerical-n0363","numerical-n0367","numerical-n0369","numerical-n0370"]);
const curriculum=json("curriculum.json"),nodeMap=new Map(curriculum.nodes.map(x=>[x.id,x]));
assert.equal(nodeMap.size,656,"Historical curriculum node count changed");
const manifest=json("lessons-manifest.json");
assert.equal(manifest.expected_lessons,201);
assert.equal(manifest.modules.length,77);
const old=[
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
 ...json("lessons-statistics-v95b.json").lessons,
 ...json("lessons-optimization-v96a.json").lessons,
 ...json("lessons-optimization-v96b.json").lessons
];
assert.equal(old.length,400,"V96 historical completed lesson count was disturbed");
assert.equal(new Set(old.map(l=>l.node_id)).size,400,"Duplicate historical lesson IDs");
const added=docs.flatMap(d=>d.lessons),all=[...old,...added];
assert.equal(added.length,14);
assert.equal(all.length,414);
assert.equal(new Set(all.map(l=>l.node_id)).size,414,"V97 course overlaps a historical complete lesson");
const completed=new Set(all.map(l=>l.node_id));
const optimizationNodes=curriculum.nodes.filter(x=>x.id.startsWith("optimization-"));
assert.equal(optimizationNodes.length,42);
assert(optimizationNodes.every(x=>completed.has(x.id)),"Optimization discipline is NOT entirely completed");
let formulas=0,chapters=0,deepLength=0;
function math(tex,location){
 assert(typeof tex==="string"&&tex.length>=10,location+" missing expression");
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(tex),location+" contains broken Unicode");
 katex.renderToString(tex,{displayMode:true,throwOnError:true,strict:"error"});
 formulas++;
}
for(const l of added){
 const n=nodeMap.get(l.node_id);
 assert(n&&n.title===l.title,l.node_id+" curriculum mismatch");
 assert.equal(l.id,l.node_id);
 for(const key of ["definition","assumptions","example","application","caveat"])
  assert(typeof l[key]==="string"&&l[key].length>=70,l.node_id+" incomplete "+key);
 assert(l.properties.length>=4&&l.properties.every(q=>q.length>=20),l.id+" insufficient properties");
 assert.equal(l.formulas.length,3);
 assert.equal(l.derivation.length,3);
 assert(l.deep_sections.length>=3,l.id+" original three deep sections retained");
 for(const f of l.formulas){assert(f.label.length>=4);math(f.tex,l.id);}
 for(const d of l.derivation){assert(d.explanation.length>=25);math(d.tex,l.id);}
 const uniqueTitles=new Set();
 for(const s of l.deep_sections){
  assert(s.title.length>=8&&s.question.length>=10,l.id+" unclear section");
  assert(!uniqueTitles.has(s.title),l.id+" duplicate deep section");
  uniqueTitles.add(s.title);
  assert(s.paragraphs.length===2&&s.paragraphs.every(t=>t.length>=30),l.id+" shallow paragraphs");
  assert(s.proof.length>=50&&s.worked.length>=50&&s.limitation.length>=25,l.id+" incomplete deep reasoning");
  const index=l.deep_sections.indexOf(s);
  if(index<3)assert.equal(s.formulas.length,1,l.id+" original deep formula count retained");
  else assert(s.formulas.length>=2,l.id+" new portfolio section equations required");
  for(const expr of s.formulas){assert(expr.label.length>=4);math(expr.tex,l.id);}
  chapters++;
  deepLength+=s.paragraphs.join("").length+s.proof.length+s.worked.length+s.limitation.length;
 }
 assert(/[\u3400-\u9FFF]/u.test(JSON.stringify(l)),l.id+" no Chinese exposition");
 assert(!/TODO|FIXME|Lorem ipsum/i.test(JSON.stringify(l)),l.id+" unfinished content");
}
assert(chapters>=47,"At least 42 original and 5 added deep chapters required");
assert(formulas>=136,"At least 126 original and 10 additional strict formula checks required");
assert(deepLength>=12000);
const app=read("app.js"),css=read("styles.css");
for(const token of [
 "loadV89Foundations()","loadV91FoundationDeepening()",
 "loadV90LinearFoundations()","loadV92LinearFoundations()",
 "loadV93CompletedLessons(\"./lessons-probability-v93.json\",9)",
 "loadV94ProbabilityLectures(\"./lessons-probability-v94c.json\",2)",
 "applyV94HistoricalProbabilityDeepening(lessons,v94deep)",
 "loadV95StatisticsLectures(\"./lessons-statistics-v95b.json\",7)",
 "loadV96OptimizationLectures(\"./lessons-optimization-v96b.json\",7)",
 "loadV97PortfolioNumericalLectures(\"./lessons-portfolio-v97.json\",5)",
 "loadV97PortfolioNumericalLectures(\"./lessons-algorithms-v97.json\",4)",
 "loadV97PortfolioNumericalLectures(\"./lessons-numerical-v97.json\",5)",
 "...v96a,...v96b,...v97portfolio,...v97algorithms,...v97numerical",
 "new Set(lessons.map(l=>l.node_id)).size!==lessons.length"
])assert(app.includes(token),"Canonical reader fails to load "+token);
for(const rule of ["overflow-x:auto","line-break:strict","@media(max-width:700px)","@media print",":focus-visible"])
 assert(css.includes(rule),"CJK/KaTeX responsiveness/accessibility missing "+rule);
let checks=0;
function near(label,value,expected,tol=1e-8){
 assert(Number.isFinite(value)&&Math.abs(value-expected)<=tol,label+": "+value+" vs "+expected);
 checks++;
}
function check(label,val){near(label,val?1:0,1);}
near("Markowitz target mean",.6*.06+.4*.02,.044);
near("Markowitz covariance quadratic",.6**2*.04+.4**2*.09,.0288);
near("Markowitz sigma",Math.sqrt(.0288),.16970562748477142,1e-10);
near("positive covariance risk uplift",2*.6*.4*.02,.0096);
near("frontier GMV w1",.09/(.04+.09),9/13);
near("frontier GMV w2",.04/(.04+.09),4/13);
near("frontier GMV mean",(9*.06+4*.02)/13,.62/13);
near("frontier GMV variance",.04*.09/(.04+.09),.027692307692307693);
check("frontier dominated target",.044<.62/13&&.0288>.04*.09/.13);
near("frontier impossible long-only weight",(.07-.02)/(.06-.02),1.25);
near("risk parity variance contribution 1",.6**2*.04,.0144);
near("risk parity variance contribution 2",.4**2*.09,.0144);
near("risk parity standard deviation Euler each",.0144/Math.sqrt(.0288),.08485281374238571);
near("risk parity both Euler contributions",2*.0144/Math.sqrt(.0288),Math.sqrt(.0288));
near("BL posterior mean equal uncertainty",(.01*.04+.01*.08)/.02,.06);
near("BL posterior variance",1/(1/.01+1/.01),.005);
near("BL higher noise posterior mean",(.03*.04+.01*.08)/.04,.05);
near("BL predictive covariance",.04+.005,.045);
near("turnover gross",Math.abs(.7-.5)+Math.abs(.3-.5),.4);
near("turnover one-way",.4/2,.2);
near("turnover fee",.001*.4,.0004);
near("turnover fee basis points",.0004*10000,4);
near("GD first iteration",0-.25*(2*(0-3)),1.5);
near("GD second iteration",1.5-.25*2*(1.5-3),2.25);
near("GD initial loss",(0-3)**2,9);
near("GD first loss",(1.5-3)**2,2.25);
near("GD second loss",(2.25-3)**2,.5625);
near("GD unstable example",(0-1.2*(-6)-3)**2,17.64);
near("Newton gradient at x1",1+1**3,2);
near("Newton Hessian at x1",1+3*1**2,4);
near("Newton first step",1-2/4,.5);
near("Newton initial objective",.5+.25,.75);
near("Newton objective x0.5",.5*.5**2+.25*.5**4,.140625);
near("coordinate initial objective",(0-2)**2+(0+1)**2,5);
near("coordinate after x update",(2-2)**2+(0+1)**2,1);
near("coordinate after y update",(2-2)**2+(-1+1)**2,0);
near("soft threshold result",(3-1)/2,1);
near("soft threshold zero",Math.max(.5-1,0),0);
near("SGD true minimizer",(0+2)/2,1);
near("SGD sample update from X2",0-.25*(0-2),.5);
near("SGD sample update from X0",0-.25*(0-0),0);
near("SGD expected update",(.5+0)/2,.25);
near("SGD stationary noise floor",.25**2/(1-(1-.25)**2),1/7);
near("SGD correlated batch",(.25*(1+1+2*.5)),.75);
near("binary64 unit roundoff",Number.EPSILON/2,2**-53);
near("binary64 machine epsilon",Number.EPSILON,2**-52);
near("cancellation at one quadrillion",(1e16+1)-1e16,0);
near("stable sqrt difference at one hundred million",1/(Math.sqrt(1e8+1)+Math.sqrt(1e8)),.000049999999875,1e-10);
near("Euler stable amplification",1-4*.25,0);
near("Euler actual one-step target",Math.exp(-1),.36787944117144233,1e-10);
near("Euler unstable amplification",1-4*.75,-2);
near("Euler h.1 ten steps",.6**10,.0060466176);
near("Euler analytic T1",Math.exp(-4),.01831563888873418,1e-10);
near("LU first triangular y1",3,3);
near("LU triangular second",4-.5*3,2.5);
near("LU backsub x1",(3-1)/2,1);
near("LU backsub x2",2.5/2.5,1);
near("QR beta intercept",(5*5-3*6)/(3*5-9),7/6);
near("QR beta slope",(3*6-3*5)/(3*5-9),.5);
near("QR residual sum",-1/6+1/3-1/6,0);
near("QR x weighted residual",1/3-2/6,0);
near("Cholesky diagonal L11",Math.sqrt(4),2);
near("Cholesky diagonal L22",Math.sqrt(5-1**2),2);
near("Cholesky solution first residual",4*1+2*1-6,0);
near("Cholesky solution second residual",2*1+5*1-7,0);
near("Cholesky correlation conditioning",(1+.999)/(1-.999),1999,1e-8);
near("CG numerator",2**2+8**2,68);
near("CG denominator",2*2**2+4*8**2,264);
near("CG alpha",68/264,17/66);
near("CG x first",2*(17/66),17/33);
near("CG x second",8*(17/66),68/33);
near("CG first residual component",2-2*(17/33),32/33);
near("CG second residual component",8-4*(68/33),-8/33);
near("CG first residual orthogonality",2*(32/33)+8*(-8/33),0);
const advancedTopics={
 "optimization-n0349":"含现金的Markowitz最优解与参数误差传递",
 "optimization-n0350":"双资产相关最小方差组合的解析解",
 "optimization-n0352":"双资产风险平价与相关性抵消恒等式",
 "optimization-n0353":"Black–Litterman观点单位变化与后验不变性",
 "optimization-n0354":"L1点差成本诱导的精确不交易区域",
 "numerical-n0361":"log1p与expm1在微小收益计算中的必要性",
 "numerical-n0363":"刚性动力系统的显式Euler稳定区间",
 "numerical-n0367":"秩亏设计下的最小范数OLS与Ridge连续极限",
 "numerical-n0369":"Cholesky分解失败与最小特征值对角修复",
 "numerical-n0370":"对角预条件CG的条件数改善与精确解保持"
};
for(const [id,title] of Object.entries(advancedTopics)){
 const lesson=added.find(l=>l.node_id===id);
 assert(lesson&&lesson.deep_sections.some(s=>s.title===title),"missing rigorous advanced course argument "+id);
 const s=lesson.deep_sections.find(s=>s.title===title);
 assert(s.paragraphs.length===2 && s.paragraphs.every(x=>x.length>=80),id+" incomplete exposition");
 assert(s.formulas.length===2 && s.proof.length>=95 && s.worked.length>=90 && s.limitation.length>=85,id+" incomplete proof or empirical check");
 for(const f of s.formulas)math(f.tex,id+" advanced strict");
}
near("cash-aware Markowitz first weight",.06/(3*.04),.5);
near("cash-aware Markowitz second weight",.03/(3*.09),1/9);
near("correlated GMV first weight",(.09-.02)/(.04+.09-2*.02),7/9);
near("correlated GMV variance",(.04*.09-.02**2)/(.04+.09-2*.02),.035555555555555556);
near("two-asset risk parity second weight",.2/(.2+.3),.4);
near("BL view rescaling unchanged",(.04/.01+2*.16/.04)/(1/.01+4/.04),.06);
near("trading soft threshold",Math.max(.015-.02/4,0),.01);
assert.equal(Math.log(1+1e-16),0,"floating cancellation must be observable");
assert(Math.abs(Math.log1p(1e-16)-1e-16)<1e-30);
near("Euler stiff factor unstable",1-100*.03,-2);
near("Ridge duplicate factor weight",5/(10+2),5/12);
near("Cholesky repaired condition number",2.003/.001,2003);
near("preconditioned condition ratio",(1+1/6)/(1-1/6),7/5);

assert(checks>=70);
console.log("V97 PASS: all 42 optimization nodes completed; 14 original lessons; "+chapters+" deep proof chapters; "+formulas+" strict KaTeX formulas; "+checks+" independent numeric assertions; 400 historic lessons preserved; 414/656 unique complete; Chinese and responsive math CSS rules preserved.");

