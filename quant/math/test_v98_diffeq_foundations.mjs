import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root = new URL("./", import.meta.url);
const read = name => fs.readFileSync(new URL(name, root), "utf8");
const json = name => JSON.parse(read(name));
const curriculum = json("curriculum.json");
const byId = new Map(curriculum.nodes.map(n => [n.id,n]));
assert.equal(byId.size, 656);
const manifest = json("lessons-manifest.json");
assert.equal(manifest.modules.length, 77);
const historicalFiles = [
 "lessons-foundations.json","lessons-advanced.json","lessons-more.json",
 ...manifest.modules.map(m => m.path.replace(/^\.\//,"")),
 "lessons-foundations-v89.json","lessons-linear-v90.json","lessons-linear-v92.json",
 "lessons-matrix-v93.json","lessons-probability-v93.json",
 "lessons-probability-v94a.json","lessons-probability-v94b.json","lessons-probability-v94c.json",
 "lessons-statistics-v95a.json","lessons-statistics-v95b.json",
 "lessons-optimization-v96a.json","lessons-optimization-v96b.json",
 "lessons-portfolio-v97.json","lessons-algorithms-v97.json","lessons-numerical-v97.json"
];
const historical = historicalFiles.flatMap(p => json(p).lessons);
assert.equal(historical.length, 414, "V97 historical lesson count changed");
assert.equal(new Set(historical.map(x=>x.node_id)).size, 414, "Historical duplicate node IDs");
const v90=json("lessons-linear-v90.json");
assert.equal(v90.lessons.length,8);
assert.equal(new Set(v90.lessons.map(x=>x.node_id)).size,8,"V90 loader formerly assumed four unique IDs");
const a=json("lessons-diffeq-v98a.json"), b=json("lessons-diffeq-v98b.json");
const current=[...a.lessons,...b.lessons];
for(const d of [a,b]) {
 assert.equal(d.version,1);
 assert.equal(d.revision,"V98");
 assert.equal(d.lessons.length,3);
}
assert.deepEqual(current.map(l=>l.node_id),Array.from({length:6},(_,i)=>"diffeq-n"+String(391+i).padStart(4,"0")));
const combined=[...historical,...current];
assert.equal(combined.length,420);
assert.equal(new Set(combined.map(l=>l.node_id)).size,420,"V98 overwrote an existing lesson");
const known=new Set(byId.keys());
let equations=0,sections=0,deepChars=0;
const compile=(tex,id)=>{
 assert(typeof tex==="string" && tex.length>=9,id+" short formula");
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(tex),id+" invalid Unicode");
 katex.renderToString(tex,{displayMode:true,throwOnError:true,strict:"error"});
 equations++;
};
for(const l of current) {
 assert.equal(l.id,l.node_id);
 assert.equal(l.title,byId.get(l.node_id)?.title,"curriculum title mismatch");
 for(const field of ["definition","assumptions","example","application","caveat"])
   assert(l[field]?.length>=70,l.id+" short "+field);
 assert(l.properties.length>=4 && l.properties.every(x=>x.length>=20));
 assert.equal(l.formulas.length,3);
 assert.equal(l.derivation.length,3);
 assert.equal(l.deep_sections.length,3);
 for(const f of l.formulas){assert(f.label.length>=4);compile(f.tex,l.id);}
 for(const d of l.derivation){assert(d.explanation.length>=25);compile(d.tex,l.id);}
 const titles=new Set();
 for(const s of l.deep_sections) {
   assert(!titles.has(s.title),l.id+" repeated section title");
   titles.add(s.title);
   assert(s.title.length>=8 && s.question.length>=10);
   assert(s.paragraphs.length===2 && s.paragraphs.every(p=>p.length>=30));
   assert(s.proof.length>=50 && s.worked.length>=50 && s.limitation.length>=25);
   assert.equal(s.formulas.length,1);
   compile(s.formulas[0].tex,l.id);
   deepChars+=s.paragraphs.join("").length+s.proof.length+s.worked.length+s.limitation.length;
   sections++;
 }
 for(const link of l.links||[])if(link.kind==="math")assert(known.has(link.id),l.id+" dangling link "+link.id);
 assert(/[\u3400-\u9FFF]/u.test(JSON.stringify(l)),l.id+" no Chinese");
 assert(!/TODO|FIXME|Lorem ipsum/i.test(JSON.stringify(l)),l.id+" placeholder");
}
assert.equal(equations,54);
assert.equal(sections,18);
assert(deepChars>=4800,"V98 detailed derivations lost");
const app=read("app.js"),css=read("styles.css");
for(const needle of [
 "new Set(doc.lessons.map(l=>l.node_id)).size!==8",
 'loadV98DifferentialEquations("./lessons-diffeq-v98a.json",3)',
 'loadV98DifferentialEquations("./lessons-diffeq-v98b.json",3)',
 "...v97portfolio,...v97algorithms,...v97numerical,...v98a,...v98b",
 "new Set(lessons.map(l=>l.node_id)).size!==lessons.length"
]) assert(app.includes(needle),"Missing runtime integration "+needle);
for(const rule of ["overflow-x:auto","line-break:strict","@media(max-width:700px)","@media print",":focus-visible"])
 assert(css.includes(rule),"Missing CJK, KaTeX or accessibility rule "+rule);
let checks=0;
function near(label,value,expected,tol=1e-9){
 assert(Number.isFinite(value)&&Math.abs(value-expected)<=tol,label+": "+value+" vs "+expected);
 checks++;
}
near("linear integrating factor time log2",Math.exp(2*Math.log(2)),4);
near("linear solution",2*(1-Math.exp(-2*Math.log(2))),1.5);
near("linear residual",-4*Math.exp(-2*.5)+2*(2*(1-Math.exp(-1))),4-8*Math.exp(-1));
near("linear OU mean",.04+(.02-.04)*Math.exp(-2*Math.log(2)),.035);
near("linear initial sensitivity",Math.exp(-2*Math.log(2)),.25);
near("logistic initial derivative",2*2*(1-2/10),3.2);
near("logistic at half log2",10/(1+4*Math.exp(-2*Math.log(2)/2)),10/3);
near("logistic positive upper bound",Number(10/(1+4*Math.exp(-2))<10),1);
near("separable exponential",3*Math.exp(.5**2),3*Math.exp(.25));
near("second-order root 1",(-3+Math.sqrt(9-8))/2,-1);
near("second-order root 2",(-3-Math.sqrt(9-8))/2,-2);
near("second-order initial derivative",-1+2,1);
near("second-order at log2",Math.exp(-Math.log(2))-Math.exp(-2*Math.log(2)),.25);
near("repeated root t1",1*Math.exp(-1),Math.E**-1);
near("complex real part",-1/2,-.5);
near("Picard contraction",2*.25,.5);
near("Picard second iterate",1-2*.25+2*(.25**2),.625);
near("Picard exact",Math.exp(-2*.25),.6065306597126334);
near("sqrt delayed t2 a1",(2-1)**2/4,.25);
near("sqrt immediate t2",2**2/4,1);
near("equilibrium stable derivative 1",1-3*1**2,-2);
near("equilibrium stable derivative -1",1-3*(-1)**2,-2);
near("equilibrium unstable derivative 0",1-0,1);
near("nonlinear cubic trajectory",2/Math.sqrt(1+2*2**2),2/3);
near("Lyapunov energy initial",.5*(4-2)**2,2);
near("Lyapunov energy at half-life",.5*(3-2)**2,.5);
near("phase stable coord 1",Math.exp(-Math.log(2)),.5);
near("phase stable coord 2",2*Math.exp(-2*Math.log(2)),.5);
near("phase stable slow ratio",Math.exp(-2*Math.log(2)),.25);
near("saddle unstable growth",.01*Math.exp(Math.log(100)),1);
near("spiral norm time pi2",Math.exp(-Math.PI/2),.20787957635076193);
near("spiral cosine pi",Math.cos(2*Math.PI/2),-1);
assert(checks>=30);
console.log("V98 PASS:",current.length,"new ODE lessons,",combined.length,"canonical lectures,",equations,
 "strict KaTeX formulas,",sections,"proof chapters,",checks,"numerical checks; V90 unique IDs fixed");
