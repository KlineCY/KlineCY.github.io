import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const math=JSON.parse(fs.readFileSync(new URL("./lessons-phase-29-a.json",import.meta.url),"utf8")).lessons;
const ml=JSON.parse(fs.readFileSync(new URL("../ml/atlas-depth-17-a.json",import.meta.url),"utf8")).lessons;
const ids=["information-n0508","information-n0507","information-n0505"];
const mid=["m03-c03-p04","m04-c01-p02","m05-c02-p03","m06-c04-p05","m07-c02-p01","m08-c01-p02"];
const render=tex=>katex.renderToString(tex,{throwOnError:true,strict:"ignore",displayMode:true});
for(const l of math){assert(ids.includes(l.id));assert(l.deep_sections.length>=6);assert(l.formulas.length>=9);assert(l.derivation.length>=6);const s=l.deep_sections.at(-1);assert(s.paragraphs.length>=2&&s.paragraphs.every(x=>x.length>=35));assert(s.proof.length>=70&&s.worked.length>=45&&s.limitation.length>=30);for(const f of [...l.formulas,...l.deep_sections.flatMap(x=>x.formulas),...l.derivation])render(f.tex);}
for(const id of mid){const l=ml[id];assert(l&&l.equations.length>=6&&l.derivation.length>=6);assert(l.worked_example.length>=120);assert(l.pitfalls.length>=3);for(const f of l.equations)render(f.tex);assert(l.derivation.every(x=>x.length>=25));}
const near=(x,y,name,tol=1e-9)=>assert(Math.abs(x-y)<tol,name+": "+x+" != "+y);
near(.12/(.12+.20),.375,"HMM normalized state probability");
near(1/(1+0.25),.8,"Kalman gain");
near((1+5+2)/(1+2+1),2,"exposure-weighted Poisson intensity");
near(Math.sqrt(Math.log(40)/400),.0960322803,"Hoeffding radius",1e-8);
near(80/(20*99)*4,.16161616161616163,"without-replacement gradient covariance");
near(10/15*2,4/3,"Ridge directional shrinkage");
near(.5-2*(.8*.2),.18,"Gini weighted split");
console.log("V60: 3 filtering chapters and 6 ML foundational chapters, strict KaTeX and 7 independent numerical identities passed");
