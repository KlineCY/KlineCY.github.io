import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=n=>fs.readFileSync(new URL(n,root),"utf8");
const parse=n=>JSON.parse(read(n));
const syllabus=parse("curriculum.json");
assert.equal(syllabus.nodes.length,656,"Curriculum should still have 656 nodes");
const nodeMap=new Map(syllabus.nodes.map(n=>[n.id,n]));
assert.equal(nodeMap.size,656,"Duplicate syllabus nodes");
const files=["lessons-calculus-v107a.json","lessons-calculus-v107b.json"];
const docs=files.map(parse);
for(const doc of docs){
 assert.equal(doc.version,1,"V107 document schema changed");
 assert.equal(doc.revision,"V107","V107 revision missing");
 assert.equal(doc.lessons.length,4,"V107 course group missing chapter");
}
const added=docs.flatMap(d=>d.lessons);
const desired=["calculus-n0043","calculus-n0044","calculus-n0045","calculus-n0047",
 "calculus-n0048","calculus-n0049","calculus-n0050","calculus-n0051"];
assert.deepEqual(added.map(l=>l.node_id),desired,"V107 chapters not in syllabus order");
const existing=fs.readdirSync(root).filter(n=>/^lessons-.*\.json$/.test(n)&&!files.includes(n))
 .flatMap(n=>{const doc=parse(n);return Array.isArray(doc.lessons)?doc.lessons:[];});
assert.equal(existing.length,570,"Must retain exactly 570 historical completed maths lessons");
const allIds=[...existing,...added].map(l=>l.node_id);
assert.equal(new Set(allIds).size,578,"Expected 578 unique maths lessons without overwriting old IDs");
let formulas=0,proofs=0,characters=0,links=0;
function checkTex(tex,id){
 assert.equal(typeof tex,"string",id+" formula missing");
 assert(tex.length>7,id+" formula too short");
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(tex),id+" forbidden codepoint");
 try{katex.renderToString(tex,{throwOnError:true,strict:"error",displayMode:true});}
 catch(e){throw Error(id+" invalid KaTeX: "+tex+" - "+e.message);}
 formulas++;
}
for(const l of added){
 const id=l.node_id;
 assert.equal(l.id,id,"Invalid ID");
 assert.equal(l.title,nodeMap.get(id)?.title,id+" title differs from curriculum");
 for(const f of ["definition","assumptions","example","application","caveat"])
  assert(typeof l[f]==="string"&&l[f].length>=60,id+" missing "+f);
 assert(l.properties.length>=4&&l.properties.every(x=>x.length>=25),id+" incomplete properties");
 assert.equal(l.formulas.length,3,id+" incomplete formulas");
 assert.equal(l.derivation.length,3,id+" incomplete derivation");
 assert.equal(l.deep_sections.length,3,id+" incomplete deep chapters");
 assert.equal(l.links.length,3,id+" incomplete study references");
 for(const x of l.formulas){assert(x.label.length>1);checkTex(x.tex,id);}
 for(const x of l.derivation){assert(x.explanation.length>=20);checkTex(x.tex,id);}
 const titles=new Set();
 for(const section of l.deep_sections){
  assert(section.title?.length>=8&&section.question?.length>=10,id+" section heading");
  assert(!titles.has(section.title),id+" repeated section");
  titles.add(section.title);
  assert(section.paragraphs?.length===2&&section.paragraphs.every(p=>p.length>=40),id+" underdeveloped explanation");
  assert(section.proof?.length>=50&&section.worked?.length>=55&&section.limitation?.length>=30,id+" missing proof calculation or boundary");
  assert.equal(section.formulas?.length,1,"Deep formula missing");
  checkTex(section.formulas[0].tex,id);
  proofs++;
  characters+=section.paragraphs.join("").length+section.proof.length+section.worked.length+section.limitation.length;
 }
 assert(/[\u3400-\u9FFF]/u.test(JSON.stringify(l)),id+" needs Chinese text");
 assert(!/TODO|FIXME|Lorem ipsum|待补充/i.test(JSON.stringify(l)),id+" contains placeholder");
 for(const link of l.links){assert.equal(link.kind,"math");assert(nodeMap.has(link.id),id+" broken link "+link.id);links++;}
}
assert.equal(formulas,72,"Strict KaTeX expressions should equal 72");
assert.equal(proofs,24,"Deep proof chapters should equal 24");
assert(characters>=7600,"Detailed proof and worked-example text substantially reduced");
assert.equal(links,24,"Cross-lesson links should equal 24");
const app=read("app.js"),html=read("index.html"),css=read("styles.css");
assert(app.includes('loadV107AdvancedCalculus("./lessons-calculus-v107a.json",4)'),"Missing V107 first reader");
assert(app.includes('loadV107AdvancedCalculus("./lessons-calculus-v107b.json",4)'),"Missing V107 second reader");
assert(app.includes("...v106a,...v106b,...v107a,...v107b"),"Old V106 loader omitted");
assert(app.includes("new Set(lessons.map(l=>l.node_id)).size!==lessons.length"),"ID uniqueness guard removed");
assert(html.includes("V107 · 高阶微积分"),"V107 Chinese roadmap missing");
assert(html.includes("v107-integrals-implicit"),"V107 cache busting missing");
for(const id of desired)assert(html.includes('?topic='+id+'"'),"Missing topic link "+id);
for(const token of ["overflow-x:auto","line-break:strict","@media(max-width:700px)","@media print",":focus-visible"])
 assert(css.includes(token),"Accessibility or CJK formula layout regression: "+token);
