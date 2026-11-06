import assert from "node:assert/strict";
import fs from "node:fs";
import katex from "katex";
const doc=JSON.parse(fs.readFileSync(new URL("./atlas-depth-18-a.json",import.meta.url),"utf8"));
const near=(a,b,e=1e-9)=>assert(Math.abs(a-b)<e,"expected "+b+", got "+a);
for(const id of ["m20-c02-p01","m21-c01-p03","m22-c04-p03","m23-c04-p02"]){
 const l=doc.lessons[id];assert(l&&l.overview.length>80,"missing "+id);
 assert(l.equations.length>=9&&l.derivation.length>=8&&l.protocol.length>=7,"incomplete "+id);
 assert(l.worked_example.length>280,"thin numerical example "+id);
 assert.equal(new Set(l.equations.map(x=>x.label)).size,l.equations.length,"duplicate labels "+id);
 for(const eq of l.equations)katex.renderToString(eq.tex,{throwOnError:true,strict:"ignore"});
}
const improved=1.2+.5*4;near(improved,3.2);assert(improved>8/3);
const drBuy=.5*3+(.5/.2)*(4-3),drSkip=.5*3;
near((drBuy+drSkip)/2,2.75);near(.2*drBuy+.8*drSkip,2);
const q11=.1+.1*4+.8,q22=.1+.1+.8,q12=.1*.2+.1*2+.8*.2;
near(q11,1.3);near(q12,.38);
const rho=q12/Math.sqrt(q11*q22);near(rho,.3332820473366711,1e-10);
near(.2*rho*.3,.01999692284020027,1e-10);
near(100+100-100,100);near(100+110-105,105);
near(105-100*.02/12,104.83333333333333,1e-10);
console.log("V59 ML strict formulas and four independent numerical checks PASS.");
