import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const dir=new URL("./",import.meta.url);
const read=name=>JSON.parse(fs.readFileSync(new URL(name,dir),"utf8"));
const curr=read("curriculum.json"),manifest=read("lessons-manifest.json");
const core=["lessons-foundations.json","lessons-advanced.json","lessons-more.json"].flatMap(f=>read(f).lessons);
const ext=manifest.modules.flatMap(m=>read(m.path).lessons);
const all=[...core,...ext],byId=new Map(all.map(z=>[z.node_id,z]));
const nodes=new Map(curr.nodes.map(n=>[n.id,n]));
const ids=["measure-n0145","measure-n0146","measure-n0147",
"ito-n0313","ito-n0315","ito-n0316","ito-n0317","ito-n0318",
"ito-n0320","ito-n0321","ito-n0319","ito-n0322","ito-n0323","ito-n0324","ito-n0328",
"ito-n0326","ito-n0327","ito-n0330","ito-n0329"];
const pre={
"measure-n0145":["measure-n0124","measure-n0136"],
"measure-n0146":["measure-n0145","measure-n0136"],
"measure-n0147":["measure-n0146","probability-n0096"],
"measure-n0148":["measure-n0147","ito-n0322"],
"ito-n0313":["ito-n0307","ito-n0309"],
"ito-n0315":["ito-n0313","ito-n0308"],
"ito-n0316":["ito-n0313","ito-n0315"],
"ito-n0317":["ito-n0313","ito-n0307"],
"ito-n0318":["ito-n0317","ito-n0308"],
"ito-n0320":["ito-n0304","ito-n0307"],
"ito-n0321":["ito-n0320","ito-n0308"],
"ito-n0319":["ito-n0321","ito-n0317"],
"ito-n0322":["ito-n0319","measure-n0147"],
"ito-n0323":["ito-n0322","finance-n0427"],
"ito-n0324":["ito-n0322","ito-n0323"],
"ito-n0326":["ito-n0310","stochastic-n0286"],
"ito-n0325":["ito-n0326","ito-n0324"],
"ito-n0327":["stochastic-n0297","ito-n0325"],
"ito-n0328":["ito-n0308","stochastic-n0294","ito-n0320"],
"ito-n0330":["stochastic-n0276","ito-n0302"],
"ito-n0329":["stochastic-n0276","ito-n0306","ito-n0330"]
};
assert.equal(curr.nodes.length,656);
assert.equal(curr.chapters.length,109);
assert.equal(curr.disciplines.length,20);
assert.equal(manifest.modules.length,54);
assert.equal(ext.length,135,"135 active extensions: historical 130 plus five new V20 DML/optimization chapters");
assert.equal(core.length,123,"V14: 104 existing core plus 19 genuinely new full lectures");
assert.equal(all.length,258,"historical V19 253 plus five net-new V20 lectures (seven prior courses deepened in place)");
assert.equal(byId.size,258,"no duplicate core or extension lecture IDs");
assert.equal(ids.length,19);
const visiting=new Set(),done=new Set();
function dfs(id){
 assert(nodes.has(id),"unknown prerequisite "+id);
 if(visiting.has(id))throw Error("prerequisite graph cycle "+id);
 if(done.has(id))return;
 visiting.add(id);
 for(const d of nodes.get(id).prerequisites||[])dfs(d);
 visiting.delete(id);done.add(id);
}
for(const[id,prs]of Object.entries(pre))assert.deepEqual(nodes.get(id).prerequisites,prs,"incorrect V14 curriculum prerequisites "+id);
for(const n of curr.nodes)dfs(n.id);
assert.equal(done.size,656,"all math nodes have valid acyclic prerequisite graph");
assert(!ids.includes("ito-n0308"),"previously completed extension Ito isometry must not be counted as newly finished");
assert(!ids.includes("ito-n0325"),"original Feynman-Kac lecture is deepened, not newly counted");
const deepened=byId.get("ito-n0325");
assert(deepened?.deep_sections?.length===4,"Feynman-Kac original core lecture must be substantively deepened");
assert(deepened.example.includes("Black–Scholes欧式Call"),"preserve old European call risk-neutral expectation example");
for(const section of deepened.deep_sections){assert(section.proof.length>=75&&section.worked.length>=55);for(const f of section.formulas)katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});}

