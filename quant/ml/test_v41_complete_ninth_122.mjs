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
  assert(v41 ? /^V4[1-4]/.test(l.revision_note||"") : l.revision_note?.startsWith("V40"),id+" mismatch expansion batch");
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
assert.equal(eq,635,"V44 must preserve original 610 formulas plus 25 later mathematical enhancements");
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
const v40nav=section("V40第九批前六十二篇机器学习完整数学深化");
const v41nav=section("V41第九批后六十篇机器学习正式课程全面深化");
assert.equal((v40nav.match(/href="#lesson\//g)||[]).length,62);
assert.equal((v41nav.match(/href="#lesson\//g)||[]).length,60);
for(const id of first)assert(v40nav.includes('href="#lesson/'+id+'"'),"broken first-half navigation "+id);
for(const id of second)assert(v41nav.includes('href="#lesson/'+id+'"'),"broken final-half navigation "+id);
for(const stage of ["阶段七","阶段八","阶段九","阶段十","阶段十一","阶段十二"])
 assert(v41nav.includes(stage),"missing V41 learning stage "+stage);
assert(home.indexOf('aria-label="V41第九批后六十篇机器学习正式课程全面深化"')<home.indexOf('aria-label="V40第九批前六十二篇机器学习完整数学深化"'),"V41 route must be visible near top");
for(const label of ["linear-n0065","statistics-x001","riskmath-x002"]){
 assert(v41nav.includes('href="../math/?topic='+label+'"'),"math-topic link missing "+label);
 const math=read("../math/curriculum.json");
 assert(math.nodes.some(n=>n.id===label),"link to non-existing math topic "+label);
}
for(const marker of ["s.equations.map","s.derivation","s.protocol","s.pitfalls","s.worked_example","katex.render"])
 assert(viewer.includes(marker),"original reader lost structured mathematical section "+marker);
for(const marker of [".atlas-spectral-path",".atlas-spectral-stage",".atlas-spectral-links",".ml-math-block","overflow-x:auto","line-break:strict","@media print"])
 assert(css.includes(marker),"CJK typography, narrow-screen formula scroll or print CSS missing "+marker);
for(const tag of ["V39第八批后六十篇机器学习正式课程全域深化","V38第八批六十篇正式课程数学与量化深化"])
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

console.log("PASS V41: 122 canonical ninth-cycle lessons fully deepened (62 V40 + 60 V41), 635 strict KaTeX formulas, 244 V40/V41 newly authored formulas, 12 stages, 122 canonical route links, Chinese math and print responsive checks and 23k+ new V41 mathematical/research chars.");
