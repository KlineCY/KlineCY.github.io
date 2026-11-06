import assert from "node:assert/strict";
import fs from "node:fs";
import katex from "katex";
const doc=JSON.parse(fs.readFileSync(new URL("./lessons-phase-30-b.json",import.meta.url),"utf8"));
const L=new Map(doc.lessons.map(x=>[x.node_id,x]));
const near=(a,b,e=1e-8)=>assert(Math.abs(a-b)<e,"expected "+b+", got "+a);
for(const id of ["finance-n0440","finance-n0441","finance-n0442"]){
 const x=L.get(id);assert(x,"missing "+id);
 assert(x.deep_sections.length>=6&&x.formulas.length>=9&&x.derivation.length>=6,"insufficient depth "+id);
 assert(x.deep_sections[5].paragraphs.length>=2&&x.deep_sections[5].proof.length>150,"missing detailed sixth chapter "+id);
 for(const eq of [...x.formulas,...x.derivation,...x.deep_sections.flatMap(s=>s.formulas)])
  katex.renderToString(eq.tex,{throwOnError:true,strict:"ignore"});
}
const p2=(101.8-5*.97)/105;
near(p2,.9233333333333333,1e-12);near(5*.97+105*p2,101.8,1e-12);
near(Math.log(.97/p2),.04930576098415406,1e-10);
const ev=Math.exp(-1/12),mean=.04+(.06-.04)*ev,variance=.02**2/2*(1-Math.exp(-2/12));
near(mean,.05840088829258647,1e-11);near(variance,.00003070365502187717,1e-13);
near(Math.log(2),.6931471805599453,1e-12);
const c=.2**2*(1-Math.exp(-1))/4,nu=4*.04/.2**2;
const lambda=4*Math.exp(-1)*.06/(.2**2*(1-Math.exp(-1)));
near(c,.006321205588285578,1e-12);near(nu,4,1e-12);
near(lambda,3.491860241215958,1e-10);
near(c*(nu+lambda),.04+(.06-.04)*Math.exp(-1),1e-12);
console.log("V59 math strict formulas and three numerical derivations PASS.");
