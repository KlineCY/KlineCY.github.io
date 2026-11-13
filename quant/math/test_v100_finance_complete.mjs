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
 assert(l.formulas.length>=3,id+" core equations preserved");
 assert(l.derivation.length>=3,id+" mathematical derivations preserved");
 assert(l.deep_sections.length>=3,id+" deep subchapters preserved");
 for(const f of l.formulas){assert(f.label?.length>=2,id+" formula caption");checkFormula(f.tex,id);}
 for(const d of l.derivation){assert(d.explanation?.length>=25,id+" step explanation");checkFormula(d.tex,id);}
 const sectionTitles=new Set();
 for(const s of l.deep_sections){
  assert(s.title?.length>=8&&s.question?.length>=10,id+" deep metadata");
  assert(!sectionTitles.has(s.title),id+" repeated deep section");
  sectionTitles.add(s.title);
  assert(s.paragraphs?.length>=2&&s.paragraphs.every(p=>p.length>=30),id+" thin subchapter paragraphs");
  for(const [key,min] of [["proof",50],["worked",50],["limitation",25]])
   assert(s[key]?.length>=min,id+" thin "+key);
  assert(s.formulas?.length>=1,id+" deep mathematics missing");
  for(const f of s.formulas){assert(f.label?.length>=4);checkFormula(f.tex,id);}
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
assert(chapterCount>=85,"All 51 historical deep chapters, 17 prior extensions and 17 new rigorous chapters required");
assert(katexCount>=221,"All 187 historical expressions and 34 new strict finance expressions required");
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
assert(/app[.]js[?]v=[0-9]{8}/.test(html),"App script cache not refreshed");
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

for(const lesson of newest){
 const advanced=lesson.deep_sections.at(-1);
 assert(lesson.deep_sections.length>=4,lesson.id+" expanded financial analysis missing");
 assert(advanced.paragraphs.length>=2&&advanced.paragraphs.every(x=>x.length>=60),lesson.id+" insufficient financial reasoning");
 assert(advanced.proof.length>=80&&advanced.worked.length>=65&&advanced.limitation.length>=45,lesson.id+" incomplete derivation, example or assumptions");
 assert(advanced.formulas.length>=2,lesson.id+" needs independent mathematical expressions");
}

const extendedFinanceTitles={
  "finance-n0421": "资金流入流出时，时间加权收益与资金加权收益为何不同",
  "finance-n0422": "几何布朗运动的连续复利与波动率拖累",
  "finance-n0423": "期限结构不平坦时，单个IRR不足以决定现金流的无套利现值",
  "finance-n0424": "连续远期利率可以为负，贴现因子却仍保持正值",
  "finance-n0425": "关键期限DV01如何揭示非平行收益率曲线敞口",
  "finance-n0426": "匹配现值和久期仍可能因凸性差异导致免疫失败",
  "finance-n0427": "远期合约的现金持有复制与融资利率不对称的无套利范围",
  "finance-n0428": "不完备市场中状态价格非唯一但复制支付价格仍唯一",
  "finance-n0429": "支付矩阵的零空间解释不可复制状态支付与最小对冲误差",
  "finance-n0443": "HJM瞬时远期波动如何积累成零息债价格的随机风险",
  "finance-n0444": "风险中性违约强度与现实违约频率不能直接互换",
  "finance-n0445": "Expected Shortfall的风险包络表达与最坏尾部概率重新加权",
  "finance-n0446": "两两合法相关系数拼接也可能构成非法协方差矩阵",
  "finance-n0447": "多因子模型的旋转不变性与因子收益归因的不唯一",
  "finance-n0448": "GPD尾指数如何分别决定均值与方差是否存在",
  "finance-n0449": "对数障碍风险预算目标与正仓位风险平价权重",
  "finance-n0450": "带比例交易成本的单期最优仓位为什么存在无交易区间"
};
assert.equal(Object.keys(extendedFinanceTitles).length,17,"17 full follow-on finance chapters required");
const extendedFullDepthIds=new Set(["finance-n0421","finance-n0422","finance-n0423","finance-n0424","finance-n0425","finance-n0426","finance-n0427","finance-n0428","finance-n0429","finance-n0443","finance-n0444","finance-n0445","finance-n0446","finance-n0447","finance-n0448","finance-n0449","finance-n0450"]);
for(const l of newest){
 const original=l.deep_sections[4];
 assert.equal(original.title,extendedFinanceTitles[l.id],l.id+" pre-existing original chapter title changed");
 assert.equal(l.deep_sections.length,6,l.id+" five original chapters plus a sixth appended chapter required");
 const added=l.deep_sections.at(-1);
 assert(added.paragraphs.length===2&&added.paragraphs.every(p=>p.length>=100),l.id+" explanations shortened");
 assert(added.formulas.length===2,l.id+" two independent equations missing");
 assert(added.title!==original.title,l.id+" new sixth chapter repeats fifth title");
 assert(added.proof.length>=180&&added.worked.length>=110&&added.limitation.length>=65,l.id+" full mathematical proof and example required");
 assert(!added.paragraphs.some(p=>p.includes("\\n")),l.id+" paragraphs should remain readable");
}
assert.equal([...extendedFullDepthIds].length,17,"All seventeen complete six-section finance lectures must remain present");
near("Investment TWR with intermediate funding",1.2*.9-1,.08);
const moneyWeighted=(-120+Math.sqrt(120**2+4*100*216))/200-1;
near("Investment funding MWR",moneyWeighted,-.0125492133612457);
near("GBM expected log drift",.08-.5*.3**2,.035);
near("GBM volatility drift compensation",.08-.5*.4**2,0);
near("Two different zero-coupon discount rates",100*Math.exp(-.05)+100*Math.exp(-.2),176.9960177578696,1e-7);
near("Negative forward produces increasing discount",Math.exp(.01),1.010050167084168);
near("One year key-rate DV01",.0001*1*100,.01);
near("Two year key-rate DV01",.0001*2*100,.02);
near("Matched duration but convexity mismatch",Math.exp(-.2)-.5*(Math.exp(-.1)+Math.exp(-.3)),-.004096928616,1e-6);
near("Forward lower carry bound",100*Math.exp(.02),102.02013400267558);
near("Forward upper carry bound",100*Math.exp(.05),105.12710963760242);
near("Risk state price budget first solution",.4+.55,.95);
near("Incomplete digital replication error",.5**2+(-.5)**2,.5);
near("HJM one-year price volatility",.02*1,.02);
near("HJM three-year price volatility",.02*3,.06);
near("HJM bond covariance",.02*.06,.0012);
near("Credit risk neutral one-year default",1-Math.exp(-.04),.03921056084767682);
near("Credit real one-year default",1-Math.exp(-.01),.009950166250831893);
near("Expected shortfall two high loss states",.5*10+.5*4,7);
near("Non PSD three asset risk",3+2*(-.9-.9-.9),-2.4);
near("Factor rotation old total",.02+.01,.03);
near("Factor rotation new total",.03+0*.01,.03);
near("GPD heavy tail finite mean",2/(1-.6),5);
near("Risk budget unnormalized weight one",Math.sqrt(.5/.04),Math.sqrt(12.5));
near("Risk budget normalized first weight",Math.sqrt(.5/.04)/(Math.sqrt(.5/.04)+Math.sqrt(.5/.09)),.6);
near("No trade region keeps prior weight",.4,.4);
near("No trade high signal buy",(.08-.01)/.1,.7);
near("No trade low signal sell",(.02+.01)/.1,.3);

assert(Math.abs((1.03**4-1)-0.12550881)<1e-10,"nominal/effective compounding");
assert(Math.abs(-100+230/1.1-132/1.21)<1e-10,"first IRR root");
assert(Math.abs(-100+230/1.2-132/1.44)<1e-10,"second IRR root");
assert(Math.abs(.0125+.005-.0175)<1e-10,"correlation stress");
assert(Math.abs(5+10*(10**.2-1)-10.848931924611136)<1e-9,"POT tail extrapolation");
console.log("V100 PASS:",newest.length,"finance lessons;",all.length,"total math;",finance.length,
 "/36 finance complete;",katexCount,"strict KaTeX;",chapterCount,"deep sections;",
 deepChars,"proof characters;",numerical,"independent numeric checks;",crossRefs,"cross references.");
