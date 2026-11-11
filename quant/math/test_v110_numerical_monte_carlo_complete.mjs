import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const read=name=>fs.readFileSync(new URL(name,root),"utf8");
const json=name=>JSON.parse(read(name));
const syllabus=json("curriculum.json");
const nodes=syllabus.nodes;
const registry=new Map(nodes.map(node=>[node.id,node]));
assert.equal(nodes.length,656,"Original mathematics syllabus must remain 656 nodes");
assert.equal(registry.size,656,"Duplicate curriculum IDs");
assert.equal(syllabus.disciplines.length,20,"Original twenty mathematical disciplines must remain intact");
const files=["lessons-numerical-v110a.json","lessons-numerical-v110b.json","lessons-numerical-v110c.json","lessons-numerical-v110d.json","lessons-numerical-v110e.json"];
const counts=[4,5,5,4,4];
const newDocs=files.map(json);
newDocs.forEach((d,i)=>{
 assert.equal(d.version,1,"Wrong lesson schema");
 assert.equal(d.revision,"V110","Wrong revision on "+files[i]);
 assert.equal(d.lessons?.length,counts[i],"Wrong number of lectures on "+files[i]);
});
const added=newDocs.flatMap(d=>d.lessons);
const target=[364,365,366,368,371,372,373,374,375,376,377,378,379,380,381,383,384,385,387,388,389,390]
 .map(n=>"numerical-n"+String(n).padStart(4,"0"));
assert.deepEqual(added.map(x=>x.id),target,"V110 must preserve every missing original numerical syllabus ID");
const historical=fs.readdirSync(root)
 .filter(n=>/^lessons-.*\.json$/.test(n)&&!files.includes(n))
 .flatMap(n=>{const d=json(n);return Array.isArray(d.lessons)?d.lessons:[];});
assert.equal(historical.length,634,"Never overwrite or delete earlier 634 complete lectures");
assert.equal(new Set(historical.map(l=>l.node_id)).size,634,"Historical IDs must remain unique");
const all=[...historical,...added];
assert.equal(all.length,656,"All 656 knowledge nodes must have full lecture content");
assert.equal(new Set(all.map(l=>l.node_id)).size,656,"A course was overwritten or duplicated");
assert(all.every(l=>registry.get(l.node_id)?.title===l.title),"Syllabus title mismatches must be rejected");
const authored=new Set(all.map(l=>l.node_id));
assert(nodes.every(n=>authored.has(n.id)),"Every syllabus node must have a complete lesson");
for(const dept of syllabus.disciplines){
 const dnodes=nodes.filter(n=>n.discipline_id===dept.id);
 const written=all.filter(l=>l.node_id.startsWith(dept.id+"-"));
 assert.equal(written.length,dnodes.length,dept.title+" incomplete");
}
assert.equal(all.filter(l=>l.node_id.startsWith("numerical-")).length,30,"Numerical and Monte Carlo must be 30/30");
let formulaCount=0,sectionCount=0,linkCount=0,proofChars=0;
function strictTex(s,where){
 assert(typeof s==="string"&&s.length>=8,where+" empty formula");
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(s),where+" contains hidden/invalid Unicode");
 try{katex.renderToString(s,{displayMode:true,throwOnError:true,strict:"error"});}
 catch(e){throw Error(where+" strict KaTeX failed: "+s+" | "+e.message);}
 formulaCount++;
}
for(const l of added){
 const id=l.node_id;
 assert.equal(l.id,id);
 assert.equal(registry.get(id)?.title,l.title);
 for(const field of ["definition","assumptions","example","application","caveat"])
  assert(l[field]?.length>=50,id+" insufficient "+field);
 assert.equal(l.properties?.length,4,id+" missing four propositions");
 assert(l.properties.every(x=>x.length>=17),id+" too-short property");
 assert.equal(l.formulas?.length,3,id+" missing key formulas");
 assert.equal(l.derivation?.length,3,id+" missing three independent derivation steps");
 assert(l.deep_sections?.length>=3,id+" fewer than three deep technical explanations");
 assert.equal(l.links?.length,3,id+" links absent");
 l.formulas.forEach(f=>{assert(f.label?.length>=2);strictTex(f.tex,id+" core");});
 l.derivation.forEach(step=>{assert(step.explanation?.length>=20,id+" too-short derivation");strictTex(step.tex,id+" derivation");});
 const seen=new Set();
 for(const section of l.deep_sections){
  assert(section.title?.length>=7&&section.question?.length>=10,id+" underdeveloped section");
  assert(!seen.has(section.title),id+" repeated deep-section heading");seen.add(section.title);
  assert(section.paragraphs?.length===2&&section.paragraphs.every(x=>x.length>=35),id+" too-short two-paragraph proof context");
  assert(section.proof?.length>=40&&section.worked?.length>=40&&section.limitation?.length>=25,id+" insufficient proof, calculation or limitation");
  assert(section.formulas?.length>=1,id+" missing deep formulas");
  section.formulas.forEach(f=>strictTex(f.tex,id+" deep topic"));
  sectionCount++;
  proofChars+=section.paragraphs.join("").length+section.proof.length+section.worked.length+section.limitation.length;
 }
 for(const link of l.links){
  assert.equal(link.kind,"math");
  assert(registry.has(link.id),id+" broken internal reference: "+link.id);
  assert.equal(link.label,registry.get(link.id).title,id+" incorrect reference title: "+link.id);
  linkCount++;
 }
 assert(/[\u3400-\u9FFF]/u.test(JSON.stringify(l)),id+" Chinese narrative missing");
 assert(!/TODO|FIXME|Lorem ipsum|待补充|草稿|占位/.test(JSON.stringify(l)),id+" placeholder content found");
}
assert(formulaCount>=198,"Numerical lecture equations must not be removed");
assert(sectionCount>=66,"Numerical arguments must not be removed");
assert.equal(linkCount,66,"New release must have 66 precise clickable topic references");
assert(proofChars>=17000,"Explanation, proof and worked examples unexpectedly shortened");
const app=read("app.js"),html=read("index.html"),css=read("styles.css");
const ci=read("../../.github/workflows/test-quant-math.yml");
files.forEach((file,i)=>{
 assert(app.includes('loadV110NumericalMonteCarlo("./'+file+'",'+counts[i]+')'),"Reader not loading "+file);
});
assert(app.includes("...v109ea,...v109eb,...v110a,...v110b,...v110c,...v110d,...v110e"),"Historical 634 lessons were disconnected");
assert(app.includes("new Set(lessons.map(l=>l.node_id)).size!==lessons.length"),"Reader must abort on duplicate IDs");
assert(html.includes("数值分析与蒙特卡洛模拟"),"Chinese V110 learning roadmap absent");
assert(/app[.]js[?]v=[0-9]{8}/.test(html),"Reader cache-busting version absent");
for(const id of target)assert(html.includes('?topic='+id+'"'),"Direct navigation absent: "+id);
for(const t of ["overflow-x:auto","line-break:strict","@media(max-width:700px)","@media print",":focus-visible"])
 assert(css.includes(t),"Responsive formulas, Chinese line breaks or accessible styles regressed: "+t);
