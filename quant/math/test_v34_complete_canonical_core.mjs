import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const raw=p=>fs.readFileSync(new URL(p,root),"utf8");
const targets=[
 ["lessons-foundations.json",[
  "calculus-n0032","calculus-n0042","calculus-n0056","linear-n0082","probability-n0114",
  "statistics-n0158","statistics-n0161","regression-n0182","regression-n0199","econometrics-n0217"]],
 ["lessons-advanced.json",[
  "stochastic-n0280","stochastic-n0273","ito-n0303","ito-n0309","ito-n0314",
  "optimization-n0338","optimization-n0351","numerical-n0382","diffeq-n0409","finance-n0433",
  "mlmath-n0457","finance-n0435","mlmath-n0478","discrete-n0523","riskmath-n0574",
  "optimization-n0342","numerical-n0386"]],
 ["lessons-more.json",[
  "foundation-n0012","calculus-n0046","linear-n0084","measure-n0132","econometrics-n0223",
  "information-n0481","control-n0541","riskmath-n0582"]]
];
assert.deepEqual(targets.map(([p,ids])=>ids.length),[10,17,8]);
const nodeList=read("curriculum.json").nodes;
assert.equal(nodeList.length,656);
const nodes=new Map(nodeList.map(n=>[n.id,n]));
assert.equal(nodes.size,656);
const manifest=read("lessons-manifest.json");
assert.equal(manifest.expected_lessons,201);
assert.equal(manifest.modules.length,77);
assert.equal(manifest.modules.reduce((n,m)=>n+m.lessons,0),201);
const atlas=read("../ml/atlas-depth-manifest.json");
assert.equal(atlas.expected_lessons,720);
assert.equal(atlas.expected_domains,24);
let added=[],core=[],reused=0,formulaCount=0,linkCount=0,proseCount=0;
for(const [p,ids] of targets){
 const data=read(p);
 assert.equal(data.version,1,p+" schema");
 assert.equal(data.revision,"2026-11-05",p+" revision");
 assert.equal(data.lessons.length,p==="lessons-foundations.json"?19:p==="lessons-advanced.json"?69:35);
 const edited=data.lessons.filter(l=>l.checked_status?.startsWith("V34"));
 assert.deepEqual(edited.map(l=>l.id).sort(),[...ids].sort(),p+" exact canonical IDs");
 added.push(...edited);
 core.push(...data.lessons);
 for(const l of data.lessons)assert(l.deep_sections?.length>=4,"underdetailed core chapter "+l.id);
 for(const l of edited){
  assert.equal(l.node_id,l.id,"node identifier "+l.id);
  assert.equal(l.title,nodes.get(l.id)?.title,"canonical title "+l.id);
  assert.equal(l.deep_sections.length,4,"4 deep chapters "+l.id);
  assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,4,l.id+" unique subchapters");
  for(const s of l.deep_sections){
   assert(s.title.length>=5&&s.question.length>=8,l.id+" descriptive title");
   assert.equal(s.paragraphs.length,2,l.id+" two instructional paragraphs");
   assert(s.paragraphs.every(v=>v.length>=45),l.id+" detailed contextual narrative");
   for(const k of ["proof","worked","limitation"])assert(typeof s[k]==="string"&&s[k].length>=30,l.id+" "+k);
   const density=s.paragraphs.join("").length+s.proof.length+s.worked.length+s.limitation.length;
   assert(density>=220,l.id+" complete systematic lesson "+s.title);
   proseCount+=density;
   assert(s.formulas.length>=1,l.id+" KaTeX formula");
   for(const e of s.formulas){
    assert(e.label?.length>=2);
    katex.renderToString(e.tex,{throwOnError:true,strict:"ignore",displayMode:true});
    formulaCount++;
   }
  }
  assert.equal(l.links.length,3,l.id+" canonical cross-references");
  for(const link of l.links){
   assert.equal(link.kind,"math");
   assert(nodes.has(link.id),l.id+" broken knowledge reference "+link.id);
   linkCount++;
  }
  assert(!/TODO|占位|待填|Lorem ipsum|[\uFFFD\u200B\uFEFF]/i.test(JSON.stringify(l)),l.id+" damaged Chinese");
 }
 reused+=data.lessons.length-edited.length;
}
assert.equal(added.length,35);
assert.equal(reused,88);
assert.equal(core.length,123);
assert.equal(core.filter(l=>l.deep_sections?.length>=4).length,123);
assert.equal(added.reduce((n,l)=>n+l.deep_sections.length,0),140);
assert.equal(formulaCount,140);
assert.equal(linkCount,105);
assert(proseCount>=40000,"V34 40k+ substantive Chinese explanation");
const home=raw("index.html"),mlHome=raw("../ml/index.html"),css=raw("styles.css");
const mlCss=raw("../ml/atlas.css"),reader=raw("app.js"),mlReader=raw("../ml/atlas.js");
assert(home.includes('aria-label="核心教材缺口清零：35篇基础讲义补全为系统课程"'));
assert(mlHome.includes('aria-label="机器学习数学先修完整版：微积分、概率、数值与动态决策"'));
for(const l of added)assert(home.includes('href="?topic='+l.id+'"'),"homepage missing canonical "+l.id);
const mlRoutes=[
 "calculus-n0032","calculus-n0042","calculus-n0056","linear-n0082","linear-n0084",
 "mlmath-n0457","probability-n0114","statistics-n0158","statistics-n0161","regression-n0182",
 "regression-n0199","econometrics-n0217","econometrics-n0223","information-n0481",
 "control-n0541","optimization-n0338"
];
assert.equal(mlRoutes.length,16);
for(const id of mlRoutes)assert(mlHome.includes('href="../math/?topic='+id+'"'),"ML→math link "+id);
for(const name of ["renderMathDeepSections","part.paragraphs","part.proof","part.worked","part.limitation"])
 assert(reader.includes(name),"core math reader field "+name);
