import assert from "node:assert/strict";
import fs from "node:fs";
const read=path=>JSON.parse(fs.readFileSync(new URL(path,import.meta.url),"utf8"));
const manifest=read("./lessons-manifest.json");
const chapters=new Map(manifest.modules.flatMap(m=>read(m.path).lessons.map(l=>[l.node_id,l])));
const cases=[
["foundation-n0013",1/101,0.009900990099009901,"101"],
["calculus-n0040",(8*3+3*4)/5,7.2,"7.2"],
["linear-n0065",1+2,3,"1+2=3"],
["probability-n0095",.018/.116,9/58,"15.52%"],
["measure-n0121",.5*3+.5*1,2,"=2"],
["statistics-n0162",(3/10)/4,.075,"0.075"],
["regression-n0188",200*.06,12,"7.815"],
["econometrics-n0212",2+3*1/2,3.5,"3.5"],
["timeseries-n0241",2*(-1),-2,"2(−1)^h"],
["stochastic-n0279",.8*.8+.2*.3,.7,"0.70"],
["ito-n0308",4/3,1.333333333333,"1.3333"],
["optimization-n0340",2-2**2/4,1,"g(2)=1"],
["numerical-n0362",1/.01,100,"100"],
["diffeq-n0397",3*Math.exp(-2*Math.log(2)),.75,"0.75"],
["finance-n0430",10/1.05,9.523809523809524,"9.5238"],
["mlmath-n0453",.5*(1-2)**2+.5*(3-2)**2+4,5,"=5"],
["information-n0484",.9*Math.log(1.8)+.1*Math.log(.2),.3680642071684971,"0.3681"],
["discrete-n0519",2/(4-1),2/3,"2/3"],
["control-n0545",1+.5*4,3,"V*(s1)=3"],
["riskmath-n0584",.25*(.04625+.08375)+.5*.0135,.03925,"0.03925"],
["foundation-n0005",6,6,"sup A=6"],
["calculus-n0041",2*6-2**2,8,"=8"],
["linear-n0070",11-2*5,1,"y₂=11−2×5=1"],
["probability-n0100",45*.2**2*.8**8,.301989888,"0.30199"],
["timeseries-x001",.01**2+(-.02)**2+.03**2,.0014,"0.0014"],
["finance-x001",.6*(.5/(1.05*.6))*1.2+.4*(.5/(1.05*.4))*.9,1,"=1"],
["optimization-x001",5.05+4.95,10,"x+y=10"],
["riskmath-x002",(.03*10+.02*100)/.05,46,"=46"]
];
assert.equal(chapters.size,28,"twenty-eight unique mathematics lesson IDs");
assert.equal(cases.length,28);
for(const [id,actual,expect,token] of cases){
 const lesson=chapters.get(id);
 assert(lesson,"missing numerical lesson "+id);
 assert(Math.abs(actual-expect)<1e-9,"wrong independent computation "+id);
 assert(lesson.example.includes(token),"worked example must contain checked arithmetic "+id+": "+token);
}
console.log("PASS: 20 mathematical disciplines, 28 independent numeric/research example invariants.");