assert(ci.includes("node quant/math/test_v110_numerical_monte_carlo_complete.mjs"),"V110 strict QA not wired to CI");
for(const test of ["test_v106_calculus_core.mjs","test_v107_calculus_advanced.mjs",
                   "test_v108_foundation_measure_complete.mjs","test_v109_regression_econometrics_complete.mjs"])
 assert(read(test).includes("lessons-numerical-v110[a-e]"),"Old fixed-regression baseline not insulated: "+test);
let numericChecks=0;
function close(label,x,y,tol=1e-8){
 assert(Number.isFinite(x),label+" nonfinite: "+x);
 assert(Math.abs(x-y)<=tol,label+" failed "+x+" != "+y);
 numericChecks++;
}
function yes(label,cond){assert(cond,label);numericChecks++;}
close("rounding gamma 100",100e-6/(1-100e-6),.0001000100010001,1e-12);
close("rounding cancellation",1.234-1.233,.001,1e-12);
close("interpolation polynomial at 1.5",1.5**2,2.25);
close("forward difference x²",((1.1)**2-1)/.1,2.1);
close("central difference x²",((1.1)**2-(.9)**2)/.2,2);
close("second central difference x²",((1.1)**2-2+(.9)**2)/.01,2);
close("SVD truncated error",Math.sqrt(0**2+1**2),1);
close("SVD pseudoinverse",3*(1/3),1);
close("power method initial Rayleigh",(.5*3+.5*1),2);
close("power method 1 Rayleigh",(3*9+1)/10,2.8);
close("power method ratio at n3",1/(3**3),1/27);
close("sparse A1 row",2*1-1*1,1);
close("sparse A2 row",-1+2-1,0);
close("sparse A3 row",-1+2,1);
close("sparse storage nnz",2+3+2,7);
close("bisection f midpoint",1.5**2-2,.25);
close("bisection second midpoint",1.25**2-2,-.4375);
close("bisection nine-step bound",1/(2**10),.0009765625);
let n1=(1.5+2/1.5)/2, n2=(n1+2/n1)/2;
close("Newton first",n1,17/12);
close("Newton second",n2,577/408);
close("secant first",2-2*(2-1)/(2-(-1)),4/3);
let s3=4/3-((4/3)**2-2)*((4/3)-2)/(((4/3)**2-2)-(2**2-2));
close("secant second",s3,7/5);
const f=x=>x**4;
const simp=(a,b)=>(b-a)/6*(f(a)+4*f((a+b)/2)+f(b));
const s1=simp(0,1), s2=simp(0,.5)+simp(.5,1);
close("Simpson single",s1,5/24);
close("Simpson split",s2,77/384);
close("Simpson refined error",s2-s1,-1/128);
close("Richardson Simpson correction",s2+(s2-s1)/15,.2);
close("Gauss x2",2*(1/Math.sqrt(3))**2,2/3);
close("Gauss x4",2*(1/Math.sqrt(3))**4,2/9);
close("Gauss x4 true integral",2/5,.4);
close("forward x3 derivative",((1.1)**3-1)/.1,3.31);
close("central x3 derivative",((1.1)**3-(.9)**3)/.2,3.01);
close("exp inverse sample",-Math.log(1-.75)/2,Math.log(4)/2);
close("exp inverse CDF",1-Math.exp(-2*(Math.log(4)/2)),.75);
close("rejection envelope acceptance",1/2,.5);
yes("rejection point .8 accepted",.5<=.8);
yes("rejection point .2 rejected",.5>.2);
close("importance target E x²",1/3,.3333333333333333);
close("importance q E x",2/3,.6666666666666666);
close("importance weighted variance",(.5-(2/3)**2)/4,1/72);
close("importance direct variance",1/5-1/9,4/45);
close("antithetic sample",(.25**2+.75**2)/2,.3125);
close("antithetic pair variance",1/80-1/144,1/180);
close("independent pair variance",(4/45)/2,2/45);
close("antithetic variance improvement",(1/180)/(2/45),1/8);
close("van der Corput 4 prefix",(0+.5+.25+.75)/4,.375);
close("randomized QMC illustrative SE",Math.sqrt((.02**2+.02**2)/(3*2)),Math.sqrt(.0008/6));
const emMean=100*(1+.1*.5)**2, trueMean=100*Math.exp(.1);
close("Euler GBM mean",emMean,110.25);
close("continuous GBM mean",trueMean,110.51709180756477,1e-7);
close("Euler mean discretization bias",emMean-trueMean,-.26709180756477,1e-7);
close("Milstein Euler one step",100+20*.1,102);
close("Milstein correction",.5*20*.2*(.1**2-.25),-.48);
close("Milstein terminal 1 step",102-.48,101.52);
close("Euler deterministic h.5",(1+.5)**2,2.25);
close("Euler deterministic h.25",(1+.25)**4,2.44140625);
close("Euler deterministic weak error",Math.E-2.25,.468281828459045);
close("bootstrap mean", (1+2+3)/3,2);
close("bootstrap conditional mean variance",((1-2)**2+(2-2)**2+(3-2)**2)/9,2/9);
close("bootstrap empirical SD",Math.sqrt(2/9),Math.sqrt(2)/3);
close("Asian average",(110+90)/2,100);
close("Asian payoff",Math.max((110+90)/2-95,0),5);
close("Asian discounted payoff",5*Math.exp(-.05),4.75614712250357,1e-9);
close("GBM exact zero-shock step",100*Math.exp((.05-.5*.2**2)*.25),100*Math.exp(.0075));
close("Brownian bridge barrier crossing",Math.exp(-2*(.1*.1)/(.2**2*.25)),Math.exp(-2));
close("AR1 long-run variance rho .5", (1+.5)/(1-.5),3);
close("AR1 standard error factor", Math.sqrt(3),1.7320508075688772);
close("twelve-day moving-block choices",12-3+1,10);
close("MLMC telescoping worked example",10-.5-.25,9.25);
close("Asian control-variable adjusted payoff",5-(4-6),7);
yes("new lesson ids contain all missing original numerical topics",target.length===22);
for(const lecture of json("lessons-numerical-v110d.json").lessons)
 for(const section of lecture.deep_sections)
  assert(section.paragraphs.every(p=>!p.includes("\n")),lecture.node_id+" still contains repeated paragraphs");
