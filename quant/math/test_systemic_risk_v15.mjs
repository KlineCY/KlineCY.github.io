import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const c=read("curriculum.json"),m=read("lessons-manifest.json");
const core=["lessons-foundations.json","lessons-advanced.json","lessons-more.json"].flatMap(p=>read(p).lessons);
const extra=m.modules.flatMap(x=>read(x.path).lessons);
const all=[...core,...extra],ids=new Set(all.map(x=>x.node_id));
assert.equal(c.nodes.length,656);
assert.equal(core.length,123);
assert.equal(m.modules.length,42);
assert.equal(m.expected_lessons,105);
assert(m.modules.some(x=>x.path==="./lessons-phase-15-a.json"&&x.lessons===2));
assert.equal(extra.length,105);
assert.equal(all.length,228);
assert.equal(ids.size,228,"no duplicate complete mathematics courses");
const legacy=all.filter(x=>!["riskmath-x005","riskmath-x006"].includes(x.node_id));
assert.equal(legacy.length,226,"previous complete math courses must remain intact");

const chapters=read("lessons-phase-15-a.json").lessons;
assert.deepEqual(chapters.map(x=>x.node_id),["riskmath-x005","riskmath-x006"]);
let equations=0,sections=0;
for(const l of chapters){
 assert(c.nodes.some(n=>n.id===l.id&&n.title===l.title),"not connected to its original curriculum node "+l.id);
 assert.equal(l.deep_sections.length,7);
 assert(l.derivation.length>=6&&l.formulas.length>=7);
 assert(l.sources.length>=2);
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,7);
 const itemForms=[...l.formulas,...l.derivation,...l.deep_sections.flatMap(s=>s.formulas)];
 for(const e of itemForms){katex.renderToString(e.tex,{throwOnError:true,strict:"ignore",displayMode:true});equations++}
 for(const s of l.deep_sections){
  assert(s.paragraphs.length>=2&&s.proof.length>=50&&s.worked.length>=40&&s.limitation.length>=30);
  sections++;
 }
 for(const link of l.links||[])if(link.kind==="math")assert(c.nodes.some(n=>n.id===link.id),"missing prerequisite "+link.id);
}
const mes=chapters[0],covar=chapters[1];
// Discrete ES must include the correct fractional mass at its quantile atom.
const losses=[0,1,1,3].sort((a,b)=>a-b),alpha=.5;
const quantile=losses[Math.ceil(alpha*losses.length)-1];
const es=quantile+losses.reduce((s,x)=>s+Math.max(x-quantile,0),0)/losses.length/(1-alpha);
assert.equal(quantile,1);
assert.equal(es,2);
assert(mes.deep_sections[4].worked.includes("=2"));
// Smooth Euler system ES should equal the weight-weighted marginal tail expectations.
assert.equal(.6*.03+.4*.08,.05);
assert(mes.deep_sections[5].worked.includes("0.018")&&mes.deep_sections[5].worked.includes("0.032"));
// Gaussian state-CoVaR: median shift 1 at rho=.5, system sd=2, institution sd=1.
const delta=.5*2/1*(1-0);
assert.equal(delta,1);
assert(covar.deep_sections[4].worked.includes("ΔCoVaR=1"));
// Conditional event is not conditioning on an exact institution state.
const eventSample=[2,5].sort((a,b)=>a-b);
assert.equal(eventSample[Math.ceil(.75*eventSample.length)-1],5);
assert(covar.deep_sections[5].worked.includes("CoVaR为5"));

const html=fs.readFileSync(new URL("index.html",root),"utf8");
const ml=fs.readFileSync(new URL("../ml/index.html",root),"utf8");
const css=fs.readFileSync(new URL("styles.css",root),"utf8");
assert(html.includes('aria-label="系统尾部风险从VaR与ES到MES、CoVaR的数学证明链"'));
for(const id of chapters.map(x=>x.id))assert(html.includes("?topic="+id),"missing topic in system-risk learning path "+id);
assert(ml.includes("../math/?topic=riskmath-x005"),"ML math cross-link missing");
assert(css.includes("overflow-x:auto")&&css.includes("line-break:strict")&&css.includes("@media print"),"CJK/mobile/formula print constraints missing");
console.log("PASS V15: 42 modules, 105 extensions, 228 unique full math lectures, "+sections+" systemic-risk proof sections and "+equations+" valid KaTeX formula blocks; independent discrete ES, Euler and CoVaR computations.");
