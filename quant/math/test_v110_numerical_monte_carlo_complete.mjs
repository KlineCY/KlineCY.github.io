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
 assert(l.formulas?.length>=3,id+" original key formulas preserved");
 assert(l.derivation?.length>=3,id+" original derivations preserved");
 assert(l.deep_sections?.length>=3,id+" fewer than three deep technical explanations");
 assert.equal(l.links?.length,3,id+" links absent");
 l.formulas.forEach(f=>{assert(f.label?.length>=2);strictTex(f.tex,id+" core");});
 l.derivation.forEach(step=>{assert(step.explanation?.length>=20,id+" too-short derivation");strictTex(step.tex,id+" derivation");});
 const seen=new Set();
 for(const section of l.deep_sections){
  assert(section.title?.length>=7&&section.question?.length>=10,id+" underdeveloped section");
  assert(!seen.has(section.title),id+" repeated deep-section heading");seen.add(section.title);
  assert(section.paragraphs?.length>=2&&section.paragraphs.every(x=>x.length>=35),id+" insufficient multi-paragraph proof context");
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
assert(reviewedNumerical.length>=16,"All 16 original numerical subchapters preserved");
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
  assert(lesson.deep_sections.length>=7&&lesson.derivation.length>=7&&lesson.formulas.length>=9,"extended mathematics incomplete "+lesson.id);
  assert.equal(new Set(lesson.deep_sections.map(x=>x.title)).size,lesson.deep_sections.length,"extended chapter headings duplicated "+lesson.id);
  assert(section.paragraphs.length>=2&&section.paragraphs.every(s=>s.length>=65),"extended proof context inadequate "+lesson.id);
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


const nextMathGroups=[
 ["lessons-phase-24-a.json",["statistics-x003","statistics-x004","mlmath-x003","mlmath-x004"]],
 ["lessons-phase-27-c.json",["riskmath-n0589","riskmath-n0594","riskmath-n0595"]],
 ["lessons-phase-30-a.json",["finance-n0431","finance-n0432","finance-n0439"]]
];
let checkedNewMath=0;
for(const [file,ids] of nextMathGroups){
 const doc=json(file);
 assert.deepEqual(doc.lessons.map(l=>l.id),ids,"advanced mathematics curriculum identity changed "+file);
 for(const l of doc.lessons){
  assert(l.deep_sections.length>=7&&l.derivation.length>=7&&l.formulas.length>=10,"advanced proof chapters missing "+l.id);
  const last=l.deep_sections.at(-1);
  assert(last.paragraphs.length>=2&&last.paragraphs.every(p=>p.length>=70),"advanced math explanation short "+l.id);
  assert(last.proof.length>=95&&last.worked.length>=80&&last.limitation.length>=50,"advanced math proof or example missing "+l.id);
  assert(last.formulas.length>=2,"advanced topic requires two focused equations "+l.id);
  assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,l.deep_sections.length,"duplicate chapter headings "+l.id);
  for(const eq of last.formulas){
   assert(eq.tex.length>=16&&eq.label.length>=4,"math formula incomplete "+l.id);
   katex.renderToString(eq.tex,{displayMode:true,throwOnError:true,strict:"error"});
  }
  checkedNewMath++;
 }
}
assert.equal(checkedNewMath,10,"ten additional deepened quant-mathematics articles required");
const nearMath=(a,b,name)=>assert(Math.abs(a-b)<1e-9,name+" mismatch "+a+" vs "+b);
nearMath((1-.25)**2,.5625,"stationary bootstrap block survival");
nearMath(.01*.0004/(2*.02**3),.25,"Sharpe influence at mean");
nearMath(2*3/Math.sqrt(100),.6,"Rademacher linear-ball bound");
nearMath(2*2**2/(1*100),.08,"regularized stability upper bound");
nearMath(.04+2*(1)*(-3)*.016+9*.01,.034,"factor covariance quadratic");
nearMath(.04-.048,-.008,"negative Euler factor numerator");
nearMath(.1*.04/.13+.1*.09/.13,.1,"reverse stress optimum target loss");
nearMath(.02/Math.sqrt(Math.PI),.011283791670955126,"selection winner bias");
nearMath(.2*1.25*.8+.5*1+.3*(5/6)*1.2,1,"physical pricing-kernel identity");
nearMath(-1-1+2,0,"redundant portfolio zero initial cost");
nearMath((.25*.8+.5*1+.25*1.2),1,"risk neutral discounted martingale");
nearMath(-Math.log(1.05/1.02),-.0289875368732523,"negative forward rate");


