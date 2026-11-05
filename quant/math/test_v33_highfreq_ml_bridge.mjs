import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const base=new URL("./",import.meta.url);
const read=(path)=>JSON.parse(fs.readFileSync(new URL(path,base),"utf8"));
const raw=(path)=>fs.readFileSync(new URL(path,base),"utf8");
const mathTargets=[
 ["lessons-phase-03-a.json",["timeseries-x001","riskmath-x002"]],
 ["lessons-phase-03-b.json",["timeseries-x005"]],
 ["lessons-phase-04-a.json",["statistics-x001","statistics-x005"]]
];
const mlTargets=[
 ["../ml/atlas-depth-foundations.json",["m01-c01-p05","m02-c04-p06","m03-c05-p03","m04-c03-p06","m05-c03-p03","m06-c04-p02","m07-c05-p02","m08-c04-p06"]],
 ["../ml/atlas-depth-quant.json",["m17-c01-p05","m18-c04-p06","m19-c05-p02","m20-c02-p05","m21-c04-p06","m22-c01-p05","m23-c05-p04","m24-c04-p06"]]
];
const catalog=read("curriculum.json");
assert.equal(catalog.nodes.length,656);
const mathNodes=new Map(catalog.nodes.map(n=>[n.id,n]));
const manifest=read("lessons-manifest.json");
assert.equal(manifest.expected_lessons,201);
assert.equal(manifest.modules.reduce((n,m)=>n+m.lessons,0),201);
const mlManifest=read("../ml/atlas-depth-manifest.json");
assert.equal(mlManifest.expected_lessons,720);
assert.equal(mlManifest.expected_domains,24);
const mlModules=new Map(mlManifest.modules.map(m=>[m.path,m.lessons]));
const math=[];
for(const [p,ids] of mathTargets){
 const data=read(p);
 assert.equal(data.revision,"2026-11-05");
 const target=data.lessons.filter(l=>ids.includes(l.id));
 assert.deepEqual(target.map(l=>l.id),ids,"canonical mathematics ordering "+p);
 math.push(...target);
}
assert.equal(math.length,5);
let deep=0,mathFormulas=0,mathLinks=0,mathChars=0;
for(const lesson of math){
 assert.equal(lesson.node_id,lesson.id);
 assert.equal(lesson.title,mathNodes.get(lesson.id)?.title,"canonical mathematics node");
 assert(lesson.checked_status.includes("V33"));
 assert.equal(lesson.deep_sections?.length,6,lesson.id+" independent subchapters");
 assert.equal(new Set(lesson.deep_sections.map(s=>s.title)).size,6);
 for(const s of lesson.deep_sections){
  assert(s.title.length>=6&&s.question.length>=10,lesson.id+" question");
  assert.equal(s.paragraphs.length,2,lesson.id+" instructional narrative");
  assert(s.paragraphs.every(p=>p.length>=45),lesson.id+" overly brief explanation");
  for(const k of ["proof","worked","limitation"])assert(s[k]?.length>=35,lesson.id+" "+k);
  const density=s.paragraphs.join("").length+s.proof.length+s.worked.length+s.limitation.length;
  assert(density>=245,lesson.id+" depth");
  mathChars+=density;
  assert(s.formulas.length>=1);
  for(const eq of s.formulas){katex.renderToString(eq.tex,{displayMode:true,throwOnError:true,strict:"ignore"});mathFormulas++;}
  deep++;
 }
 assert.equal(lesson.links?.length,3,lesson.id+" navigation");
 for(const link of lesson.links){assert.equal(link.kind,"math");assert(mathNodes.has(link.id),lesson.id+" broken "+link.id);mathLinks++;}
}
assert.equal(deep,30);
assert.equal(mathFormulas,30);
assert.equal(mathLinks,15);
assert(mathChars>10000,"mathematical exposition completeness");
const ml=[];
for(const [p,ids] of mlTargets){
 const data=read(p);
 assert.equal(data.revision,"2026-11-05");
 assert.equal(mlModules.get("./"+p.split("/").at(-1)),8,"manifest stability "+p);
 assert.deepEqual(Object.keys(data.lessons),ids,"canonical Atlas lesson IDs "+p);
 ml.push(...Object.entries(data.lessons));
}
assert.equal(ml.length,16);
let mlFormulas=0, mlAddedCharacters=0;
for(const [id,l] of ml){
 assert(l.revision_note?.startsWith("V33"),id+" enhancement");
 assert.equal(l.equations?.length,4,id+" 4 independent equations");
 assert.equal(l.derivation?.length,5,id+" 5 derivations");
 assert.equal(l.protocol?.length,4,id+" 4 operational verification items");
 assert.equal(l.pitfalls?.length,3,id+" 3 failure cases");
 assert(l.worked_example.length>=90,id+" preserved worked example");
 assert(l.derivation.slice(-2).every(s=>s.length>=75),id+" numerical reasoning explanation");
 assert(l.protocol.at(-1).length>=40 && l.pitfalls.at(-1).length>=25,id+" protocol/pitfall");
 for(const eq of l.equations){assert(eq.label&&eq.tex);katex.renderToString(eq.tex,{displayMode:true,throwOnError:true,strict:"ignore"});mlFormulas++;}
 mlAddedCharacters+=l.derivation.slice(-2).join("").length+l.protocol.at(-1).length+l.pitfalls.at(-1).length;
}
assert.equal(mlFormulas,64);
assert(mlAddedCharacters>4200);
const mathHome=raw("index.html"), mlHome=raw("../ml/index.html");
const css=raw("styles.css"),mlCss=raw("../ml/atlas.css"),mathReader=raw("app.js"),mlReader=raw("../ml/atlas.js");
assert(mathHome.includes('aria-label="V33高频计量与尾部风控五篇正式讲义"'));
assert(mlHome.includes('aria-label="V33十六篇机器学习量化实证深化"'));
for(const l of math)assert(mathHome.includes('href="?topic='+l.id+'"'),l.id+" math homepage route");
for(const [id] of ml)assert(mlHome.includes('href="#lesson/'+id+'"'),id+" ML Atlas route");
for(const id of ["timeseries-x001","riskmath-x002","statistics-x001","statistics-x005"])
 assert(mlHome.includes('href="../math/?topic='+id+'"'),"ML→math V33 route "+id);