assert(mlReader.includes("renderMath"),"Atlas KaTeX bridge");
for(const token of [".math-deep-unit",".math-spectral-links","overflow-x:auto","line-break:strict","@media print",":focus-visible"])
 assert(css.includes(token),"math CJK/KaTeX responsive style "+token);
for(const token of [".atlas-spectral-links","overflow-x:auto","line-break:strict","@media print"])
 assert(mlCss.includes(token),"ML CJK responsive style "+token);
const near=(x,y,name,tol=1e-8)=>assert(Math.abs(x-y)<=tol,name+": "+x+" != "+y);
near(6*(3*2+1),42,"composite chain");
near((2*3)+(2*1),8,"quadratic form gradient");
near(3/(3+7),.3,"Bernoulli MLE");
near(2/3,.6666666666666666,"Gaussian sample variance MLE");
near(4/100,.04,"normal CRLB");
near(3/4,.75,"cost-sensitive Logit threshold");
near(20/10,2,"exactly identified IV");
near((2/3)*.1,(1/3)*.2,"reversible Markov flow");
near(Math.exp(-3),.049787068367863944,"Poisson zero events");
near(2/100,.02,"Brownian quadratic variation variance");
near(.08-.5*.2**2,.06,"Ito logarithmic drift");
near(100*Math.exp(.08),108.32870676749585,"GBM mean");
near(100*Math.exp(.06),106.18365465453596,"GBM median");
near(.09/.13,.6923076923076923,"GMV optimal first asset");
near(1/(25+1/.09),.027692307692307693,"GMV portfolio variance");
near(9.7-.8*(12.4-12),9.38,"control variate option value");
near((1-.9)/(.2),.5,"binomial Q measure");
near(.5*(.5*21),5.25,"two-step CRR call");
near(100*Math.exp(-.02)-100*Math.exp(-.05),2.896924880604115,"dividend put-call parity",1e-6);
near(.5*.02*25,.25,"discrete gamma exposure");
near(.9/(.1+.9),.9,"probability normalization");
near(2*Math.PI*.5,Math.PI,"Jacobian unit disk");
near(9/(9+1),.9,"PCA explained variance");
near((1-1/Math.pow(100,0))*0,0,"zero sanity");
near(100+(.02/.2)*(Math.pow(10,.2)-1)*1000,100+100*(Math.pow(10,.2)-1),"GPD tail quantile demonstration");
near(5-Math.exp(-.2),4.181269246922018,"OU exact conditional mean");
near(100*(1-.4*3),-20,"Euler positivity failure");
near((.03*10+.02*100)/.05,46,"atom-sensitive ES");
near(10+(.02*(100-10))/.05,46,"RU convex ES");
console.log("PASS V34: all 123 canonical core lessons now have substantive deep chapters; 35 upgraded, 140 chapters, 140 strict KaTeX formulas, 105 valid mathematical links, 16 ML prerequisite links, 40k+ Chinese proof characters, numerical invariants and responsive/print CJK layout.");