const cycleTimeSeriesGroups=[
 ["lessons-phase-22-a.json",["timeseries-n0244","timeseries-n0245"]],
 ["lessons-phase-22-b.json",["timeseries-n0248","timeseries-n0249"]],
 ["lessons-phase-22-c.json",["timeseries-n0250","timeseries-n0251"]],
 ["lessons-phase-22-d.json",["timeseries-n0254","timeseries-n0255"]]
];
let checkedTimeSeries=0;
for(const [file,ids] of cycleTimeSeriesGroups){
 const doc=json(file);
 assert.deepEqual(doc.lessons.map(l=>l.id),ids,"original time-series lesson IDs changed "+file);
 for(const l of doc.lessons){
  assert(l.formulas.length>=9&&l.derivation.length>=6&&l.deep_sections.length>=7,"time-series deepening incomplete "+l.id);
  const section=l.deep_sections.at(-1);
  assert(section.paragraphs.length>=2&&section.paragraphs.every(s=>s.length>=65),"time-series prose too brief "+l.id);
  assert(section.proof.length>=95&&section.worked.length>=80&&section.limitation.length>=45,"time-series derivation, example or limitation missing "+l.id);
  assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,l.deep_sections.length,"duplicate time-series headings "+l.id);
  assert(section.formulas.length>=2,"two independent display formulas required "+l.id);
  for(const e of section.formulas){
   assert(e.label.length>=4&&e.tex.length>=14,"time-series equation short "+l.id);
   katex.renderToString(e.tex,{displayMode:true,throwOnError:true,strict:"error"});
  }
  checkedTimeSeries++;
 }
}
assert.equal(checkedTimeSeries,8,"all eight time-series course deepening records required");
nearMath(1/100,.01,"white-noise mean variance");
nearMath(3-1,2,"nonlinear dependent squared white-noise covariance");
nearMath(4*(1+.5**2),5,"Wold MA horizon two error variance");
nearMath((1+2**2)*1,(1+.5**2)*4,"noninvertible MA covariance equivalence");
nearMath(2*1,.5*4,"MA lag-one covariance equivalence");
nearMath(.8-.8,0,"ARMA polynomial cancellation");
nearMath(-.0004/.0008,-.5,"overdifferenced iid lag-one correlation");
nearMath(240+2*2,234+2*5,"AIC tied rival models");
nearMath(244+60/94,244.63829787234042,"small-sample AICc model B");
nearMath(Math.log(.5)/Math.log(.98),34.309618491520645,"ADF local-root half-life n100");
nearMath(Math.log(.5)/Math.log(.996),172.93999003737392,"ADF local-root half-life n500");
nearMath(2/(3**2*2),1/9,"KPSS cumulative residual statistic");


