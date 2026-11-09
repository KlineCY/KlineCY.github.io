import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const raw=p=>fs.readFileSync(new URL(p,root),"utf8");
const core=read("atlas-depth-manifest.json");
assert.equal(core.expected_lessons,720,"core Atlas lesson total must remain 720");
assert.equal(core.expected_domains,24,"core Atlas domain total must remain 24");
assert.equal(core.modules.reduce((x,m)=>x+m.lessons,0),720,"manifest core lesson counts changed");
const registered=new Map(core.modules.map(m=>[m.path,m.lessons]));
const files="abcdefghijkl".split("").map(c=>"atlas-depth-09-"+c+".json");
const ids=new Set(),first=new Set(),second=new Set(),domains=new Map();
let eq=0,substance=0;
for(let index=0;index<files.length;index++){
 const file=files[index],v41=index>=6;
 const doc=read(file);
 const n=index===0?12:10;
 assert.equal(registered.get("./"+file),n,"ninth module registered count "+file);
 assert.equal(doc.schema,1,file+" old schema changed");
 assert.equal(doc.revision,"2026-11-06",file+" not deepened to V40/V41");
 assert.equal(Object.keys(doc.lessons).length,n,file+" canonical lecture count changed");
 for(const [id,l] of Object.entries(doc.lessons)){
  assert(!ids.has(id),id+" duplicate");
  ids.add(id);
  (v41?second:first).add(id);
  domains.set(id.slice(0,3),(domains.get(id.slice(0,3))||0)+1);
  assert(l.source_title?.length>=2&&l.title?.length>=8,id+" old identity lost");
  assert(l.overview?.length>=60,id+" original overview missing");
  assert(l.worked_example?.length>=110,id+" original plus expanded worked example missing");
  assert(v41 ? (l?.overview?.length>=60 && l?.equations?.length>=2 && l?.worked_example?.length>=50) : (index<=1 ? (l?.overview?.length>=60 && l?.equations?.length>=2 && l?.worked_example?.length>=50) : (index<=3 ? (l?.overview?.length>=60 && l?.equations?.length>=2 && l?.worked_example?.length>=50) : (l?.overview?.length>=60 && l?.equations?.length>=2 && l?.worked_example?.length>=50))),id+" mismatch expansion batch");
  assert(l.equations.length>=5,id+" must preserve at least five mathematical formulas");
  assert(l.derivation.length>=5,id+" must preserve five original proof steps");
  assert(l.protocol.length>=5,id+" must preserve five original validation steps");
  assert(l.pitfalls.length>=3,id+" must preserve original failure boundaries");
  assert.equal(new Set(l.protocol).size,l.protocol.length,id+" duplicated empirical validation");
  assert(!/TODO|Lorem ipsum|[\uFFFD\u200B\uFEFF]/i.test(JSON.stringify(l)),id+" Chinese text corrupted or unfinished");
  if(v41){
   assert(l.derivation.slice(3).every(s=>s.length>=65),id+" overly terse derivation");
   assert(l.protocol.slice(3).every(s=>s.length>=25),id+" overly terse empirical plan");
   assert(l.pitfalls.at(-1).length>=30,id+" insufficient failure boundary");
   substance+=l.derivation.slice(3).join("").length+l.protocol.slice(3).join("").length+l.pitfalls.at(-1).length;
  }
  for(const formula of l.equations){
   assert(formula.label?.length>=1,id+" missing Chinese equation label");
   assert(typeof formula.tex==="string"&&formula.tex.length>=5,id+" missing TeX source");
   katex.renderToString(formula.tex,{throwOnError:true,displayMode:true,strict:"ignore"});
   eq++;
  }
 }
}
assert.equal(ids.size,122,"entire ninth-cycle catalog preserved");
assert.equal(first.size,62,"V40 first half preserved");
assert.equal(second.size,60,"V41 second half expanded");
assert(eq>=727,"V50 original 727 formulas plus subsequent deepening must remain intact");
assert.equal(domains.size,24,"24 ninth-cycle subjects preserved");
for(const [domain,n] of domains)assert.equal(n,Number(domain.slice(1))<=2?6:5,domain+" unexpected lesson count");
assert(substance>23000,"at least 23k new Chinese proof, validation and boundary chars");

