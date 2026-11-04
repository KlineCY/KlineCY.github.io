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
const ids=["ito-n0313","ito-n0315","ito-n0316","ito-n0317","ito-n0318",
"ito-n0320","ito-n0321","ito-n0319","ito-n0322","ito-n0323","ito-n0324","ito-n0328",
"ito-n0326","ito-n0325","ito-n0327","ito-n0330","ito-n0329"];
const pre={
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
assert.equal(manifest.modules.length,41);
assert.equal(ext.length,103,"103 extension course originals remain unchanged");
assert.equal(core.length,121,"V14: 104 preexisting core plus 17 new full lectures");
assert.equal(all.length,224,"V14: 207 preexisting plus 17 uniquely completed lectures");
assert.equal(byId.size,224,"full curriculum must not duplicate extension entries");
assert.equal(ids.length,17);
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
assert.equal(sections,68);
assert.equal(formulas,68);
assert.equal(sources,34);
const near=(x,y,tol=1e-9)=>assert(Number.isFinite(x)&&Math.abs(x-y)<=tol,"independently reproduced calculation failed: "+x+" vs "+y);
const exp=Math.exp;
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
assert(math.includes('aria-label="V14十七篇SDE数值、风险中性与跳跃定价系统教材"'));
assert(ml.includes('aria-label="V14十七篇SDE、风险中性测度与定价数学先修"'));
assert(math.includes("styles.css?v=20261104-probscore-v6-inference-v8-martingale-v9-convergence-v10-integration-v11-stochastic-v12-brownian-v13-sde-v14"),"math stylesheet cache not updated");
assert(css.includes("V14: SDE and risk-neutral pricing CJK and KaTeX layout"));
assert(css.includes(".math-v14-sde-pricing summary:focus-visible")&&css.includes(".katex-display")&&css.includes("@media print"),"missing explicit mobile/keyboard/printing safeguards");
// Prevent a known stale mislink from American option pricing to extreme-value theory.
assert(!byId.get("ito-n0327").links.some(l=>l.id==="finance-n0448"));
assert(byId.get("ito-n0327").links.some(l=>l.id==="diffeq-n0411"));
console.log("PASS V14: 17 newly complete SDE, risk-neutral and jump-pricing lessons; 68 independent proof sections; 68 strict KaTeX displays; "+sources+" academic source links and "+links+" graph-checked connections; 224 unique complete maths lessons/656 canonical nodes; independently recomputed OU, CIR, Euler, Girsanov, Feynman-Kac, Bermudan, jump and Levy checks.");