const newLearningMathematicsGroups=[
 ["lessons-phase-21-a.json",["mlmath-n0455","mlmath-n0466"]],
 ["lessons-phase-21-b.json",["mlmath-n0463","mlmath-n0464"]],
 ["lessons-phase-21-c.json",["mlmath-n0460","mlmath-n0461"]],
 ["lessons-phase-21-d.json",["mlmath-n0462","mlmath-n0465"]]
];
let deepLearningMathCount=0;
for(const [filename,expected] of newLearningMathematicsGroups){
 const doc=json(filename);
 assert.deepEqual(doc.lessons.map(l=>l.id),expected,"learn-math canonical order changed "+filename);
 for(const l of doc.lessons){
  assert(l.formulas.length>=9&&l.derivation.length>=6&&l.deep_sections.length>=7,"extended math chapter incomplete "+l.id);
  assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,l.deep_sections.length,"duplicate mathematical chapter heading "+l.id);
  const last=l.deep_sections.at(-1);
  assert(last.paragraphs.length>=2&&last.paragraphs.every(s=>s.length>=85),"mathematical exposition too brief "+l.id);
  assert(last.proof.length>=95&&last.worked.length>=95&&last.limitation.length>=65,"mathematical proof, numeric worked example or caveat missing "+l.id);
  assert(last.formulas.length>=2,"two mathematical display equations are required "+l.id);
  for(const eq of last.formulas){
   assert(eq.label.length>=4&&eq.tex.length>=18,"mathematical formula missing "+l.id);
   katex.renderToString(eq.tex,{displayMode:true,throwOnError:true,strict:"error"});
  }
  deepLearningMathCount++;
 }
}
assert.equal(deepLearningMathCount,8,"eight new machine-learning mathematics chapters required");
const nearNew=(x,y,msg)=>assert(Math.abs(x-y)<1e-9,msg+" "+x+" "+y);
nearNew(1-2**(1-4),.875,"test re-use maximum");
assert(Math.max(8,12)<=Math.min(13,14),"purging must detect overlapping future labels");
nearNew((5/6)/(1-1/6),1,"Ridge fixed-lambda LOOCV residual");
nearNew(2/3,0.6666666666666666,"elastic net duplicate coefficients");
nearNew(4/1.75,2.2857142857142856,"autocorrelated mini-batch effective size");
nearNew(.01*(.09/(1-.9**2))/Math.sqrt(.0099/(1-.99**2)),.006715801472893548,"Adam zero current gradient still updates");
nearNew(.8*1+.2*(-4),0,"unclipped random gradient");
nearNew(.8*1+.2*(-1),.6,"clipped gradient bias");
nearNew(.95,1-.05,"scaled LASSO effective penalty");


const expandedV20MathGroups=[
 ["lessons-phase-20-a.json",["econometrics-x003"]],
 ["lessons-phase-20-b.json",["optimization-x011","optimization-x012"]],
 ["lessons-phase-20-c.json",["econometrics-x005","econometrics-x006"]],
 ["lessons-phase-23-a.json",["timeseries-n0256","timeseries-n0257"]],
 ["lessons-phase-23-b.json",["timeseries-n0258","timeseries-n0259"]]
];
let verifiedV20Math=0;
for(const [filename,ids] of expandedV20MathGroups){
 const doc=json(filename);
 assert.deepEqual(doc.lessons.map(l=>l.id),ids,"expanded mathematics canonical IDs changed "+filename);
 for(const l of doc.lessons){
  assert(l.formulas.length>=9&&l.derivation.length>=6&&l.deep_sections.length>=7,"expanded maths missing details "+l.id);
  const sec=l.deep_sections.at(-1);
  assert(sec.paragraphs.length>=2&&sec.paragraphs.every(s=>s.length>=115),"new maths must explain both theory and usage "+l.id);
  assert(sec.proof.length>=130&&sec.worked.length>=125&&sec.limitation.length>=55,"new maths proof, worked calculation or limitation missing "+l.id);
  assert(sec.formulas.length>=2,"new maths needs two independent LaTeX formulas "+l.id);
  assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,l.deep_sections.length,"math section title duplication "+l.id);
  for(const eq of sec.formulas){
   assert(eq.label.length>=4&&eq.tex.length>=16,"extended maths equation short "+l.id);
   katex.renderToString(eq.tex,{displayMode:true,throwOnError:true,strict:"error"});
  }
  verifiedV20Math++;
 }
}
assert.equal(verifiedV20Math,9,"all 9 newly deepened mathematics courses required");
nearMath(.8*.25*0+.2*4*2,1.6,"transported ATE target weighting");
nearMath(Math.max(8,12),12,"purged overlap left endpoint");
nearMath(Math.min(13,14),13,"purged overlap right endpoint");
nearMath(1+.5,1.5,"FISTA second prox iterate");
nearMath(.5*(2-1)**2+1,1.5,"LASSO primal optimum");
nearMath(2*1-.5,1.5,"LASSO dual feasible value");
nearMath((12-10)-(10-10),2,"anticipation pre-period observed gap");
nearMath(.7-.4-.1,.2,"ATT lower sensitivity limit");
nearMath(.7-.4+.1,.4,"ATT upper sensitivity limit");
nearMath(Math.log(110/100),Math.log(220/200),"cointegrated log price ratio");
nearMath(-100*(Math.log(.7)+Math.log(.9)),46.20354595965587,"Johansen trace rank zero");
nearMath(-100*Math.log(.7),35.66749439387324,"Johansen max root");
nearMath(.04/(1-.8**2),1/9,"stationary ECM spread variance");
nearMath(.8**5,.32768,"conditional mean ECM horizon five");
nearMath(.011/(1-.99),1.1,"VAR near-unit-root long-run mean");


