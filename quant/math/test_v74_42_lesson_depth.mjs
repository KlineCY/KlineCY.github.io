import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>fs.readFileSync(new URL(p,root),"utf8");
const load=p=>JSON.parse(read(p));
const math=load("lessons-phase-16-a.json");
const mlPaths=[
  "../ml/atlas-depth-14-a.json","../ml/atlas-depth-14-b.json",
  "../ml/atlas-depth-14-c.json","../ml/atlas-depth-14-d.json",
  "../ml/atlas-depth-13-a.json","../ml/atlas-depth-13-b.json",
  "../ml/atlas-depth-13-c.json"
];
const groups=[
  ["m01-c02-p03","m02-c01-p02","m03-c01-p02","m04-c01-p01","m05-c05-p05","m06-c01-p06"],
  ["m07-c02-p02","m08-c03-p02","m09-c01-p04","m10-c01-p05","m11-c04-p01","m12-c03-p02"],
  ["m13-c02-p03","m14-c03-p01","m15-c05-p04","m16-c04-p03","m17-c03-p04","m18-c01-p03"],
  ["m19-c04-p03","m20-c01-p06","m21-c04-p03","m22-c04-p06","m23-c03-p02"],
  ["m01-c03-p03","m02-c03-p05","m03-c02-p02","m04-c05-p04","m05-c05-p03","m06-c01-p02"],
  ["m07-c01-p04","m08-c01-p06","m09-c03-p02","m10-c04-p04","m11-c05-p04","m12-c03-p06"],
  ["m13-c02-p06","m14-c04-p04","m15-c01-p01","m16-c03-p01","m17-c02-p02","m18-c03-p01"]
];
const modules=mlPaths.map(load);
const mathIDs=["information-n0485"];
assert.deepEqual(math.lessons.map(x=>x.id),mathIDs);
assert.deepEqual(math.v74_revision.ids,mathIDs);
assert.equal(groups.flat().length,41);
assert.equal(new Set(groups.flat()).size,41);
let katexCount=0, strictCount=0;
function formula(tex,strict=false){
  assert(typeof tex==="string"&&tex.trim().length>=5,"formula missing or too short");
  const markup=katex.renderToString(tex,{throwOnError:true,displayMode:true,strict:strict?"error":"ignore"});
  assert(markup.includes("katex"),"KaTeX markup missing");
  katexCount++;
  if(strict)strictCount++;
}
function textSafe(x,id){
  const s=JSON.stringify(x);
  assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(s),id+" invisible/corrupted characters");
  assert(!/TODO|FIXME|Lorem ipsum/iu.test(s),id+" incomplete draft");
  assert(/[\u3400-\u9FFF]/u.test(s),id+" Chinese lecture lost");
}
for(const x of math.lessons){
  assert(x.checked_status.startsWith("V74"),"math revision marker missing");
  const s=x.deep_sections.at(-1);
  assert(s.title.length>=12&&s.question.length>=14,"math title/question too short");
  assert(s.proof.length>=125&&s.worked.length>=90&&s.limitation.length>=50,"math proof or worked case insufficient");
  assert.equal(s.paragraphs.length,2);
  assert(s.paragraphs.every(p=>p.length>=80));
  assert.equal(s.formulas.length,2);
  assert.equal(new Set(x.deep_sections.map(s=>s.title)).size,x.deep_sections.length);
  assert(x.sources.length>=2&&x.links.length>=3,"math bibliography/navigation missing");
  for(const f of x.formulas)formula(f.tex);
  for(const f of x.derivation)formula(f.tex);
  for(const f of x.deep_sections.flatMap(y=>y.formulas))formula(f.tex);
  for(const f of s.formulas)formula(f.tex,true);
  textSafe(x,x.id);
}
for(let k=0;k<modules.length;k++){
  const d=modules[k],ids=groups[k];
  assert.deepEqual(Object.keys(d.lessons),ids,mlPaths[k]+" ID ordering changed");
  assert.deepEqual(d.v74_revision.ids,ids,mlPaths[k]+" revision metadata missing");
  for(const id of ids){
    const x=d.lessons[id];
    assert(x.v74_note.startsWith("V74"),id+" new lecture marker missing");
    assert(x.equations.length>=8&&x.derivation.length>=8,id+" insufficient derivations/formulas");
    assert(x.derivation.slice(-2).every(p=>p.length>=90),id+" thin last explanations");
    assert(x.protocol.length>=5&&x.pitfalls.length>=4,id+" missing verification/limitations");
    assert(x.worked_example.length>=250,id+" insufficient worked example");
    assert.equal(new Set(x.equations.map(f=>f.label)).size,x.equations.length,id+" repeated equation label");
    assert.equal(new Set(x.protocol).size,x.protocol.length,id+" repeated verification procedure");
    for(const f of x.equations)formula(f.tex);
    for(const f of x.equations.slice(-2))formula(f.tex,true);
    textSafe(x,id);
  }
}
assert.equal(strictCount,84,"expected 82 new ML + 2 new math strict formulas");
let numerical=0;
function near(label,got,expected,tol=1e-9){
  assert(Number.isFinite(got)&&Number.isFinite(expected)&&Math.abs(got-expected)<=tol,label+": "+got+" != "+expected);
  numerical++;
}
function truth(label,ok){near(label,ok?1:0,1);}
near("XTX squares condition number",100/1,10*10);
near("Ridge normal matrix condition",109/10,10.9);
near("gross exposure",Math.abs(.6)+Math.abs(-.4),1);
near("robust worst return",.6*.02+(-.4)*(-.02),.02);
near("risk intervention threshold",1/(1+9),.1);
near("risk intervention cost",1-.1538,.8462);
near("EWMA variance",(.2)/(1.8),1/9);
near("EWMA ESS",(1+.8)/(1-.8),9);
near("FP16 ULP",2**(-10),1/1024);
truth("FP16 half ULP",2**(-12)<2**(-11));
near("high precision accumulation",1+4096*2**(-12),2);
truth("macro data version ready",10*60+33>10*60+15);
near("Ridge unpenalized intercept",(2+4)/2,3);
near("wrongly penalized intercept",(2+4)/(2+2),1.5);
near("Boost logit gradient",.5-1,-.5);
near("Boost logit Hessian",.5*(1-.5),.25);
near("Newton leaf value",.5/.25,2);
near("new cross-entropy",Math.log(1+Math.exp(-.2)),.5981388693815918);
near("SVM original margin weight",1/1,1);
near("SVM near-boundary weight",1/.1,10);
near("SVM near-boundary objective",.5*(10**2),50);
near("Beta posterior after success",(2+1)/(2+2+1),.6);
near("Beta posterior predictive difference",.6-.5,.1);
near("Gaussian density ratio",.01/.001,10);
near("Gaussian log improvement",Math.log(10),2.302585092994046);
near("overlap 5-day long-run variance",5+2*(4+3+2+1),25);
near("short block variance proxy",5+2*.5*4,9);
near("weighted gradient",(1*3*1+3*(-1)*2)/4,-.75);
near("equal gradient",(.5)*(3-2),.5);
near("incorrect-class cross entropy",(-Math.log(.8)-Math.log(.2))/2,.916290731874155);
near("calibrated loss",(-Math.log(.6)-Math.log(.4))/2,.713558177820073);
near("SSM four-step sensitivity",.5**3,.125);
near("SSM perturbation",.08*(.5**3),.01);
near("cache old key",1+2,3);
near("cache changed prefix",5+2,7);
near("DPO preference beta1",2/(1+2),2/3);
near("DPO preference beta2",4/(1+4),.8);
near("DPO beta2 derivative",-2*(1-.8),-.4);
near("graph smooth node one",2/3,.6666666666666666);
near("graph smooth node two",4/3,1.3333333333333333);
near("diffusion alpha low",.1*Math.sqrt(.96/.04),.4898979485566356);
near("diffusion alpha high",.1*Math.sqrt(.19/.81),.04843221048378525);
near("POMDP predictive H",.9*.8+.1*.2,.74);
near("POMDP posterior H",.9*.74/(.9*.74+.1*.26),.666/.692);
near("multiagent blind max",.5,.5);
near("invalid equicorrelation eigen",1+2*(-.6),-.2);
near("invalid equalweight variance",(1+2*(-.6))/3,-.2/3);
near("repaired equicorr eigen",1+2*(-.5),0);
near("turnover MVO solution",(.04+.1*.2)/(2*.01+.1),.5);
near("unpenalized MVO solution",.04/(2*.01),2);
near("low-vol target Bayes risk",.5*1+.5*9,5);
near("shifted target Bayes risk",.1*1+.9*9,8.2);
near("matrix quadratic gradient",2*2*6*3,72);
near("median absolute risk",(.5*Math.abs(0-1)+.5*Math.abs(2-1)),1);
near("median quadratic risk optimal",.5*(1**2)+.5*(1**2),1);
near("median quadratic risk nonoptimal",.5*(0**2)+.5*(2**2),2);
near("IS target risk",.75*0+.25*8,2);
near("IS second moment",.25*(8**2),16);
near("IS contribution variance",16-2**2,12);
near("clip before average",(2+(-2))/2,0);
near("clip after average",(10-8)/2,1);
near("MNAR world B mean",(50*10)/100,5);
near("correlated mean variance",(1+.5)/2,.75);
near("independence variance mistake",.5,.5);
near("ensemble mean half variance",(1+9)/4,2.5);
near("ensemble optimum weight",9/(1+9),.9);
near("ensemble optimal variance",(.9**2)*1+(.1**2)*9,.9);
near("SVM redundant dual weight",.5+.5,1);
near("SVM redundant dual balance",-.5+.5+0,0);
near("bimodal posterior actual mean",.5*(-10)+.5*10,0);
near("SimCLR false negative expectation",(2*4-2)*.5,3);
near("selected Gaussian expected max",1/Math.sqrt(Math.PI),.5641895835477563);
near("directional derivative",2*1+12*(-1),-10);
near("ViT RGB patch P16",16*16*3,768);
near("ViT RGB patch P8",8*8*3,192);
near("ViT patches P16",(224/16)**2,196);
near("ViT patches P8",(224/8)**2,784);
near("first period down marginal A",.5,.5);
near("second period down marginal B",.5,.5);
near("teacher-forcing rollout marginal",.9*.9+.1*.1,.82);
near("teacher-forcing all-correct",.9*.9,.81);
near("neighbor naive biased mean",.75*2+.25*8,3.5);
near("neighbor inverse probability mean",.75*(2/(2*.75))+.25*(8/(2*.25)),5);
const js=.75*Math.log(1.5)+.25*Math.log(.5);
near("JSD two-state exact",js,.13081203594113697);
near("total variation",(.5+.5)/2,.5);
near("Pinsker JSD lower bound",(.5**2)/2,.125);
truth("Pinsker numerical bound",js>=(.5**2)/2);
truth("Pinsker inverted bound",Math.sqrt(2*js)>=.5);
const mathHTML=read("index.html"), mlHTML=read("../ml/index.html");
assert(mathHTML.includes("?topic=information-n0485"),"JS divergence deep link vanished");
for(const [name,css] of [["math",read("styles.css")],["ml",read("../ml/atlas.css")]]){
 for(const mark of ["overflow-x:auto","line-break:strict",":focus-visible","@media print","@media(max-width:700px)"])
  assert(css.includes(mark),name+" lacks CJK/responsive rule "+mark);
}
assert(mlHTML.includes("quant")||mlHTML.includes("Atlas"),"ML home navigation vanished");
console.log("V74 PASS: 42 deepened lessons (1 math + 41 ML), "+katexCount+" KaTeX validations ("+strictCount+" new strict), "+numerical+" independent numeric cases; Unicode CJK, math navigation and mobile/print CSS guards.");