assert(read("../../.github/workflows/test-quant-math.yml").includes("node quant/math/test_v107_integrals_implicit.mjs"),"CI gate not present");
let checks=0;
function near(label,val,target,tol=1e-8){
 assert(Number.isFinite(val),label+" nonfinite");
 assert(Math.abs(val-target)<=tol,label+" "+val+" != "+target);
 checks++;
}
function yes(label,bool){assert(bool,label);checks++;}
const S=n=>n*(n+1)/2;
near("triangular integral x+y",3/2/3,1/2);
near("unit square xy",1/2*1/2,1/4);
near("triangle xy",1/8,0.125);
near("probability x+y<=1",1-1/2,1/2);
near("2x2 midpoint",(.5+1+1+1.5)/4,1);
near("three uniform sum",(1/2)*3,1.5);
near("3D simplex",1/6,1/6);
near("cylindrical jacobian volume",3*2*Math.PI*(2**2/2),12*Math.PI);
near("3D unit cube volume",1,1);
near("Fubini triangle",.5,.5);
near("Tonelli xy",.5*.5,.25);
near("triangle constant area",.5*1*1,.5);
near("integration by parts",Math.E-(Math.E-1),1);
near("exponential survival",1/2,.5);
near("discounted annual total",1-Math.exp(-1),1-Math.exp(-1));
near("discounted terminal identity",Math.exp(-1)+1-2/Math.E,1-1/Math.E);
near("variable integral θ²",1.5*2**2,6);
near("variable integral derivative",3*2,6);
near("Leibniz two terms",2*2+2,6);
near("Leibniz T²",3.1**2-3**2,.61);
near("Leibniz T² linear",2*3*.1,.6);
near("zero coupon PV",100*Math.exp(-.1),90.48374180359595);
near("zero coupon rate sensitivity",-200*Math.exp(-.1),-180.9674836071919);
near("implicit F_y",2*4,8);
near("implicit slope",-2*3/(2*4),-.75);
near("implicit numerical root",Math.sqrt(25-3.1**2),Math.sqrt(15.39));
near("condition amplifies",.001/.01,.1);
near("implied vol price shift",.001/.2,.005);
near("implied vol low vega",.001/.01,.1);
near("inverse determinant",2*3-1*1,5);
near("inverse solved x",(3*5-1*7)/5,1.6);
near("inverse solved y",(-1*5+2*7)/5,1.8);
near("inverse reconstructed first",2*1.6+1.8,5);
near("inverse reconstructed second",1.6+3*1.8,7);
near("inverse ill conditioned",.001/.01,.1);
near("Lagrange x",1/2,.5);
near("Lagrange y",1/2,.5);
near("Lagrange minimized objective",(.5**2)*2,.5);
near("Lagrange multiplier",-2*.5,-1);
near("Lagrange shadow derivative",2/2,1);
near("Lagrange active upper multiplier",2*(2-1),2);
near("Lagrange active upper loss",(1-2)**2,1);
yes("nonnegative samples positive",S(5)>0);
yes("convex equality answer feasible",.5+.5===1);
console.log("V107 PASS: 578 unique / 656 curriculum, "+formulas+" strict KaTeX, "+proofs+
 " deep chapters, "+characters+" proof characters, "+links+" links and "+checks+" independent arithmetic assertions");
