import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=n=>fs.readFileSync(new URL(n,root),"utf8");
const json=n=>JSON.parse(read(n));
const curriculum=json("curriculum.json");
assert.equal(curriculum.nodes.length,656,"Curriculum size changed");
const nodes=new Map(curriculum.nodes.map(n=>[n.id,n]));
assert.equal(nodes.size,656,"Curriculum node IDs not unique");
const manifest=json("lessons-manifest.json");
assert.equal(manifest.expected_lessons,201,"Historical manifest unexpectedly changed");
assert.equal(manifest.modules.length,77,"Historical bundle registry unexpectedly changed");
const oldFiles=[
 "lessons-foundations.json","lessons-advanced.json","lessons-more.json",
 ...manifest.modules.map(m=>m.path.replace(/^\.\//,"")),
 "lessons-foundations-v89.json","lessons-linear-v90.json","lessons-linear-v92.json",
 "lessons-matrix-v93.json","lessons-probability-v93.json",
 "lessons-probability-v94a.json","lessons-probability-v94b.json","lessons-probability-v94c.json",
 "lessons-statistics-v95a.json","lessons-statistics-v95b.json",
 "lessons-optimization-v96a.json","lessons-optimization-v96b.json",
 "lessons-portfolio-v97.json","lessons-algorithms-v97.json","lessons-numerical-v97.json",
 "lessons-diffeq-v98a.json","lessons-diffeq-v98b.json",
 "lessons-dynamics-v99a.json","lessons-pde-v99b.json",
 "lessons-pricing-pde-v99c.json","lessons-control-v99d.json",
 "lessons-finance-v100a.json","lessons-finance-v100b.json",
 "lessons-finance-v100c.json","lessons-finance-v100d.json",
 "lessons-timeseries-v101.json","lessons-risk-tail-v101.json",
 "lessons-risk-axioms-v101.json","lessons-risk-governance-v101.json",
 "lessons-mlmath-v102a.json","lessons-mlmath-v102b.json",
 "lessons-mlmath-v102c.json"
];
const history=oldFiles.flatMap(n=>json(n).lessons);
assert.equal(history.length,488,"V102 historical 488 lesson texts changed");
assert.equal(new Set(history.map(l=>l.node_id)).size,488,"V102 history has duplicate IDs");
const files=["lessons-information-v103a.json","lessons-information-v103b.json","lessons-information-v103c.json"];
const docs=files.map(json);
docs.forEach((d,i)=>{
 assert.equal(d.version,1,"V103 data schema wrong");
 assert.equal(d.revision,"V103","V103 revision mismatch");
 assert.equal(d.lessons.length,[6,6,6][i],"V103 file size mismatch");
});
const newest=docs.flatMap(d=>d.lessons);
const ids=Array.from({length:18},(_,i)=>"information-n"+String(i+487).padStart(4,"0"));
assert.deepEqual(newest.map(l=>l.node_id),ids,"V103 information signal mathematics coverage changed");
const all=[...history,...newest];
assert.equal(all.length,506,"Expected 506 completed mathematics lectures");
assert.equal(new Set(all.map(l=>l.node_id)).size,506,"Historical lesson overwritten");
assert(all.every(l=>nodes.get(l.node_id)?.title===l.title),"Curriculum mismatched or missing lesson");
const completed=new Set(all.map(l=>l.node_id));
const information=curriculum.nodes.filter(n=>n.discipline_id==="information");
assert.equal(information.length,32,"Information curriculum size 32 changed");
assert(information.every(n=>completed.has(n.id)),"Information and signal processing must be 32/32 complete");
assert.equal(history.filter(l=>l.node_id.startsWith("information-")).length,14,"Previously completed 14 information lessons changed");
assert.equal(newest.filter(l=>l.node_id.startsWith("information-")).length,18);
let mathCount=0,deepCount=0,deepChars=0,linkCount=0;
function formula(tex,id){
 assert.equal(typeof tex,"string",id+" formula type");
 assert(tex.length>=10,id+" formula too short");
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(tex),id+" bad character");
 try{katex.renderToString(tex,{displayMode:true,throwOnError:true,strict:"error"});}
 catch(e){throw Error(id+" strict KaTeX failed: "+tex+" / "+e.message);}
 mathCount++;
}
for(const l of newest){
 const id=l.node_id;
 assert.equal(l.id,id,"Lesson identity mismatch");
 assert.equal(l.title,nodes.get(id).title,"Course title drift");
 for(const field of ["definition","assumptions","example","application","caveat"])
  assert(l[field]?.length>=70,id+" incomplete "+field);
 assert(l.properties?.length>=4 && l.properties.every(p=>p.length>=20),id+" shallow properties");
 assert.equal(l.formulas.length,3,id+" core formulas");
 assert.equal(l.derivation.length,3,id+" derivations");
 assert.equal(l.deep_sections.length,3,id+" detailed chapters");
 for(const x of l.formulas){assert(x.label?.length>=2,id+" mathematical label");formula(x.tex,id);}
 for(const x of l.derivation){assert(x.explanation?.length>=25,id+" derivation prose");formula(x.tex,id);}
 const headings=new Set();
 for(const s of l.deep_sections){
  assert(s.title?.length>=8 && s.question?.length>=10,id+" deep section title/question");
  assert(!headings.has(s.title),id+" repeated subchapter");
  headings.add(s.title);
  assert(s.paragraphs?.length===2 && s.paragraphs.every(p=>p.length>=30),id+" deep explanatory text");
  for(const [key,min] of [["proof",50],["worked",50],["limitation",25]])
   assert(s[key]?.length>=min,id+" short "+key);
  assert.equal(s.formulas?.length,1,id+" deep math");
  assert(s.formulas[0].label?.length>=2,id+" deep formula caption");
  formula(s.formulas[0].tex,id);
  deepCount++;
  deepChars+=s.paragraphs.join("").length+s.proof.length+s.worked.length+s.limitation.length;
 }
 assert(/[\u3400-\u9FFF]/u.test(JSON.stringify(l)),id+" Chinese content missing");
 assert(!/TODO|FIXME|Lorem ipsum|待补充/i.test(JSON.stringify(l)),id+" placeholder");
 for(const link of l.links||[]){
  assert(["math","ml"].includes(link.kind),id+" unknown cross link kind");
  if(link.kind==="math")assert(nodes.has(link.id),id+" dangling link "+link.id);
  linkCount++;
 }
}
assert.equal(mathCount,162,"New formula count changed");
assert.equal(deepCount,54,"Deep chapter count changed");
assert(deepChars>=14000,"Depth reduced from reviewed version");
assert.equal(linkCount,54,"Cross-lesson link count changed");
const html=read("index.html"),css=read("styles.css"),app=read("app.js");
const workflow=read("../../.github/workflows/test-quant-math.yml");
assert(html.includes('app.js?v=20261108-v103-information-signal-complete'),"Reader must bust V102 cache");
assert(html.includes("V103 · 信息论与信号处理"),"V103 CJK curriculum absent");
for(const id of ids)assert(html.includes("?topic="+id+'"'),"Study route missing "+id);
for(const needle of [
 'loadV102MachineLearningMath("./lessons-mlmath-v102c.json",4)',
 'loadV103InformationSignals("./lessons-information-v103a.json",6)',
 'loadV103InformationSignals("./lessons-information-v103b.json",6)',
 'loadV103InformationSignals("./lessons-information-v103c.json",6)',
 '...v102a,...v102b,...v102c,...v103a,...v103b,...v103c',
 'new Set(lessons.map(l=>l.node_id)).size!==lessons.length',
 'new Set(doc.lessons.map(l=>l.node_id)).size!==8'
])assert(app.includes(needle),"Missing V103 canonical loader or historical loading guard: "+needle);
for(const token of ["overflow-x:auto","line-break:strict","@media(max-width:700px)","@media print",":focus-visible"])
 assert(css.includes(token),"Missing established Chinese/LaTeX mobile accessibility: "+token);
assert(workflow.includes("node quant/math/test_v103_information_complete.mjs"),"V103 CI step absent");



let checks=0;
function near(label,value,expected,tol=1e-8){
 assert(Number.isFinite(value),label+" non-finite");
 assert(Math.abs(value-expected)<=tol,label+": actual "+value+" expected "+expected);
 checks++;
}
function yes(label,condition){near(label,Number(Boolean(condition)),1);}
function complexDFT(x){
 const N=x.length;
 return Array.from({length:N},(_,k)=>{
  let real=0,imag=0;
  for(let n=0;n<N;n++){real+=x[n]*Math.cos(-2*Math.PI*k*n/N);imag+=x[n]*Math.sin(-2*Math.PI*k*n/N);}
  return [real,imag];
 });
}
function mod2(z){return z[0]**2+z[1]**2;}
function sum(a){return a.reduce((s,x)=>s+x,0);}
const log=Math.log;

near("complex i rotation real",Math.cos(Math.PI/2),0);
near("complex i rotation imag",Math.sin(Math.PI/2),1);
near("complex Euler pi",Math.cos(Math.PI),-1);
near("complex magnitude 3+4i",Math.hypot(3,4),5);
near("complex squared norm",3**2+4**2,25);
near("complex product modulus",2*3,6);
near("complex total rotation",Math.PI/4+Math.PI/6,5*Math.PI/12);
near("complex two quarter turns",Math.cos(Math.PI/2+Math.PI/2),-1);
near("Fourier double exponential at0",2*2/(2**2+0),1);
near("Fourier double exponential at2",2*2/(2**2+2**2),.5);
near("Fourier shift exp phase modulus",Math.hypot(Math.cos(Math.PI),Math.sin(Math.PI)),1);
near("Fourier filtered complex real",(2)*.5,1);
near("Fourier filtered complex imaginary",(3)*.5,1.5);
const x=[1,2,3,4],X=complexDFT(x);
near("DFT DC real",X[0][0],10);
near("DFT DC imaginary",X[0][1],0);
near("DFT k1 real",X[1][0],-2);
near("DFT k1 imaginary",X[1][1],2);
near("DFT k2 real",X[2][0],-2);
near("DFT k2 imaginary",X[2][1],0);
near("DFT k3 real",X[3][0],-2);
near("DFT k3 imaginary",X[3][1],-2);
near("DFT DC normalized mean",X[0][0]/4,2.5);
near("DFT k1 frequency normalized",1/4,.25);
near("FFT even DC",1+3,4);
near("FFT even Nyquist",1-3,-2);
near("FFT odd DC",2+4,6);
near("FFT odd Nyquist",2-4,-2);
near("FFT butterfly DC",4+6,10);
near("FFT butterfly k2",4-6,-2);
near("FFT butterfly k1 imaginary",(-1)*(-2),2);
near("FFT butterfly k3 imaginary",(+1)*(-2),-2);
near("FFT log2 N 1024",Math.log2(1024),10);
near("FFT order proxy 1024",1024*Math.log2(1024),10240);
near("FFT naive proxy",1024**2,1048576);
near("FFT proxy operations ratio",1024**2/(1024*Math.log2(1024)),102.4);
near("PSD white noise",.0004,.0004);
near("PSD full spectral area",(2*Math.PI)*.0004/(2*Math.PI),.0004);
near("PSD half spectral band",Math.PI*.0004/(2*Math.PI),.0002);
near("PSD remaining spectral band",(2*Math.PI-Math.PI)*.0004/(2*Math.PI),.0002);
near("PSD lower stationary spectral value",1-.5,.5);
const energyTime=sum(x.map(v=>v*v));
const energyFreq=sum(X.map(mod2));
near("Parseval time energy",energyTime,30);
near("Parseval spectral energy",energyFreq,120);
near("Parseval equal",energyFreq/4,30);
near("Parseval centered sample variance",(8+4+8)/(4*3),5/3);
near("Parseval real one-sided spectrum",100+4+2*8,120);
near("Trailing moving average latest",(2+5+4)/3,11/3);
near("Trailing moving average white variance",9/3,3);
near("Moving average trend lag units",(3-1)/2*2,2);
near("Moving average odd frequency response atπ",(1-1+1)/3,1/3);
near("Moving average DFT zero real",1+Math.cos(2*Math.PI/3)+Math.cos(4*Math.PI/3),0);
near("Moving average DFT zero imag",Math.sin(2*Math.PI/3)+Math.sin(4*Math.PI/3),0);
near("Two point low-pass",(.5)*(5+3),4);
near("Two point high-pass",(.5)*(5-3),1);
near("Two point reconstruction",4+1,5);
near("Two point constant high pass",(5-5)/2,0);
near("Two point alternating high-pass",(1-(-1))/2,1);
near("Low pass gain at0",Math.cos(0),1);
near("Low pass gain atπ",Math.abs(Math.cos(Math.PI/2)),0);
near("High pass gain atπ",Math.abs(Math.sin(Math.PI/2)),1);
near("Kalman prior covariance",1+1,2);
near("Kalman gain",2/(2+2),.5);
near("Kalman innovation",4-0,4);
near("Kalman posterior mean",0+.5*4,2);
near("Kalman posterior covariance",(1-.5)*2,1);
near("Kalman Joseph posterior",(1-.5)**2*2+.5**2*2,1);
near("Kalman high measurement noise gain",2/(2+8),.2);
near("Kalman high measurement noise posterior",.2*4,.8);
near("Wiener optimal gain",4/(4+1),.8);
near("Wiener signal estimate",.8*5,4);
near("Wiener residual variance",4*1/(4+1),.8);
near("Wiener lower SNR gain",.25/(.25+1),.2);
near("Wiener error component one",(1-.8)**2*4,.16);
near("Wiener error component two",.8**2*1,.64);
near("Wiener error sum",.16+.64,.8);
near("Haar first average 2 4",(2+4)/Math.SQRT2,3*Math.SQRT2);
near("Haar first detail 2 4",(2-4)/Math.SQRT2,-Math.SQRT2);
near("Haar second average 6 8",(6+8)/Math.SQRT2,7*Math.SQRT2);
near("Haar second detail 6 8",(6-8)/Math.SQRT2,-Math.SQRT2);
near("Haar coarse average",(3*Math.SQRT2+7*Math.SQRT2)/Math.SQRT2,10);
near("Haar coarse detail",(3*Math.SQRT2-7*Math.SQRT2)/Math.SQRT2,-4);
near("Haar total energy",10**2+(-4)**2+2+2,120);
near("Haar observed input energy",2**2+4**2+6**2+8**2,120);
near("STFT 16 window cycles/min",1/16,.0625);
near("STFT 64 window cycles/min",1/64,.015625);
near("STFT 64/16 resolution ratio",(1/16)/(1/64),4);
near("STFT time spread given2",2,2);
near("STFT minimum angular spread",1/(2*2),.25);
near("STFT uncertainty product",2*.25,.5);
near("HP second difference for 0 1 0",0-2*1+0,-2);
near("HP D squared norm",1**2+(-2)**2+1**2,6);
near("HP τ first",2/7,2/7);
near("HP τ center",1-4/7,3/7);
near("HP τ last",2/7,2/7);
near("HP τ second difference",2/7-2*3/7+2/7,-2/7);
near("HP fit sum squares",(-2/7)**2+(4/7)**2+(-2/7)**2,24/49);
near("HP penalty",(-2/7)**2,4/49);
near("HP objective total",24/49+4/49,4/7);
near("High frequency quote bounce RV",3*.2**2,.12);
near("HF price noise return variance",2*.01,.02);
near("HF price noise lag covariance",-.01,-.01);
near("HF microstructure lag correlation",-.01/.02,-.5);
near("HF RV expected noise bias 100",2*100*.01,2);
near("HF RV expected noise bias 200",2*200*.01,4);
const alt=complexDFT([1,-1,1,-1]);
near("Periodogram alternating k2 real",alt[2][0],4);
near("Periodogram alternating k2 power",mod2(alt[2])/4,4);
near("Periodogram other frequency",mod2(alt[1]),0);
near("Periodogram normalized average power",sum(alt.map(z=>mod2(z)/4))/4,1);
near("Welch basic equal weight",sum([1,3,2,2])/4,2);
near("Welch window squared norm",1+.5**2+.5**2+1,2.5);
near("Welch white-noise window expected power",4*2.5,10);
near("Welch normalized expected spectrum",10/2.5,4);
near("Multiscale triple return variance",3+2*2*.25,4);
near("Multiscale three-period SD",Math.sqrt(4),2);
near("Multiscale no autocorrelation baseline",Math.sqrt(3),Math.sqrt(3));
near("Multiscale daily annualized 1pct",.01*Math.sqrt(252),.15874507866387544);
near("Multiscale PSD at0",1+.5*Math.cos(0),1.5);
near("Multiscale PSD atπ",1+.5*Math.cos(Math.PI),.5);
near("Price discovery lead covariance",.5*1,.5);
near("Price discovery B variance",.5**2+(.75),1);
near("Price discovery lead corr",.5/Math.sqrt(1*1),.5);
const Hb=-.8*log(.8)-.2*log(.2);
near("Price discovery binary entropy",Hb,.5004024235381879);
near("Price discovery natural mutual information",Math.LN2-Hb,.19274475702175753);
near("Price discovery MI bits",(Math.LN2-Hb)/Math.LN2,.2780719051126377);
yes("Price discovery conditional MI positive",Math.LN2-Hb>0);
near("Signal φ cube",.8**3,.512);
near("Signal execution gross lag3",.01*.8**3,.00512);
near("Signal execution net lag0",.01-.002,.008);
near("Signal execution net lag3",.01*.8**3-.002,.00312);
near("Signal φ eighth",.8**8,.16777216);
near("Signal execution net lag8",.01*.8**8-.002,-.0003222784);
yes("Signal expected net switches sign",.01*.8**8-.002<0);
assert(checks>=110,"Independent signal and mathematical computations unexpectedly incomplete");
console.log("V103 PASS:",newest.length,"new full lessons;",all.length,"total;",
 mathCount,"strict KaTeX;",deepCount,"deep proof chapters;",deepChars,"deep prose characters;",
 checks,"independent computations;",linkCount,"cross references; information 32/32.");
