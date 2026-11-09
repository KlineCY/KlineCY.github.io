import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const dir = new URL("./", import.meta.url);
const read = path => JSON.parse(fs.readFileSync(new URL(path,dir),"utf8"));
const data = [read("lessons-foundations.json"),read("lessons-advanced.json"),read("lessons-more.json")];
const curriculum = read("curriculum.json");
const nodeIds = new Set(curriculum.nodes.map(n=>n.id));
const articles = new Map(data.flatMap(bundle=>bundle.lessons).map(item=>[item.node_id,item]));
const expected = ["probability-n0119","statistics-n0169","econometrics-n0235","stochastic-n0293"];
const unique = new Set();
let formulas=0,sections=0,sources=0;
for(const id of expected){
  const item = articles.get(id);
  assert(item && nodeIds.has(id), "missing canonical lesson "+id);
  assert(!unique.has(id),"duplicated lesson ID "+id);unique.add(id);
  assert(item.definition.length>=65 && item.assumptions.length>=75 && item.example.length>=70 && item.caveat.length>=70,"shallow article "+id);
  assert.equal(item.deep_sections?.length,id==="econometrics-n0235"?8:4,id+" requires its complete original V8 sections and any V19 extension");
  assert.equal(new Set(item.deep_sections.map(s=>s.title)).size,item.deep_sections.length,"duplicate headings "+id);
  assert(item.sources?.length>=2 && item.links?.length>=4,"reference graph incomplete "+id);
  for(const link of item.links){
    assert(["math","ml"].includes(link.kind) && link.label && link.id,"bad prerequisite "+id);
    if(link.kind==="math")assert(nodeIds.has(link.id),"broken mathematics link "+link.id);
    else assert(/^m\d\d-c\d\d-p\d\d$/.test(link.id),"invalid ML id "+link.id);
  }
  for(const source of item.sources){assert(source.title&&/^https:\/\//.test(source.url));sources++;}
  for(const section of item.deep_sections.slice(0,4)){
    assert(section.title.length>=8 && section.question.length>=12,"weak argument organization "+id);
    assert(section.paragraphs.length===2 && section.paragraphs.every(p=>p.length>=50),"insufficient explanation "+id);
    assert(section.proof.length>=60 && section.worked.length>=58 && section.limitation.length>=40,"proof, independent example or failure case missing "+id);
    assert(section.formulas.length>=1,"missing formula "+id);
    for(const {label,tex} of section.formulas){
      assert(label && tex,"missing labeled equation "+id);
      katex.renderToString(tex,{throwOnError:true,strict:"ignore"});
      formulas++;
    }
    sections++;
  }
}
assert.equal(sections,16);
assert.equal(formulas,16);
const approx=(a,b,eps=1e-10)=>assert(Math.abs(a-b)<eps, "numeric invariant: "+a+" != "+b);
approx(Math.sqrt(.4*.6/100),Math.sqrt(.0024));
approx(.75/(1-.5*.5),1);
approx(1+2*.5/(1-.5),3);
approx(1+2*(2/3*.4+1/3*.2),5/3);
approx(Math.sqrt((5/3)/100),Math.sqrt(1/60));
approx((.01+.03+.02+.04)/4,.025);
const x=[.01,.03,.02,.04];
const m=x.reduce((s,v)=>s+v)/4;
const ss=x.reduce((s,v)=>s+(v-m)**2,0);
approx(ss,.0005);
approx(ss/3,1/6000);
approx(Math.sqrt(ss/(3*4)),.006454972243679028);
approx(.0001+2*.5*.00004,.00014);
approx(Math.sqrt(.00014/100),.0011832159566199232);
approx(.5*1+.25*0+.25*(-2),0);
const means=[];
for(let i=1;i<=4;i++)for(let j=1;j<=4;j++)for(let k=1;k<=4;k++)for(let l=1;l<=4;l++)means.push((i+j+k+l)/4);
means.sort((a,b)=>a-b);
assert.equal(means.length,256);
assert.equal(means[Math.ceil(.025*means.length)-1],1.5);
assert.equal(means[Math.ceil(.975*means.length)-1],3.5);
const html=fs.readFileSync(new URL("index.html",dir),"utf8");
for(const id of expected)assert(html.includes("?topic="+id),"math lecture missing from path "+id);
assert(html.includes("从中心极限定理到金融显著性：长期方差、HAC、Bootstrap与Fama–MacBeth"));
assert(html.includes("从条件期望到可选停止：滤过、停时、鞅与交易边界"));
const css=fs.readFileSync(new URL("styles.css",dir),"utf8");
assert(css.includes("overscroll-behavior-x:contain"));
assert(css.includes("break-inside:auto!important"));
assert(css.includes(".math-deep-unit .formula-block:focus-visible") ||
       css.includes(".formula-block:focus-visible"),"keyboard focus missing");
const machine=fs.readFileSync(new URL("../ml/index.html",dir),"utf8");
assert(machine.includes("预测与策略显著性的数学证据链：CLT、HAC、Bootstrap与Fama–MacBeth"));
for(const id of expected)assert(machine.includes("../math/?topic="+id),"missing ML-to-math cross-link "+id);
console.log("PASS: V8 4 upgraded canonical articles, 16 deep reasoning sections, 16 parseable KaTeX equations, "+sources+" academic sources and reproducible financial/statistical calculations.");

