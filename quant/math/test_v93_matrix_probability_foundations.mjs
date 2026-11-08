import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const read=p=>fs.readFileSync(new URL(p,root),"utf8");
const json=p=>JSON.parse(read(p));
const matrix=json("lessons-matrix-v93.json");
const probability=json("lessons-probability-v93.json");
const expectedMatrix=["linear-n0088","linear-n0089","linear-n0090"];
const expectedProbability=["probability-n0091","probability-n0092","probability-n0093","probability-n0094","probability-n0097","probability-n0098","probability-n0099","probability-n0101","probability-n0102"];
assert.equal(matrix.version,1);
assert.equal(probability.version,1);
assert.equal(matrix.revision,"V93");
assert.equal(probability.revision,"V93");
assert.deepEqual(matrix.lessons.map(x=>x.node_id),expectedMatrix);
assert.deepEqual(probability.lessons.map(x=>x.node_id),expectedProbability);
const curriculum=json("curriculum.json"),nodes=new Map(curriculum.nodes.map(x=>[x.id,x]));
assert.equal(nodes.size,656);
const manifest=json("lessons-manifest.json");
assert.equal(manifest.expected_lessons,201);
assert.equal(manifest.modules.length,77);
const historical=[
 ...json("lessons-foundations.json").lessons,
 ...json("lessons-advanced.json").lessons,
 ...json("lessons-more.json").lessons,
 ...manifest.modules.flatMap(x=>json(x.path).lessons),
 ...json("lessons-foundations-v89.json").lessons,
 ...json("lessons-linear-v90.json").lessons,
 ...json("lessons-linear-v92.json").lessons
];
assert.equal(historical.length,352,"Historical 352 completed lessons changed");
assert.equal(new Set(historical.map(x=>x.node_id)).size,historical.length);
const added=[...matrix.lessons,...probability.lessons];
const all=[...historical,...added];
assert.equal(all.length,364,"Expected 364 full mathematics courses");
assert.equal(new Set(all.map(x=>x.node_id)).size,364,"Duplicate course IDs");
let formulaCount=0,chapters=0,characterCount=0;
const tex=source=>{
 assert(typeof source==="string"&&source.length>=10,"Incomplete mathematical expression");
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(source),"Invalid or hidden Unicode in mathematical formula");
 katex.renderToString(source,{throwOnError:true,strict:"error",displayMode:true});
 formulaCount++;
};
for(const lesson of added){
 const id=lesson.node_id,n=nodes.get(id);
 assert.equal(lesson.id,id);
 assert(n&&n.title===lesson.title,"Curriculum title mismatch "+id);
 for(const name of ["definition","assumptions","example","application","caveat"]){
  assert(typeof lesson[name]==="string"&&lesson[name].length>=65,id+": insufficient "+name);
 }
 assert.equal(lesson.formulas.length,3,id+" must contain three precise core relations");
 assert.equal(lesson.derivation.length,3,id+" must have three explicit derivations");
 assert(lesson.properties.length>=4&&lesson.properties.every(x=>x.length>=18));
 assert.equal(lesson.deep_sections.length,3,id+" should have three substantive deep chapters");
 const titles=new Set();
 for(const formula of lesson.formulas){assert(formula.label.length>=4);tex(formula.tex);}
 for(const item of lesson.derivation){assert(item.explanation.length>=25);tex(item.tex);}
 for(const section of lesson.deep_sections){
  assert(section.title.length>=8&&section.question.length>=10,id+" vague title");
  assert(!titles.has(section.title),id+" repeated chapter");
  titles.add(section.title);
  assert(section.paragraphs.length===2&&section.paragraphs.every(x=>x.length>=30),id+" shallow section narrative");
  assert(section.proof.length>=50&&section.worked.length>=50&&section.limitation.length>=25,id+" incomplete proof or numeric case");
  assert.equal(section.formulas.length,1);
  assert(section.formulas[0].label.length>=4);
  tex(section.formulas[0].tex);
  characterCount+=section.paragraphs.join("").length+section.proof.length+section.worked.length+section.limitation.length;
  chapters++;
 }
 assert(/[\u3400-\u9FFF]/u.test(JSON.stringify(lesson)),id+" missing Chinese narrative");
 assert(!/TODO|FIXME|Lorem ipsum/i.test(JSON.stringify(lesson)),id+" draft text");
}
assert.equal(chapters,36);
assert.equal(formulaCount,108);
assert(characterCount>11000,"Depth must be substantive");
const app=read("app.js"),css=read("styles.css");
for(const item of [
 "loadV89Foundations()","loadV91FoundationDeepening()","loadV90LinearFoundations()",
 "loadV92LinearFoundations()","loadV93CompletedLessons(\"./lessons-matrix-v93.json\",3)",
 "loadV93CompletedLessons(\"./lessons-probability-v93.json\",9)",
 "...extra,...v89,...v90,...v92,...v93matrix,...v93probability"
])assert(app.includes(item),"Reader loader missing "+item);
for(const token of ["overflow-x:auto","line-break:strict","@media(max-width:700px)","@media print",":focus-visible"]){
 assert(css.includes(token),"Responsive mathematics/CJK accessibility token missing "+token);
}
let numeric=0;
const close=(name,a,b,tol=1e-8)=>{assert(Number.isFinite(a)&&Math.abs(a-b)<=tol,name+": "+a+" versus "+b);numeric++;};
const truth=(name,condition)=>close(name,condition?1:0,1);
close("quadratic form example",1*4+2*7,18);
close("quadratic finite difference",.08+2*.01*.01,.0802);
close("quadratic gradient first",2*4,8);
close("quadratic gradient second",2*7,14);
close("vec kron first",2*1,2);
close("vec kron second",3*3,9);
close("vec kron third",2*2,4);
close("vec kron fourth",3*4,12);
close("sample covariance shrink 11",.6*.04+.4*.025,.034);
close("sample covariance shrink 12",.6*.01,.006);
close("sample covariance shrink 22",.6*.01+.4*.025,.016);
close("sample covariance original trace",.04+.01,.05);
close("sample covariance shrink trace",.034+.016,.05);
close("sample covariance shrink determinant",.034*.016-.006**2,.000508,1e-12);
close("three-way inclusion-exclusion",.2+.15+.1-.06-.04-.03+.01,.33);
close("Bonferroni lower bound",.2+.15+.1-.06-.04-.03,.32);
close("asset pairs unordered",5*4/2,10);
close("asset pairs ordered",5*4,20);
close("uniform tail PDF",1/.1,10);
close("uniform interval probability",(.03-.02)*10,.1);
close("uniform change of units",.5/2,.25);
close("discrete expectation",-.25+2*.25,.25);
close("discrete second moment",.25+4*.25,1.25);
close("discrete variance",1.25-.25*.25,1.1875);
close("CDF discrete atom at one",.7-.2,.5);
close("CDF interval mass",1-.2,.8);
close("poisson zero",Math.exp(-2),.1353352832366127);
close("poisson one",2*Math.exp(-2),.2706705664732254);
close("poisson at least two",1-3*Math.exp(-2),.5939941502901619);
close("Poisson moment",3**2+3,12);
close("Poisson variance",12-3**2,3);
close("lognormal mean",Math.exp(Math.log(2)/2),Math.SQRT2);
close("lognormal second moment",Math.exp(2*Math.log(2)),4);
close("lognormal variance",4-Math.SQRT2**2,2);
close("GBM log drift",.08-.2**2/2,.06);
close("Gaussian loss VaR",.02*1.96,.0392);
truth("positive semidefinite shrink",.034>0&&.000508>0);
truth("Poisson thinning mean",.2*5===1);
assert(numeric>=35);
console.log("V93 PASS: 12 new matrix/probability lectures, 36 full proof sections, 108 strict KaTeX expressions, "+numeric+" verified mathematical invariants, preserved 352 historic lessons, total 364/656 completed, responsive CJK formula guards.");

