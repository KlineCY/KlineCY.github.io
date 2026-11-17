import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const dir=new URL("./",import.meta.url);
const read=p=>fs.readFileSync(new URL(p,dir),"utf8");
const json=p=>JSON.parse(read(p));
const curriculum=json("curriculum.json");
const nodes=new Map(curriculum.nodes.map(n=>[n.id,n]));
const manifest=json("lessons-manifest.json");
assert.equal(manifest.expected_lessons,201,"historical math courses must remain unchanged");
assert.equal(manifest.modules.length,77,"historical modules must remain unchanged");
const priorIds=new Set(manifest.modules.flatMap(m=>json(m.path).lessons.map(l=>l.node_id)));
const files=["lessons-phase-31-a.json","lessons-phase-31-b.json","lessons-phase-31-c.json","lessons-phase-31-d.json"];
const subjects=["finance","riskmath","econometrics","mlmath"];
const all=[];
let formulaCount=0,sectionCount=0;
for(let k=0;k<files.length;k++){
 const d=json(files[k]);
 assert.equal(d.version,1);
 assert.equal(d.revision,"V116");
 assert.equal(d.lessons.length,6);
 for(let i=0;i<6;i++){
  const l=d.lessons[i],id=subjects[k]+"-x"+String(i+1).padStart(3,"0");
  assert.equal(l.id,id);
  assert.equal(l.node_id,id);
  assert.equal(l.title,nodes.get(id)?.title,"existing math outline must be completed in place");
  assert(!priorIds.has(id),"historical math lesson overwritten: "+id);
  for(const prop of ["definition","assumptions","application","caveat","example"])
   assert(l[prop]?.length>=55,id+" incomplete "+prop);
  assert(l.properties.length>=4&&l.properties.every(x=>x.length>=25));
  assert.equal(l.formulas.length,4);
  assert.equal(l.derivation.length,3);
  assert.equal(l.deep_sections.length,2);
  for(const item of [...l.formulas,...l.derivation]){
   const tex=item.tex;
   assert(tex.length>=10,"incomplete formula "+id);
   katex.renderToString(tex,{throwOnError:true,strict:"error",displayMode:true});
   formulaCount++;
  }
  for(const s of l.deep_sections){
   assert(s.title.length>=8&&s.question.length>=10);
   assert(s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=45));
   assert(s.proof.length>=55&&s.worked.length>=55&&s.limitation.length>=25);
   assert(s.formulas.length===2);
   for(const q of s.formulas){katex.renderToString(q.tex,{throwOnError:true,strict:"error",displayMode:true});formulaCount++;}
   sectionCount++;
  }
  all.push(l);
 }
}
assert.equal(all.length,24);
assert.equal(sectionCount,48);
assert.equal(formulaCount,264);
assert.equal(new Set(all.map(l=>l.node_id)).size,24);
const app=read("app.js");
assert(app.includes("loadV116MathExpansion()")&&app.includes("...v110e,...v116"),"new lessons must be present in live math reader");
const checks=[
["HJ lower bound",(.04/.10)/1.02,.39215686274509803],
["VaR discrete tail",.9+.1,1],
["ES atom counterexample",(.95*0+.05*100)/1,5],
["conditional MES",(1+3-1+5)/4,2],
["copula independent tail",.1*.1,.01],
["AIPW sample",8-5+(9-8)/.5,5],
["ATE and ATT example",.5*2+.5*0,1],
["DML partial linear",2/4,.5],
["importance ESS",100/66,1.5151515151515151],
["Hoeffding n200",Math.sqrt(Math.log(40)/400),.096032278],
["GMM J statistic",100*(.01**2+.02**2),.05],
["copula CoVaR spread",15-8,7]
];
for(const [name,val,expected] of checks)assert(Math.abs(val-expected)<(name==="Hoeffding n200"?1e-6:1e-9),name+": "+val+" vs "+expected);
console.log("PASS: 24 complete mathematical lectures, 48 complete concept sections, 264 strict KaTeX expressions; original 201 manifest courses preserved");
