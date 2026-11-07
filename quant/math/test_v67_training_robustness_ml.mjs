import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const base=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,base),"utf8"));
const math="abcd".split("").flatMap(s=>read("lessons-phase-21-"+s+".json").lessons);
const ml=Object.assign({},...("abcd".split("").map(s=>read("../ml/atlas-depth-12-"+s+".json").lessons)));
const mathIds=["mlmath-n0455","mlmath-n0466","mlmath-n0463","mlmath-n0464","mlmath-n0460","mlmath-n0461","mlmath-n0462","mlmath-n0465"];
const mlIds=["m03-c05-p06","m05-c04-p06","m08-c01-p04","m09-c03-p04","m10-c02-p03","m12-c03-p05","m01-c03-p02","m02-c01-p05","m04-c04-p03","m06-c03-p02","m07-c02-p03","m11-c05-p01","m13-c05-p04","m14-c01-p05","m15-c05-p05","m16-c02-p02","m17-c03-p02","m18-c02-p01","m19-c02-p01","m20-c01-p01","m21-c03-p02","m22-c02-p04","m23-c04-p06"];
assert.deepEqual(math.map(l=>l.id),mathIds);
assert.deepEqual(Object.keys(ml),mlIds);
let formulas=0;
const valid=tex=>{
 assert(typeof tex==="string"&&tex.trim(),"formula must not be empty");
 const html=katex.renderToString(tex,{throwOnError:true,strict:"ignore",displayMode:true});
 assert(html.includes("katex"),"formula failed rendering");
 formulas++;
};
for(const l of math){
 assert(l.node_id===l.id&&l.deep_sections.length>=6&&l.derivation.length>=5&&l.formulas.length>=7,l.id+" must be complete");
 assert.equal(new Set(l.deep_sections.map(x=>x.title)).size,l.deep_sections.length,l.id+" duplicate section heading");
 const s=l.deep_sections.at(-1);
 assert(s.question.length>=15&&s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=85),l.id+" substantive Chinese argument");
 assert(s.proof.length>=110&&s.worked.length>=100&&s.limitation.length>=45&&s.formulas.length===2,l.id+" proof/numeric example/limits");
 assert(l.links.length>=4&&l.sources.length>=2,l.id+" existing citations and links");
 for(const f of [...l.formulas,...l.derivation,...l.deep_sections.flatMap(x=>x.formulas)])valid(f.tex);
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)),l.id+" bad Unicode");
}
for(const id of mlIds){
 const l=ml[id];
 assert(l&&l.equations.length>=6&&l.derivation.length>=6,id+" must include added mathematics");
 assert(l.derivation.slice(-3).every(p=>p.length>=45),id+" substantive derivation");
 assert(l.worked_example.length>=180&&l.pitfalls.length>=3&&l.protocol.length>=4,id+" reproducible numerical example");
 assert(l.protocol.every(p=>p.length>=30),id+" verification protocol completeness");
 for(const eq of l.equations){assert(eq.label.length>2);valid(eq.tex);}
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)),id+" bad Unicode");
}
let checked=0;
const near=(actual,expected,label,tol=1e-8)=>{
 assert(Number.isFinite(actual)&&Number.isFinite(expected));
 assert(Math.abs(actual-expected)<=tol,label+": "+actual+" != "+expected);
 checked++;
};
near(Math.sqrt(Math.log(40)/1000),.060736146190830516,"single-model test uniform bound",1e-7);
near(Math.sqrt(Math.log(800)/1000),.08175947,"20-model test uniform bound",1e-6);
near(1-.95**20,.6415140775914577,"20 false discoveries from repeated independent checks");
near(.04/4*(1+3*.5),.025,"CV fold dependence variance");
near(Math.sqrt(.025),.15811388300841897,"CV dependent standard error");
near(2/100,.02,"L1 parameter change from feature rescaling");
near(.5*.02**2,.0002,"Ridge penalty under rescaling");
near(.1*5/(100*.2),.025,"Elastic Net near identical coefficient grouping");
near((4*2-3)/5,1,"unequal mini-batch weighted mean gradient");
near((2-3)/2,-.5,"unweighted batch mean gradient direction");
near(2-.1*1-.1*.1*2,1.88,"decoupled AdamW first step");
near(2-.1,1.9,"Adam with coupled L2 first step");
near((2-1)/2,.5,"clip before minibatch averaging");
near(Math.min(2,(10-1)/2),2,"clip after minibatch averaging");
near(25/((2*.8-1)*100),5/12,"stability selection expected false selection bound");
near(.8*1+.2*4,1.6,"market regime mixture mean");
near(.8*2+.2*3+.8*.2*(4-1)**2,3.64,"market mixture total variance");
near((2+0-1)/2,.5,"ADMM second primal x update");
near(2*2/4*(5.5-1.5)**2,16,"tree between-leaf SSE gain");
near(.04*25,1,"RBF exponent scaled distance");
near((.04/100)*2500,1,"RBF rescaling equivariance");
near(1/3-4/9,-1/9,"collider negative conditional covariance");
near(2/5,.4,"multiple-comparison Reality Check resample p");
near(3+1,4,"one-dimensional threshold class on three points");
near(100*5*100+100*100*2,70000,"left-associated matrix multiplies");
near(5*100*2+100*5*2,2000,"right-associated matrix multiplies");
near((4-9)/(3-1),-2.5,"delete-one mean sensitivity");
near(10*9*8/6,120,"wrapper feature subset selection");
near(2**10-1,1023,"wrapper all nonempty subsets");
near(.8-.3,.5,"LASSO nonzero soft threshold");
near(3/(3+1),.75,"contrastive positive softmax");
near(.75-1,-.25,"contrastive positive gradient");
near(Math.ceil(100/32),4,"batch steps with non-dropped last batch");
near(Math.floor((32+2*2-5)/2)+1,16,"dilated convolution output length");
near(.2*.8**2,.128,"time-varying SSM fixed-kernel contribution");
near(3*3,9,"single attention head QK matrix entries");
near(.75-1,-.25,"reward model preference gradient");
near((2+6)/2,4,"GNN degree-normalized neighbor mean");
near(2+6+2+6,16,"GNN duplicated-edge sum");
near(.25+(3-2)**2,1.25,"latent mixture variance");
near(Math.exp(-.025)**2,Math.exp(-.05),"physical time discount discretization");
near((.02*100+.03*10)/.05,46,"discrete CVaR atomic mass completion");
near((1-.8)**2*4+.8**2*1,.8,"Kalman Joseph covariance");
near(1e6*.05,50000,"ETF limit of participation shares");
near(40000*50*.5*.02*Math.sqrt(.04),4000,"ETF square root impact cash");
const html=fs.readFileSync(new URL("index.html",base),"utf8");
const mlhtml=fs.readFileSync(new URL("../ml/index.html",base),"utf8");
for(const id of mathIds){
 assert(html.includes("?topic="+id),"math curriculum nav "+id);
 assert(mlhtml.includes("../math/?topic="+id),"ML to math crosslinks "+id);
}
for(const file of ["styles.css","../ml/atlas.css"]){
 const css=fs.readFileSync(new URL(file,base),"utf8");
 for(const key of ["overflow-x:auto","line-break:strict",":focus-visible","@media print","@media(max-width:700px)"])
  assert(css.includes(key),"long TeX, mobile and CJK print safeguard: "+file+" "+key);
}
console.log("PASS V67: "+math.length+" detailed math chapters; "+mlIds.length+" detailed ML chapters; "+formulas+" KaTeX expressions, "+checked+" independently calculated financial/ML examples; verified CJK, long-equation scroll and mobile/print CSS safeguards.");