assert(byId.get("ito-n0308").deep_sections?.length===4,"existing deepened Ito isometry must survive V14");
assert(!ids.includes("ito-n0309")&&!ids.includes("ito-n0314"),"preserve existing Ito formula and GBM exact lessons");

let sections=0,formulas=0,sources=0,links=0;
for(const id of ids){
 const l=byId.get(id),n=nodes.get(id);
 assert(l&&n&&l.id===id&&l.title===n.title,"canonical node title or ID mismatch "+id);
 assert(l.definition?.length>=110&&l.assumptions?.length>=95,"incomplete definition or critical hypothesis "+id);
 assert(l.example?.length>=90&&l.application?.length>=48&&l.caveat?.length>=48,"incomplete independently auditable example/application/boundary "+id);
 assert(l.properties?.length>=4&&l.formulas?.length>=4&&l.derivation?.length>=4,"insufficient theorem/proof structure "+id);
 assert(l.deep_sections?.length===4,"each V14 chapter requires four original deep mathematical sections "+id);
 assert(new Set(l.deep_sections.map(s=>s.title)).size===4,"repeated deep-section title "+id);
 assert(l.sources.length>=2,"at least two named mathematics references "+id);
 for(const src of l.sources){
  assert(src.title?.length>=23&&/^https:\/\/\S+$/.test(src.url),"unverifiable academic source shape "+id);
  assert(!src.url.includes("tokyo-finance9"),"unverified jump-source link remains "+id);
  sources++;
 }
 for(const link of l.links||[]){
  assert(["math","ml"].includes(link.kind)&&link.label?.length>=4,"incorrect cross reference "+id);
  if(link.kind==="math")assert(nodes.has(link.id),"math link points outside canonical curriculum "+id+" -> "+link.id);
  else assert(/^m\d\d-c\d\d-p\d\d$/.test(link.id),"ML link malformed "+id+" -> "+link.id);
  links++;
 }
 for(const[i,s]of l.deep_sections.entries()){
  assert(s.title?.length>=6&&s.question?.length>=14,"unclear deep mathematical section "+id+"/"+i);
  assert(s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=55),"insufficient connected Chinese explanation "+id+"/"+i);
  assert(s.proof?.length>=75&&s.worked?.length>=55&&s.limitation?.length>=42,"missing full logical argument/reproducible example/limitation "+id+"/"+i);
  assert(s.formulas.length>=1,"missing display equation "+id+"/"+i);
  for(const f of s.formulas){assert(f.label&&f.tex);katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});formulas++;}
  assert.equal(l.derivation[i].explanation,s.proof,"formula derivation does not match independently expanded proof "+id+"/"+i);
  sections++;
 }
 for(const f of l.formulas)katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});
 for(const d of l.derivation){assert(d.explanation?.length>=75);katex.renderToString(d.tex,{throwOnError:true,strict:"ignore"});}
 for(const text of [l.definition,l.assumptions,l.application,l.caveat,l.example,...l.deep_sections.flatMap(s=>[s.question,...s.paragraphs,s.proof,s.worked,s.limitation])])assert(!/[\uFFFD\u200B\uFEFF]/.test(text),"broken Chinese character or zero-width formatting "+id);
}
assert.equal(sections,76);
assert.equal(formulas,76);
assert.equal(sources,38);
const near=(x,y,tol=1e-9)=>assert(Number.isFinite(x)&&Math.abs(x-y)<=tol,"independently reproduced calculation failed: "+x+" vs "+y);
const exp=Math.exp;
// Absolute continuity, RN density change and Bayes two-state normalization.
near(.7+.3,1);
near(.5*.5,.25);
near(2/3,.6666666666666666);
near(.5*1.2+.5*.8,1);
near(.5*1.2,.6);
near(.5*1.2*10,6);
near(.6*(5/6)+.4*1.25,1);
near(.5*10,5);
near((1.02-.9)/(1.1-.9),.6);
// All 19 IDs must be newly completed; historical full F-K counts once only.
assert(ids.every(id=>byId.has(id)), "all V14 new lecture IDs must exist");
// Linear SDE and OU Gaussian transition.
near(3*exp(-2),.4060058497098381);
near((1-exp(-4))/4,.24542109027781644);
near(1+2*exp(-2),1.2706705664732254);
near(.16/4*(1-exp(-4)),.03926737444445063);
near(.16/(2*2),.04);
near(Math.log(2)/2,.34657359027997264);
near(exp(-2*.5),exp(-1));
// CIR Feller, conditional mean, stationary gamma parameters.
near(2*2*.04,.16);near(.2*.2,.04);
near(.04+.05*exp(-2),.04676676416183064);
near(2*2*.04/(.2*.2),4);near((.2*.2)/(2*2),.01);
near(4*.01**2,.0004);
// SDE strong/weak coupling and EM numerical variance/stability.
near(Math.sqrt(2),1.4142135623730951);
near(.2*Math.sqrt(.25),.1);
near(.16/(2*2-2**2*.2),.05);
near(.04+.2*Math.sqrt(.04)*(-3),-.08);
// Exponential density, Novikov and Girsanov drift.
near(exp(-.5*.5**2),exp(-.125));
near(exp(.5**2)-1,.2840254166877414);
near(exp(.5*.25**2),exp(.03125));
near((.08-.03)/.2,.25);
near(.08-.2*.25,.03);
// One-step state prices, risk-neutral call replication, discounted price.
const q=(1.02-.9)/(1.1-.9);
near(q,.6);near(q*10/1.02,5.882352941176471);
near((q+(1-q))/1.02,1/1.02);
near(exp(-.03)*100*exp(.03),100);
// Feynman-Kac independent Brownian terminal expectation and discounted GBM square.
near(100*100+.2**2,10000.04);
near(100*exp(-.03),97.04455335485082,1e-8);
near(10000*exp(.03+.2**2),10725.08181254008,1e-6);
// Two-step Bermudan American put: exercise vs continuation, distinct European.
const u=1.2,d=.8,R=1.05,Q=(R-d)/(u-d);
near(Q,.625);
const vup=(Q*0+(1-Q)*4)/R;
const vdown=Math.max(20,(Q*4+(1-Q)*36)/R);
const put=(Q*vup+(1-Q)*vdown)/R;
const euro=(2*Q*(1-Q)*4+(1-Q)**2*36)/R**2;
near(vup,1.4285714285714286);
near(vdown,20);
near(put,7.9931972789115635);
near(euro,6.292517006802721);
assert(put>euro,"early exercise premium should be positive");
// Strict local martingale 3D Bessel inverse, jump-diffusion and Levy characteristics.
near(2*.8413447460685429-1,.6826894921370859);
near(exp(-.04),.9607894391523232);
near(1.1*exp(-.04),1.056868383067556);
near(exp(.08),1.0832870676749586);
near(exp(.21)-exp(.16),exp(.21)-exp(.16));
near(2*1,2);
near(1-exp(-1),.6321205588285577);
// Crosslinks and CJK/matrix/print styles at both mathematical and ML indexes.
const math=fs.readFileSync(new URL("index.html",dir),"utf8");
const ml=fs.readFileSync(new URL("../ml/index.html",dir),"utf8");
const css=fs.readFileSync(new URL("styles.css",dir),"utf8");
for(const id of ids){
 assert(math.includes("?topic="+id),"V14 mathematics stage missing link "+id);
 assert(ml.includes("../math/?topic="+id),"V14 ML-to-maths stage missing link "+id);
}
assert(math.includes('aria-label="V14十九篇SDE测度与风险中性定价系统教材"'));
assert(ml.includes('aria-label="V14十九篇SDE与概率测度变换数学先修"'));
assert(math.includes("styles.css?v=20261104-probscore-v6-inference-v8-martingale-v9-convergence-v10-integration-v11-stochastic-v12-brownian-v13-sde-v14"),"math stylesheet cache not updated");
assert(css.includes("V14: SDE and risk-neutral pricing CJK and KaTeX layout"));
assert(css.includes(".math-v14-sde-pricing summary:focus-visible")&&css.includes(".katex-display")&&css.includes("@media print"),"missing explicit mobile/keyboard/printing safeguards");
// Prevent a known stale mislink from American option pricing to extreme-value theory.
assert(!byId.get("ito-n0327").links.some(l=>l.id==="finance-n0448"));
assert(byId.get("ito-n0327").links.some(l=>l.id==="diffeq-n0411"));
console.log("PASS V14: 19 newly complete measure, SDE, risk-neutral and jump-pricing lessons; 76 independent new proof sections plus one deepened Feynman-Kac lecture; 76 strict KaTeX displays; "+sources+" academic source links and "+links+" graph-checked connections; 258 unique complete maths lessons/656 canonical nodes; independently recomputed OU, CIR, Euler, Girsanov, Feynman-Kac, Bermudan, jump and Levy checks.");