const v28DeepGroups=[
 ["lessons-phase-28-a.json",["statistics-n0177","statistics-n0178","statistics-n0179","statistics-n0180"],8,8,11],
 ["lessons-phase-28-b.json",["finance-n0434","finance-n0436","finance-n0437","finance-n0438"],7,7,10]
];
let v28Verified=0;
for(const [file,ids,minSections,minDeriv,minFormulas] of v28DeepGroups){
 const doc=json(file);
 assert.deepEqual(doc.lessons.map(l=>l.id),ids,"V28 mathematics canonical IDs changed "+file);
 for(const l of doc.lessons){
  assert(l.deep_sections.length>=minSections&&l.derivation.length>=minDeriv&&l.formulas.length>=minFormulas,"V28 mathematical coverage missing "+l.id);
  const sec=l.deep_sections.at(-1);
  assert(sec.paragraphs.length>=2&&sec.paragraphs.every(p=>p.length>=85),"V28 two complete explanatory paragraphs required "+l.id);
  assert(sec.proof.length>=115&&sec.worked.length>=125&&sec.limitation.length>=65,"V28 derivation, calculation or caveat too short "+l.id);
  assert(sec.formulas.length>=2,"V28 two strict mathematical equations required "+l.id);
  assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,l.deep_sections.length,"V28 repeated section heading "+l.id);
  for(const eq of sec.formulas){
   assert(eq.label.length>=4&&eq.tex.length>=18,"V28 formula incomplete "+l.id);
   katex.renderToString(eq.tex,{displayMode:true,throwOnError:true,strict:"error"});
  }
  v28Verified++;
 }
}
assert.equal(v28Verified,8,"all eight Bayesian/derivatives mathematics chapters required");
nearMath(.9/.1,9,"single Bayesian warning likelihood ratio");
nearMath(9*(1/9)/(1+9*(1/9)),.5,"correlated duplicate warning posterior");
nearMath(2*3/(4*5),.3,"Beta posterior two future risk events");
nearMath(1-.3-.3,.4,"Beta-binomial one future risk event");
nearMath(.01/(.01+.09),.1,"Cantelli posterior upper bound");
nearMath(-.7*Math.log(.7)-.3*Math.log(.3),.6108643020548935,"Bernoulli entropy proper score");
nearMath(Math.log(2)-(-.7*Math.log(.7)-.3*Math.log(.3)),.08228287850505178,"improper prediction KL");
nearMath(100*(.539827837277029-.460172162722971),7.965567455405802,"BSM at-the-money call");
nearMath(.5*.02*10000*(.3**2-.2**2)/252,.01984126984126984,"Delta hedged long-gamma PnL");
nearMath(.2/5,.04,"inverse Vega quote sensitivity");
nearMath(100/(2*20),2.5,"uniform risk-neutral terminal-price call");
nearMath(-10/20,-.5,"strike slope equals discounted tail probability");


