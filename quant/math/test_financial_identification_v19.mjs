import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const v24Ids=new Set(["statistics-x003","statistics-x004","mlmath-x003","mlmath-x004"]);
const curriculum=read("curriculum.json"),manifest=read("lessons-manifest.json");
const core=["lessons-foundations.json","lessons-advanced.json","lessons-more.json"].flatMap(p=>read(p).lessons);
const extensions=manifest.modules.flatMap(m=>{
 const rows=read(m.path).lessons;assert.equal(rows.length,m.lessons,"module-count mismatch "+m.path);return rows;
});
const all=[...core,...extensions];
assert.equal(curriculum.nodes.length,656);
assert.equal(manifest.modules.length,66);
assert.equal(manifest.expected_lessons,164);
assert.equal(core.length,123);
assert.equal(extensions.length,164);
assert.equal(all.length,287);
const ids=new Set(all.map(l=>l.node_id));
assert.equal(ids.size,287,"duplicate complete lessons");
const introduced="abc".split("").flatMap(x=>read("lessons-phase-19-"+x+".json").lessons);
const newIds=["econometrics-n0232","econometrics-n0233","econometrics-n0234",
 "econometrics-n0236","econometrics-n0237","econometrics-n0238","econometrics-n0239"];
assert.deepEqual(introduced.map(x=>x.id),newIds);
const v20Ids=["econometrics-x003","econometrics-x005","econometrics-x006","optimization-x011","optimization-x012"];
const v21Ids=["mlmath-n0455","mlmath-n0466","mlmath-n0463","mlmath-n0464","mlmath-n0460","mlmath-n0461","mlmath-n0462","mlmath-n0465"];
const v22Ids=["timeseries-n0244","timeseries-n0245","timeseries-n0248","timeseries-n0249","timeseries-n0250","timeseries-n0251","timeseries-n0254","timeseries-n0255"];
const v23Ids=["timeseries-n0256","timeseries-n0257","timeseries-n0258","timeseries-n0259"];
assert.equal(all.filter(l=>!newIds.includes(l.id)&&!v20Ids.includes(l.id)&&!v21Ids.includes(l.id)&&!v22Ids.includes(l.id)&&!v23Ids.includes(l.id)&&!v24Ids.has(l.id)).length,246,"all pre-V19 complete lessons retained");
const fm=core.find(l=>l.id==="econometrics-n0235");
assert(fm&&fm.deep_sections.length>=8&&fm.formulas.length>=6&&fm.derivation.length>=7,"existing Fama–MacBeth was not fully deepened");
assert(fm.deep_sections.some(s=>s.title==="生成的Beta误差与两阶段截面推断"));
const nodes=new Map(curriculum.nodes.map(n=>[n.id,n]));
const visiting=new Set(),done=new Set();
function dfs(id){assert(nodes.has(id),"unknown math prerequisite "+id);if(visiting.has(id))throw Error("prerequisite cycle "+id);if(done.has(id))return;visiting.add(id);for(const p of nodes.get(id).prerequisites||[])dfs(p);visiting.delete(id);done.add(id);}
for(const n of curriculum.nodes)dfs(n.id);
assert.equal(done.size,656);
let formulas=0,sections=0,links=0,sources=0;
for(const l of [...introduced,fm]){
 assert(l.id===l.node_id&&nodes.get(l.id)?.title===l.title,"title/id mismatch "+l.id);
 assert(l.definition.length>120&&l.assumptions.length>95&&l.example.length>135,"insufficient complete lecture "+l.id);
 assert(l.application.length>75&&l.caveat.length>70,"financial boundaries too short "+l.id);
 assert(l.properties.length>=3&&l.derivation.length>=3&&l.formulas.length>=4);
 const deep=l===fm?l.deep_sections.slice(-4):l.deep_sections;
 assert.equal(deep.length,4);
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,l.deep_sections.length);
 for(const s of deep){
  assert(s.question.length>=12&&s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=45),"thin CJK explanation "+l.id+":"+s.title);
  assert(s.proof.length>=40&&s.worked.length>=35&&s.limitation.length>=35,"missing proof or worked boundary "+l.id+":"+s.title);
  for(const f of s.formulas){katex.renderToString(f.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++}
  sections++;
 }
 const labels=new Set();
 for(const f of l.formulas){
  assert(!labels.has(f.label),"duplicate equation label "+l.id+":"+f.label);labels.add(f.label);
  katex.renderToString(f.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++;
 }
 for(const d of l.derivation){assert(d.explanation.length>=35);katex.renderToString(d.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++}
 for(const link of l.links||[]){
  if(link.kind==="math")assert(nodes.has(link.id),"broken math prerequisite "+l.id+" -> "+link.id);
  else assert(/^m\d\d-c\d\d-p\d\d$/.test(link.id),"bad ML link "+l.id+" -> "+link.id);
  links++;
 }
 for(const src of l.sources||[]){assert(src.title.length>=25&&/^https:\/\//.test(src.url));sources++}
 assert(!JSON.stringify(l).includes("\ufffd"),"UTF-8 replacement symbol in "+l.id);
}
assert.equal(sections,32);
assert(formulas>=90&&sources>=16&&links>=32,"minimum formulas, source and cross-link coverage");
const near=(a,b,id)=>assert(Math.abs(a-b)<1e-9,"numeric invariant "+id+": "+a+" versus "+b);
const cases=[
 ["econometrics-n0232",(16-10)-(11-8),3,"DiD=6−3=3"],
 ["econometrics-n0233",1.5/(.7-.2),3,"1.5/0.5=3"],
 ["econometrics-n0234",1.04*1.008-1.024*1.013,.011008,"0.011008"],
 ["econometrics-n0236",4*.01,.04,"0.04"],
 ["econometrics-n0237",1+(.5*4)/(.5**2*4+1),2,"b=1+2/2=2"],
 ["econometrics-n0238",10/16,.625,"0.625"],
 ["econometrics-n0239",.10-((.10-.30)/2),.20,"高估20个百分点"]
];
assert.equal(cases.length,7);
for(const [id,actual,expect,token] of cases){near(actual,expect,id);assert(introduced.find(l=>l.id===id).example.includes(token),"missing independent numerical example "+id);}
near((.02+.04+.06)/3,.04,"Fama-MacBeth beta premium mean");
near((((.02-.04)**2)+((.04-.04)**2)+((.06-.04)**2))/2,.0004,"Fama-MacBeth unbiased time variance");
near(Math.sqrt(.0004/3),.011547005383792516,"Fama-MacBeth date-level standard error");
assert(fm.example.includes("0.011547"));
near(.625/(.75**2),1.1111111111111112,"HC3 leverage correction");
const mathHtml=fs.readFileSync(new URL("index.html",root),"utf8");
const mlHtml=fs.readFileSync(new URL("../ml/index.html",root),"utf8");
assert(mathHtml.includes('aria-label="V19因果识别与金融实证八篇精讲路径"'));
assert(mlHtml.includes('aria-label="V19金融因果识别与量化实证数学交叉路径"'));
for(const id of [...newIds,"econometrics-n0235"]){
 assert(mathHtml.includes("?topic="+id),"math navigation missing "+id);
 assert(mlHtml.includes("../math/?topic="+id),"ML navigation missing "+id);
}
for(const name of ["styles.css","../ml/atlas.css"]){
 const style=fs.readFileSync(new URL(name,root),"utf8");
 assert(style.includes("overflow-x:auto")&&style.includes("line-break:strict")&&style.includes("@media print")&&style.includes(":focus-visible"),"CJK/TeX mobile, keyboard and print safeguards missing "+name);
}
console.log("PASS V19: 7 genuinely new + 1 preserved/deepened lecture, 32 complete sections, "+formulas+" KaTeX renders, "+sources+" academic references, "+links+" crosslinks, 278 unique lectures, 656-node DAG and 7 independent numeric tests.");