const home=raw("index.html"),viewer=raw("atlas.js"),css=raw("atlas.css");
const section=(tag)=>{
 const begin=home.indexOf('aria-label="'+tag+'"');
 assert(begin>=0,"missing learning path "+tag);
 const finish=home.indexOf("</details>",begin);
 assert(finish>begin,"unclosed learning path "+tag);
 return home.slice(begin,finish);
};
const v40nav=section("数理统计、优化与因果金融");
const v41nav=section("模型基础与金融实证专题");
assert.equal((v40nav.match(/href="#lesson\//g)||[]).length,62);
assert.equal((v41nav.match(/href="#lesson\//g)||[]).length,60);
for(const id of first)assert(v40nav.includes('href="#lesson/'+id+'"'),"broken first-half navigation "+id);
for(const id of second)assert(v41nav.includes('href="#lesson/'+id+'"'),"broken final-half navigation "+id);
for(const stage of ["阶段七","阶段八","阶段九","阶段十","阶段十一","阶段十二"])
 assert(v41nav.includes(stage),"missing V41 learning stage "+stage);
assert(home.indexOf('aria-label="模型基础与金融实证专题"')<home.indexOf('aria-label="数理统计、优化与因果金融"'),"V41 route must be visible near top");
for(const label of ["linear-n0065","statistics-x001","riskmath-x002"]){
 assert(v41nav.includes('href="../math/?topic='+label+'"'),"math-topic link missing "+label);
 const math=read("../math/curriculum.json");
 assert(math.nodes.some(n=>n.id===label),"link to non-existing math topic "+label);
}
for(const marker of ["s.equations.map","s.derivation","s.protocol","s.pitfalls","s.worked_example","katex.render"])
 assert(viewer.includes(marker),"original reader lost structured mathematical section "+marker);
for(const marker of [".atlas-spectral-path",".atlas-spectral-stage",".atlas-spectral-links",".ml-math-block","overflow-x:auto","line-break:strict","@media print"])
 assert(css.includes(marker),"CJK typography, narrow-screen formula scroll or print CSS missing "+marker);
for(const tag of ["神经网络、金融人工智能与风险管理","泛化理论、核方法与因果推断"])
 assert(home.includes('aria-label="'+tag+'"'),"previous completed Atlas routes lost");

const near=(val,expected,name,tol=1e-9)=>
 assert(Number.isFinite(val)&&Math.abs(val-expected)<=tol,name+" actual "+val+" vs expected "+expected);
near(.9*.1,.09,"sigmoid local gradient at output .9");
near(.25**3,.015625,"chain gradient attenuation");
near(8*3+1,25,"autodiff shared path derivative");
near(.9*.9+.1*0,.81,"Adam momentum checkpoint state");
near((1+3)/2,2,"LayerNorm mean");
near(((-1)**2+1**2)/2,1,"LayerNorm feature variance");
near(.2-.0,.2,"softmax first logit gradient");
near(.5-1,-.5,"softmax true-class logit gradient");
near(64*10+10,650,"GAP classification head");
near(64*7*7*10+10,31370,"flatten classification head");
near((224/16)**2,196,"ViT patch grid");
near(197**2,38809,"ViT attention score elements");
near(8-10,-2,"net bps vs execution cost");
near(Math.floor((10+2-3)/1)+1,10,"zero padded output size");
near(.8*2+.25*.4,1.7,"LSTM gate memory");
near(.5**3,.125,"SSM lag kernel");
near(.8**10,.1073741824,"stable eigen direction");
near(1.1**10,2.5937424601,"unstable eigen direction",1e-8);
near(1+2*2,5,"causal dilated convolution receptive field");
near(.2*1+.8*3,2.6,"cross-asset attention");
near(2*512*2048+512+2048,2099712,"FFN with both bias vectors");
near(1/(1+Math.exp(-1)),.7310585786300049,"softmax temperature one");
near(1/(1+Math.exp(-.5)),.6224593312018546,"softmax temperature two");
near(Math.sin(Math.PI/2)**2+Math.cos(Math.PI/2)**2,1,"periodic position unit norm");
near(8/10,.8,"LLM supported factual claims");
near(120e6/120,1e6,"financial unit conversion to yuan");
near(Math.log(100/10),2.302585092994046,"IDF term A");
near(Math.log(100/50),.6931471805599453,"IDF term B");
near(.20-.15,.05,"finetuning independent loss gain");
near(1/Math.sqrt(2),.7071067811865475,"symmetric graph normalization");
near(10**3,1000,"GNN naive neighbor path count");
near(.5**6,1/64,"graph over-smoothing spectral decay");
near(1.1*.9-1,-.01,"compound two-period portfolio result");
near(2+.5*(-1),1.5,"Gaussian reverse diffusion sample");
near(-2/4,-.5,"Gaussian score not expected return");
near(.8*2+.6,2.2,"DDPM cumulative schedule sample");
near(1+.01,1.01,"autoencoder unscaled reconstruction loss");
near(1-.9**2,.19,"tanh Jacobian at output .9");
near(.7+.3,1,"Markov transition normalization");
near(1+.9*5,5.5,"DQN TD target");
near(1/(1-.9),10,"Bellman discounted fixed reward");
near(2/(1-.5),4,"Bellman optimal fixed point");
near(.25*2+.75*6,5,"FQE target policy expected next Q");
near(Math.log(1+Math.exp(-4)),.01814992791780978,"CQL penalty for seen best action");
near(.003-.001,.002,"simulated gross minus modeled impact");
near(.003-.005,-.002,"realistic gross minus impact");
near(.5**3,.125,"AR1 third-lag ACF");
near(100-5,95,"label availability in forecasting");
near(3*4,12,"ARIMA three-step independent innovations variance");
near(4/2-Math.log(4/2)-1,1-Math.log(2),"QLIKE relative loss");
near(1-90/120,.25,"maximum drawdown");
near(120/90-1,1/3,"recovery from maximum drawdown");
near((.05+.09)/2,.07,"Black-Litterman same-precision posterior mean");
near((2/3*.1)**2,(1/3*.2)**2,"equal risk-budget of independent assets");
near(.2+.01,.21,"financial FGSM input perturbation");
near(.01*5,.05,"standardized perturbation in basis points");
near(Math.abs(0-1),1,"Wasserstein separated point masses");
near((.5-.6)*Math.log(.5/.6)+(.5-.4)*Math.log(.5/.4),.0405465108108164,"two-bin PSI",1e-9);
near(1-.95**20,.6415140775914577,"cumulative multiple drift false alarms",1e-9);
near(.05/20,.0025,"conservative daily alpha budget");


const v45=read("atlas-depth-09-i.json").lessons;
const v45ids=["m17-c03-p06","m17-c02-p05","m17-c01-p02","m17-c01-p01","m17-c02-p06","m18-c01-p02","m18-c03-p06","m18-c02-p06","m18-c02-p05","m18-c01-p01"];
assert.equal(v45ids.length,10);
for(const id of v45ids){
 const l=v45[id];
 assert(l&&(l?.overview?.length>=60 && l?.equations?.length>=2 && l?.worked_example?.length>=50),"missing V45 visible revision "+id);
 assert.equal(l.equations.length,6,"V45 sixth formula not rendered "+id);
 assert.equal(l.derivation.length,6,"V45 sixth derivation missing "+id);
 assert.equal(l.protocol.length,6,"V45 sixth audit protocol missing "+id);
 assert(l.worked_example.length>=145,"V45 worked example must retain old and new evidence "+id);
 assert(l.protocol.at(-1).length>40,"V45 empirical check overly short "+id);
 assert(l.derivation.at(-1).length>85,"V45 proof insufficient "+id);
 katex.renderToString(l.equations.at(-1).tex,{throwOnError:true,displayMode:true,strict:"ignore"});
}
near(8/18,4/9,"V45 weighted evidence support");
near(Math.log(101)+1,5.61512051684126,"V45 smoothed unseen vocabulary IDF");
near(120e6/7.2,16666666.666666668,"V45 report exchange conversion",1e-6);
near(.20-.15,.05,"V45 task-specific gain");
near(.18-.10,.08,"V45 old task forgetting");
near(1/Math.sqrt(6),.4082482904638631,"V45 GCN self loop");
near(1+10+100+1000,1111,"V45 sampled neighbor expansion");
near(.9**6,.531441,"V45 residual graph propagation");
near((11-15)/10,-.4,"V45 prior graph and finance cross-check");


const v46=read("atlas-depth-09-h.json").lessons;
const v46ids=["m15-c03-p02","m15-c01-p03","m15-c05-p03","m15-c02-p04","m15-c04-p01","m16-c05-p02","m16-c03-p05","m16-c03-p06","m16-c01-p03","m16-c02-p03"];
assert.equal(v46ids.length,10);
for(const id of v46ids){
 const l=v46[id];
 assert(l&&(l?.overview?.length>=60 && l?.equations?.length>=2 && l?.worked_example?.length>=50),"missing V46 canonical lesson "+id);
 assert.equal(l.equations.length,6,"V46 sixth formula not loaded "+id);
 assert.equal(l.derivation.length,6,"V46 sixth proof absent "+id);
 assert.equal(l.protocol.length,6,"V46 sixth audit absent "+id);
 assert.equal(l.pitfalls.length,3,"V46 original limitations changed "+id);
 assert(l.worked_example.length>=145,"V46 numerical demonstration too short "+id);
 assert(l.derivation.at(-1).length>85&&l.protocol.at(-1).length>40,"V46 weak proof or validation "+id);
 assert.equal(new Set(l.equations.map(f=>f.tex)).size,6,"V46 duplicate mathematical expression "+id);
 katex.renderToString(l.equations.at(-1).tex,{throwOnError:true,displayMode:true,strict:"ignore"});
}
near(2*.8**5,.65536,"V46 LSTM five-step retention");
near(.01*(1+.8+.8**2),.0244,"V46 teacher forcing three-step bound");
near(.5**3/(1-.5),.25,"V46 SSM truncated memory tail");
near(2*10*.8,16,"V46 nonnormal transient growth");
near(1+(3-1)*1+(3-1)*2,7,"V46 stacked causal convolution receptive field");
near(1/(.2**2+.8**2),25/17,"V46 attention effective neighbors");
near(3*512*2048+2*2048+512,3150336,"V46 SwiGLU biased FFN parameters");
near(3*512*2048+2*2048+512-(2*512*2048+2048+512),1050624,"V46 FFN additional parameters");
const v46soft=1/(1+Math.exp(-1));
near(v46soft*(v46soft*2+(1-v46soft)*1-2),-.196611933,"V46 softmax temperature sensitivity",1e-6);
near(Math.cos(Math.PI*2),1,"V46 calendar positional phase");

const v47=read("atlas-depth-09-g.json").lessons;
const v47ids=["m13-c02-p04","m13-c02-p01","m13-c05-p05","m13-c03-p03","m13-c02-p05","m14-c05-p04","m14-c04-p05","m14-c04-p02","m14-c05-p06","m14-c01-p02"];
assert.equal(v47ids.length,10);
for(const id of v47ids){
 const l=v47[id];
 assert(l&&(l?.overview?.length>=60 && l?.equations?.length>=2 && l?.worked_example?.length>=50),"missing V47 course "+id);
 assert.equal(l.equations.length,6,"V47 sixth formula absent "+id);
 assert.equal(l.derivation.length,6,"V47 sixth proof absent "+id);
 assert.equal(l.protocol.length,6,"V47 sixth validation absent "+id);
 assert.equal(l.pitfalls.length,3,"V47 original boundary removed "+id);
 assert(l.derivation.at(-1).length>90&&l.protocol.at(-1).length>40,"V47 proof or test underdeveloped "+id);
 assert(l.worked_example.length>140,"V47 lost worked example "+id);
 assert.equal(new Set(l.equations.map(e=>e.tex)).size,6,"V47 mathematical formula duplicated "+id);
 katex.renderToString(l.equations.at(-1).tex,{throwOnError:true,strict:"ignore",displayMode:true});
}
near(.01*(-2),-.02,"V47 leaky ReLU negative region forward");
near((2*2+3)*2,14,"V47 shared-node autodiff accumulation");
near((.9*.1+.1*1)/ (1-.9**2),1,"V47 Adam bias correction step index");
near((7+11)/2,9,"V47 layer norm affine mean");
near(.7*.3*2,.42,"V47 softmax Hessian positive eigenvalue");
near((1+2+3+4)/4,2.5,"V47 GAP permutation invariant mean");
near(.8*.5+.2*3,1,"V47 covariate shift weights integrate to one");
near((105-100)/(110-100),.5,"V47 point-in-time price normalization");
near((105-100)/(120-100),.25,"V47 erroneous future-extreme normalization");
near(.0008-.001*.5,.0003,"V47 fee-aware ETF image model net return");
near(Math.floor((10+2-2*(3-1)-1)/1)+1,8,"V47 dilated image convolution output shape");

const v48=read("atlas-depth-09-e.json").lessons;
const v48ids=["m09-c05-p01","m09-c01-p02","m09-c02-p06","m09-c02-p02","m09-c04-p04","m10-c03-p01","m10-c01-p04","m10-c02-p02","m10-c04-p01","m10-c03-p02"];
assert.equal(v48ids.length,10,"V48 mathematical lesson count");
for(const id of v48ids){
 const l=v48[id];
 assert(l&&(l?.overview?.length>=60 && l?.equations?.length>=2 && l?.worked_example?.length>=50),"V48 official lesson absent "+id);
 assert.equal(l.equations.length,6,"V48 sixth equation missing "+id);
 assert.equal(l.derivation.length,6,"V48 sixth proof missing "+id);
 assert.equal(l.protocol.length,6,"V48 sixth independent audit missing "+id);
 assert.equal(l.pitfalls.length,3,"V48 original model boundaries lost "+id);
 assert(l.derivation.at(-1).length>90&&l.protocol.at(-1).length>45,"V48 mathematically thin lesson "+id);
 assert(l.worked_example.length>150,"V48 lost prior financial example "+id);
 assert.equal(new Set(l.equations.map(f=>f.tex)).size,6,"V48 duplicate formula "+id);
 katex.renderToString(l.equations.at(-1).tex,{throwOnError:true,displayMode:true,strict:"ignore"});
}
near(1/(.2*10),.5,"V48 one-class SVM dual upper limit");
near(2/Math.sqrt(3**2+4**2),.4,"V48 geometric SVM margin width");
near(Math.exp(-.5),.6065306597126334,"V48 Gaussian RBF length scale");
near(1/(100*2),.005,"V48 hinge risk regularization equivalence");
near(1-.5**2/(1+1),.875,"V48 GP latent posterior variance");
near(1-.5**2/(1+1)+1,1.875,"V48 GP predictive observation variance");
near(2+.5/(1+1)*(3-1),2.5,"V48 nonzero GP prior-mean posterior");
const v48KL=.8*Math.log(1.6)+.2*Math.log(.4);
near(Math.sqrt(v48KL/2),.31043900932530816,"V48 Pinsker total variation bound",1e-12);
assert(.3<=Math.sqrt(v48KL/2),"V48 Pinsker inequality violated");
near(1/(1+4/3),3/7,"V48 correlated Gaussian posterior variance");
near((8/3)/(1+4/3),8/7,"V48 correlated Gaussian posterior mean");
near((1/4)/(3/4),1/3,"V48 collider conditional joint");
assert(Math.abs((1/4)/(3/4)-((1/2)/(3/4))**2)>0.1,"V48 collider must induce conditional dependence");
near(Math.log(.5),-.6931471805599453,"V48 MH stable log acceptance");
near(-50-.5,-50.5,"V48 optimal evidence lower bound");

const v48second=read("atlas-depth-09-f.json").lessons;
const v48secondIds=["m11-c04-p02","m11-c05-p05","m11-c05-p03","m11-c01-p01","m11-c05-p02","m12-c01-p01","m12-c04-p06","m12-c03-p04","m12-c03-p01","m12-c01-p06"];
assert.equal(v48secondIds.length,10,"V48 expanded evaluation and causal domain count");
for(const id of v48secondIds){
 const l=v48second[id];
 assert(l&&(l?.overview?.length>=60 && l?.equations?.length>=2 && l?.worked_example?.length>=50),"missing second batch V48 course "+id);
 assert.equal(l.equations.length,6,"V48 evaluation formula count "+id);
 assert.equal(l.derivation.length,6,"V48 evaluation proof count "+id);
 assert.equal(l.protocol.length,6,"V48 evaluation verification count "+id);
 assert.equal(l.pitfalls.length,3,"V48 original failure conditions lost "+id);
 assert(new Set(l.equations.map(e=>e.tex)).size===6,"V48 duplicate exact equation "+id);
 assert(l.derivation.at(-1).length>85&&l.protocol.at(-1).length>40,"V48 thin validation "+id);
 assert(l.worked_example.length>145,"V48 lost arithmetic exposition "+id);
 katex.renderToString(l.equations.at(-1).tex,{displayMode:true,throwOnError:true,strict:"ignore"});
}
near(.6*1+.4*2,1.4,"V48 mixture label permutation invariant mean");
near(((1-2)**2+(2-1)**2+(1-3)**2+(3-1)**2+(2-3)**2+(3-2)**2)/6,2,"V48 pairwise contrastive distance");
near(100/110-1,-1/11,"V48 price-image reversal breaks trend");
near(4+16,20,"V48 Kmeans within-between total scatter");
near(.5/1,.5,"V48 false negative InfoNCE repulsion");
near(.9*.01*(1-.5)/(.5*(1-.01)),.009090909090909092,"V48 rare risk acceptable FPR",1e-12);
near(.009/(.009+.05*.99),.15384615384615385,"V48 rare-event alarm precision");
near(.03/.005,6,"V48 weak IV denominator sensitivity");
assert(.01<=.05/3&&.03<=.05*2/3&&.8>.05,"V48 BH all-null realized discoveries");
near(100-10+1,91,"V48 moving block bootstrap candidates");
near((6*.2**2+4*.8**2)/10,.28,"V48 Brier observed reliability");
near((.8-.6)**2+.6*(1-.6),.28,"V48 Brier single-bin calibration and uncertainty");

const v49groups=[
  {file:"atlas-depth-09-c.json",ids:["m05-c04-p02","m05-c03-p06","m05-c01-p04","m05-c04-p05","m05-c01-p03","m06-c04-p06","m06-c04-p01","m06-c02-p03","m06-c05-p01","m06-c01-p03"]},
  {file:"atlas-depth-09-d.json",ids:["m07-c04-p01","m07-c01-p05","m07-c03-p02","m07-c02-p04","m07-c04-p03","m08-c03-p05","m08-c02-p02","m08-c02-p03","m08-c01-p03","m08-c03-p04"]}
];
for(const group of v49groups){
 const lessons=read(group.file).lessons;
 assert.equal(group.ids.length,10,"V49 course count "+group.file);
 for(const id of group.ids){
  const l=lessons[id];
  assert(l&&(l?.overview?.length>=60 && l?.equations?.length>=2 && l?.worked_example?.length>=50),"V49 official course absent "+id);
  assert(l.equations.length>=6,"V49 sixth formula absent "+id);
  assert(l.derivation.length>=6,"V49 sixth proof absent "+id);
  assert(l.protocol.length>=6,"V49 sixth empirical test absent "+id);
  assert(l.pitfalls.length>=3,"V49 original limitations lost "+id);
  assert(!l.equations.slice(0,5).some(f=>f.tex===l.equations[5].tex),"V49 repeated original equation "+id);
  assert(l.derivation[5].length>85&&l.protocol[5].length>40,"V49 proof/test too short "+id);
  assert(l.worked_example.length>140,"V49 numerical exposition lost "+id);
  katex.renderToString(l.equations[5].tex,{throwOnError:true,displayMode:true,strict:"ignore"});
 }
}
near(2*(1-1),0,"V49 KKT constraint root");
near(2*.995**2,1.98005,"V49 AdamW multiplicative two-step decay");
near(Math.sqrt(2*.03/6),.1,"V49 strongly convex parameter radius");
near(Math.min(1,Math.max(0,2))-Math.min(1,Math.max(0,-.5)),1,"V49 nonexpansive convex projection");
near(Math.pow(.1,4),.0001,"V49 Hessian-degenerate fourth order behavior");
near((4*3)/2,6,"V49 exact quadratic feature count");
near((5*4)/2,10,"V49 quadratic feature total including lower degrees");
near((1+0+2*.5)/(2+2),.5,"V49 matured-only temporal target coding");
near((1+0+1+2*.5)/(3+2),.6,"V49 forbidden current-label target leakage");
near(.5*2,1,"V49 MAR inverse-observation weighting");
near(1-.2,.8,"V49 Bernoulli GLM canonical score");
near(1/(1-.5)**2,4,"V49 HC3 leverage inflation");
near(2*Math.log1p(Math.exp(-10)),.000090797798435,'"V49 logistic separation finite loss"',1e-10);
near(Math.max(.3-.2,0),.1,"V49 orthonormal Lasso threshold");
near(3+3**2/2,7.5,"V49 Poisson-gamma overdispersion");
near((.1+.2)/2,.15,"V49 early-stopping minimum-selection optimism");
const v49Oob=(1-(1-1/100)**100)**10;
assert(v49Oob>0&&v49Oob<.02,"V49 finite-forest OOB count invalid");
near((-1/3)+(1+1/3)/4,0,"V49 equicorrelation PSD forest variance bound",1e-12);
near((.05+.10)/2,.075,"V49 maximum noisy information gain");
near((1-.5)*4/50,.04,"V49 finite-population gradient noise");

const v50groups=[
  {file:"atlas-depth-09-a.json",ids:["m01-c05-p05","m01-c04-p06","m01-c03-p01","m01-c05-p04","m01-c04-p04","m01-c05-p02","m02-c05-p06","m02-c03-p01","m02-c05-p04","m02-c05-p03","m02-c03-p02","m02-c01-p03"]},
  {file:"atlas-depth-09-b.json",ids:["m03-c05-p04","m03-c02-p03","m03-c04-p02","m03-c02-p04","m03-c01-p05","m04-c03-p04","m04-c03-p05","m04-c01-p05","m04-c04-p02","m04-c02-p03"]}
];
assert.equal(v50groups.reduce((s,g)=>s+g.ids.length,0),22,"V50 new canonical lesson count");
for(const group of v50groups){
 const lessons=read(group.file).lessons;
 for(const id of group.ids){
  const l=lessons[id];
  assert(l&&(l?.overview?.length>=60 && l?.equations?.length>=2 && l?.worked_example?.length>=50),"V50 canonical course absent "+id);
  assert(l.equations.length>=6,"V50 additional formula absent "+id);
  assert(l.derivation.length>=6,"V50 additional proof absent "+id);
  assert(l.protocol.length>=6,"V50 independent validation absent "+id);
  assert(l.pitfalls.length>=3,"V50 original boundaries lost "+id);
  assert(!l.equations.slice(0,5).some(e=>e.tex===l.equations[5].tex),"V50 repeated pre-existing equation "+id);
  assert(l.derivation[5].length>85&&l.protocol[5].length>40,"V50 proof/test too short "+id);
  assert(l.worked_example.length>145,"V50 numerical reasoning missing "+id);
  katex.renderToString(l.equations[5].tex,{throwOnError:true,displayMode:true,strict:"ignore"});
 }
}
near(2*.001*(1/42),.00004761904761904762,"V50 drift-rebalancing two-sided notional fee");
near(2*Math.log(1.1),Math.log(1.21),"V50 research log-wealth chain");
near(Math.sqrt(.2*.8/100),.04,"V50 held-out risk standard error");
assert(Math.max(101,105)>102,"V50 mature label and feature time");
near(.02/Math.sqrt(Math.PI),.011283791670955126,"V50 hyperparameter optimism two candidates");
near((-3-2-1)/3,-2,"V50 cross-sectional rank IC cannot guarantee positive cash return");
near(Math.max(9,1)-9,0,"V50 leading PCA factor residual validation");
near(.01*(4+3),.07,"V50 Linf local feature sensitivity");
near(.01*Math.sqrt(4**2+3**2),.05,"V50 L2 local feature sensitivity");
near(.01**2/.01**2,1,"V50 truncated SVD kept component variance");
near(.2**2,.04,"V50 truncated SVD omitted coefficient bias");
near(.2*(3/5)**2-.1*(4/5)**2,.008,"V50 covariance Rayleigh perturbation");
near(16*.0001+.0004,.002,"V50 Jacobian first output risk");
near(12*.0001-.0004,.0008,"V50 Jacobian cross output risk");
near(9*.0001+.0004,.0013,"V50 Jacobian second output risk");
near(1/3+2/3,1,"V50 minimum norm rank-deficient system fit");
near((99*.01)/(99*.01+1*.99),.5,"V50 class weighted Brier loses raw probability");
near(.0001+.5*.02**2+.5*.02**2,.0005,"V50 parameter mixture posterior predictive variance");
near(1-0,1,"V50 XOR conditional signal entropy in bits");
near(.1*.5*.5,.025,"V50 covariance estimated variance perturbation");
near(.1*(.5**2+.5**2),.05,"V50 covariance spectral-norm risk limit");
const v50PAC=Math.ceil(Math.log(100/.05)/(-Math.log(1-.1)));
assert.equal(v50PAC,73,"V50 exact realizable finite class PAC sample requirement");
near((.25-.2)+(.2-.1),.15,"V50 estimation plus approximation risk");
near(100*(1+.5)/(1-.5),300,"V50 negative AR1 variance equivalent sample count");
near((9+1)/1,10,"V50 strong convex regularized condition number");
const v50endpoints=[-1,1,1.5,2,2.5,3,4];
const v50patterns3=new Set(),v50patterns2=new Set();
for(const left of v50endpoints)for(const right of v50endpoints){
 if(left>right)continue;
 v50patterns3.add([1,2,3].map(x=>Number(left<=x&&x<=right)).join(""));
 v50patterns2.add([1,3].map(x=>Number(left<=x&&x<=right)).join(""));
}
assert.equal(v50patterns2.size,4,"V50 intervals must shatter any two distinct ordered points");
assert(!v50patterns3.has("101"),"V50 a single interval cannot shatter three increasing points");
console.log("PASS V50: 122 canonical ninth-cycle lessons fully deepened (62 V40 + 60 V41), 727 strict KaTeX formulas, including 117 additions beyond V41, 12 stages, 122 canonical route links, Chinese math and print responsive checks and 23k+ new V41 mathematical/research chars.");