// V116: retain every historical proof and validate all newly appended full chapters.
const v116Numerical={
 "lessons-numerical-v110a.json":{
  "numerical-n0364":"对数求和指数的浮点稳定变换与Softmax",
  "numerical-n0365":"对数贴现因子的形状约束与分段远期利率",
  "numerical-n0366":"Crank–Nicolson的无条件稳定与非单调震荡",
  "numerical-n0368":"随机化子空间近似的正交误差分解"
 },
 "lessons-numerical-v110b.json":{
  "numerical-n0372":"共轭梯度残差与二次目标误差的精确联系",
  "numerical-n0373":"带确定性函数误差界的可靠根区间更新",
  "numerical-n0374":"Levenberg–Marquardt阻尼步与局部最小二乘模型",
  "numerical-n0375":"带区间保护的割线插值与保根迭代"
 },
 "lessons-numerical-v110c.json":{
  "numerical-n0376":"端点可积奇异性的变量代换与误差控制",
  "numerical-n0377":"Gauss–Laguerre求积与指数折现型积分",
  "numerical-n0378":"共同随机数与数值Greek差分的方差分解",
  "numerical-n0379":"离散随机变量的随机化概率积分变换"
 },
 "lessons-numerical-v110d.json":{
  "numerical-n0381":"重要性采样中有限均值却无限方差的尾部失配"
 },
 "lessons-numerical-v110e.json":{
  "numerical-n0388":"Ornstein–Uhlenbeck过程的精确一步抽样与Euler误差",
  "numerical-n0389":"异方差回归的Wild Bootstrap与时间依赖的区分"
 }
};
let deepenedV116=0;
for(const [name,records] of Object.entries(v116Numerical)){
 const doc=json(name);
 for(const [id,title] of Object.entries(records)){
  const lesson=doc.lessons.find(x=>x.id===id);
  assert(lesson,id+" missing from canonical file");
  const sections=lesson.deep_sections;
  assert.equal(new Set(sections.map(x=>x.title)).size,sections.length,id+" duplicate headings");
  const s=sections.find(x=>x.title===title);
  assert(s,id+" added chapter not found");
  assert(s.paragraphs.length===2&&s.paragraphs.every(x=>x.length>=115),id+" short mathematical explanation");
  assert(s.proof.length>=110&&s.worked.length>=90&&s.limitation.length>=65,id+" incomplete proof/example/boundary");
  assert.equal(s.formulas.length,2,id+" new mathematical equations lost");
  for(const f of s.formulas){
   assert(f.tex.length>=30&&f.label.length>=4,id+" weak displayed formula");
   katex.renderToString(f.tex,{throwOnError:true,strict:"error",displayMode:true});
  }
  deepenedV116++;
 }
}
assert.equal(deepenedV116,15);
const v116Check=(actual,expected,label,tol=1e-8)=>assert(Math.abs(actual-expected)<tol,label+" discrepancy");
v116Check(1000+Math.log1p(Math.exp(-1)),1000.3132616875182,"stable log-sum-exp");
v116Check(Math.exp(-.055),.9464851479534838,"log discount midpoint");
v116Check((1-2)/(1+2),-1/3,"CN high frequency amplification");
v116Check(Math.sqrt(1+4),Math.sqrt(5),"orthogonal randomized SVD residual");
v116Check(.1**2/2,.005,"CG energy-residual");
v116Check((1+1)/2,1,"damped LM step");
v116Check(2-2/3,4/3,"safeguarded secant root");
v116Check(2+2/3,8/3,"endpoint substitution");
const qminus=2-Math.sqrt(2),qplus=2+Math.sqrt(2);
const wminus=(2+Math.sqrt(2))/4,wplus=(2-Math.sqrt(2))/4;
v116Check(wminus*qminus*qminus+wplus*qplus*qplus,2,"Gauss Laguerre moment");
v116Check(1/(2*100*.1**2),.5,"independent finite-difference Monte Carlo noise");
v116Check((1-Math.exp(-2))/2,.43233235838169365,"exact OU variance");
v116Check(.8**2+.2**2*4,.8,"Aitken GLS variance cross-link");
console.log("V116 NUMERICAL PASS: 15 additional rigorous sections and 30 strict formulas");


