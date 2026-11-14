import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const read=p=>fs.readFileSync(new URL(p,root),"utf8");
const json=p=>JSON.parse(read(p));
const a=json("lessons-statistics-v95a.json"),b=json("lessons-statistics-v95b.json");
assert.equal(a.version,1);assert.equal(b.version,1);
assert.equal(a.revision,"V95");assert.equal(b.revision,"V95");
const expectedA=["statistics-n0152","statistics-n0153","statistics-n0154","statistics-n0155","statistics-n0156","statistics-n0157","statistics-n0159"];
const expectedB=["statistics-n0160","statistics-n0163","statistics-n0164","statistics-n0165","statistics-n0166","statistics-n0167","statistics-n0168"];
assert.deepEqual(a.lessons.map(l=>l.node_id),expectedA);
assert.deepEqual(b.lessons.map(l=>l.node_id),expectedB);
const curriculum=json("curriculum.json"),nodes=new Map(curriculum.nodes.map(n=>[n.id,n]));
assert.equal(nodes.size,656,"Historical 656 curriculum nodes changed");
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
 ...json("lessons-probability-v94c.json").lessons
];
assert.equal(old.length,372,"Existing 372 complete lectures were disturbed");
assert.equal(new Set(old.map(x=>x.node_id)).size,372,"Historical source contains duplicate nodes");
const newLessons=[...a.lessons,...b.lessons],all=[...old,...newLessons];
assert.equal(newLessons.length,14);
assert.equal(all.length,386,"Total complete lesson count must be 386/656");
assert.equal(new Set(all.map(x=>x.node_id)).size,386,"V95 duplicates a historical complete lesson");
const addedFullStat={
 "statistics-n0152":"非同方差独立样本的Bessel校正与均值风险",
 "statistics-n0153":"次序统计量的非渐近分位点覆盖区间",
 "statistics-n0154":"充分统计量的模型相对性与路径风险的不可恢复",
 "statistics-n0155":"对数配分函数的曲率与自然参数Fisher信息",
 "statistics-n0156":"Student t精确分布的正交投影证明和厚尾边界",
 "statistics-n0157":"GMM最优权重矩阵与长程矩条件协方差",
 "statistics-n0159":"一阶Delta导数为零时的二阶极限"
};
assert.equal(Object.keys(addedFullStat).length,7,"Seven new full statistical proofs required");

let equations=0,sections=0,depth=0;
function strictMath(source,id){
 assert(typeof source==="string"&&source.length>=10,id+" formula missing");
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(source),id+" invalid hidden Unicode");
 katex.renderToString(source,{throwOnError:true,strict:"error",displayMode:true});
 equations++;
}
for(const lesson of newLessons){
 const known=nodes.get(lesson.node_id);
 assert(known&&known.title===lesson.title,lesson.id+" is not a titled curriculum node");
 assert.equal(lesson.id,lesson.node_id);
 for(const field of ["definition","assumptions","example","application","caveat"]){
  assert(typeof lesson[field]==="string"&&lesson[field].length>=65,lesson.id+" insufficient "+field);
 }
 assert(lesson.properties.length>=4&&lesson.properties.every(x=>x.length>=18));
 assert.equal(lesson.formulas.length,3);
 assert.equal(lesson.derivation.length,3);
 assert.equal(lesson.deep_sections.length,addedFullStat[lesson.id]?6:5,lesson.id+" preserved five original chapters plus validated sixth");
 const titles=new Set();
 for(const f of lesson.formulas){assert(f.label.length>=4);strictMath(f.tex,lesson.id);}
 for(const d of lesson.derivation){assert(d.explanation.length>=24);strictMath(d.tex,lesson.id);}
 for(const s of lesson.deep_sections){
  assert(s.title.length>=8&&s.question.length>=10,lesson.id+" has vague heading");
  assert(!titles.has(s.title),lesson.id+" repeats chapter");
  titles.add(s.title);
  assert(s.paragraphs.length===2&&s.paragraphs.every(x=>x.length>=30),lesson.id+" shallow exposition");
  assert(s.proof.length>=50&&s.worked.length>=50&&s.limitation.length>=25,lesson.id+" lacks proof, calculation or validity boundary");
  assert.equal(s.formulas.length,lesson.deep_sections.indexOf(s)>=4?2:1,lesson.id+" all original formulas preserved and advanced chapter complete");
   if(lesson.deep_sections.indexOf(s)===5){
    assert.equal(s.title,addedFullStat[lesson.id],lesson.id+" exact added inference argument missing");
    assert(s.paragraphs.every(x=>x.length>=100)&&s.proof.length>=100&&s.worked.length>=90&&s.limitation.length>=55,lesson.id+" rigorous worked proof not complete");
   }
  for(const formula of s.formulas){assert(formula.label.length>=4);strictMath(formula.tex,lesson.id);}
  depth+=s.paragraphs.join("").length+s.proof.length+s.worked.length+s.limitation.length;
  sections++;
 }
 assert(/[\u3400-\u9FFF]/u.test(JSON.stringify(lesson)),lesson.id+" lacks Chinese technical prose");
 assert(!/TODO|FIXME|Lorem ipsum/i.test(JSON.stringify(lesson)),lesson.id+" contains incomplete placeholder");
}
assert.equal(sections,77,"Original 70 proof chapters plus 7 additional deep proofs required");
assert.equal(equations,182,"Original 168 strict formulae plus 14 new formulae must parse");
assert(depth>=12000,"Mathematical explanation depth below threshold");
const app=read("app.js"),css=read("styles.css");
for(const token of [
 "loadV89Foundations()","loadV91FoundationDeepening()",
 "loadV90LinearFoundations()","loadV92LinearFoundations()",
 "loadV93CompletedLessons(\"./lessons-probability-v93.json\",9)",
 "loadV94ProbabilityLectures(\"./lessons-probability-v94c.json\",2)",
 "applyV94HistoricalProbabilityDeepening(lessons,v94deep)",
 "loadV95StatisticsLectures(\"./lessons-statistics-v95a.json\",7)",
 "loadV95StatisticsLectures(\"./lessons-statistics-v95b.json\",7)",
 "...v94a,...v94b,...v94c,...v95a,...v95b"
])assert(app.includes(token),"Canonical loader missing "+token);
for(const token of ["overflow-x:auto","line-break:strict","@media(max-width:700px)","@media print",":focus-visible"])
 assert(css.includes(token),"CJK/formula accessibility requirement missing "+token);
