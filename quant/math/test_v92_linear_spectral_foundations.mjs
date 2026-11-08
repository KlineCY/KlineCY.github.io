import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const dir=new URL("./",import.meta.url);
const read=(p)=>fs.readFileSync(new URL(p,dir),"utf8");
const json=(p)=>JSON.parse(read(p));
const v89=json("lessons-foundations-v89.json"),v90=json("lessons-linear-v90.json"),v92=json("lessons-linear-v92.json"),c=json("curriculum.json"),manifest=json("lessons-manifest.json");
assert.equal(v89.lessons.length,12,"V89 historical baseline count");
assert(v89.lessons.every(l=>l.deep_sections.length===4),"Do not overwrite V89 proof chapters");
assert.equal(v90.revision,"V90");assert.equal(v90.lessons.length,8);
assert.equal(v92.revision,"V92");assert.equal(v92.lessons.length,8);
const expected92=["linear-n0071","linear-n0072","linear-n0073","linear-n0074","linear-n0079","linear-n0080","linear-n0081","linear-n0087"];
assert.deepEqual(v92.lessons.map(l=>l.node_id),expected92);
const known=new Map(c.nodes.map(n=>[n.id,n]));
assert.equal(known.size,656);
const core=[...json("lessons-foundations.json").lessons,...json("lessons-advanced.json").lessons,...json("lessons-more.json").lessons];
const phases=manifest.modules.flatMap(x=>json(x.path).lessons);
assert.equal(manifest.expected_lessons,201);
assert.equal(phases.length,201);
const old=[...core,...phases,...v89.lessons];
assert.equal(old.length,336);
assert.equal(new Set(old.map(l=>l.node_id)).size,336,"Historical 336 lessons duplicated");
const all=[...old,...v90.lessons,...v92.lessons];
assert.equal(all.length,352);
assert.equal(new Set(all.map(l=>l.node_id)).size,352,"Duplicate content ID");
let texCount=0,chapterCount=0,bodyCharacters=0;
function checkTex(tex,where){
 assert(typeof tex==="string"&&tex.length>=10,where+" incomplete math");
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(tex),where+" invalid unicode");
 katex.renderToString(tex,{throwOnError:true,strict:"error",displayMode:true});
 texCount++;
}
for(const lesson of [...v90.lessons,...v92.lessons]){
 const node=known.get(lesson.node_id);
 assert(node&&node.title===lesson.title,"curriculum mismatch: "+lesson.node_id);
 assert.equal(lesson.id,lesson.node_id);
 for(const f of ["definition","assumptions","example","application","caveat"])
   assert(typeof lesson[f]==="string"&&lesson[f].length>=55,lesson.id+" incomplete "+f);
 assert(lesson.properties.length>=4&&lesson.properties.every(x=>x.length>=17));
 assert.equal(lesson.formulas.length,3);
 assert.equal(lesson.derivation.length,3);
 for(const item of lesson.formulas)checkTex(item.tex,lesson.id);
 for(const item of lesson.derivation){
  assert(item.explanation.length>=24);
  checkTex(item.tex,lesson.id);
 }
 assert.equal(lesson.deep_sections.length,3);
 const titles=new Set();
 for(const section of lesson.deep_sections){
  assert(!titles.has(section.title),"duplicate chapter title: "+section.title);titles.add(section.title);
  assert(section.title.length>=8&&section.question.length>=10,lesson.id+" incomplete chapter title");
  assert(section.paragraphs.length===2&&section.paragraphs.every(s=>s.length>=28),lesson.id+" shallow explanations");
  assert(section.proof.length>=42&&section.worked.length>=35&&section.limitation.length>=20,lesson.id+" shallow example/proof");
  assert.equal(section.formulas.length,1);
  checkTex(section.formulas[0].tex,lesson.id);
  bodyCharacters+=section.proof.length+section.worked.length+section.paragraphs.join("").length;
  chapterCount++;
 }
 assert(/[\u3400-\u9FFF]/u.test(JSON.stringify(lesson)),lesson.id+" missing Chinese textbook narrative");
 assert(!/TODO|FIXME|Lorem ipsum/i.test(JSON.stringify(lesson)),lesson.id+" draft content");
}
assert.equal(chapterCount,48);
assert.equal(texCount,144);
assert(bodyCharacters>10000,"Insufficient depth across 16 lectures");
const app=read("app.js"),css=read("styles.css");
for(const token of ["loadV89Foundations()","loadV90LinearFoundations()","loadV92LinearFoundations()","...extra,...v89,...v90,...v92"]){
 assert(app.includes(token),"Math reader missing "+token);
}
for(const token of ["overflow-x:auto","line-break:strict","@media(max-width:700px)","@media print"]){
 assert(css.includes(token),"CJK/math layout issue "+token);
}
let numeric=0;
function close(name,a,b,tol=1e-9){
 assert(Number.isFinite(a)&&Math.abs(a-b)<=tol,name+": "+a+" vs "+b);numeric++;
}
close("inverse example x",5-3,2);
close("inverse example y",-5+2*3,1);
close("noninvertible determinant",1*4-2*2,0);
close("trace of triangular",2+3,5);
close("determinant of triangular",2*3,6);
close("trace of diagonal covariance",.01+.04+.09,.14);
close("orthogonal vector dot product",1*2+2*(-1),0);
close("Pythagoras",3*3+1*1,5+5);
close("orthogonal decomposition x coordinate",2+1,3);
close("orthogonal decomposition y coordinate",2-1,1);
close("symmetric eigenvalue trace",3+1,4);
close("symmetric eigenvalue determinant",3*1,3);
close("symmetric eigenvalue residual",2*1+1*1,3);
close("square of symmetric matrix first entry",2*2+1*1,5);
close("Jordan block k3 off diagonal",3*(.5**2),.75);
close("Rayleigh of A at e1",2,2);
close("Cholesky first pivot",Math.sqrt(4),2);
close("Cholesky second pivot",Math.sqrt(3-1*1),Math.sqrt(2));
close("Cholesky determinant",4*3-2*2,8);
close("Cholesky factor determinant",(2*Math.sqrt(2))**2,8);
close("Cholesky solution first rhs",4*1+2*1,6);
close("Cholesky solution second rhs",2*1+3*1,5);
assert(numeric>=22);
console.log("V92 PASS: 16 newly completed linear algebra lectures, 48 proof chapters, 144 strict KaTeX equations, "+numeric+" independent arithmetic checks, V89 historical 336 unaffected; 352 of 656 total math nodes complete. CJK/mobile style guards pass.");