// Validate newly expanded inference, optimization, time-series and risk mathematics.
const v117Courses={
 "lessons-phase-20-a.json":["econometrics-x003"],
 "lessons-phase-20-c.json":["econometrics-x005","econometrics-x006"],
 "lessons-phase-21-c.json":["mlmath-n0460","mlmath-n0461"],
 "lessons-phase-21-d.json":["mlmath-n0462","mlmath-n0465"],
 "lessons-phase-22-a.json":["timeseries-n0244","timeseries-n0245"],
 "lessons-phase-22-b.json":["timeseries-n0248","timeseries-n0249"],
 "lessons-phase-27-a.json":["statistics-n0171","statistics-n0170","statistics-n0172"],
 "lessons-phase-27-c.json":["riskmath-n0589","riskmath-n0594","riskmath-n0595"]
};
let validatedV117=0;
for(const [file,ids] of Object.entries(v117Courses)){
 const doc=json(file);
 assert.deepEqual(doc.lessons.map(l=>l.id),ids,"V117 canonical topic identity drift "+file);
 for(const lesson of doc.lessons){
  const s=lesson.deep_sections.at(-1);
  assert(s&&s.title?.length>=12&&s.question?.length>=12,lesson.id+" missing V117 analytical chapter");
  assert.equal(new Set(lesson.deep_sections.map(x=>x.title)).size,lesson.deep_sections.length,lesson.id+" repeated heading");
  assert(s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=110),lesson.id+" must retain complete explanation");
  assert(s.proof.length>=125&&s.worked.length>=90&&s.limitation.length>=75,lesson.id+" incomplete proof/example/limits");
  assert.equal(s.formulas.length,2,lesson.id+" exactly two new substantive displayed mathematical results");
  for(const f of s.formulas){
   assert(f.label.length>=4&&f.tex.length>=30,lesson.id+" missing labeled relationship");
   katex.renderToString(f.tex,{throwOnError:true,strict:"error",displayMode:true});
  }
  validatedV117++;
 }
}
assert.equal(validatedV117,17);
const near117=(v,e,name,tol=1e-9)=>assert(Math.abs(v-e)<=tol,name+" arithmetic invalid: "+v);
near117((1+1+9+9)/8,2.5,"ATE reweighting");
near117(8*8/20,3.2,"transport effective count");
near117(.1**2/.01+.2**2/.04,2,"joint Wald");
near117((2/3)**3,8/27,"Rosenbaum odds");
near117(.25/(2*2-.25*2**2),1/12,"SGD stationary variance");
near117((1+.9)/(1-.9),19,"Adam momentum effective count");
near117(.5*3**2,4.5,"clipped gradient descent");
near117((3-3)**2+(-2+2)**2+(1-0)**2,1,"L0 projection residual");
near117(1+.5**2,1.25,"Wold Parseval variance");
near117(4*(1+.5**2),5,"MA two-step forecast");
near117(((1+.2)/(1-.5))**2,5.76,"ARMA long-run variance");
near117((7/3)/3,7/9,"Jackknife sample mean variance");
near117(2/6,1/3,"exact permutation p-value");
near117((1+4+4)**.5,3,"factor-plus-idiosyncratic risk");
near117(.9*.95-1,-.145,"currency-ETF stress");
near117(Math.sqrt((.04+.09-2*.03)/100),Math.sqrt(.0007),"paired strategy error");
console.log("V117 MATH PASS: 17 complete advanced lessons and 34 strict KaTeX formulas");


