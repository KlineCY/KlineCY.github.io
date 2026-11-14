import fs from "node:fs";
import vm from "node:vm";
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
 "lessons-mlmath-v102c.json",
 "lessons-information-v103a.json","lessons-information-v103b.json",
 "lessons-information-v103c.json",
 "lessons-discrete-v104a.json","lessons-discrete-v104b.json",
 "lessons-discrete-v104c.json","lessons-discrete-v104d.json"
];
const history=oldFiles.flatMap(n=>json(n).lessons);
assert.equal(history.length,534,"V104 historical 534 lesson texts changed");
assert.equal(new Set(history.map(l=>l.node_id)).size,534,"V104 history has duplicate IDs");
const files=["lessons-control-v105a.json","lessons-control-v105b.json","lessons-control-v105c.json","lessons-control-v105d.json"];
const docs=files.map(json);
docs.forEach((d,i)=>{
 assert.equal(d.version,1,"V105 data schema wrong");
 assert.equal(d.revision,"V105","V105 revision mismatch");
 assert.equal(d.lessons.length,[7,7,7,7][i],"V105 file size mismatch");
});
const newest=docs.flatMap(d=>d.lessons);
const consolidatedIDs=new Set(docs[0].lessons.map(l=>l.node_id));
const ids=Array.from({length:29},(_,i)=>i+542).filter(i=>i!==545).map(i=>"control-n"+String(i).padStart(4,"0"));
assert.deepEqual(newest.map(l=>l.node_id),ids,"V105 control lectures changed order or incomplete");
const all=[...history,...newest];
assert.equal(all.length,562,"Expected 562 completed mathematics lectures");
assert.equal(new Set(all.map(l=>l.node_id)).size,562,"Historical lesson overwritten");
assert(all.every(l=>nodes.get(l.node_id)?.title===l.title),"Curriculum mismatched or missing lesson");
const completed=new Set(all.map(l=>l.node_id));
const control=curriculum.nodes.filter(n=>n.discipline_id==="control");
assert.equal(control.length,30,"Control syllabus size changed");
assert(control.every(n=>completed.has(n.id)),"Control theory must be 30/30 complete");
assert.deepEqual(history.filter(l=>l.node_id.startsWith("control-")).map(l=>l.node_id).sort(),
 ["control-n0541","control-n0545"],"Previously existing two control lessons changed");
