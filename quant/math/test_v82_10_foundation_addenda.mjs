import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";
const root=new URL("./",import.meta.url);
const specs=[
  {
    "path": "lessons-phase-01-a.json",
    "lessons": [
      {
        "id": "foundation-n0013",
        "hash": "f2cf8f8c",
        "oldSections": 4,
        "newTitle": "逐点收敛与一致收敛的量词区别"
      },
      {
        "id": "calculus-n0040",
        "hash": "543a45d3",
        "oldSections": 4,
        "newTitle": "逆函数定理与局部参数识别"
      },
      {
        "id": "linear-n0065",
        "hash": "31fc9fd6",
        "oldSections": 4,
        "newTitle": "投影矩阵与可识别拟合值的区别"
      }
    ]
  },
  {
    "path": "lessons-phase-01-b.json",
    "lessons": [
      {
        "id": "probability-n0095",
        "hash": "9fd4358f",
        "oldSections": 4,
        "newTitle": "先验分布漂移下的概率重加权与后验修正"
      },
      {
        "id": "measure-n0121",
        "hash": "efdfc491",
        "oldSections": 4,
        "newTitle": "停止时与可预测持仓的过滤适应性"
      },
      {
        "id": "statistics-n0162",
        "hash": "661bfd98",
        "oldSections": 4,
        "newTitle": "时间相关数据的Delta方法与长程方差"
      }
    ]
  },
  {
    "path": "lessons-phase-01-c.json",
    "lessons": [
      {
        "id": "regression-n0188",
        "hash": "f7217334",
        "oldSections": 4,
        "newTitle": "HC3高杠杆修正与White检验的职责分离"
      },
      {
        "id": "econometrics-n0212",
        "hash": "09ef858b",
        "oldSections": 4,
        "newTitle": "碰撞变量选择导致的虚假相关反例"
      },
      {
        "id": "timeseries-n0241",
        "hash": "56ba018f",
        "oldSections": 4,
        "newTitle": "ARCH的二阶平稳并不保证四阶矩存在"
      },
      {
        "id": "stochastic-n0279",
        "hash": "8690a0bd",
        "oldSections": 4,
        "newTitle": "非齐次Markov链的时间有序矩阵乘积"
      }
    ]
  }
];
const hash=o=>{let h=2166136261>>>0;for(const c of JSON.stringify(o))h=Math.imul(h^c.charCodeAt(0),16777619)>>>0;return h.toString(16).padStart(8,"0")};
const all=[];
let strict=0;
for(const group of specs){
  const d=JSON.parse(fs.readFileSync(new URL(group.path,root),"utf8"));
  assert.equal(d.version,1);
  assert.deepEqual(d.lessons.map(x=>x.id),group.lessons.map(x=>x.id));
  for(let i=0;i<group.lessons.length;i++){
    const x=d.lessons[i],expected=group.lessons[i];
    assert.equal(hash({...x,deep_sections:x.deep_sections.slice(0,expected.oldSections)}),expected.hash,x.id+" changed base content");
    assert.equal(x.deep_sections.length,expected.oldSections+1);
    const s=x.deep_sections.at(-1);
    assert.equal(s.title,expected.newTitle);
    assert.equal(s.paragraphs.length,2);
    assert(s.paragraphs.every(v=>v.length>=80),x.id+" short Chinese explanation");
    assert(s.proof.length>=100&&s.worked.length>=65&&s.limitation.length>=55,x.id+" thin proof/example/limits");
    assert.equal(s.formulas.length,2);
    assert.equal(new Set(x.deep_sections.map(y=>y.title)).size,x.deep_sections.length,x.id+" duplicate section");
    for(const f of s.formulas){
      assert(f.label.length>=4&&f.tex.length>=15,x.id+" incomplete formula");
      katex.renderToString(f.tex,{strict:"error",throwOnError:true,displayMode:true});
      strict++;
    }
    assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(JSON.stringify(x)),x.id+" damaged Chinese");
    assert(!/TODO|FIXME|Lorem ipsum/iu.test(JSON.stringify(s)),x.id+" placeholder");
    all.push(x.id);
  }
}
assert.equal(all.length,10);
assert.equal(new Set(all).size,10);
assert.equal(strict,20);
let numeric=0;
function near(name,actual,expected,tol=1e-9){assert(Number.isFinite(actual)&&Math.abs(actual-expected)<=tol,name+": "+actual+" != "+expected);numeric++}
near("pointwise convergence",0.5**10,0.0009765625);
near("not uniform at x=.99",0.99**10,0.9043820750088044);
near("inverse Jacobian tiny det",1*1.01-1*1,0.01);
near("Jacobian solution theta1",-1+1,0);
near("rank deficient projection",1/(1+4),0.2);
near("rank deficient projection y2",2/5,0.4);
near("prior shift marginal signal",.9*.1+.1*.9,.18);
near("prior shift posterior",.9*.1/.18,.5);
near("original prior posterior",.9*.02/(.9*.02+.1*.98),.018/.116);
near("AR1 long run variance",1+2*.5/(1-.5),3);
near("dependent delta standard error",Math.sqrt(3/100),0.17320508075688773);
near("HC3 weight residual",2**2/(1-.5)**2,16);
near("HC0 weight residual",2**2,4);
near("collider selected covariance",.5-.5*.5,.25);
near("collider selected regression coefficient",.25/.25,1);
near("ARCH 2nd moment",1/(1-.6),2.5);
near("ARCH 4th condition fails",3*.6**2,1.08);
near("nonhomogeneous first state two steps",.9*.6+.1*.3,.57);
near("nonhomogeneous second state two steps",.9*.4+.1*.7,.43);
near("homogeneous false two-step transition",.9*.9+.1*.2,.83);
const html=fs.readFileSync(new URL("index.html",root),"utf8");
const css=fs.readFileSync(new URL("styles.css",root),"utf8");
for(const id of all)assert(html.includes("?topic="+id),id+" missing from official math navigation");
for(const marker of ["overflow-x:auto","line-break:strict",":focus-visible","@media print","@media(max-width:700px)"])assert(css.includes(marker),"responsive math/CJK style missing "+marker);
console.log("V82 PASS: "+all.length+" preserved math lessons, "+strict+" strict KaTeX formulas, "+numeric+" independent numerical validations, CJK and navigation.");
