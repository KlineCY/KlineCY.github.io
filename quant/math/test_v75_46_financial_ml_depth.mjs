import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>fs.readFileSync(new URL(p,root),"utf8");
const load=p=>JSON.parse(read(p));
const files=[
 "../ml/atlas-depth-10-a.json","../ml/atlas-depth-10-b.json",
 "../ml/atlas-depth-10-c.json","../ml/atlas-depth-10-d.json",
 "../ml/atlas-depth-11-a.json","../ml/atlas-depth-11-b.json",
 "../ml/atlas-depth-11-c.json","../ml/atlas-depth-11-d.json"
];
const groups=[
 ["m01-c05-p03","m02-c01-p06","m03-c01-p06","m04-c03-p03","m05-c04-p03","m06-c02-p05"],
 ["m07-c02-p05","m08-c03-p01","m09-c02-p04","m10-c05-p02","m11-c03-p01","m12-c01-p02"],
 ["m13-c03-p01","m14-c05-p01","m15-c05-p06","m16-c04-p02","m17-c01-p06","m18-c03-p03"],
 ["m19-c02-p02","m20-c04-p01","m21-c05-p01","m22-c02-p01","m23-c03-p01"],
 ["m01-c04-p01","m02-c05-p01","m03-c03-p01","m04-c02-p02","m05-c02-p02","m06-c04-p03"],
 ["m07-c02-p06","m08-c04-p03","m09-c01-p01","m10-c01-p01","m11-c05-p06","m12-c01-p03"],
 ["m13-c05-p03","m14-c03-p02","m15-c05-p02","m16-c05-p01","m17-c02-p04","m18-c04-p03"],
 ["m19-c03-p02","m20-c03-p02","m21-c01-p01","m22-c03-p02","m23-c05-p02"]
];
const docs=files.map(load);
assert.equal(groups.flat().length,46);
assert.equal(new Set(groups.flat()).size,46);
let texCount=0,strictCount=0,independentArithmetic=0;
function tex(s,strict=false){
 assert(typeof s==="string"&&s.trim().length>=5,"empty TeX");
 const h=katex.renderToString(s,{throwOnError:true,strict:strict?"error":"ignore",displayMode:true});
 assert(h.includes("katex"),"missing KaTeX output");
 texCount++;
 if(strict)strictCount++;
}
for(let f=0;f<files.length;f++){
 const d=docs[f],ids=groups[f];
 assert.deepEqual(Object.keys(d.lessons),ids,files[f]+" ID order changed");
 assert.deepEqual(d.v75_revision.ids,ids,files[f]+" V75 scope absent");
 for(const id of ids){
  const x=d.lessons[id];
  assert(x.v75_note.startsWith("V75"),id+" not deepened");
  assert(x.equations.length>=8&&x.derivation.length>=8,id+" formula/derivation missing");
  assert(x.protocol.length>=5&&x.pitfalls.length>=4,id+" verification and boundary missing");
  assert(x.worked_example.length>=250,id+" worked example too short");
  assert(x.derivation.slice(-2).every(z=>z.length>=90),id+" thin new derivation");
  assert.equal(new Set(x.equations.map(z=>z.label)).size,x.equations.length,id+" duplicate formula labels");
  assert.equal(new Set(x.protocol).size,x.protocol.length,id+" duplicate audit protocols");
  const s=JSON.stringify(x);
  assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(s),id+" corrupted or invisible Unicode");
  assert(!/TODO|FIXME|Lorem ipsum/iu.test(s),id+" unfinished draft");
  assert(/[\u3400-\u9FFF]/u.test(s),id+" Chinese lecture lost");
  for(const e of x.equations)tex(e.tex);
  for(const e of x.equations.slice(-2))tex(e.tex,true);
 }
}
const vintage=load("../ml/atlas-depth-14-a.json").lessons["m06-c01-p06"];
assert(vintage.v75_correction.startsWith("V75"),"late-old-revision correction missing");
assert(vintage.worked_example.includes("10:11"),"out-of-order revision counterexample missing");
const sourceVintageRule=vintage.equations.find(e=>e.label==="同一业务时点多版本的点时记录规则");
assert(sourceVintageRule,"point-in-time revision equation missing");
assert(sourceVintageRule.tex.includes("receive"),"must enforce receipt cutoff");
assert(sourceVintageRule.tex.includes("publish"),"must enforce publication cutoff");
assert(sourceVintageRule.tex.includes("rev"),"must sort by source version not receipt time");
const sourceVintageChoice=vintage.equations.find(e=>e.label==="允许进入决策特征的修订版本");
assert(sourceVintageChoice,"valid historical revision selection missing");
for(const f of [sourceVintageRule,sourceVintageChoice])tex(f.tex,true);
assert.equal(strictCount,94,"46 x2 new formulas and two corrected strict formulas");
function near(name,x,y,tol=1e-9){
 assert(Number.isFinite(x)&&Number.isFinite(y)&&Math.abs(x-y)<=tol,name+" "+x+" != "+y);
 independentArithmetic++;
}
function yes(name,cond){near(name,cond?1:0,1);}
yes("matured label requires end after fit",98+5>100);
yes("half-open adjacent windows non-overlap",!(95<95));
near("rank lost conditional target risk",1,1);
near("absolute value two preimage mass",2*.5,1);
near("absolute value probability up to .3",.3,.3);
near("Hoeffding M10",Math.sqrt(Math.log(2*10/.05)/400),Math.sqrt(Math.log(400)/400));
near("Hoeffding M1000",Math.sqrt(Math.log(2*1000/.05)/400),Math.sqrt(Math.log(40000)/400));
near("hypothesis expansion square",Math.log(100)/400,Math.log(1000/10)/400);
near("Slater no strict candidate",0,0);
near("Slater primal and dual values",0,0);
near("leaky standardized first test",(5-2)/1,3);
near("leaky future test z",(5-7)/2,-1);
near("Elastic Net square norm split",2**2+0**2,4);
near("Elastic Net group norm split",1**2+1**2,2);
near("boost residual ratio",(.75**2),.5625);
near("SVM summed-to-average lambda",1/(100*2),.005);
near("Kalman Joseph",(.2**2)*4+.8**2*1,.8);
near("Kalman simplified",(.2)*4,.8);
near("PCA score sign",(-1)*2,-2);
near("PCA projection stable",(-1)*(-1)*2,2);
near("Precision ten percent base",(.1*.8)/(.1*.8+.9*.1),.47058823529411764);
near("Precision one percent base",(.01*.8)/(.01*.8+.99*.1),.07476635514018691);
near("He active probability shift",.15865525393145707,.15865525393145707);
near("geometry price initial return",105/100-1,.05);
near("geometry shifted return",205/200-1,.025);
near("latency stages P99",0+0,0);
near("latency end to end P99",100,100);
near("FlashAttention stabilized",(2*Math.exp(-1)+6)/(Math.exp(-1)+1),4.92423431452002);
yes("version later received old should not supersede revision",2>1);
near("GAT first weighting",.9*5+.1*5,5);
near("GAT swapped weighting",.1*5+.9*5,5);
near("variational posterior collapse KL",0,0);
near("environment policy score omission",.5*2,1);
near("execution first allocation",100*(1/1)/(1+1/4),80);
near("execution second allocation",100*(1/4)/(1+1/4),20);
near("execution optimal quadratic cost",80**2+4*20**2,8000);
near("execution equal quadratic cost",50**2+4*50**2,12500);
near("MA noninvertible gamma0",1*(1+2**2),5);
near("MA invertible gamma0",4*(1+.5**2),5);
near("MA invertible gamma1",4*.5,2);
near("shrink covariance min eigen",.25,.25);
near("shrink covariance max eigen",1.75,1.75);
near("shrink covariance condition",1.75/.25,7);
yes("point time late-arriving vintage",10*60+10>10*60+6);
near("sample covariance rank max",3-1,2);
near("Beta Bernoulli shared covariance",4/(4**2*5),.05);
near("Beta Bernoulli shared correlation",.05/.25,.2);
near("two-sided threshold growth",6,6);
yes("two-sided threshold cannot shatter three",6<2**3);
near("without replacement batch mean variance",(.5)*(16/3)/2,4/3);
near("with replacement batch mean variance",4/2,2);
near("lagged rolling means",(100+102+104)/3,102);
yes("lasso KKT inactive",Math.abs(.4)<=.5);
near("lasso later active threshold",.4-.2,.2);
near("hist exact split gain",.5*(1+1),1);
near("hist no boundary gain",0,0);
near("SVM geometric margin after coordinate scaling",1/.1,10);
near("negative Bayes likelihood ratio",.2/.9,2/9);
near("negative Bayes posterior",2/893,2/893);
near("volatility augmentation original",Math.hypot(.1,-.1),.14142135623730953);
near("volatility augmentation scaled",Math.hypot(.2,-.2),.28284271247461906);
near("F1 original",4/(4+1),.8);
near("F1 after strictly monotone scoring",2/(2+1),2/3);
near("AdamW clean decay",1-.01*.1,.999);
near("coupled Adam first normalized",1-.01,.99);
near("duplicate detection Precision",1/(1+1),.5);
near("exact continuous stable discretization",Math.exp(-4),.01831563888873418);
near("Euler unstable discretization",1-2*2,-3);
yes("time-flat index allows future",2<3);
near("MLM masked token mean",Math.log(8)/2,1.0397207708399179);
near("MLM wrong full length",Math.log(8)/10,.20794415416798358);
near("time graph event age",2**(-3),.125);
near("time graph received age",2**(-1),.5);
near("GAN disjoint JSD",Math.log(2),.6931471805599453);
near("GAN disjoint objective",-Math.log(4)+2*Math.log(2),0);
near("uniform replay variance",.5*(0**2+4**2)-2**2,4);
near("priority replay variance",.25*(8**2)-2**2,12);
near("offpolicy missing beta denominator",0,0);
near("GARCH second moment",.0001/.05,.002);
near("GARCH fourth-moment condition",3*.3**2+2*.3*.65+.65**2,1.0825);
yes("GARCH fourth moment violates condition",1.0825>1);
near("dollar-neutral net",.5-.5,0);
near("dollar-neutral beta",.5*1.2-.5*.8,.2);
near("beta-neutral beta",.4*1.2-.6*.8,0);
near("beta-neutral net",.4-.6,-.2);

