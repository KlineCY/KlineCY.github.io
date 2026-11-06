import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const read=(p)=>JSON.parse(fs.readFileSync(new URL(p,import.meta.url),"utf8"));
const math=[...read("./lessons-phase-28-a.json").lessons,...read("./lessons-phase-29-b.json").lessons];
const ml=read("../ml/atlas-depth-17-b.json").lessons;
const ids=["statistics-n0177","statistics-n0178","statistics-n0179","statistics-n0180","information-n0509","information-n0506","information-n0510"];
const mid=["m09-c03-p01","m10-c01-p03","m11-c04-p04","m12-c03-p03","m13-c02-p02","m14-c01-p01"];
const render=tex=>{assert.equal(typeof tex,"string");assert(tex.length>8);katex.renderToString(tex,{throwOnError:true,strict:"error",displayMode:true});};
assert.deepEqual(math.map(l=>l.id),ids);
let count=0;
for(const l of math){
 assert(l.id===l.node_id&&l.deep_sections.length>=6,l.id+" full section count");
 assert(l.formulas.length>=8&&l.derivation.length>=6,l.id+" formula/derivation coverage");
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,l.deep_sections.length);
 const last=l.deep_sections.at(-1);
 assert(last.paragraphs.length===2&&last.paragraphs.every(s=>s.length>=37));
 assert(last.proof.length>=55&&last.worked.length>=36&&last.limitation.length>=35);
 assert(last.formulas.length>=2);
 for(const f of [...l.formulas,...l.deep_sections.flatMap(s=>s.formulas),...l.derivation]){render(f.tex);count++;}
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)));
}
for(const id of mid){
 const l=ml[id];assert(l&&l.equations.length>=6&&l.derivation.length>=6,id+" content");
 assert(l.worked_example.length>=120&&l.pitfalls.length>=3);
 assert(l.derivation.every(x=>x.length>=35));
 for(const e of l.equations){assert(e.label);render(e.tex);count++;}
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)));
}
const near=(actual,want,what,tol=1e-8)=>assert(Math.abs(actual-want)<=tol,what+": "+actual+" != "+want);
near(6*.1/(6*.1+.9),.4,"prior-sensitive Bayes probability");
near(6/(1+5*.1)**2,8/3,"Bayes derivative");
near((2+6)/(1+3),2,"Gamma-Poisson posterior intensity");
near(2*2+2**2*(8/16),6,"Gamma-Poisson forecast count variance");
near(1/(4+1),.2,"SBC five posterior ranks");
near((1/3)*.5,1/6,"randomized PIT event zero");
near(1/3+(2/3)*.5,2/3,"randomized PIT event one");
near(.5*1+.5*9,5,"UT quadratic mean");
near(2*(4-5)**2+.5*(1-5)**2+.5*(9-5)**2,18,"UT quadratic variance");
near(1/((2/3)**2+2*(1/6)**2),2,"particle ESS");
near((1-(1/3)**4)+2*(1-(5/6)**4),2622/1296,"expected distinct ancestors");
near((.25+.20)/2,.225,"expected pathwise max drawdown");
near(1-90/100,.1,"drawdown of expected path");
near(.5*1**2,.5,"SVM hard margin primal");
near(2*.5-.5*1**2,.5,"SVM dual optimum");
near(4*5/(6*7),10/21,"joint Bayesian Bernoulli successes");
near(8/(6**2*7),2/63,"predictive Bernoulli covariance");
near(.8*(0-.4)**2+.2*(2-.4)**2,.64,"GMM M-step variance");
near(5+2*(4+3+2+1),25,"HAC overlapping long-run variance");
near((3*1+4*2),11,"vector forward dot product");
near((3*1+4*2)*2,22,"vector backprop second coordinate");
near(3+(3-1)*1,5,"two layer convolution receptive field");
console.log("PASS V61: 7 mathematics chapters, 6 ML foundational lectures, "+count+" strict KaTeX expressions, 22 numerical checks");
