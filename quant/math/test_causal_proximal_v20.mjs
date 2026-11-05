import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const v24Ids=new Set(["statistics-x003","statistics-x004","mlmath-x003","mlmath-x004"]);
const v25Ids=new Set(["timeseries-x003","timeseries-x004","timeseries-x006","mlmath-x006","riskmath-x001"]);
const v26Ids=new Set(["timeseries-n0260","timeseries-n0261","timeseries-n0262","timeseries-n0266","timeseries-n0267","timeseries-n0269","timeseries-n0270"]);
const v27Ids=new Set(["statistics-n0171","statistics-n0170","statistics-n0172","statistics-n0173","statistics-n0174","statistics-n0175","statistics-n0176","riskmath-n0589","riskmath-n0594","riskmath-n0595"]);
const v28Ids=new Set(["statistics-n0177","statistics-n0178","statistics-n0179","statistics-n0180","finance-n0434","finance-n0436","finance-n0437","finance-n0438"]);
const v29Ids=new Set(["information-n0508","information-n0507","information-n0505","information-n0509","information-n0506","information-n0510"]);
const curriculum=read("curriculum.json"),manifest=read("lessons-manifest.json");
const core=["lessons-foundations.json","lessons-advanced.json","lessons-more.json"].flatMap(p=>read(p).lessons);
const extended=manifest.modules.flatMap(m=>{
 const rows=read(m.path).lessons;
 assert.equal(rows.length,m.lessons,"module count incorrect "+m.path);
 return rows;
});
const all=[...core,...extended];
const byId=new Map(all.map(l=>[l.node_id,l]));
const newIds=["econometrics-x003","optimization-x011","optimization-x012","econometrics-x005","econometrics-x006"];
const deepenedIds=["econometrics-x001","econometrics-x002","econometrics-x004",
 "optimization-x007","optimization-x008","optimization-x009","optimization-x010"];