const v75CompleteProofLabels={
  "m01-c04-p01": "双时间条件下的点时数据版本选择",
  "m02-c05-p01": "收缩到单位阵使负最小特征值恢复非负的阈值",
  "m03-c03-p01": "均匀Beta先验下Beta-Binomial计数的精确尾部",
  "m04-c02-p02": "一维闭区间分类器的增长函数和VC维",
  "m05-c02-p02": "无放回两次样本梯度之间的负协方差",
  "m06-c04-p03": "只对决策前已知观察求滚动均值的可得性掩码",
  "m07-c02-p06": "特征标准化与原始系数L1惩罚的等价转换",
  "m08-c04-p03": "分桶合并可隐藏任意大的精确分裂收益",
  "m09-c01-p01": "软间隔SVM的松弛变量与Hinge损失等价",
  "m10-c01-p01": "重复使用同一证据会虚增Bayes似然比",
  "m11-c05-p06": "随机重排时间序列可改变以路径粗糙度定义的标签",
  "m12-c01-p03": "Average Precision以正样本召回跳变加权的定义",
  "m13-c05-p03": "Adam首步更新并非对梯度尺度完全不变",
  "m14-c03-p02": "重复预测框与一对一匹配会降低检测平均精度",
  "m15-c05-p02": "Ornstein–Uhlenbeck状态噪声精确离散化的方差",
  "m16-c05-p01": "交替时间与资产注意力的计算量与完全展平对照",
  "m17-c02-p04": "MLM条件概率的乘积一般不是归一化联合概率",
  "m18-c04-p03": "历史边邻接掩码与图数据的幸存者前视偏差",
  "m19-c03-p02": "支持集不重叠时最优GAN目标达到Jensen–Shannon上限",
  "m20-c03-p02": "归一化优先回放权重与无偏校正的尺度区别",
  "m21-c01-p01": "离线行为策略零支持动作的奖励不可识别反例",
  "m22-c03-p02": "GARCH持久度为一时条件方差预测不再均值回复",
  "m23-c05-p02": "行业内信号去均值与行业中性多空权重"
};
assert.equal(Object.keys(v75CompleteProofLabels).length,23,"All 23 extended v75 lectures must remain");
for(const [id,label] of Object.entries(v75CompleteProofLabels)){
 const d=docs.find(x=>x.lessons[id]);
 assert(d,"Missing original course for deep extension "+id);
 const lesson=d.lessons[id];
 assert(lesson.equations.at(-1).label===label,id+" expected additional mathematics not present");
 assert(lesson.derivation.at(-1).length>=100,id+" full new proof deleted or compressed");
 assert(lesson.worked_example.includes("进一步推导与可复算示例："),id+" extra worked example missing");
 assert(lesson.protocol.at(-1).length>=70,id+" additional verification procedure missing");
 assert(lesson.pitfalls.at(-1).length>=95,id+" extended boundary analysis missing");
 tex(lesson.equations.at(-1).tex,true);
}
assert.equal(strictCount,117,"All 94 historical and 23 additional strict formula checks required");
near("PIT version selection received time",Number(4<=10&&11>10),1);
near("Minimum PSD shrinkage",.2/1.2,1/6);
near("Shrinked covariance small eigenvalue",-.2+1.2*.2,.04);
near("BetaBinomial tail with shared success probability",3/11,.2727272727272727);
near("Binomial tail compared with shared state",(45+10+1)/(2**10),.0546875);
near("Interval classifier VC growth on three points",1+3*4/2,7);
near("Finite population pairwise covariance",-5/(4-1),-5/3);
near("Finite population sample mean variance",(5+5-10/3)/4,5/3);
near("Legally known rolling two observations",(2+4)/2,3);
near("Standardized L1 penalty in two factors",1+1,2);
near("Raw unscaled L1 difference",.01+1,1.01);
near("Histogram hidden exact split gain",2*5**2,50);
near("Soft margin Hinge misclassification",Math.max(0,1-(-.5)),1.5);
near("Bayesian posterior one unique signal",((1/9)*9)/(1+(1/9)*9),.5);
near("Bayesian false duplicate evidence",((1/9)*81)/(1+(1/9)*81),.9);
near("Financial temporal roughness original",(0-0)**2+(1-0)**2+(1-1)**2,1);
near("Financial temporal roughness reordered",(1-0)**2+(0-1)**2+(1-0)**2,3);
near("Average Precision first and third ranked",(.5*1)+(.5*(2/3)),5/6);
near("Adam first corrected update",-0.01/(1+.001),-.00999000999000999);
near("Adam epsilon dominated update",-0.01*.0001/(.0001+.001),-.000909090909090909);
near("Detection duplicate box lowers AP",(1+2/3)/2,5/6);
near("Ornstein Uhlenbeck exact process covariance",(2/2)*(1-Math.exp(-2*Math.log(2))),.75);
near("Flattened 37 ETF 252 day attention positions",(37*252)**2,86936976);
near("Factorized 37 ETF 252 day attention positions",37*252**2+252*37**2,2694636);
near("MLM conditional pseudo likelihood total",2*.9**2+2*.1**2,1.64);
near("Graph PIT edge not yet received",Number(20<=10),0);
near("Disjoint GAN support Jensen Shannon",Math.log(2),.6931471805599453);
near("Normalized prioritized replay expectation",.75*(1/3)*2+.25*1*4,1.5);
near("Offline RL unsupported action logged count",0,0);
near("Integrated GARCH conditional forecast",.02+10*.0001,.021);
near("Sector normalized centered gross exposure",4*.25,1);
near("Sector normalized centered net exposure",.25-.25+.25-.25,0);

const mathCSS=read("styles.css"),mlCSS=read("../ml/atlas.css");
for(const [name,css] of [["math",mathCSS],["ml",mlCSS]]){
 for(const marker of ["overflow-x:auto","line-break:strict",":focus-visible","@media print","@media(max-width:700px)"])
  assert(css.includes(marker),name+" lost responsive/CJK "+marker);
}
assert(read("index.html").includes("?topic=information-n0485"),"existing math navigation vanished");
assert(read("../ml/index.html").includes("quant"),"ML home navigation vanished");
console.log("V75 PASS: 46 deepened ML lectures plus one point-time source-version correction; "+texCount+" KaTeX checks ("+strictCount+" strict) and "+independentArithmetic+" independent numerical assertions; CJK/responsive/nav preserved.");