for(const part of ["renderMathDeepSections","part.paragraphs","part.proof","part.worked","part.limitation"])assert(mathReader.includes(part),"reader "+part);
for(const part of ["s.equations.map","s.derivation","s.protocol","s.pitfalls","s.worked_example"])assert(mlReader.includes(part),"Atlas "+part);
assert(css.includes(".math-deep-unit")&&css.includes(".math-spectral-links")&&css.includes("line-break:strict")&&css.includes("@media print")&&css.includes("overflow-x:auto"));
assert(mlCss.includes(".ml-math-block")&&mlCss.includes(".atlas-spectral-links")&&mlCss.includes("line-break:strict")&&mlCss.includes("@media print")&&mlCss.includes("overflow-x:auto"));
const near=(x,y,label,tol=1e-10)=>assert(Math.abs(x-y)<=tol,label+": "+x+" vs "+y);
near(.01**2+(-.02)**2+.03**2,.0014,"realized variance");
near(.0009+.05**2,.0034,"diffusion+jump variation");
near(2*1000*.0001**2,.00002,"RV microstructure noise bias");
near(.03*10/.05+.02*100/.05,46,"expected shortfall with atom");
near(10+.02*(100-10)/.05,46,"ES RU variational value");
near((100+.5*40)/1.5,80,"partial sample tail weight");
near(.6/2,.3,"Hawkes branching ratio");
near(.2/(1-.6/2),2/7,"stationary Hawkes intensity");
near(.2+.6*Math.exp(-2),.2812011699419676,"Hawkes decay");
near(.2*2+(.6/2)*(1-Math.exp(-2)),.6593994150290162,"Hawkes likelihood compensator");
near((100*2+2*99)/100**2,.0398,"MA1 exact variance");
near(100*2/4,50,"MA1 equivalent sample size");
near(100/5,20,"five-day label effective sample");
near(1-.95**20,.6415140775914577,"independent multi-test FWER");
near(8**2/28,16/7,"importance weighting ESS");
near(.004/(1-.999),4,"Adam second moment correction");
near(.75*2+.25*0,1.5,"off-policy max bias counterexample");
near(100+30-10-25,95,"order book queue conservation");
assert(math.find(l=>l.id==="riskmath-x002").deep_sections[0].worked.includes("0.03×10"),"atom fraction checked");
assert(math.find(l=>l.id==="timeseries-x005").deep_sections[3].proof.includes("事件"),"Hawkes predictable pre-event rate");
const originalIto=read("lessons-phase-01-d.json").lessons.find(l=>l.id==="ito-n0308");
assert.equal(originalIto.deep_sections.length,4,"existing Ito content preserved");
console.log("PASS V33: 5 canonical 6-section math lectures (30 new strict KaTeX formulae), 16 ML research lectures (64 strict KaTeX formulae), 15 valid math links, 21 homepage routes, numeric checks, responsive CJK layouts and unchanged module counts.");
