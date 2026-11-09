import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root = new URL("./",import.meta.url);
const read = name => fs.readFileSync(new URL(name,root),"utf8");
const json = name => JSON.parse(read(name));
const curriculum=json("curriculum.json");
const nmap=new Map(curriculum.nodes.map(n=>[n.id,n]));
assert.equal(nmap.size,656);
const manifest=json("lessons-manifest.json");
assert.equal(manifest.expected_lessons,201);
assert.equal(manifest.modules.length,77);
const oldNames=[
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
 "lessons-pricing-pde-v99c.json","lessons-control-v99d.json"
];
const historical=oldNames.flatMap(n=>json(n).lessons);
assert.equal(historical.length,442,"V99 historical lecture count unexpectedly changed");
assert.equal(new Set(historical.map(l=>l.node_id)).size,442,"Historical duplicated IDs");
const files=["lessons-finance-v100a.json","lessons-finance-v100b.json","lessons-finance-v100c.json","lessons-finance-v100d.json"];
const docs=files.map(json);
docs.forEach((d,i)=>{
 assert.equal(d.version,1);
 assert.equal(d.revision,"V100");
 assert.equal(d.lessons.length,[6,3,2,6][i]);
});
const newest=docs.flatMap(d=>d.lessons);
assert.equal(newest.length,17);
const expected=[
 ...Array.from({length:9},(_,i)=>"finance-n"+String(i+421).padStart(4,"0")),
 ...Array.from({length:8},(_,i)=>"finance-n"+String(i+443).padStart(4,"0"))
];
assert.deepEqual(newest.map(l=>l.node_id),expected,"Finance knowledge sequence missing/overwritten");
const all=[...historical,...newest];
assert.equal(all.length,459,"Expected full lesson count 459");
assert.equal(new Set(all.map(l=>l.node_id)).size,459,"Global lesson ID collision");
assert(all.every(l=>nmap.get(l.node_id)?.title===l.title),"Global node-title mismatch");
const finance=curriculum.nodes.filter(n=>n.discipline_id==="finance");
assert.equal(finance.length,36,"Unexpected finance curriculum size");
const completed=new Set(all.map(l=>l.node_id));
assert(finance.every(n=>completed.has(n.id)),"Finance 36/36 completeness failed");
const priorFinance=historical.filter(l=>l.node_id.startsWith("finance-"));
assert.equal(priorFinance.length,19,"Prior 19 finance lectures must stay unmodified");
let katexCount=0,chapterCount=0,deepChars=0,crossRefs=0;
function checkFormula(tex,id){
 assert.equal(typeof tex,"string",id+" bad formula type");
 assert(tex.length>=10,id+" formula too short");
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(tex),id+" Unicode invisibles in formula");
 try{katex.renderToString(tex,{displayMode:true,throwOnError:true,strict:"error"});}
 catch(e){throw Error(id+" invalid strict KaTeX: "+tex+" / "+e.message);}
 katexCount++;
}
for(const l of newest){
 const id=l.node_id;
 assert.equal(l.id,id);
 assert.equal(l.title,nmap.get(id).title);
 for(const key of ["definition","assumptions","example","application","caveat"])
  assert(l[key]?.length>=70,id+" thin "+key);
 assert(l.properties?.length>=4 && l.properties.every(p=>p.length>=20),id+" properties incomplete");
 assert.equal(l.formulas.length,3,id+" core equations");
 assert.equal(l.derivation.length,3,id+" math derivation");
 assert.equal(l.deep_sections.length,3,id+" deep subchapters");
 for(const f of l.formulas){assert(f.label?.length>=2,id+" formula caption");checkFormula(f.tex,id);}
 for(const d of l.derivation){assert(d.explanation?.length>=25,id+" step explanation");checkFormula(d.tex,id);}
 const sectionTitles=new Set();
 for(const s of l.deep_sections){
  assert(s.title?.length>=8&&s.question?.length>=10,id+" deep metadata");
  assert(!sectionTitles.has(s.title),id+" repeated deep section");
  sectionTitles.add(s.title);
  assert(s.paragraphs?.length===2&&s.paragraphs.every(p=>p.length>=30),id+" thin subchapter paragraphs");
  for(const [key,min] of [["proof",50],["worked",50],["limitation",25]])
   assert(s[key]?.length>=min,id+" thin "+key);
  assert.equal(s.formulas?.length,1,id+" deep mathematics");
  assert(s.formulas[0].label?.length>=4);
  checkFormula(s.formulas[0].tex,id);
  chapterCount++;
  deepChars+=s.paragraphs.join("").length+s.proof.length+s.worked.length+s.limitation.length;
 }
 for(const link of l.links||[]){
  assert(["math","ml"].includes(link.kind),id+" malformed link");
  if(link.kind==="math")assert(nmap.has(link.id),id+" dangling link "+link.id);
  crossRefs++;
 }
 assert(/[\u3400-\u9FFF]/u.test(JSON.stringify(l)),id+" no Chinese body");
 assert(!/TODO|FIXME|Lorem ipsum|待补充/i.test(JSON.stringify(l)),id+" placeholder");
}
assert.equal(chapterCount,51);
assert.equal(katexCount,153);
assert(deepChars>=13500);
assert(crossRefs>=48);
const html=read("index.html"),app=read("app.js"),css=read("styles.css");
const workflow=read("../../.github/workflows/test-quant-math.yml");
for(const phrase of [
 'loadV99DynamicsPDE("./lessons-control-v99d.json",6)',
 'loadV100Finance("./lessons-finance-v100a.json",6)',
 'loadV100Finance("./lessons-finance-v100b.json",3)',
 'loadV100Finance("./lessons-finance-v100c.json",2)',
 'loadV100Finance("./lessons-finance-v100d.json",6)',
 '...v99a,...v99b,...v99c,...v99d,...v100a,...v100b,...v100c,...v100d',
 'new Set(lessons.map(l=>l.node_id)).size!==lessons.length',
 'new Set(doc.lessons.map(l=>l.node_id)).size!==8'
])assert(app.includes(phrase),"Missing historical or V100 reader integration: "+phrase);
assert(html.includes('app.js?v=20261108-v105-control-game-market-making-complete'),"App script cache not refreshed");
assert(html.includes("金融数学与资产定价"),"Finance navigator missing");
for(const l of newest)assert(html.includes("?topic="+l.node_id+'"'),"Navigator missing "+l.node_id);
for(const link of ["finance-n0431","finance-n0433","finance-n0434","finance-x001","finance-x006"])
 assert(html.includes("?topic="+link+'"'),"Cross curriculum link missing "+link);
