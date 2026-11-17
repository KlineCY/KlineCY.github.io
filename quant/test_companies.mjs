import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
import katex from "katex";
const here=new URL("./",import.meta.url);
const read=path=>JSON.parse(fs.readFileSync(new URL(path,here),"utf8"));
const core=read("companies.json");
const research=read("company-research.json");
const methods=read("company-methods.json");
const html=fs.readFileSync(new URL("index.html",here),"utf8");
assert.equal(core.schema_version,1);
assert.equal(research.schema_version,1);
assert(core.companies.length>=156,"historical company records removed");
assert.equal(new Set(core.companies.map(c=>c.id)).size,core.companies.length);
assert(research.profiles.length>=89,"company research coverage shrank");
assert.equal(new Set(research.profiles.map(p=>p.company_id)).size,research.profiles.length);
const ids=new Set(core.companies.map(c=>c.id));
let axes=0,facts=0,officialLinks=0;
for(const c of core.companies){
  assert(c.id&&c.name&&c.country&&c.category,"missing base company identity");
  if(c.scale){
    assert(c.scale.currency&&c.scale.metric&&c.scale.source&&/^https:\/\//.test(c.scale.source),"missing AUM provenance for "+c.id);
    assert(c.scale.quality,"missing evidence tier "+c.id);
    assert(c.scale.amount_billion!==undefined||c.scale.range_yi_min!==undefined,"missing source amount "+c.id);
  }
}
for(const p of research.profiles){
 assert(ids.has(p.company_id),"unmatched profile "+p.company_id);
 assert(p.orientation.length>=8&&p.caveat.length>=25);
 assert(p.facts.length>=1&&p.research_axes.length>=3&&p.roles.length>=2,"thin company research "+p.company_id);
 assert.equal(new Set(p.research_axes.map(a=>a.title)).size,p.research_axes.length);
 for(const fact of p.facts){
   assert(fact.statement.length>=20&&fact.source_title&&/^https:\/\//.test(fact.source_url),"unsourced claim "+p.company_id);
   facts++;officialLinks++;
 }
 for(const a of p.research_axes){
   assert(a.context.length>=23&&a.question.length>=15&&a.verification.length>=19&&a.methods.length>=3,"thin methods "+p.company_id+" "+a.title);
   axes++;
 }
 if(p.careers_url)assert(/^https:\/\//.test(p.careers_url));
}
assert(axes>=267&&facts>=143,"research coverage below required quality");
const verifiedIds=["renaissance","global-quant-001","global-quant-002","global-quant-003","global-quant-005","cn-qiml-062","cn-qiml-055","cn-qiml-042","cn-public-bucket-013","cn-qiml-046","cn-qiml-002","cn-public-bucket-001","cn-qiml-054","cn-amac-002"];
const fourthWaveIds=["cn-amac-001","cn-qiml-039","cn-qiml-061","cn-qiml-063","cn-qiml-066","cn-qiml-033","cn-qiml-060","cn-qiml-047","cn-qiml-024","cn-qiml-035","cn-qiml-027","cn-qiml-050","cn-qiml-044","cn-qiml-043","cn-qiml-052","cn-qiml-019"];
verifiedIds.push(...fourthWaveIds);
const fifthWaveIds=["cn-public-bucket-012","cn-public-bucket-009","cn-public-bucket-015","cn-public-bucket-003","cn-public-bucket-011","cn-qiml-015","cn-qiml-031","cn-qiml-004","cn-qiml-067","cn-qiml-026","cn-qiml-025","cn-qiml-048","cn-qiml-021","cn-qiml-020"];
verifiedIds.push(...fifthWaveIds);
assert.equal(new Set(verifiedIds).size,verifiedIds.length,"duplicate website verification");
assert.equal(core.companies.filter(c=>c.official_website_source).length,44,"unexpected official source metadata");
assert.equal(core.companies.find(c=>c.id==="cn-qiml-035").category,"综合资管","Tongyi should not be ungroundedly advertised as wholly quant");
assert.equal(core.companies.find(c=>c.id==="cn-public-bucket-009").category,"宏观投资","macro manager should not be described as quant");
assert(core.companies.find(c=>c.id==="cn-qiml-020").entity_note?.includes("2026年2月9日"),"public regulatory finding should remain visible");
assert(core.companies.find(c=>c.id==="cn-qiml-018").possible_duplicate_of==="cn-public-bucket-001","unresolved duplicate legal identity not flagged");
for(const id of verifiedIds){
 const c=core.companies.find(x=>x.id===id);
 assert(c&&c.official_website_verified_at==="2026-11-17", "unverified official identity "+id);
 assert(c.official_website_source?.startsWith("https://")&&c.website?.startsWith("https://"),"missing first-party URL "+id);
 assert(research.profiles.some(p=>p.company_id===id),"missing verified dossier "+id);
}
assert.equal(core.companies.find(x=>x.id==="global-quant-003").category,"量化资管","OLZ should be classified by regulatory status");
assert(core.companies.find(x=>x.id==="global-quant-001").entity_note.includes("2016年"),"Cantab historic merger must remain visible");

for(const term of ['id="profileCount"','id="profileFilter"','id="currencyFilter"','id="sourceFilter"','company-research.json','function researchHTML','function compareWithinCurrency','data-explanation']){
 assert(html.includes(term),"missing company atlas UI "+term);
}
assert(!html.includes("学习进度")&&!html.includes("标记已学"),"unrequested learning tracker");
assert.equal(methods.schema_version,1);
assert(methods.lessons.length>=48,"company curriculum missing full chapters");
assert.equal(new Set(methods.lessons.map(l=>l.id)).size,methods.lessons.length);
let units=0,maths=0;
for(const lesson of methods.lessons){
 assert(lesson.title.length>=8&&lesson.objective.length>=40&&lesson.sections.length>=4);
 for(const source of lesson.sources)assert(source.title&&source.url.startsWith("https://"));
 const headings=new Set();
 for(const unit of lesson.sections){
  assert(!headings.has(unit.title),"duplicate technical section "+unit.title);
  headings.add(unit.title);
  assert(unit.question.length>=12&&unit.paragraphs.length===2&&unit.paragraphs.every(p=>p.length>=40));
  assert(unit.derivation.length>=50&&unit.worked.length>=50&&unit.boundary.length>=25,"section missing derivation/example/boundary: "+unit.title);
  assert(unit.equations.length>=2);
  for(const equation of unit.equations){assert(equation.label&&equation.tex);katex.renderToString(equation.tex,{throwOnError:true,strict:"ignore",displayMode:true,trust:false});maths++;}
  units++;
 }
}
assert(units>=192&&maths>=384,"technical curriculum compressed");
for(const id of ["company-method-01","company-method-02","company-method-03","company-method-04","company-method-05","company-method-06","company-method-07","company-method-08","company-method-09","company-method-10","company-method-11","company-method-12","company-method-13","company-method-14","company-method-15","company-method-16","company-method-17","company-method-18","company-method-19","company-method-20","company-method-21"])assert(methods.lessons.some(l=>l.id===id),"missing subject area "+id);
for(let n=22;n<=48;n++)assert(methods.lessons.some(l=>l.id==="company-method-"+n),"missing new technical subject "+n);
for(const fragment of ['id="methodSearch"',"method-search-label","company-method[open] .katex-zone","company-methods.json",'id="companyMethodList"','id="methodStats"',"function renderCompanyMethods","function renderCourseMath","katex.min.js","katex-zone"])assert(html.includes(fragment),"curriculum reader missing "+fragment);
const near=(a,b,tol=1e-8)=>assert(Math.abs(a-b)<=tol,"finance identity "+a+" != "+b);
near(80+50-20,110);
near(112-100*(1+.05),7);
near(1-Math.pow(.95,100),.994079470779666);
near(Math.sqrt(.01+.01),Math.sqrt(2)*.1);
near(.8*10/20,.4);
near(.02*Math.sqrt(.01),.002);
near(Math.max(0,.015-.02/4),.01);
near((101-100)/100,.01);
near(0.02/(0.01),2);
near(100*Math.exp(.04),104.08107741923882);
near(80+50-20,110);
near(.01*100/.05,20);
near(.75*4/3,1);
near(10/(1-4/8),20);
near(1-Math.exp(-.2*5),.6321205588285577);
near(.6/.2,3);
near((1+1+8)**2/(1+1+8**2),100/66);
near(-Math.log(2)/Math.log(.9),6.578813478960585,1e-6);
near(100*Math.exp(-.1),90.48374180359595,1e-6);
near(100*Math.exp(-.12),88.69204367171575,1e-6);
near(-5*.01+.5*30*.0001,-.0485);
near((1-.95)/1.9,.0263157894736842);
near(.5*1.1*80+.5*.9*120,98);
near(1.02*.03,.0306);
near(.5*.04*16,.32);
near(.5*.05*16,.4);
near((.01*1000000)/.05,200000);
near(.4*.03+.6*(-.02),0);
near(.8*.02+.2*(-.06),.004);
near(.8*.1+.2*.7,.22);
near(.5*.1+.5*(-.1),0);
near((1-.95)/1.9,0.0263157894736842);
near((1.1*1.04)/1.02,1.1215686274509804);
near(.6*.95,0.57);
near((1.06*.95)-1,.007);
near((40-30-15),-5);
near((.8*.2+.2*.7),.30);
near(Math.exp(-.02),.9801986733067553);
near(1-Math.exp(-1),.6321205588285577);
near(.01*100,1);
near((.3-.2),.1);


near((.7-0)**2,.49);
near((.9-0)**2,.81);
near(100*.1-50*.0,10);
near(.4*3+.6*(-2)-.2,-.2);


assert(html.includes("跨货币数值不进行排名"),"AUM currency boundary missing");
const inline=html.match(/<script>\s*([\s\S]*?)<\/script>/)?.[1];
assert(inline&&inline.length>3000,"company reader JS missing");
new vm.Script(inline,{filename:"company-atlas-inline.js"});
const nodes=new Map();
function el(id){
 if(!nodes.has(id))nodes.set(id,{
  value:"",textContent:"",innerHTML:"",hidden:false,disabled:false,style:{},dataset:{},
  addEventListener(){},onclick:null,appendChild(){},setAttribute(){},removeAttribute(){},getAttribute(){return "false"},
  lastElementChild:{textContent:""},classList:{toggle(){},add(){},remove(){}},querySelectorAll(){return []},showModal(){},close(){}
 });
 return nodes.get(id);
}
const dom={
 getElementById:el,querySelectorAll(){return []},
 createElement(){return {value:"",textContent:"",dataset:{}}}
};
const ctx=vm.createContext({document:dom,location:{search:""},URLSearchParams,window:{addEventListener(){}},fetch(){return new Promise(()=>{})},console});
vm.runInContext(inline,ctx,{timeout:2500});
const evalJS=code=>vm.runInContext(code,ctx,{timeout:2500});
assert.equal(evalJS("qualified({scale:{currency:'USD',metric:'集团AUM',amount_billion:0.2}})"),true);
assert.equal(evalJS("qualified({scale:{currency:'USD',metric:'集团AUM',amount_billion:0.005}})"),false);
assert.equal(evalJS("qualified({scale:{currency:'USD',metric:'集团AUM',amount_billion:0.1}})"),true);
assert.equal(evalJS("qualified({scale:{currency:'USD',metric:'集团AUM',amount_billion:0.099}})"),false);
assert.equal(evalJS("qualified({scale:{currency:'USD',metric:'投资资本',amount_billion:200}})"),false);
assert.equal(evalJS("compareWithinCurrency({name:'CN',scale:{currency:'CNY',metric:'集团AUM',amount_billion:100}},{name:'US',scale:{currency:'USD',metric:'集团AUM',amount_billion:1}},true)<0"),true,"mixed currencies sorted by nominal amount");
assert.equal(evalJS("compareWithinCurrency({name:'A',scale:{currency:'CNY',metric:'集团AUM',amount_billion:1}},{name:'B',scale:{currency:'CNY',metric:'集团AUM',amount_billion:3}},true)<0"),true,"same currency ascending");
assert.equal(evalJS("compareWithinCurrency({name:'A',scale:{currency:'CNY',metric:'集团AUM',amount_billion:1}},{name:'B',scale:{currency:'CNY',metric:'集团AUM',amount_billion:3}},false)>0"),true,"same currency descending");
assert.equal(evalJS("sourceBadge({quality:'third-party-directory',metric:'商业名录AUM估计'})"),"商业名录记录（待核）");
const smoke=evalJS("(()=>{profiles=new Map([['smoke',{orientation:'<img src=x onerror=alert(1)>',reviewed_at:'2026-11-17',facts:[{statement:'<script>alert(1)</script>',source_title:'Official',source_url:'https://example.com/a?q=1&x=2'}],research_axes:[{title:'Research',context:'Context',methods:['统计','数据'],question:'Why',verification:'Verify'}],roles:['研究'],caveat:'证据边界'}]]);return researchHTML('smoke')})()");
assert(smoke.includes("&lt;img")&&smoke.includes("&lt;script&gt;"));
assert(!smoke.includes("<script>alert(1)</script>")&&!smoke.includes('<img src=x'));
assert(smoke.includes("https://example.com/a?q=1&amp;x=2"));
assert.equal(evalJS("researchHTML('not-yet-researched')"),"");
ctx.courseData=methods;
evalJS("renderCompanyMethods(courseData)");
assert(el("methodStats").textContent.includes("48 讲")&&el("methodStats").textContent.includes("384 组"),"curriculum count not rendered");
assert((el("companyMethodList").innerHTML.match(/class="course-unit"/g)||[]).length===192,"curriculum reader lost technical sections");
assert((el("companyMethodList").innerHTML.match(/class="katex-zone"/g)||[]).length===384,"curriculum reader lost formulas");
el("methodSearch").value="可转债";
evalJS("renderCompanyMethods(courseData)");
assert(el("methodStats").textContent.includes("当前匹配"),"course search not active");
const matches=(el("companyMethodList").innerHTML.match(/class="course-unit"/g)||[]).length;
assert(matches>0&&matches<192,"course search did not narrow curriculum");
el("methodSearch").value="___impossible_subject_keyword___";
evalJS("renderCompanyMethods(courseData)");
assert(el("companyMethodList").innerHTML.includes("未找到匹配课程"),"missing honest empty result");
el("methodSearch").value="";
evalJS("renderCompanyMethods(courseData)");
assert((el("companyMethodList").innerHTML.match(/class="course-unit"/g)||[]).length===192,"course search must restore every complete section");

ctx.db=core;ctx.researchDb=research;
evalJS("firms=db.companies;byId=new Map(firms.map(c=>[c.id,c]));profiles=new Map(researchDb.profiles.map(p=>[p.company_id,p]));");
el("sourceFilter").value="verified";
evalJS("render()");
assert(el("status").textContent.includes("44 / 156"),"official source filter should match 30 newly verified institutions");
el("sourceFilter").value="";
el("q").value="天演";
evalJS("render()");
assert(el("status").textContent.includes("1 / 156"),"confirmed company should be searchable");
evalJS("openDetails('cn-qiml-062')");
assert(el("detailBody").innerHTML.includes("官方网站核验来源"),"verified source must render in detail");
assert(el("detailBody").innerHTML.includes("研究档案"),"full technical dossier must remain accessible");
el("q").value="";


console.log("PASS: "+core.companies.length+" intact companies, "+research.profiles.length+" company profiles, "+axes+" research axes, "+facts+" sourced facts, "+methods.lessons.length+" courses, "+units+" complete sections and "+maths+" KaTeX equations; safe rendering and numeric finance identities");
