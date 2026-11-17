import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
import katex from "katex";
import {gunzipSync} from "node:zlib";
const dir=new URL("./",import.meta.url);
const read=p=>JSON.parse(fs.readFileSync(new URL(p,dir),"utf8"));
const core=JSON.parse(gunzipSync(Buffer.concat([1,2,3,4].map(i=>fs.readFileSync(new URL("atlas/data.part"+i,dir))))).toString("utf8"));
const mathIds=new Set(read("../math/curriculum.json").nodes.map(x=>x.id));
const oldNames=["atlas-new-lessons.json","atlas-volatility-chapter.json","atlas-linear-regularization-chapter.json","atlas-evaluation-evidence-chapter.json","atlas-generalization-v2.json","atlas-kernel-covariance-v2.json","atlas-stability-kernel-v3.json","atlas-robust-validation-v3.json"];
const prior=oldNames.map(read),priorIds=new Set(prior.flatMap(d=>d.chapters.flatMap(c=>c.lessons.map(l=>l.id))));
const names=["atlas-quantile-calibration-v4.json","atlas-tailrisk-models-v4.json"],docs=names.map(read);
const rows=docs.flatMap(d=>d.chapters.flatMap(c=>c.lessons));
const expected=["m04-c09-p01","m12-c09-p01","m12-c09-p02","m22-c07-p01","m23-c09-p01","m23-c09-p02"];
assert.deepEqual(rows.map(x=>x.id),expected,"ordered V4 curriculum IDs");
assert.equal(new Set(expected).size,6);
for(const d of docs)assert(d.schema===1&&d.chapters.length>=2);
let units=0,tex=0,sources=0;
for(const l of rows){
 assert(!priorIds.has(l.id)&&!core.catalog.items.some(x=>x.id===l.id),"duplicate ML lecture ID "+l.id);
 assert(l.goal.length>=70&&l.protocol.length>=4&&l.sources.length>=2);
 assert.equal(l.sections.length,["m22-c07-p01","m23-c09-p01"].includes(l.id)?6:l.id==="m12-c09-p02"?9:["m04-c09-p01","m12-c09-p01"].includes(l.id)?8:7);
 assert.equal(new Set(l.sections.map(s=>s.title)).size,l.sections.length);
 for(const s of l.sections){
  assert(s.question?.length>=12&&s.paragraphs.length===2&&s.paragraphs.every(x=>x.length>=55),"incomplete Chinese reasoning "+l.id+" "+s.title);
  assert(s.worked.length>=40&&s.boundary.length>=35,"missing worked quantitative example or caveat "+l.id+" "+s.title);
  assert(s.equations.length>=1);
  for(const f of s.equations){assert(f.label&&f.tex);katex.renderToString(f.tex,{throwOnError:true,strict:"ignore"});tex++;}
  units++;
 }
 for(const link of l.prerequisites){
  assert(link.label);
  if(link.kind==="math")assert(mathIds.has(link.id),"broken mathematical link "+link.id);
  else{assert(core.catalog.items.some(x=>x.id===link.id)||priorIds.has(link.id)||expected.includes(link.id),"missing ML prerequisite "+link.id);if(expected.includes(link.id))assert(expected.indexOf(link.id)<expected.indexOf(l.id),"forward prerequisite "+l.id+" → "+link.id);}
 }
 for(const s of l.sources){assert(s.title&&/^https:\/\//.test(s.url));sources++;}
}
assert(!/\uFFFD/.test(JSON.stringify(rows)),"replacement character in Chinese or mathematical text");
const scripts=["atlas.js","new-topics.js"].map(x=>fs.readFileSync(new URL(x,dir),"utf8"));
for(const s of scripts)new vm.Script(s);
for(const f of names)assert(scripts[0].includes('"./'+f+'"'),"runtime V4 data missing: "+f);
const ctx={window:{},encodeURIComponent};
vm.runInNewContext(scripts[1],ctx);
for(const d of [...prior,...docs])ctx.window.quantMlExpansion.install(core,d);
assert.equal(core.catalog.items.length,770,"full 770-lesson runtime count");
assert.equal(new Set(core.catalog.items.map(x=>x.id)).size,770,"duplicate runtime lesson ID");
for(const id of expected){
 const lesson=core.domains[id.slice(0,3)][id];
 assert(lesson?.html?.includes("atlas-concept-toc"),"not rendered to article "+id);
 assert.equal((lesson.html.match(/class="atlas-concept-unit"/g)||[]).length,rows.find(l=>l.id===id).sections.length,"depth lost in reader "+id);
 assert(core.catalog.items.some(x=>x.id===id));
}
for(const l of rows.filter(l=>["m04-c09-p01","m12-c09-p01","m12-c09-p02"].includes(l.id)))for(const s of l.sections.slice(5)){
 assert(s.paragraphs.length===2&&s.paragraphs.every(p=>p.length>=65)&&s.equations.length===2&&s.worked.length>=75&&s.boundary.length>=55,"advanced quantile material incomplete "+l.id);
 for(const eq of s.equations)katex.renderToString(eq.tex,{throwOnError:true,strict:"error"});
}
const tailAdvancedTopics={
 "m22-c07-p01":["单位方差Student-t创新的VaR与ES尺度不能混淆"],
 "m23-c09-p01":["历史模拟组合VaR必须重放联合市场情景"],
 "m23-c09-p02":["ES尾部均值的超额损失识别矩与原子处理","滚动VaR与ES回测的模型选择偏差和有效样本规模"]
};
for(const [id,titles] of Object.entries(tailAdvancedTopics)){
 const l=rows.find(x=>x.id===id);
 for(const title of titles){
  const sec=l.sections.find(s=>s.title===title);
  assert(sec&&sec.paragraphs.length===2&&sec.paragraphs.every(p=>p.length>=85),"missing complete tail risk theory "+title);
  assert(sec.equations.length===2&&sec.worked.length>=125&&sec.boundary.length>=75,"new tail backtest validation incomplete "+title);
  for(const e of sec.equations)katex.renderToString(e.tex,{throwOnError:true,strict:"error"});
 }
}
const addedQuantileCoverage={
 "m04-c09-p01":["经验Pinball目标的次梯度条件与整段最优分位集合"],
 "m12-c09-p01":["Conformal校准分数严格单调变换下的秩不变性"],
 "m12-c09-p02":["零次VaR超限不等于已证明覆盖：二项分布上置信界","总体超限率正确而风险状态内失效的VaR构造"]
};
for(const [id,titles] of Object.entries(addedQuantileCoverage)){
 const l=rows.find(r=>r.id===id);
 for(const title of titles){
  const sec=l.sections.find(s=>s.title===title);
  assert(sec&&sec.paragraphs.length===2&&sec.paragraphs.every(p=>p.length>=90),"missing full quantile validity reasoning "+id);
  assert(sec.equations.length===2&&sec.worked.length>=105&&sec.boundary.length>=75,"missing finite-sample quantile counterexample "+id);
  for(const e of sec.equations)katex.renderToString(e.tex,{throwOnError:true,strict:"error"});
 }
}
const close=(a,b,t=1e-9)=>assert(Math.abs(a-b)<t,"arithmetic invariant "+a+" ≠ "+b);
close(6.8/5,1.36);
close(Math.ceil((9+1)*.8),8);
close(.8,.8);
close(.95*.02,.019);
close(1.96*.02,.0392);
close(1.645*.02,.0329);
close(.25*(.01**2+.02**2)+2*.5*.5*.25*.01*.02,.00015);
close(Math.sqrt(.00015),.01224744871391589);
close(Math.ceil(5*.8),4);
close(Math.sqrt(1000*.05*.95),Math.sqrt(47.5));
close((1+1+1+1-4)/5,0);
close(1-.95**20,.6415140775914581);
close(.75*.25*.4+.25*.75*.6,.1875);
close((.02+.01)/2,.015);
close(Math.ceil((9+1)*.9),9);
close(Math.ceil((4+1)*.8),4);
close(2-.3,1.7);
close(5/95,.05263157894736842);
close(4/9,.4444444444444444);
close(4/5,.8);
close(Math.sqrt((4-2)/4),Math.SQRT1_2);
close((2+(10-2)/.2/5),10);
close((10+2)/2,6);
close(1-.05, .95);
close(Math.sqrt(.05*.95/100),.02179449471770337);
close(10+0,10);
close((2/3)*10/3,20/9);
close(2*(1/3)*10/3,20/9);
close(Math.pow(3,2),9);
close(Math.pow(.99,100),0.3660323412732292);
close(1-Math.pow(.05,1/100),0.02951304960703993);
close(.5*.2+.5*0,.1);
const html=fs.readFileSync(new URL("index.html",dir),"utf8"),css=fs.readFileSync(new URL("atlas.css",dir),"utf8");
for(const id of expected)assert(html.includes("#lesson/"+id),"new learning path missing "+id);
assert(html.includes("atlas.js?v=20261109"));
assert(html.includes("atlas.css?v=20261109"));
assert(html.includes("第四条高级学习链"));
assert(css.includes("overscroll-behavior-x:contain")&&css.includes("@media print"),"math layout regression");
console.log("PASS: six V4 distributional forecast and risk lectures, "+units+" full subchapters, "+tex+" KaTeX expressions, "+sources+" research references, 770 distinct navigable lessons and numeric/CJK layout checks.");