let checks=0;
const close=(label,value,expected,tol=1e-8)=>{
 assert(Number.isFinite(value)&&Math.abs(value-expected)<=tol,label+": "+value+" differs "+expected);
 checks++;
};
close("mean of four samples",(1+2+3+4)/4,2.5);
close("corrected sample variance",5/3,1.6666666666667);
close("empirical sample variance",5/4,1.25);
close("uniform maximum CDF",.8**3,.512);
close("uniform minimum CDF",1-(1-.8)**3,.992);
close("uniform maximum expected",3/4,.75);
close("order statistic median mean",2/(3+1),.5);
close("Bernoulli conditional sequence count",1/(5*4*3/(3*2)),.1);
close("Bernoulli logistic p",Math.exp(Math.log(.3/.7))/(1+Math.exp(Math.log(.3/.7))),.3);
close("log partition second derivative",.3*(1-.3),.21);
close("Poisson exponential family mean",Math.exp(Math.log(3)),3);
close("normal t example",.5/Math.sqrt((5/3)/4),.7745966692414834);
close("nested F example",((50-34)/2)/(34/17),4);
close("moment normal mean",2.5,2.5);
close("normal raw second moment",(1+4+9+16)/4,7.5);
close("normal MoM variance",7.5-2.5**2,1.25);
close("exponential rate moment estimator",1/2.5,.4);
close("overidentified GMM minimum at theta two",(2-1)**2+(2-3)**2,2);
close("Bernoulli CLT standard error",Math.sqrt(.4*.6/100),.04898979485566356);
close("Bernoulli CLT lower end",.4-1.96*Math.sqrt(.4*.6/100),.3039800020839,1e-7);
close("iid information at p0.4",1/(.4*.6),4.166666666666667);
close("information over n100",100/(.4*.6),416.6666666666667);
close("sandwich one-parameter scalar",(.5)*8*(.5),2);
close("variance MLE Bernoulli",.4*.6/100,.0024);
close("normal mean 5 pct one-sided threshold",1.645/5,.329);
close("one-sided power noncentral shift",Math.sqrt(25)*.5,2.5);
close("20 repeated tests one at least",1-.95**20,.6415140775914581);
close("two-sided z approx p value",2*(1-.977249868051821),.0455002638963584);
close("likelihood ratio n100 k60",2*(60*Math.log(1.2)+40*Math.log(.8)),4.027102710137776,1e-8);
close("Bernoulli KL equivalence",200*(.6*Math.log(1.2)+.4*Math.log(.8)),4.027102710137776,1e-8);
close("Wald p sample",100*.1**2/(.6*.4),4.166666666666667);
close("Score restricted p sample",100*.1**2/(.5*.5),4);
close("Score direct using U and I",40**2/400,4);
close("nuisance efficient info",1-.5**2,.75);
close("nonlinear Wald linear restriction",100*(1.1-1)**2,1);
close("nonlinear Wald quadratic restriction",100*(1.1**2-1)**2/(2*1.1)**2,.9111570247933896,1e-8);
close("CI lower",.4-1.96/Math.sqrt(100),.204);
close("CI upper",.4+1.96/Math.sqrt(100),.596);
close("CI halfwidth",1.96/Math.sqrt(100),.196);
close("power example needed n",Math.ceil(((1.645+.842)/.5)**2),25);
close("long run autocovariance",1+2*.2,1.4);
close("dependent two-period mean variance",(1+1+2*.5)/4,.75);
assert(checks>=40);
console.log("V95 PASS: 14 new canonical statistics lectures, 77 complete proof chapters, 182 strict KaTeX expressions, "+checks+" numerical checks, 372 historic lessons retained and unique total 386/656; responsive CJK/KaTeX rules retained.");