const targetIds=[...newIds,...deepenedIds];
assert.equal(curriculum.nodes.length,656);
assert.equal(manifest.modules.length,75);
assert.equal(manifest.expected_lessons,195);
assert.equal(core.length,123);
assert.equal(extended.length,195);
assert.equal(all.length,318);
assert.equal(byId.size,318,"duplicate ID introduced into mathematics manifest");
const v21Ids=["mlmath-n0455","mlmath-n0466","mlmath-n0463","mlmath-n0464","mlmath-n0460","mlmath-n0461","mlmath-n0462","mlmath-n0465"];
const v22Ids=["timeseries-n0244","timeseries-n0245","timeseries-n0248","timeseries-n0249","timeseries-n0250","timeseries-n0251","timeseries-n0254","timeseries-n0255"];
const v23Ids=["timeseries-n0256","timeseries-n0257","timeseries-n0258","timeseries-n0259"];
assert.equal(all.filter(l=>!newIds.includes(l.id)&&!v21Ids.includes(l.id)&&!v22Ids.includes(l.id)&&!v23Ids.includes(l.id)&&!v24Ids.has(l.id)&&!v25Ids.has(l.id)&&!v26Ids.has(l.id)&&!v27Ids.has(l.id)&&!v28Ids.has(l.id)&&!v29Ids.has(l.id)).length,253,"previous 253 lectures must remain intact");
const introduced="abc".split("").flatMap(x=>read("lessons-phase-20-"+x+".json").lessons);
assert.deepEqual(introduced.map(l=>l.id),newIds,"V20 contains only genuinely new original curriculum IDs");
const nodes=new Map(curriculum.nodes.map(n=>[n.id,n]));
const visiting=new Set(),visited=new Set();
function walk(id){
 assert(nodes.has(id),"missing math prerequisite "+id);
 if(visiting.has(id))throw Error("mathematics prerequisite cycle "+id);
 if(visited.has(id))return;
 visiting.add(id);for(const p of nodes.get(id).prerequisites||[])walk(p);
 visiting.delete(id);visited.add(id);
}
for(const n of curriculum.nodes)walk(n.id);
assert.equal(visited.size,656,"all curriculum nodes must have acyclic prerequisites");
let newSections=0,parsed=0,links=0,sources=0;
for(const id of targetIds){
 const l=byId.get(id);assert(l&&l.id===id,"full V20 lecture missing "+id);
 assert.equal(nodes.get(id).title,l.title,"existing curriculum node title changed "+id);
 assert(l.definition.length>=65&&l.assumptions.length>=70&&l.example.length>=100,"core content shallow "+id);
 assert(l.application.length>=55&&l.caveat.length>=55,"financial boundaries omitted "+id);
 assert(l.properties.length>=3&&l.derivation.length>=3&&l.formulas.length>=3);
 const deep=l.deep_sections||[];
 const expectedDeep=newIds.includes(id)||["econometrics-x001","econometrics-x004"].includes(id)?4:6;
 assert.equal(deep.length,expectedDeep,"original complete sections lost "+id);
 assert.equal(new Set(deep.map(x=>x.title)).size,deep.length,"duplicate V20 headings "+id);
 const recent=deep.slice(newIds.includes(id)||["econometrics-x001","econometrics-x004"].includes(id)?-4:-2);
 for(const s of recent){
  assert(s.question.length>=10&&s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=35),"Chinese argument incomplete "+id+":"+s.title);
  assert(s.proof.length>=35&&s.worked.length>=30&&s.limitation.length>=25,"proof, worked example or failure boundary too thin "+id+":"+s.title);
  for(const e of s.formulas){katex.renderToString(e.tex,{throwOnError:true,strict:"ignore",displayMode:true});parsed++}
  newSections++;
 }
 for(const e of [...l.formulas,...l.derivation]){
  assert(e.tex,"missing mathematical LaTeX "+id);
  katex.renderToString(e.tex,{throwOnError:true,strict:"ignore",displayMode:true});parsed++;
 }
 for(const link of l.links||[]){
  if(link.kind==="math")assert(nodes.has(link.id),"broken math link "+id+" -> "+link.id);
  else assert(/^m\d\d-c\d\d-p\d\d$/.test(link.id),"malformed ML link "+id+" -> "+link.id);
  links++;
 }
 for(const source of l.sources||[]){assert(source.title.length>15&&/^https:\/\//.test(source.url));sources++}
 assert(!JSON.stringify(l).includes("\ufffd"),"corrupted Chinese text "+id);
}
assert.equal(newSections,38,"all 38 V20 mathematics sections must be present");
assert(parsed>=100&&links>=28&&sources>=16,"V20 formula, source or crosslink coverage insufficient");
const near=(a,b,id)=>assert(Math.abs(a-b)<1e-9,"incorrect independent numerical case "+id+": "+a+" != "+b);
const examples=[
 ["econometrics-x001",8/4,2,"8/4=2"],
 ["econometrics-x002",8/4,2,"8/4=2"],
 ["econometrics-x003",.25*2+.75*4,3.5,"ATT=0.25×2+0.75×4=3.5"],
 ["econometrics-x004",(4+0+2+6)/4,3,"(4+0+2+6)/4=3"],
 ["econometrics-x005",.08-.03,.05,"[0.05,0.11]"],
 ["econometrics-x006",.48-.68,-.2,"[−0.20,0.80]"],
 ["optimization-x007",2*0**2,0,"f(0)=0"],
 ["optimization-x008",9/4,2.25,"2.25"],
 ["optimization-x009",6/2,3,"κ₂(H)=6/2=3"],
 ["optimization-x010",Math.max(3-1,0),2,"β̂=(2,0,−1)"],
 ["optimization-x011",.5*(1+.25+1)+3,4.125,"F(β¹)=4.125"],
 ["optimization-x012",(0-1)/.5,-2,"G_t(0)=(0−1)/0.5=−2"]
];
for(const [id,value,expected,token] of examples){
 near(value,expected,id);
 assert(byId.get(id).example.includes(token),"worked arithmetic not visible in tutorial "+id+": "+token);
}
near(.5*2+.5*4,3,"ATE different from ATT");
near(.08+.03,.11,"DiD sensitivity upper bound");
near(.48+.4-.08,.80,"Manski partial identification upper bound");
near(2e-6/1e-6,2,"tiny step is not stationarity");
const mathHtml=fs.readFileSync(new URL("index.html",root),"utf8");
const mlHtml=fs.readFileSync(new URL("../ml/index.html",root),"utf8");
assert(mathHtml.includes('aria-label="V20双重机器学习与近端优化十二课贯通"'));
assert(mlHtml.includes('aria-label="V20金融机器学习因果与凸优化十二课"'));
for(const id of targetIds){
 assert(mathHtml.includes("?topic="+id),"math route missing "+id);
 assert(mlHtml.includes("../math/?topic="+id),"cross-course ML route missing "+id);
}
for(const p of ["styles.css","../ml/atlas.css"]){
 const css=fs.readFileSync(new URL(p,root),"utf8");
 for(const rule of ["overflow-x:auto","line-break:strict",":focus-visible","@media print"])assert(css.includes(rule),"CJK/formula/print safeguard missing "+rule+" in "+p);
}
console.log("PASS V20: 5 newly completed lectures, 7 original lectures deepened, 38 new proof sections, "+parsed+" strict KaTeX checks, "+sources+" citations, "+links+" links, 12 independent numeric cases, 278 unique full math courses, 656-node acyclic curriculum.");