for(const token of ["overflow-x:auto","line-break:strict","@media(max-width:700px)","@media print",":focus-visible"])
 assert(css.includes(token),"Missing KaTeX/CJK mobile layout "+token);
assert(workflow.includes("node quant/math/test_v100_finance_complete.mjs"),"V100 CI task missing");
let numerical=0;
function near(label,val,expected,tol=1e-8){
 assert(Number.isFinite(val),label+" not finite");
 assert(Math.abs(val-expected)<=tol,label+": observed "+val+" expected "+expected);
 numerical++;
}
function yes(label,val){near(label,Number(Boolean(val)),1);}
near("Monthly 12percent APR factor",1.01**12,1.1268250301319698);
near("Quarterly 12percent APR factor",1.03**4,1.12550881);
near("Nominal 10pct effective to monthly",12*(1.1**(1/12)-1),.09568968514684517);
near("Comp 2yr at10",100*1.1**2,121);
near("Simple 2yr at10",100*(1+2*.1),120);
near("Continuous 2yr factor",100*Math.exp(.05*2),110.51709180756477);
near("Continuous effective 1yr",Math.exp(.05)-1,.05127109637602412);
near("Continuous 5percent eff to annual",Math.log(1.05),.04879016416943204);
near("Time varying r 4/6",.04*.5+.06*.5,.05);
near("Geometric log 20 and -20",Math.log(1.2*.8),Math.log(.96));
near("PV first cash",50/1.1,45.45454545454545);
near("PV second cash",60/1.1**2,49.58677685950413);
near("PV both cash",50/1.1+60/1.1**2,95.04132231404958);
near("NPV project",50/1.1+60/1.1**2-90,5.04132231404958);
near("Future cash 2yr",50*1.1+60,115);
near("Future original investment",90*1.1**2,108.9);
near("Future NPV", (50/1.1+60/1.1**2-90)*1.21,6.1);
near("IRR root10", -100+230/1.1-132/1.1**2,0);
near("IRR root20",-100+230/1.2-132/1.2**2,0);
near("Discount two future rates",Math.exp(-(.02+.04)),Math.exp(-.06));
near("Discount semigroup",Math.exp(-.02)*Math.exp(-.04),Math.exp(-.06));
near("Discount rate curve mean",(.02+.04)/2,.03);
near("Jensen risk rates LHS",(1+Math.exp(-.1))/2,.9524187090179798);
yes("Jensen inequality",(1+Math.exp(-.1))/2>Math.exp(-.05));
near("Coupon bond PV",5/1.04+105/1.04**2,101.8860946745562);
near("Coupon bond par",5/1.05+105/1.05**2,100);
near("Coupon bond derivative",-5/1.04**2-210/1.04**3,-191.31201638598088);
yes("Bond downward slope",-5/1.04**2-210/1.04**3<0);
near("Zero ytm 105/100",105/100-1,.05);
near("Zero Macaulay duration",1,1);
near("Zero modified duration",1/1.05,.9523809523809523);
near("Zero convexity",2/1.05**2,1.8140589569160996);
near("Zero next year price",105/1.06,99.0566037735849);
near("Zero exact price loss",(105/1.06-100)/100,-.009433962264151052);
near("Zero Taylor price change",-(1/1.05)*.01+.5*(2/1.05**2)*.01**2,-.009433106575963718);
near("Law of one price arb",97-95,2);
near("State arb bond price",1/1.05,20/21);
near("State up price",10/21,.47619047619047616);
near("State probability up",1.05*10/21,.5);
near("State probability down",1.05*10/21,.5);
near("State stock replication",120*10/21+90*10/21,100);
near("State Arrow sum",10/21+10/21,20/21);
near("Arrow call price",20*(10/21),200/21);
near("Arrow call down payoff",Math.max(90-100,0),0);
near("HJM constant vol drift",.02**2*2,.0008);
near("HJM integrated drift",.0004*2**2/2,.0008);
near("HJM Ito bond correction",.5*(.02*2)**2,.0008);
near("HJM two independent factors",(.02**2+.01**2)*2,.001);
near("Zero term 3pct two years",Math.exp(-.03*2),Math.exp(-.06));
near("Survival 1yr lambda2",Math.exp(-.02),.9801986733067553);
near("Default 1yr probability",1-Math.exp(-.02),.01980132669324475);
near("Default 2yr probability",1-Math.exp(-.04),.03921056084767682);
near("Default zero recovery price",Math.exp(-(.03+.02)),.951229424500714);
near("No default risk free price",Math.exp(-.03),.9704455335485082);
near("Credit CDS fair spread",.02*(1-.4),.012);
near("Credit CDS low recovery",.02*(1-.2),.016);
near("Credit with recovery cash",Math.exp(-.05)+.4*.02/.05*(1-Math.exp(-.05)),.9590327165805997);
near("VaR discrete at .90",0,0);
near("VaR discrete at .95",10,10);
near("ES tail average .90",.1*10/.1,10);
near("Incorrect conditional mean example",.9*0+.1*10,1);
near("RU objective at 0",0+(.1*10)/.1,10);
near("RU objective at 5",5+(.1*5)/.1,10);
near("Cov portfolio var",.25*.04+.25*.09+.5*.01,.0375);
near("Cov vol",Math.sqrt(.0375),.19364916731037085);
near("Cov correlation",.01/(.2*.3),1/6);
near("Cov determinant",.04*.09-.01**2,.0035);
near("Factor covariance",1*2*.01,.02);
yes("Sample cov singular for37 assets19 rank",19<37);
near("Factor return market contribution",1.2*.02,.024);
near("Factor value short contribution",-.3*(-.01),.003);
near("Factor net attributed",1.2*.02-.3*(-.01),.027);
near("Factor with alpha",1.2*.02-.3*(-.01)+.001,.028);
near("Factor asset beta",.6*1+.4*2,1.4);
near("Factor variance correlated",.01+.01+2*.005,.03);
near("Factor alpha mean",.01-1.2*.005,.004);
near("EVT POT VaR at99",5+2*Math.log(.1/.01),9.605170185988092);
near("EVT POT ES",5+2*Math.log(10)+2,11.605170185988092);
near("EVT tail alpha survival",.1*Math.exp(-(9.605170185988092-5)/2),.01);
near("EVT shape .5 finite tail exponent",1/.5,2);
yes("EVT shape1.2 invalid ES",1.2>=1);
near("Risk variance parity",(.6*.2)**2+(.4*.3)**2,.0288);
near("Risk total volatility",Math.sqrt(.0288),.16970562748477142);
near("Risk contribution asset1",.0144/Math.sqrt(.0288),.0848528137423857);
near("Risk contribution asset2",.0144/Math.sqrt(.0288),.0848528137423857);
near("Risk contribution Euler sum",2*.0144/Math.sqrt(.0288),Math.sqrt(.0288));
near("Risk parity ratio",(.6*.2)/(.4*.3),1);
near("Risk hedge first contribution",.2,.2);
near("Risk hedge second contribution",-.1,-.1);
near("Risk hedge sum",.2-.1,.1);
near("Dynamic net wealth buy",1+.02-.005,1.015);
near("Dynamic no trade wealth",1,1);
near("Dynamic weak signal buy",1+.003-.005,.998);
near("Dynamic trade payoff",.02-.005,.015);
near("Dynamic weak payoff",.003-.005,-.002);
yes("Dynamic positive alpha covers cost",.02>.005);
assert(numerical>=90,"Insufficient independent financial arithmetic");
console.log("V100 PASS:",newest.length,"finance lessons;",all.length,"total math;",finance.length,
 "/36 finance complete;",katexCount,"strict KaTeX;",chapterCount,"deep sections;",
 deepChars,"proof characters;",numerical,"independent numeric checks;",crossRefs,"cross references.");