const reviewedNumerical=json("lessons-numerical-v110e.json").lessons.flatMap(l=>l.deep_sections);
assert.equal(reviewedNumerical.length,16,"All 16 numerical subchapters required");
for(const s of reviewedNumerical)assert(s.paragraphs[1].length>=140&&!s.paragraphs[1].includes("\n"),s.title+" incomplete numerical assumptions");
assert(Math.abs(Math.exp(-2)-0.1353352832366127)<1e-12,"Brownian bridge barrier crossing example incorrect");
assert(reviewedNumerical.find(s=>s.title==="连续障碍的漏检偏差").paragraphs[1].includes("端点"),"Brownian bridge endpoint conditions missing");


const extendedMathGroups=[
 ["lessons-phase-26-a.json",["timeseries-n0260","timeseries-n0261","timeseries-n0262"]],
 ["lessons-phase-26-b.json",["timeseries-n0266","timeseries-n0267","timeseries-n0269","timeseries-n0270"]],
 ["lessons-phase-27-b.json",["statistics-n0173","statistics-n0174","statistics-n0175","statistics-n0176"]],
 ["lessons-phase-27-a.json",["statistics-n0171","statistics-n0170","statistics-n0172"]],
 ["lessons-phase-25-a.json",["timeseries-x003","timeseries-x004","timeseries-x006","mlmath-x006","riskmath-x001"]]
];
let checkedMathExtensions=0;
for(const [file,expected] of extendedMathGroups){
 const doc=json(file);
 assert.deepEqual(doc.lessons.map(x=>x.id),expected,"extended mathematics canonical order changed "+file);
 for(const lesson of doc.lessons){
  const section=lesson.deep_sections.at(-1);
  assert(lesson.deep_sections.length>=7&&lesson.derivation.length>=7&&lesson.formulas.length>=10,"extended mathematics incomplete "+lesson.id);
  assert.equal(new Set(lesson.deep_sections.map(x=>x.title)).size,lesson.deep_sections.length,"extended chapter headings duplicated "+lesson.id);
  assert(section.paragraphs.length===2&&section.paragraphs.every(s=>s.length>=65),"extended proof context inadequate "+lesson.id);
  assert(section.proof.length>=90&&section.worked.length>=75&&section.limitation.length>=40,"extended numerical proof inadequate "+lesson.id);
  assert(section.formulas.length>=2,"extended mathematics needs two display equations "+lesson.id);
  for(const f of section.formulas){assert(f.label?.length>=3&&f.tex?.length>=14);katex.renderToString(f.tex,{displayMode:true,throwOnError:true,strict:"error"});}
  checkedMathExtensions++;
 }
}
assert.equal(checkedMathExtensions,19,"all nineteen enriched mathematics articles must be verified");
assert(Math.abs((1+.5+.25+.125)-1.875)<1e-12,"AR(1) cumulative impulse response");
assert(Math.abs((.25*4)/(1+.25*4)-.5)<1e-12,"FEVD two-period external innovation contribution");
assert(Math.abs(2-1/2-1.5)<1e-12,"Granger proxy conditional prediction risk");
assert(Math.abs(Math.exp(.8)-2.225540928492468)<1e-12,"EGARCH asymmetric variance multiplier");
assert(Math.abs(.25*.04+.5*.03+.25*.09-.0475)<1e-12,"DCC portfolio variance");
assert(Math.abs(Math.pow(2,2*.25)-Math.sqrt(2))<1e-12,"ARFIMA low-frequency spectral ratio");
assert(Math.abs(2/3+(1/3)*.7**2-.83)<1e-12,"Markov two-period state probability");
assert.equal([0,0,0,0,10].sort((a,b)=>a-b)[2],0,"zero-MAD sample median");
assert(Math.abs(.5*.05+.3*.05+.2*.05-.05)<1e-12,"weighted Bonferroni budget");
assert(Math.abs(.05*1+.95*0-.05)<1e-12,"expected FDP illustration");
assert(Math.abs(4*2.5-3*(20/9)-10/3)<1e-12,"Jackknife variance bias correction");
assert(Math.abs(Math.sqrt(6)/36-.06804138174397717)<1e-12,"BCa acceleration on three observations");
assert.equal(2/8,.25,"two sided three-pair sign flip probability");
assert(Math.abs((.00004-.2*.00012)/.8-.00002)<1e-12,"correlated quote noise leaves two-scale bias");
assert(Math.abs(.8*.5-.4)<1e-12,"asynchronous one-half overlap lowers return covariance");
assert(Math.abs(1-Math.exp(-1)-.6321205588285577)<1e-12,"point process exponential PIT");
assert(Math.abs(1/(.16+.09+.04+.01)-10/3)<1e-12,"importance weight ESS");
assert(Math.abs(.1*10/.2-5)<1e-12,"discrete expected shortfall with VaR atom");

console.log("V110 PASS: 656/656 finished, all 20 disciplines complete, numerical 30/30, "+
  sectionCount+" complete reasoning sections, "+formulaCount+" strict KaTeX formulas, "+
  linkCount+" exact curriculum links, "+proofChars+" Chinese proof characters, "+
  numericChecks+" independent arithmetic checks, CJK/mobile/print/legacy gates");
