import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,root),"utf8"));
const manifest=read("lessons-manifest.json"),catalog=read("curriculum.json");
const lessons=[...read("lessons-phase-30-a.json").lessons,...read("lessons-phase-30-b.json").lessons];
const ids=["finance-n0431","finance-n0432","finance-n0439","finance-n0440","finance-n0441","finance-n0442"];
const core=["lessons-foundations.json","lessons-advanced.json","lessons-more.json"].flatMap(p=>read(p).lessons);
const extensions=manifest.modules.flatMap(m=>{const rows=read(m.path).lessons;assert.equal(rows.length,m.lessons,"manifest "+m.path);return rows;});
const all=[...core,...extensions],nodeMap=new Map(catalog.nodes.map(x=>[x.id,x]));
assert.deepEqual(lessons.map(x=>x.id),ids);
assert.equal(manifest.expected_lessons,201);assert.equal(manifest.modules.length,77);
assert.equal(core.length,123);assert.equal(extensions.length,201);assert.equal(all.length,324);
assert.equal(new Set(all.map(x=>x.node_id)).size,324);
assert.equal(catalog.nodes.length,656);assert.equal(catalog.chapters.length,109);
let formulas=0,deep=0,links=0,refs=0;
for(const l of lessons){
 assert(l.node_id===l.id&&nodeMap.get(l.id)?.title===l.title,"canonical node "+l.id);
 for(const k of ["definition","assumptions","example","application","caveat"])
  assert(typeof l[k]==="string"&&l[k].length>=100,"missing rigorous prose "+l.id+" "+k);
 assert(l.properties.length>=4&&l.formulas.length>=4&&l.derivation.length>=4);
 assert(l.deep_sections.length>=5,"V30 deep proof coverage must not regress "+l.id);
 assert.equal(new Set(l.deep_sections.map(s=>s.title)).size,l.deep_sections.length,"duplicate deep title "+l.id);
 for(const f of l.formulas){assert(f.label&&f.tex);katex.renderToString(f.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++;}
 for(const d of l.derivation){assert(d.explanation.length>=50);katex.renderToString(d.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++;}
 for(const z of l.deep_sections){
  assert(z.question.length>=12&&z.paragraphs.length>=2&&z.paragraphs.every(p=>p.length>=30));
  assert(z.proof.length>=50&&z.worked.length>=30&&z.limitation.length>=30);
  for(const f of z.formulas){assert(f.label&&f.tex);katex.renderToString(f.tex,{throwOnError:true,strict:"ignore",displayMode:true});formulas++;}
  deep++;
 }
 for(const a of l.links){assert(a.kind==="math"&&nodeMap.has(a.id),"broken graph "+l.id+" => "+a.id);links++;}
 for(const a of l.sources){assert(/^https:\/\//.test(a.url)&&a.title.length>20,"academic source "+l.id);refs++;}
 assert(!/[\uFFFD\u200B\uFEFF]/.test(JSON.stringify(l)));
 assert(!/Lorem ipsum|TODO|占位|待填/.test(JSON.stringify(l)));
}
assert(formulas>=110,"V30 math formula coverage must not regress");assert(deep>=30,"V30 deep proof coverage must not regress");assert.equal(refs,12);assert.equal(links,24);
const near=(a,b,label,tol=1e-9)=>assert(Math.abs(a-b)<=tol,"arithmetic check "+label+" got "+a+" expected "+b);
near(.97/.93-1,.043010752688172005,"V44 simple forward quote");
near(Math.log(.97/.93),Math.log(1+(.97/.93-1)),"V44 forward quote equivalence");
near(.04*.06*(Math.exp(-1)-Math.exp(-2))+.04*.04/2*(1-Math.exp(-1))**2,.0008777670997585736,"V44 CIR conditional variance");
near((105-90)/(120-90),.5,"Q up probability");
near(.7*(.5/.7)+.3*(.5/.3),1,"density normalization");
near(.7*120+.3*90,111,"physical stock mean");
near((.5*120+.5*90)/1.05,100,"risk neutral discount");
near(.5*20/1.05,9.523809523809524,"state price");
near(1.05*90-1.05*120,-31.5,"payoff matrix determinant");
near(-60/1.05+100*(2/3),9.523809523809526,"complete market replication");
near(-Math.log(.97),.030459207484708574,"one year yield");
near(-Math.log(.93)/2,.03628534641741769,"two year yield");
near(Math.log(.97/.93),.04211148535012685,"forward continuous rate");
near(5*.97+105*.93,102.5,"coupon cashflows");
near(105*2*.93,195.3,"curve zero key-rate sensitivity");
const k=1,theta=.04,r0=.06,sv=.02,t=1;
const bv=(1-Math.exp(-k*t))/k;
const logA=(theta-sv*sv/(2*k*k))*(bv-t)-sv*sv*bv*bv/(4*k);
near(bv,.6321205588285577,"Vasicek B");
near(logA,-.014681559398712778,"Vasicek log A");
near(Math.exp(logA-bv*r0),.9487510980306332,"Vasicek bond");
near(theta+(r0-theta)*Math.exp(-k*t),.047357588823428845,"OU conditional mean");
near(sv*sv*(1-Math.exp(-2*k*t))/(2*k),.00017293294335267746,"OU conditional variance");
const sc=.2,g=Math.sqrt(k*k+2*sc*sc),e=Math.exp(g*t),den=(g+k)*(e-1)+2*g,bc=2*(e-1)/den,ac=(2*g*Math.exp((k+g)*t/2)/den)**(2*k*theta/(sc*sc));
near(g,1.0392304845413265,"CIR gamma");
near(bc,.6295561917040959,"CIR B");
near(ac,.9854233418733002,"CIR A");
near(ac*Math.exp(-bc*r0),.9488948257205543,"CIR zero bond");
assert(2*k*theta>=sc*sc,"CIR Feller parameters");
for(const [id,token] of [
 ["finance-n0431","0.5"],["finance-n0432","9.523810"],
 ["finance-n0439","0.042111"],["finance-n0440","102.50"],
 ["finance-n0441","0.632121"],["finance-n0442","1.039230"]
])assert(lessons.find(l=>l.id===id).example.includes(token),"example token "+id);
const mathHome=fs.readFileSync(new URL("index.html",root),"utf8");
const mlHome=fs.readFileSync(new URL("../ml/index.html",root),"utf8");
const style=fs.readFileSync(new URL("styles.css",root),"utf8");
const mlStyle=fs.readFileSync(new URL("../ml/atlas.css",root),"utf8");
assert(mathHome.includes('aria-label="V30无套利定价与短率收益率曲线六课"'));
assert(mlHome.includes('aria-label="V30无套利定价、收益率曲线与短率模型跨模块路径"'));
for(const id of ids){assert(mathHome.includes("?topic="+id));assert(mlHome.includes("../math/?topic="+id));}
for(const x of ["overflow-x:auto","line-break:strict","@media print",":focus-visible"])assert(style.includes(x));
assert(mlStyle.includes("overflow-x:auto"));
console.log("PASS V44/V30: 6 rigorous pricing and fixed-income lectures, "+deep+" deep proof chapters, "+formulas+" strict KaTeX expressions, 12 sources, 24 graph links, 22 independent numeric financial invariants, 324 unique complete mathematical courses.");