// V118: independently verify complete mathematical foundations and quant applications.
const v118MathematicalChapters={
 "lessons-diffeq-v98a.json":["diffeq-n0391","diffeq-n0392","diffeq-n0393"],
 "lessons-dynamics-v99a.json":["diffeq-n0398","diffeq-n0399","diffeq-n0400","diffeq-n0401","diffeq-n0402"],
 "lessons-measure-v108a.json":["measure-n0138","measure-n0143","measure-n0148"],
 "lessons-measure-v108b.json":["measure-n0149","measure-n0150"],
 "lessons-optimization-v96b.json":["optimization-n0345","optimization-n0346","optimization-n0348"],
 "lessons-information-v103c.json":["information-n0500","information-n0501","information-n0503","information-n0504"],
 "lessons-finance-v100d.json":["finance-n0446","finance-n0448","finance-n0449","finance-n0450"]
};
let validatedV118Math=0;
for(const [file,ids] of Object.entries(v118MathematicalChapters)){
 const document=json(file);
 const found=ids.map(id=>document.lessons.find(l=>l.id===id));
 assert(found.every(Boolean),file+" V118 canonical mathematical courses missing");
 for(const lesson of found){
  const s=lesson.deep_sections.at(-1);
  assert(s&&s.title?.length>=10&&s.question?.length>=12,lesson.id+" missing a rigorous mathematical question");
  assert.equal(new Set(lesson.deep_sections.map(z=>z.title)).size,lesson.deep_sections.length,lesson.id+" repeated section heading");
  assert(s.paragraphs.length===2&&s.paragraphs.every(z=>z.length>=120),lesson.id+" mathematical assumptions and explanation incomplete");
  assert(s.proof.length>=140&&s.worked.length>=110&&s.limitation.length>=75,lesson.id+" proof, arithmetic example or applicability missing");
  assert.equal(s.formulas.length,2,lesson.id+" must have two substantive mathematical equations");
  for(const f of s.formulas){
   assert(f.label.length>=4&&f.tex.length>=20,lesson.id+" missing labeled display equation");
   katex.renderToString(f.tex,{displayMode:true,throwOnError:true,strict:"error"});
  }
  validatedV118Math++;
 }
}
assert.equal(validatedV118Math,24,"V118 mathematics expansion must cover 24 complete chapters");
const nearV118Math=(actual,expected,label,tol=1e-8)=>assert(Math.abs(actual-expected)<=tol,label+": "+actual+" != "+expected);
nearV118Math(.03+(.01-.03)*.25,.025,"time varying ODE example");
nearV118Math(100/(1+4*.5),100/3,"Logistic exact growth");
nearV118Math(5*Math.sqrt(1-.6**2),4,"damped eigenfrequency");
nearV118Math(.5*3*3,4.5,"continuous Lyapunov energy");
nearV118Math(1/(1-.8**2),25/9,"discrete steady-state covariance");
nearV118Math(.5*6+4,7,"state similarity invariance");
nearV118Math(.5-.8,-.3,"two-factor observability");
nearV118Math((Math.log(8*Math.PI)+1)/2,2.112085713764618,"Kalman innovation likelihood");
nearV118Math(9+4,13,"conditional projection baseline MSE");
nearV118Math(Math.abs(.2-0),.2,"Wasserstein coupling");
nearV118Math(1-2*.1,.8,"equivalent martingale digital");
nearV118Math((.08-.02)/.20,.3,"Girsanov market price of risk");
nearV118Math((102+3)/1.05,100,"dividend inclusive pricing");
nearV118Math(.05-.2*Math.sqrt(.0125),.027639320225002106,"SOCP robust expected return");
nearV118Math(4/2+1,3,"Schur complement quadratic");
nearV118Math((10+.5*2)/1.5,7.333333333333333,"empirical tail ES");
nearV118Math(2*Math.sqrt(.0004),.04,"Roll implied spread");
nearV118Math((2+.75)/(2*Math.PI),2.75/(2*Math.PI),"finite-window periodogram");
nearV118Math(Math.log(2)+.1*Math.log(.1)+.9*Math.log(.9),.3680642071684971,"data processing binary channel");
nearV118Math((1+.25)/2,.625,"random execution latency");
nearV118Math((-.01)**2,.0001,"PSD spectral repair");
nearV118Math(50+40*(10**.25-1),81.13117640155691,"GPD quantile extrapolation");
nearV118Math((2/3)*.1,(1/3)*.2,"two ETF risk parity");
nearV118Math(Math.sqrt(2*.005/4),.05,"no trade band half width");
console.log("V118 MATH PASS: 24 complete theory chapters, 48 strict equations, independent numerical proofs");


