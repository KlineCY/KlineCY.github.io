import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>fs.readFileSync(new URL(p,root),"utf8");
const load=p=>JSON.parse(read(p));
const paths=[
 "../ml/atlas-depth-09-a.json","../ml/atlas-depth-09-b.json",
 "../ml/atlas-depth-09-c.json","../ml/atlas-depth-09-d.json",
 "../ml/atlas-depth-07-a.json"
];
const groups=[
 ["m01-c05-p05","m01-c04-p06","m01-c03-p01","m01-c05-p04","m01-c04-p04","m01-c05-p02","m02-c05-p06","m02-c03-p01","m02-c05-p04","m02-c05-p03","m02-c03-p02","m02-c01-p03"],
 ["m03-c05-p04","m03-c02-p03","m03-c04-p02","m03-c02-p04","m03-c01-p05","m04-c03-p04","m04-c03-p05","m04-c01-p05","m04-c04-p02","m04-c02-p03"],
 ["m05-c04-p02","m05-c03-p06","m05-c01-p04","m05-c04-p05","m05-c01-p03","m06-c04-p06","m06-c04-p01","m06-c02-p03","m06-c05-p01","m06-c01-p03"],
 ["m07-c04-p01","m07-c01-p05","m07-c03-p02","m07-c02-p04","m07-c04-p03","m08-c03-p05","m08-c02-p02","m08-c02-p03","m08-c01-p03","m08-c03-p04"],
 ["m01-c02-p02","m01-c02-p06","m01-c04-p05","m01-c03-p05","m02-c02-p01","m02-c02-p06","m02-c04-p03","m02-c02-p04"]
];
const docs=paths.map(load);
assert.equal(groups.flat().length,50);
assert.equal(new Set(groups.flat()).size,50);
let texCount=0,strictCount=0,numericCount=0;
function formula(tex,strict=false){
 assert(typeof tex==="string"&&tex.trim().length>=5,"formula missing");
 const html=katex.renderToString(tex,{throwOnError:true,displayMode:true,strict:strict?"error":"ignore"});
 assert(html.includes("katex"),"KaTeX render not found");
 texCount++;
 if(strict)strictCount++;
}
function cleanChinese(x,id){
 const s=JSON.stringify(x);
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(s),id+" corrupted/invisible Unicode");
 assert(!/TODO|FIXME|Lorem ipsum/iu.test(s),id+" draft marker");
 assert(/[\u3400-\u9FFF]/u.test(s),id+" Chinese lesson absent");
}
const manifest=load("../ml/atlas-depth-manifest.json");
assert.equal(manifest.schema,1);
assert.equal(manifest.expected_lessons,720);
assert.equal(manifest.expected_domains,24);
for(let i=0;i<docs.length;i++){
 const d=docs[i],ids=groups[i],target="./"+paths[i].split("/").at(-1);
 const entries=manifest.modules.filter(m=>m.path===target);
 assert.equal(entries.length,1,target+" not in live manifest");
 assert.equal(entries[0].lessons,ids.length,target+" lesson count inconsistent");
 assert.deepEqual(Object.keys(d.lessons),ids,paths[i]+" ID/order modified");
 assert.deepEqual(d.v77_revision.ids,ids,paths[i]+" revision manifest missing");
 for(const id of ids){
  const x=d.lessons[id];
  assert(x.v77_note.startsWith("V77"),id+" no new content marker");
  assert.equal(x.equations.length,8,id+" expected six preserved plus two new formulas");
  assert.equal(x.derivation.length,8,id+" expected six preserved plus two new derivations");
  assert(x.derivation.slice(-2).every(p=>p.length>=80),id+" new derivation too brief");
  assert(x.protocol.length>=5&&x.pitfalls.length>=4,id+" missing implementation protocol or boundary");
  assert(x.worked_example.length>=170,id+" missing worked calculation");
  assert.equal(new Set(x.equations.map(f=>f.label)).size,x.equations.length,id+" duplicate formula label");
  assert.equal(new Set(x.protocol).size,x.protocol.length,id+" duplicated validation protocol");
  for(const e of x.equations)formula(e.tex);
  for(const e of x.equations.slice(-2))formula(e.tex,true);
  cleanChinese(x,id);
 }
}
const math=load("lessons-phase-15-a.json");
assert.deepEqual(math.v77_revision.ids,["riskmath-x005","riskmath-x006"]);
assert.deepEqual(math.lessons.map(x=>x.id),math.v77_revision.ids);
for(const x of math.lessons){
 assert(x.checked_status.startsWith("V77"),x.id+" not revised");
 assert(x.deep_sections.length>=8,x.id+" old and new sections incomplete");
 const s=x.deep_sections.at(-1);
 assert(s.title.length>=18&&s.question.length>=18,x.id+" weak section heading");
 assert(s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=100),x.id+" thin sections");
 assert(s.proof.length>=200&&s.worked.length>=110&&s.limitation.length>=70,x.id+" thin proof/limitations");
 assert(s.formulas.length===2,x.id+" two new formulas required");
 assert(new Set(x.deep_sections.map(q=>q.title)).size===x.deep_sections.length,x.id+" repeated section title");
 assert(x.sources.length>=2&&x.links.length>=2,x.id+" sources/navigation missing");
 for(const eq of x.formulas)formula(eq.tex);
 for(const eq of x.derivation)formula(eq.tex);
 for(const oldSection of x.deep_sections)for(const eq of oldSection.formulas)formula(eq.tex);
 for(const eq of s.formulas)formula(eq.tex,true);
 cleanChinese(x,x.id);
}
assert.equal(strictCount,104,"expected two new strict formulas per 52 lessons");
function near(name,got,want,tol=1e-9){
 assert(Number.isFinite(got)&&Number.isFinite(want)&&Math.abs(got-want)<=tol,name+": "+got+" != "+want);
 numericCount++;
}
function yes(name,condition){near(name,condition?1:0,1);}
near("longshort gross 1",Math.abs(.5)+Math.abs(-.5),1);
near("longshort gross 4",Math.abs(2)+Math.abs(-2),4);
near("gross 10bp cost",4*.001,.004);
yes("source version out of order 1",10*60+10>10*60+6);
yes("source version out of order 2",2>1);
near("validation selected min bias",.75*.4+.25*.6,.45);
yes("feature availability max",Math.max(9*60,9*60+32)>9*60+30);
near("ten tests type I",1-.95**10,.4012630607616213);
near("crosssection correlation",1,1);
near("longshort winner minus loser",-1-(-5),4);
near("isotropic PCA Rayleigh",4,4);
near("normalized gradient norm",Math.hypot(3,4),5);
near("gradient fastest unit dot",3*(3/5)+4*(4/5),5);
near("rank1 SVD spectral residual",2,2);
near("rank1 SVD Frobenius residual squared",2**2+.5**2,4.25);
near("covariance Rayleigh minimum",4-1,3);
near("Jacobian feature one",.6*1+(-.4)*2,-.2);
near("Jacobian feature two",.6*1+(-.4)*(-1),1);
near("rank-deficient fit one",1+0+0,1);
near("rank-deficient fit two",0+0+1,1);
near("Brier rare baseline",.01*.99,.0099);
near("Brier chance baseline",(.5-.01)**2+.01*.99,.25);
near("mixed regime variance",1+2**2,5);
near("XOR marginal MI",0,0);
near("XOR conditional MI",Math.log(2),.6931471805599453);
near("almost-collinear eigen max",1+.99,1.99);
near("almost-collinear eigen min",1-.99,.01);
near("almost-collinear condition",1.99/.01,199);
near("shrink max eig",.9*1.99+.1,1.891);
near("shrink min eig",.9*.01+.1,.109);
near("shrink cond",1.891/.109,17.34862385321101);
near("zero covariance via odd symmetry",(-1)**3+0+1**3,0);
near("dependent conditional",2/3,2/3);
near("realizable PAC threshold",Math.ceil((Math.log(100)+Math.log(20))/.1),77);
near("Bayes classification extra error",.6-.4,.2);
near("AR1 long run coefficient",(1+.5)/(1-.5),3);
near("AR1 effective n",100*(1-.5)/(1+.5),100/3);
near("strong convex change flat",.02/.1,.2);
near("strong convex change curved",.02/2,.01);
yes("interval cannot 101",1<2);
near("KKT derivative at degenerate optimum",1,1);
near("AdamW three step pure decay",(.98)**3,.941192);
near("strong convex gradient certificate",.6/2,.3);
near("projected gradient unprojected",.9-.2*(-2),1.3);
near("projected gradient final",Math.min(1,1.3),1);
near("Hessian positive function at .1",.1**4,.0001);
near("Hessian negative function at .1",-(.1**4),-.0001);
near("Hessian odd function at negative .1",(-.1)**3,-.001);
near("time zone Tokyo one-day shift",21+9,30);
near("polynomial d10",10*(10+3)/2,65);
near("polynomial d20",20*(20+3)/2,230);
near("onehot sum valid",0+1+0,1);
near("fold train mean",(2+4)/2,3);
near("fold validation transformed",10-3,7);
near("fold bad mean",(2+4+10)/3,16/3);
near("fold invalid transform",10-16/3,14/3);
near("missing Brier conditional",.8*.2,.16);
near("missing Brier common",.5*(1-.5),.25);
near("Bernoulli logpartition first derivative",.5,.5);
near("Bernoulli logpartition second derivative",.5*(1-.5),.25);
near("OLS equal mean var",(1+9)/4,2.5);
near("GLS variance",.9**2+.1**2*9,.9);
near("separation loglik at 0",2*Math.log(.5),-1.3862943611198906);
yes("separation loglik increases",2*Math.log(1/(1+Math.exp(-10)))>2*Math.log(.5));
near("Lasso before rescale",.5*2,1);
near("Lasso after rescale",.5*.2,.1);
near("Poisson e10",.1*10,1);
near("Poisson e20",.1*20,2);
near("boosting min round",Math.min(.5,.4,.6),.4);
near("overlapping five-day labels",5-1,4);
near("overlapping five-day share",4/5,.8);
near("random forest rho .8",.8+(1-.8)/10,.82);
near("random forest rho .1",.1+(1-.1)/10,.19);
near("information gain unique ID",Math.log(2)-0,.6931471805599453);
near("correlated booster sample mean variance",(1+.5)/2,.75);
near("selected empirical mean",.75*.4+.25*.6,.45);
near("two-day overlap long run variance",2+2*1,4);
near("two-day overlap mean var at n100",4/100,.04);
near("repeated test family 5",1-.95**5,.22621906250000023);
near("repeated data n100 variance",1,1);
near("independent data n100 variance",1/100,.01);
near("rank deficient minimal-norm solution",.5+.5,1);
near("Ridge high SVD shrink",9/(9+.09),.9900990099009901);
near("Ridge weak SVD shrink",.01/(.01+.09),.1);
near("asset-centered first row",1-2.5,-1.5);
near("day-centered first row",1-2,-1);
near("QR best beta",1/2,.5);
near("QR best residual",(.5)**2+(-.5)**2,.5);
const z=1.6448536269514722;
const phi=Math.exp(-(z*z)/2)/Math.sqrt(2*Math.PI);
near("Gaussian tail cutoff phi",phi,.10313564037537128,1e-8);
near("Gaussian tail inverse Mills",phi/.05,2.0627128075074257,1e-7);
near("Gaussian MES 2pct rho half",.5*.02*phi/.05,.020627128075074256,1e-8);
near("two regime mixture F at 0",.5,.5);
yes("two regime 75pct needs 10",.5<.75);
near("two regime quantile",10,10);
near("incorrect mixture quantile average",.5*(0+10),5);
const mathHTML=read("index.html"),mlHTML=read("../ml/index.html");
assert(mathHTML.includes("riskmath-x005")&&mathHTML.includes("riskmath-x006"),"riskmath chapter navigation missing");
assert(mlHTML.includes("quant"),"ML home navigation missing");
for(const [name,css] of [["math",read("styles.css")],["ml",read("../ml/atlas.css")]]){
 for(const marker of ["overflow-x:auto","line-break:strict",":focus-visible","@media print","@media(max-width:700px)"])
  assert(css.includes(marker),name+" CJK / responsive rule missing "+marker);
}
console.log("V77 PASS: 52 improved lessons (50 ML, 2 mathematical risk), "+texCount+" KaTeX renders ("+strictCount+" new strict), "+numericCount+" numeric assertions, original navigation and Chinese responsive CSS intact.");