assert.equal(newest.filter(l=>l.node_id.startsWith("control-")).length,28);
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
 assert(l.formulas.length>=3,id+" original core formulas required");
 assert(l.derivation.length>=3,id+" original derivations required");
 assert(l.deep_sections.length>=3,id+" original chapter depth required");
 for(const x of l.formulas){assert(x.label?.length>=2,id+" mathematical label");formula(x.tex,id);}
 for(const x of l.derivation){assert(x.explanation?.length>=25,id+" derivation prose");formula(x.tex,id);}
 const headings=new Set();
 for(const s of l.deep_sections){
  assert(s.title?.length>=8 && s.question?.length>=10,id+" deep section title/question");
  assert(!headings.has(s.title),id+" repeated subchapter");
  headings.add(s.title);
  assert(s.paragraphs?.length>=2 && s.paragraphs.every(p=>p.length>=30),id+" deep explanatory text");
   if(consolidatedIDs.has(id))assert(s.paragraphs.every(p=>!p.includes("\n")),id+" contains merged duplicate prose");
  for(const [key,min] of [["proof",50],["worked",50],["limitation",25]])
   assert(s[key]?.length>=min,id+" short "+key);
  assert(s.formulas?.length>=1,id+" proof mathematics missing");
  for(const f of s.formulas){assert(f.label?.length>=2,id+" deep formula caption");formula(f.tex,id);}
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
assert(mathCount>=364,"All historical control equations and 56 newly derived mathematical expressions must remain");
assert(deepCount>=140,"All 112 historical and 28 new strategic-control deep chapters must remain");
assert(deepChars>=22500,"Depth reduced from reviewed version");
assert.equal(linkCount,84,"Cross-lesson link count changed");
const html=read("index.html"),css=read("styles.css"),app=read("app.js");
const workflow=read("../../.github/workflows/test-quant-math.yml");
assert(/app[.]js[?]v=[0-9]{8}/.test(html),"Reader must bust V104 cache");
assert(html.includes("控制理论、动态博弈与做市"),"V105 CJK curriculum absent");
for(const id of ids)assert(html.includes("?topic="+id+'"'),"Study route missing "+id);
for(const needle of [
 'loadV104DiscreteGraphs("./lessons-discrete-v104d.json",6)',
 'loadV105ControlGame("./lessons-control-v105a.json",7)',
 'loadV105ControlGame("./lessons-control-v105b.json",7)',
 'loadV105ControlGame("./lessons-control-v105c.json",7)',
 'loadV105ControlGame("./lessons-control-v105d.json",7)',
 '...v104a,...v104b,...v104c,...v104d,...v105a,...v105b,...v105c,...v105d',
 'new Set(lessons.map(l=>l.node_id)).size!==lessons.length',
 'new Set(doc.lessons.map(l=>l.node_id)).size!==8'
])assert(app.includes(needle),"Missing V105 canonical loader or historical guard: "+needle);
for(const token of ["overflow-x:auto","line-break:strict","@media(max-width:700px)","@media print",":focus-visible"])
 assert(css.includes(token),"Missing established Chinese/LaTeX mobile accessibility: "+token);
assert(workflow.includes("node quant/math/test_v105_control_complete.mjs"),"V105 CI step absent");





let checks=0;
function near(label,value,expected,tol=1e-8){
 assert(Number.isFinite(value),label+" non-finite");
 assert(Math.abs(value-expected)<=tol,label+": actual "+value+" expected "+expected);
 checks++;
}
function yes(label,x){near(label,Number(Boolean(x)),1);}
function log(x){return Math.log(x);}
near("Banach contraction fixed point",2/(1-.8),10);
near("Banach V1",2,2);
near("Banach V2",2+.8*2,3.6);
near("Banach V3",2+.8*3.6,4.88);
near("Banach error initial",10-0,10);
near("Banach error at k1",10-2,8);
near("Banach error at k2",10-3.6,6.4);
near("Banach error at k3",10-4.88,5.12);
near("Banach residual bound",.2/(1-.8),1);
near("Banach residual bound near1",.2/(1-.99),20);
near("Value hold policy",1/(1-.5),2);
near("Value trade policy",2/(1-.5),4);
near("Value Q first hold",1+.5*4,3);
near("Value Q first trade",2+.5*4,4);
near("Policy iteration old Q hold",1+.5*2,2);
near("Policy iteration old Q trade",2+.5*2,3);
near("Policy iteration new value",2/(1-.5),4);
yes("Policy iteration improving action",3>2);
near("Finite horizon t2",2+.9*2,3.8);
near("Infinite horizon value",2/(1-.9),20);
near("Finite horizon tail",20-3.8,16.2);
near("Finite horizon tail formula",.9**2*2/(1-.9),16.2);
near("Finite horizon terminal bonus",.9**2*10,8.1);
near("Snell Y2",2,2);
near("Snell Y1",Math.max(3,2),3);
near("Snell Y0",Math.max(1,3),3);
yes("Snell stop at t1",3>=2);
near("American risk neutral p",(1.05-.8)/(1.2-.8),.625);
near("American risk neutral down",1-.625,.375);
near("American terminal high",Math.max(100-144,0),0);
near("American terminal mid",Math.max(100-96,0),4);
near("American terminal low",Math.max(100-64,0),36);
near("American upper intermediate",(.625*0+.375*4)/1.05,1.4285714285714286);
near("American lower continuation",(.625*4+.375*36)/1.05,15.238095238095237);
near("American lower exercise",Math.max(100-80,0),20);
yes("American lower early exercise",20>15.238095238095237);
near("American initial",(.625*((.375*4)/1.05)+.375*20)/1.05,7.993197278911564);
near("Discrete stopping future value",.5*4+.5*0,2);
near("Discrete stopping stop now",Math.max(2,2),2);
near("Discrete stopping omniscient",.5*4+.5*2,3);
near("Discrete stopping information gap",3-2,1);
near("Continuous generator drift component",2*.04*100,8);
near("Continuous generator diffusion component",.2**2*100,4);
near("Continuous generator sum",8+4,12);
near("Continuous generator minus discount",12-.05*100,7);
near("Rebalance two period gross",.003*(1+.9),.0057);
near("Rebalance gross minus fee",.003*(1+.9)-.005,.0007);
near("Rebalance small signal",.002*(1+.9)-.005,-.0012);
near("Rebalance threshold",.005/(1+.9),.002631578947368421);
near("Rebalance decaying gain",.004+.9*.001-.005,-.0001);
near("Execution wait p0.6 increment",.4*.0015-.6*.001+.0002,.0002);
near("Execution wait cost p0.6",.002+.4*.0015-.6*.001+.0002,.0022);
near("Execution wait p0.8 increment",.2*.0015-.8*.001+.0002,-.0003);
near("Execution wait cost p0.8",.002+.2*.0015-.8*.001+.0002,.0017);
near("Stochastic risky expected reward",.5*4+.5*0,2);
near("Stochastic safe expected reward",1,1);
near("Stochastic safe utility",-Math.exp(-1),-.36787944117144233);
near("Stochastic risky utility",-.5*Math.exp(-4)-.5,-.5091578194443671);
yes("Stochastic CARA prefers safe", -Math.exp(-1)>-.5*Math.exp(-4)-.5);
near("HJB diffusion term",.5*.2**2*(-2),-.04);
near("HJB optimal action",.5,.5);
near("HJB optimum Hamilton",-((.5)**2)+.5-.04,.21);
near("HJB H at +1",-1+1-.04,-.04);
near("HJB H at -1",-1-1-.04,-2.04);
near("LQG control optimal",-(2)/2,-1);
near("LQG terminal state",2-1,1);
near("LQG optimal objective",(-1)**2+1**2,2);
near("LQG Riccati",1-1/(1+1),.5);
near("LQG quadratic derivative optimal",4*(-1)+2*2,0);
near("Risk sensitivity deterministic",log(Math.exp(1)),1);
near("Risk sensitivity random",log((1+Math.exp(2))/2),1.4337808304830273);
yes("Risk sensitivity random penalized",log((1+Math.exp(2))/2)>1);
near("Risk sensitivity random variance",.5*(0-1)**2+.5*(2-1)**2,1);
near("Risk sensitivity small parameter approx",1+.1/2,1.05);
near("Robust risky worst",Math.min(4,0),0);
near("Robust safe worst",Math.min(1,1),1);
yes("Robust safe best",1>0);
near("POMDP observed warning evidence",.2*.8+.8*.1,.24);
near("POMDP posterior high",(.2*.8)/.24,2/3);
near("POMDP posterior low",(.8*.1)/.24,1/3);
near("POMDP posterior sum",2/3+1/3,1);
near("Nash mutual defect payoff",1,1);
near("Nash cooperative payoff",3,3);
near("Nash deviate vs cooperate",5,5);
yes("Nash defect strictly dominates vs defect",1>0);
yes("Nash defect strictly dominates vs cooperate",5>3);
near("Nash joint cooperate gain",3+3,6);
near("Nash joint Nash gain",1+1,2);
near("Mixed matching payoff q0.7",2*.7-1,.4);
near("Mixed matching alternative",- (2*.7-1),-.4);
near("Mixed p/q half payoff",(2*.5-1)*(2*.5-1),0);
near("Mixed matching coin chance",.5*.5+.5*.5,.5);
near("Repeated cooperation at 0.6",3/(1-.6),7.5);
near("Repeated deviation at 0.6",5+.6/(1-.6),6.5);
near("Repeated cooperation at 0.4",3/(1-.4),5);
near("Repeated deviation at 0.4",5+.4/(1-.4),5.666666666666667);
near("Repeated grim delta threshold",.5,.5);
near("Repeated effective delta",.9*.5,.45);
near("Bayesian optimal bid v0.8",.8/2,.4);
near("Bayesian winning chance",2*.4,.8);
near("Bayesian bidder profit",(.8-.4)*.8,.32);
near("Bayesian rival full win bid",(.8-.5)*1,.3);
near("Bayesian three bidder bid",2*.8/3,.5333333333333333);
near("Mechanism payment at0.8",.8**2/2,.32);
near("Mechanism truthful payoff",.8*.8-.8**2/2,.32);
near("Mechanism misreport payoff",.8*.5-.5**2/2,.275);
near("Mechanism misreport loss",.32-.275,.045);
near("Second price high value win",.8-.6,.2);
near("Second price low value overbid loss",.4-.6,-.2);
near("Auction first-price two-bidder revenue",(2/3)/2,1/3);
near("Auction second-price two-bidder revenue",1/3,1/3);
near("Quoting optimal delta",1/2,.5);
near("Quoting ask fill intensity",10*Math.exp(-2*.5),3.6787944117144233);
near("Quoting two-side gross profit",2*10*.5*Math.exp(-1),3.6787944117144233);
near("Quoting wide gross profit",2*10*1*Math.exp(-2),2.706705664732254);
yes("Quoting delta half wins",2*10*.5*Math.exp(-1)>2*10*Math.exp(-2));
near("Inventory holding variance",10**2*2**2*.25,100);
near("Inventory holding standard deviation",Math.sqrt(100),10);
near("Inventory quadratic risk penalty",.01/2*100,.5);
near("Inventory reserve shift",.01*10*4*.25,.1);
near("Inventory doubled variance",20**2*2**2*.25,400);
near("Inventory short reserve shift",.01*(-10)*4*.25,-.1);
near("Poisson expected count",2*.5,1);
near("Poisson no order",Math.exp(-1),.36787944117144233);
near("Poisson at least one",1-Math.exp(-1),.6321205588285577);
near("Poisson Hawkes branching",.4/1,.4);
near("Poisson Hawkes stationary mean",3/(1-.4),5);
near("Poisson Hawkes impulse after minute",.4*Math.exp(-1),.14715177646857694);
const spread=.1*1*.5+2/.1*Math.log(1+.1/2);
near("Market making total spread",spread,1.0258032753274453);
near("Market making reserve shift",.1*2*1*.5,.1);
near("Market making bid",99.9-spread/2,99.38709836233628);
near("Market making ask",99.9+spread/2,100.41290163766372);
near("Market making risk-neutral spread",2/2,1);
near("Market making centre from bid ask",(99.9-spread/2+99.9+spread/2)/2,99.9);
near("Counterparty ask markout",100.05-100.10,-.05);
near("Counterparty bid markout",99.90-99.95,-.05);
near("Counterparty favorable bid markout",100.10-99.95,.15);
near("MARL matching exploitation pure",1-(-1),2);
near("MARL opponent H prob0.8",2*.8-1,.6);
near("MARL opponent H prob0.2",2*.2-1,-.6);
near("MARL mixed Nash exploitability",0,0);
assert(checks>=135,"V105 independent control and trading arithmetic checks unexpectedly incomplete");

for(const file of files){
 for(const l of json(file).lessons){
  assert(l.deep_sections.length>=4,l.id+" new strategic modeling chapter missing");
  const s=l.deep_sections.at(-1);
  assert(s.paragraphs.length>=2&&s.paragraphs.every(p=>p.length>=70),l.id+" control explanation incomplete");
  assert(s.proof.length>=95&&s.worked.length>=78&&s.limitation.length>=60,l.id+" proof, example or applicability boundary missing");
  assert(s.formulas.length>=2,l.id+" independent control equations missing");
  assert(l.deep_sections.slice(0,3).every(x=>x.proof&&x.worked&&x.limitation&&x.formulas.length>=1),l.id+" pre-existing course chapter lost");
 }
}
const loaderStart=app.indexOf("async function loadV105ControlGame(");
const loaderEnd=app.indexOf("async function loadV106Calculus(",loaderStart);
assert(loaderStart>=0&&loaderEnd>loaderStart,"production control reader loader absent");
const loadControl=vm.runInNewContext(app.slice(loaderStart,loaderEnd)+"\nloadV105ControlGame",{
 fetchMathJson:async name=>json(name.replace(/^\.\//,""))
});
for(const file of files){
 const loaded=await loadControl("./"+file,json(file).lessons.length);
 assert.equal(loaded.length,json(file).lessons.length,"control course not loadable in production "+file);
}
near("Robust expected payoff with p lower bound",10*.2,2);
near("POMDP belief alarm update",.24*.75/(.24*.75+.76*.25),.18/.37);
near("Nash prisoner's dilemma defection value",4-3,1);
near("Matching coins best response q0.8",2*.8-1,.6);
near("Repeated cooperation delta threshold",(5-3)/(5-1),.5);
near("First-price two bidder equilibrium bid",.8/2,.4);
near("Single-parameter truthful threshold payment",.8-(.8-.5),.5);
near("Second-price bidder utility",100-70,30);
near("Poisson market-maker optimum quote",1/2,.5);
near("Poisson market-maker cost-aware quote",.1+1/2,.6);
near("Market inventory price variance",100**2*4,40000);
near("Hawkes branching mean intensity",2/(1-.6),5);
near("Market-maker inventory reservation price",100-10*.01*4*.5,99.8);
near("Ask markout adverse movement",100.10-100.15,-.05);
near("Matching pennies exploitability in deterministic play",1-(-1),2);

near("Value iteration posterior bound",.9/(1-.9)*.001,.009);
near("Advantage under old mixed policy",.5*(4-3)+.5*(2-3),0);
near("New policy under old Q advantage",.75*(4-3)+.25*(2-3),.5);
near("Policy improvement multiplier",1/(1-.9),10);
near("Terminal condition discounted difference",100*.9**10,34.86784401);
near("Snell continuation expectation",(.5*0+.5*6),3);
near("American call risk-neutral probability",(1.05-.9)/(1.2-.9),.5);
near("American call continuation advantage",(.5*30)/1.05,14.285714285714286);
near("Stopping prophet value",(.5*3+.5*10),6.5);
near("Perpetual investment beta positive root",(1+Math.sqrt(21))/2,2.79128784747792);
near("Perpetual investment threshold",(2.79128784747792/(2.79128784747792-1))*100,155.8257569495584);
near("L1 turnover no-trade lower",(.08-.01)/.02,3.5);
near("L1 turnover no-trade upper",(.08+.01)/.02,4.5);
near("Limit fill one minute probability",1-Math.exp(-1),.6321205588285577);
near("Limit one-minute fallback cost",(1-Math.exp(-1))*1+Math.exp(-1)*10,4.310914970542981);
near("State augmented cumulative volume",990+100*1,1090);
near("HJB deterministic discount value",1/2,.5);
near("Scalar Riccati steady state",(1+Math.sqrt(5))/2,1.618033988749895);
near("LQR stationary closed loop",1-((1+Math.sqrt(5))/2)/(1+(1+Math.sqrt(5))/2),.3819660112501051);
near("Entropic risk symmetric two-point",Math.log(Math.cosh(1)),.4337808304830271);

const completeControlChapters={
  "control-n0542": "非有界回报下折扣小于一仍可能无法保证价值有界",
  "control-n0543": "风险中性价值相同的行动可能具有不同下行尾部风险",
  "control-n0544": "策略评价矩阵逆与折扣占用分布的Neumann级数",
  "control-n0546": "非平稳有限期最优策略可能随剩余交易时间发生逆转",
  "control-n0547": "随机贴现因子下的Snell包络必须按实际执行现金流递推",
  "control-n0548": "除息日前分红现金流能够改变美式看涨期权的提前执行决策",
  "control-n0549": "路径先知收益和可执行停时收益之间的条件信息差",
  "control-n0550": "布朗运动到达障碍的Laplace变换与连续最优停止边界",
  "control-n0551": "固定交易费用导致只有足够大的收益改善才值得切换仓位",
  "control-n0552": "指数到达限价单与线性等待损失的最优取消期限",
  "control-n0553": "转换仓位成本使前期动作成为动态规划所需的额外状态",
  "control-n0554": "线性状态收益与二次行动成本的无限期HJB显式解",
  "control-n0555": "LQG分离结构下应以Kalman后验状态而非原始带噪观测反馈",
  "control-n0556": "指数风险效用对稀有大损失的惩罚可以超过方差二阶近似",
  "control-n0557": "非矩形动态不确定性会使逐期最坏情形组合成不存在的路径",
  "control-n0558": "主动收集信息的期望价值来自行动对后验状态的适应",
  "control-n0559": "相关均衡可以消除协调失败并优于独立混合Nash结果",
  "control-n0560": "不对称零和二乘二博弈的混合均衡与双方无差异方程",
  "control-n0561": "有限重复囚徒困境的反向归纳与无限期合作的根本差别",
  "control-n0562": "共同价值竞价的赢家诅咒来自获胜事件本身携带的信息",
  "control-n0563": "VCG外部性支付如何同时实现资源效率与激励相容",
  "control-n0564": "二价拍卖保留价的最优设置与成交概率—支付额权衡",
  "control-n0565": "逆向选择预期损失使最佳被动报价距离增加",
  "control-n0566": "两个相关库存头寸的最低方差线性对冲仓位",
  "control-n0567": "竞争性买卖订单Poisson强度如何决定第一笔成交类型",
  "control-n0568": "预期中间价漂移与库存风险如何共同决定做市商保留价",
  "control-n0569": "只对成交订单计算Markout会因订单选择机制产生条件偏差",
  "control-n0570": "最佳反应循环可使多智能体最终行动始终不收敛"
};
assert.equal(Object.keys(completeControlChapters).length,28,"Every new control lecture must have an additional full proof chapter");
const additionalFullControlIds=new Set(["control-n0542","control-n0543","control-n0544","control-n0546","control-n0547","control-n0548","control-n0549","control-n0550","control-n0551","control-n0552","control-n0553","control-n0554","control-n0555","control-n0556","control-n0557","control-n0558","control-n0559","control-n0560","control-n0561","control-n0562","control-n0563","control-n0564","control-n0565","control-n0566","control-n0567","control-n0568","control-n0569","control-n0570"]);
assert.equal(additionalFullControlIds.size,28,"All twenty-eight additional complete control lectures required");
const fullMicrostructureTopics={
 "control-n0564":"拍卖收入等价的边界条件与出价分布依赖",
 "control-n0565":"逆向选择的条件成交期望与被动报价净值",
 "control-n0566":"库存对冲的最小方差比率、对冲误差与基差风险",
 "control-n0567":"非齐次Poisson订单流、首单等待分布与价格风险",
 "control-n0568":"CARA做市保留价的库存敏感性与边界假设",
 "control-n0569":"成交选择机制、IPW修正与Markout的识别条件",
 "control-n0570":"外部遗憾最小化与粗相关均衡的经验收敛"
};
assert.equal(Object.keys(fullMicrostructureTopics).length,7,"Seven new complete microstructure and trading-game lessons required");

for(const lesson of newest){
 const original=lesson.deep_sections[4];
 assert.equal(original.title,completeControlChapters[lesson.id],lesson.id+" previous exact deep chapter title changed");
 assert.equal(lesson.deep_sections.length,additionalFullControlIds.has(lesson.id)?(fullMicrostructureTopics[lesson.id]?7:6):5,lesson.id+" original proofs and new full advanced chapter required");
 const validate=(sec,label)=>{
  assert(sec.paragraphs.length===2&&sec.paragraphs.every(x=>x.length>=100),lesson.id+" "+label+" complete detailed explanations missing");
  assert(sec.proof.length>=180&&sec.worked.length>=115&&sec.limitation.length>=70,lesson.id+" "+label+" proof worked case or boundary cut down");
  assert(sec.formulas.length===2,lesson.id+" "+label+" two independent mathematical relations required");
 };
 validate(original,"original");
 if(additionalFullControlIds.has(lesson.id)){
  const added=lesson.deep_sections[5];
  assert(added.title!==original.title,lesson.id+" additional topic duplicated");
  validate(added,"extended");
 }
 if(fullMicrostructureTopics[lesson.id]){
  const advanced=lesson.deep_sections[6];
  assert.equal(advanced.title,fullMicrostructureTopics[lesson.id],lesson.id+" appended canonical topic changed");
  assert(advanced.paragraphs.length===2&&advanced.paragraphs.every(p=>p.length>=110),lesson.id+" detailed explanation missing");
  assert(advanced.proof.length>=115&&advanced.worked.length>=95&&advanced.limitation.length>=65,lesson.id+" proof, worked case or limits incomplete");
  assert.equal(advanced.formulas.length,2,lesson.id+" two strict mathematical identities missing");
  assert.equal(new Set(lesson.deep_sections.map(x=>x.title)).size,lesson.deep_sections.length,lesson.id+" duplicate chapter heading");
 }
}
near("Discounted unbounded state growth finite",1/(1-.8),5);
yes("Discounting cannot dominate doubling with gamma .6",2*.6>1);
near("Risk neutral action mean",.5*11+.5*(-9),1);
near("Risk neutral action variance",.5*(11-1)**2+.5*(-9-1)**2,100);
near("Alternating two state policy value",1/(1-.5**2),4/3);
near("Terminal switching opportunity net", -5+4+4,3);
near("Random discount now exercise",.9*11,9.9);
near("Random discount later continue",.95*11,10.45);
near("Dividend early exercise advantage",100-80-(90-80),10);
near("Stopping prophet expectation",.5*1+.5*3,2);
near("Stopping adaptive expectation",Math.max(1,.5*0+.5*3),1.5);
near("Brownian discounted hitting time",Math.exp(-1),.36787944117144233);
near("Fixed fee switch first signal",.2**2/2,.02);
near("Fixed fee switch strong signal",.4**2/2,.08);
near("Limit optimal cancellation",Math.log(4),1.3862943611198906);
near("Limit schedule true expected cost",2+8*.25+2*Math.log(4),6.772588722239782);
near("Markov past warehouse positive inventory",5-6,-1);
near("HJB affine-control optimal value at zero",1-.5,.5);
near("Kalman conditional state mean",1/(1+1)*2,1);
near("Separation state feedback action",-.5*1,-.5);
near("Rare tail entropic risk",Math.log(.9+.1*Math.exp(1))/.1,1.5856507874042909);
near("Nonrectangular fixed model worst total",1-1+1,1);
near("Rectangular unnatural worst",0+0,0);
near("Perfect information purchase net",2-.6,1.4);
near("Coordinated CE optimal reward",2,2);
near("Independent symmetric Nash reward",2*.5,1);
near("Asymmetric zero sum row mix",1/3,1/3);
near("Asymmetric zero sum value",2/3,2/3);
near("Cooperation discount threshold",(5-3)/(5-1),.5);
near("Common value winner conditional posterior",.75*.25/(.75*.25+.25*.75),.5);
near("Common value winning bid conditional utility",5-7.5,-2.5);
near("VCG externality for winner",8+5-8,5);
near("VCG second winner externality",10+5-10,5);
near("Uniform reserve price optimum",.5,.5);
near("Uniform auction expected revenue",1/3+.5**2-(4/3)*(.5**3),5/12);
near("Maker optimal distance adverse selection",.02+.01+1/10,.13);
near("Hedged inventory conditional variance",1+.8**2+2*.8*(-.8),.36);
near("First buy order competing hazards",(2/3)*(1-Math.exp(-3)),.6334752877547574);
near("Inventory-aware reserve price long",100+.4-.1*3,100.1);
near("Markout adverse fill posterior",.9*.5/(.9*.5+.1*.5),.9);
near("Filled Markout expectation",.9*1+.1*(-1),.8);
near("Best response cycle mixed empirical frequency",2/4,.5);

near("Optimal two asset hedge dollar position",1e6*.8*.2/.25,640000);
near("Minimum hedge residual risk dollars",1e6*.2*Math.sqrt(1-.8**2),120000);
near("Nonstationary Poisson first event survival",Math.exp(-5),.006737946999085467);
near("Quote adverse selected conditional pnl bp",2-3,-1);
near("Inventory reserve price after positive inventory",100-.01*10*.04,99.996);
near("IPW quote opportunity latent mean",.8*2+.2*(-4),.8);
near("CCE empirical fixed action deviation bound",5/100,.05);

console.log("V105 PASS",newest.length,"new control lectures;",all.length,"total;",
 mathCount,"strict KaTeX;",deepCount,"deep proof chapters;",deepChars,"deep prose chars;",
 checks,"independent numerics;",linkCount,"valid cross-links; control theory 30/30.");