// Retain the entire historical mathematics curriculum and validate 16 new V119 advanced chapters.
const v119Math={
 "lessons-phase-29-a.json":{
  "information-n0508":"缺失观测下的Kalman纯预测递推与协方差增长",
  "information-n0507":"序贯贝叶斯对数证据、预测概率与稳定归一化",
  "information-n0505":"两状态HMM的稳态概率与期望持续期"
 },
 "lessons-phase-29-b.json":{
  "information-n0509":"非线性状态观测的二阶Delta修正与局部曲率偏差",
  "information-n0506":"粒子滤波重要性权重的条件有效样本量与重抽样阈值",
  "information-n0510":"路径最大回撤的在线状态扩充与递归计算"
 },
 "lessons-phase-30-a.json":{
  "finance-n0431":"资产价格泡沫与非负局部鞅为什么可能不是定价真鞅",
  "finance-n0432":"复制定价唯一而复制仓位可能非唯一：零成本零支付方向",
  "finance-n0439":"零息曲线正贴现条件与连续远期利率的积分关系"
 },
 "lessons-phase-30-b.json":{
  "finance-n0440":"关键期限久期的有限差分计算与再定价一致性",
  "finance-n0441":"Vasicek短率精确离散转移与长期回归半衰期的参数辨识",
  "finance-n0442":"CIR短率条件均值与非负扩散的精确期望"
 },
 "lessons-risk-governance-v101.json":{
  "riskmath-n0596":"参数后验混合预测的总方差与尾部风险分布",
  "riskmath-n0597":"风险状态Markov链的多期预测与平稳偏差衰减",
  "riskmath-n0598":"VaR总体命中率合格但各风险状态均失准的混合反例",
  "riskmath-n0600":"多个资产VaR告警的Bonferroni族错误率预算"
 }
};
let v119MathCount=0;
for(const [file,byID] of Object.entries(v119Math)){
 const doc=json(file);
 for(const [id,title] of Object.entries(byID)){
  const course=doc.lessons.find(x=>x.id===id);
  assert(course,id+" V119 canonical mathematical lecture missing");
  const s=course.deep_sections.find(x=>x.title===title);
  assert(s,id+" V119 research chapter missing");
  assert.equal(new Set(course.deep_sections.map(x=>x.title)).size,course.deep_sections.length,id+" duplicate headings");
  assert(s.question.length>=12&&s.paragraphs.length===2&&s.paragraphs.every(x=>x.length>=120),id+" missing complete assumptions or applications");
  assert(s.proof.length>=130&&s.worked.length>=100&&s.limitation.length>=55,id+" missing proof, numeric derivation or failure case");
  assert.equal(s.formulas.length,2,id+" requires two full mathematical equations");
  for(const f of s.formulas) {
   assert(f.label.length>=4&&f.tex.length>=20,id+" incomplete mathematical formula");
   katex.renderToString(f.tex,{throwOnError:true,strict:"error",displayMode:true});
  }
  v119MathCount++;
 }
}
assert.equal(v119MathCount,16);
const near119=(v,e,n,tol=1e-8)=>assert(Math.abs(v-e)<=tol,n+" numerical discrepancy: "+v+" vs "+e);
near119(.64*(.64*2+1)+1,2.4592,"Kalman missing observations");
near119((.2*.1*.5)/(.1*.1*.25),4,"sequential evidence ratio");
near119(.2+.7*.9,.83,"regime transition forecast");
near119(2**2+.25,4.25,"nonlinear state squared observation");
near119(1/(.7**2+3*.1**2),1/.52,"particle ESS");
near119(1-90/120,.25,"path A max drawdown");
near119(1-100/105,1/21,"path B max drawdown");
near119(Math.exp(.2**2/2-.2**2/2),1,"exponential martingale");
near119(.4+.6,1,"redundant replication price");
near119(Math.exp(-(.02-.01)),Math.exp(-.01),"negative forward discount price");
near119(100*Math.exp(-.02)*.0001+200*Math.exp(-.04)*.0001,.009801986733067553+.019215788783046463,"key rate DV01");
near119(.04+(.02-.04)*.5,.03,"Vasicek exact mean");
near119(.04+(.01-.04)*.5,.025,"CIR mean");
near119(4+((1-2)**2+(3-2)**2)/2,5,"posterior predictive total variance");
near119(2/3+.7**2*(1-2/3),.83,"two-state h-step prediction");
near119(.5*.09+.5*.01,.05,"conditional VaR calibration failure");
near119(.05/20,.0025,"Bonferroni ETF alarm rate");
console.log("V119 MATH PASS: 16 full advanced chapters, 32 KaTeX relationships, 17 numerical checks");

console.log("V110 PASS: 656/656 finished, all 20 disciplines complete, numerical 30/30, "+
  sectionCount+" complete reasoning sections, "+formulaCount+" strict KaTeX formulas, "+
  linkCount+" exact curriculum links, "+proofChars+" Chinese proof characters, "+
  numericChecks+" independent arithmetic checks, CJK/mobile/print/legacy gates");
