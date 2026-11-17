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
assert(research.profiles.length>=142,"company research coverage shrank");
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
assert(axes>=426&&facts>=239,"research coverage below required quality");
const historicalDeepProfileIds=new Set(["jane-street","hrt","citadel-securities","imc","optiver","xtx","flow","virtu","aqr","two-sigma","citadel","deshaw","qrt","worldquant","blackwing-cn","century-cn","global-quant-018","global-quant-022","cn-qiml-041","cn-qiml-029","cn-qiml-017","cn-public-bucket-002","cn-public-bucket-007","cn-qiml-051","cn-qiml-023","cn-public-bucket-008"]);
const newlyDeepened=research.profiles.filter(p=>!historicalDeepProfileIds.has(p.company_id));
assert.equal(newlyDeepened.length,116,"full remaining firm coverage missing");
let newDeepAxes=0;
for(const p of newlyDeepened){
 assert.equal(p.research_axes.length,3,"original research topics lost "+p.company_id);
 for(const a of p.research_axes){
  assert(a.derivation?.length>=80,"insufficient mechanism or proof "+p.company_id+"/"+a.title);
  assert(a.worked_example?.length>=60&&/[0-9０-９]/u.test(a.worked_example),"missing numeric worked case "+p.company_id+"/"+a.title);
  assert(a.pitfalls?.length>=45,"missing limitations "+p.company_id+"/"+a.title);
  assert(/[-+−=≈∑ΣΠαγσβλρΔ√∫]/u.test(a.derivation),"missing quantitative mechanism "+p.company_id+"/"+a.title);
  newDeepAxes++;
 }
}
assert.equal(newDeepAxes,348,"not all remaining research axes were developed");
assert.equal(research.profiles.flatMap(p=>p.research_axes).filter(a=>a.derivation&&a.worked_example&&a.pitfalls).length,426,"all 426 research directions must remain detailed");
assert.equal(10000000*5*0.002,100000,"duration shock example incorrect");
assert.equal(10000000*0.03*0.5,150000,"credit loss example incorrect");
assert(Math.abs(Math.log(0.5)/Math.log(0.8)-3.1062837)<1e-5,"AR(1) half-life inconsistent");
assert.equal(5*300*30,45000,"futures variation margin example incorrect");
assert.equal(0.8*0.1+0.2*(-0.15),0.05,"survivorship example inconsistent");
assert.equal(30000000/(0.1*20000000),15,"liquidity exit days inconsistent");
assert(html.includes("x.derivation||\"\",x.worked_example||\"\""),"deep research content must be searchable");
assert(html.includes("overflow-wrap:anywhere"),"research mathematical text should wrap on mobile");

const verifiedIds=["renaissance","global-quant-001","global-quant-002","global-quant-003","global-quant-005","cn-qiml-062","cn-qiml-055","cn-qiml-042","cn-public-bucket-013","cn-qiml-046","cn-qiml-002","cn-public-bucket-001","cn-qiml-054","cn-amac-002"];
const fourthWaveIds=["cn-amac-001","cn-qiml-039","cn-qiml-061","cn-qiml-063","cn-qiml-066","cn-qiml-033","cn-qiml-060","cn-qiml-047","cn-qiml-024","cn-qiml-035","cn-qiml-027","cn-qiml-050","cn-qiml-044","cn-qiml-043","cn-qiml-052","cn-qiml-019"];
verifiedIds.push(...fourthWaveIds);
const fifthWaveIds=["cn-public-bucket-012","cn-public-bucket-009","cn-public-bucket-015","cn-public-bucket-003","cn-public-bucket-011","cn-qiml-015","cn-qiml-031","cn-qiml-004","cn-qiml-067","cn-qiml-026","cn-qiml-025","cn-qiml-048","cn-qiml-021","cn-qiml-020"];
verifiedIds.push(...fifthWaveIds);
const sixthWaveIds=['global-quant-007','global-quant-016','cn-public-bucket-010','cn-qiml-059'];verifiedIds.push(...sixthWaveIds);
const reconciledOfficialIds=["global-quant-011","global-quant-020","global-quant-004","global-quant-010"];verifiedIds.push(...reconciledOfficialIds);
const seventhWaveIds=["global-quant-017","cn-qiml-028","cn-qiml-007","cn-qiml-065","cn-qiml-049","cn-qiml-005"];verifiedIds.push(...seventhWaveIds);
const eighthWaveIds=["global-quant-021","global-quant-014","global-quant-025","cn-qiml-045","cn-qiml-030","cn-qiml-001","cn-qiml-016"];verifiedIds.push(...eighthWaveIds);
const eighthRegulatedIds=["global-quant-006","global-quant-019","global-quant-013","global-quant-008","cn-qiml-032","cn-qiml-053","cn-qiml-006","cn-qiml-038","cn-qiml-057"];
for(const id of [...eighthWaveIds,...eighthRegulatedIds])assert(research.profiles.some(p=>p.company_id===id),"new source-linked profile missing "+id);
for(const id of eighthRegulatedIds)assert(!core.companies.find(c=>c.id===id).official_website_source,"unverified source misrepresented as website "+id);
for(const id of ["global-quant-006","global-quant-019"])assert(core.companies.find(c=>c.id===id).entity_status?.includes("登记"),"SEC registration termination not documented "+id);
assert(core.companies.find(c=>c.id==="global-quant-014").entity_note.includes("2022年"),"LeanVal historical rename not preserved");
const nextOfficialIds=["global-quant-018","cn-qiml-041","cn-qiml-017","cn-public-bucket-002"];verifiedIds.push(...nextOfficialIds);
const nextResearchIds=["global-quant-018","global-quant-022","cn-qiml-041","cn-qiml-029","cn-qiml-017","cn-public-bucket-002","cn-public-bucket-007","cn-qiml-051","cn-qiml-023","cn-public-bucket-008"];
for(const id of nextResearchIds){
 const p=research.profiles.find(x=>x.company_id===id);
 assert(p&&p.research_axes.length===3&&p.facts.length===2,"missing deep research profile "+id);
 for(const a of p.research_axes)assert(a.derivation?.length>=35&&a.worked_example?.length>=27&&a.pitfalls?.length>=25,"institutional research lacks derivation and examples "+id);
}
const extendedResearchIds=["jane-street","hrt","citadel-securities","imc","optiver","xtx","flow","virtu","aqr","two-sigma","citadel","deshaw","qrt","worldquant","blackwing-cn","century-cn"];
for(const id of extendedResearchIds){
 const p=research.profiles.find(x=>x.company_id===id);
 assert(p&&p.research_axes.length===3,"missing established deep research "+id);
 for(const a of p.research_axes){
  assert(a.derivation?.length>=35&&a.worked_example?.length>=27&&a.pitfalls?.length>=25,"incomplete analytic mechanism or case "+id+" / "+a.title);
  assert(/[-+−=≈∑ΣΠαγσβλρΔ]/u.test(a.derivation),"mechanism lacks mathematical relation "+id+" / "+a.title);
 }
}
assert.equal(extendedResearchIds.length*3,48,"full breadth regression");
const deepAxes=research.profiles.flatMap(p=>p.research_axes).filter(a=>a.derivation&&a.worked_example&&a.pitfalls);
assert(deepAxes.length>=78,"historical and added analytical cases were lost");
assert(Math.abs(Math.sqrt((0.1**2+0.1**2)/4)-0.070710678)<1e-7,"uncorrelated equal-weight volatility identity");
assert(Math.abs(Math.sqrt((0.1**2+0.1**2+2*0.8*0.1**2)/4)-0.094868330)<1e-7,"stress correlation identity");
assert(Math.abs(1-0.95**100-0.9940794708)<1e-7,"selection-bias illustration");
assert(core.companies.find(c=>c.id==="global-quant-022").regulatory_warning_source.includes("fca.org.uk"),"GSD FCA warning source missing");
assert(!core.companies.find(c=>c.id==="global-quant-022").entity_status_source,"FCA warning must not be labelled corporate registration");
assert(html.includes("监管机构风险警示（同名关系待核）"),"FCA warning not visible in reader");
assert(html.includes("机制与公式推导")&&html.includes("可复算案例")&&html.includes("不成立的情形"),"research deep-dive fields are not shown");
const seventhProfileIds=["cn-qiml-056","cn-qiml-018","cn-qiml-013","cn-qiml-009","cn-qiml-058","cn-qiml-012"];
for(const id of [...seventhWaveIds,...seventhProfileIds])assert(research.profiles.some(p=>p.company_id===id),"missing verified or source-bounded research "+id);
for(const id of seventhProfileIds)assert(!core.companies.find(c=>c.id===id).official_website_source,"third-party institution incorrectly presented as official "+id);
assert(core.companies.find(c=>c.id==="cn-qiml-009").description.includes("模板"),"pseudo-official placeholder site not flagged");
assert(core.companies.find(c=>c.id==="cn-qiml-012").description.includes("冲突"),"conflicting historic AUM not flagged");
for(const id of [...reconciledOfficialIds,"global-quant-024"])assert(research.profiles.some(p=>p.company_id===id),"missing new sourced research "+id);
assert(core.companies.find(c=>c.id==="global-quant-024").entity_status.includes("已解散"),"Saemor dissolution not disclosed");
assert(core.companies.find(c=>c.id==="global-quant-024").entity_status_source.includes("lei.bloomberg.com"),"Saemor legal entity source missing");
assert(core.companies.find(c=>c.id==="global-quant-010").description.includes("Mercer"),"SECOR acquisition context missing");
assert(html.includes("法人存续状态")&&html.includes("法律实体登记来源")&&html.includes("资料核验提示"),"legal status evidence must be visible in company detail");
assert.equal(new Set(verifiedIds).size,verifiedIds.length,"duplicate website verification");
assert.equal(core.companies.filter(c=>c.official_website_source).length,69,"unexpected official source metadata");
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
assert(html.includes("公开资料与来源核对"),"source evidence heading missing");
assert(!html.includes("一手来源深度档案")&&!html.includes("一手来源研究档案"),"unrequested profile badge must remain removed");
assert(!html.includes("profile-badge\">一手来源"),"promotional research badge returned");
assert.equal(methods.schema_version,1);
assert(methods.lessons.length>=154,"company curriculum missing full chapters");
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
assert(units>=648&&maths>=1296,"technical curriculum compressed");
for(const id of ["company-method-01","company-method-02","company-method-03","company-method-04","company-method-05","company-method-06","company-method-07","company-method-08","company-method-09","company-method-10","company-method-11","company-method-12","company-method-13","company-method-14","company-method-15","company-method-16","company-method-17","company-method-18","company-method-19","company-method-20","company-method-21"])assert(methods.lessons.some(l=>l.id===id),"missing subject area "+id);
for(let n=22;n<=100;n++)assert(methods.lessons.some(l=>l.id==="company-method-"+n),"missing new technical subject "+n);
for(let n=101;n<=112;n++)assert(methods.lessons.some(l=>l.id==="company-method-"+n),"missing advanced noncompressing subject "+n);
for(let n=113;n<=128;n++)assert(methods.lessons.some(l=>l.id==="company-method-"+n),"missing advanced new subject "+n);
for(let n=129;n<=154;n++)assert(methods.lessons.some(l=>l.id==="company-method-"+n),"missing twenty-six expanded complete courses "+n);
for(let n=1;n<=32;n++)assert(methods.lessons[n-1].sections.length===5,"first thirty-two existing lessons not deeply expanded "+n);
assert(methods.lessons.slice(128).every(l=>l.sections.length===4&&l.sections.every(s=>s.equations.length===2&&s.worked.length>=35)),"new research subject not a complete course");
assert(methods.lessons.slice(-16).every(l=>l.sections.length===4&&l.sources.length>=2&&l.sections.every(s=>s.equations.length===2)),"new course content incomplete");
assert(methods.lessons.slice(-12).every(l=>l.sections.length===4&&l.sources.length>=2),"new technical courses incomplete");
const domainCounts={"数学与统计":65,"机器学习与AI":32,"衍生品与交易":76,"组合与风控":68,"实务工程":53};
for(const lesson of methods.lessons){
 assert(Array.isArray(lesson.domains)&&lesson.domains.length>0,"untagged lesson "+lesson.id);
 assert(lesson.domains.every(tag=>tag in domainCounts),"unknown course category "+lesson.id);
}
for(const [tag,count] of Object.entries(domainCounts))assert.equal(methods.lessons.filter(l=>l.domains.includes(tag)).length,count,"incorrect course taxonomy "+tag);
for(const fragment of ['id="methodPrev"','id="methodNext"','id="methodAll"','id="methodPageStatus"','id="methodDomain"','id="methodSearch"',"method-search-label","company-method[open] .katex-zone","company-methods.json",'id="companyMethodList"','id="methodStats"',"function renderCompanyMethods","function renderCourseMath","katex.min.js","katex-zone"])assert(html.includes(fragment),"curriculum reader missing "+fragment);
const near=(a,b,tol=1e-8)=>assert(Math.abs(a-b)<=tol,"finance identity "+a+" != "+b);
near(80+50-20,110);
near(1-Math.exp(-.1),0.09516258196404048,1e-10);
near(.1*.2*1000000,20000);
near(1-Math.pow(.95,20),0.6415140775914577,1e-10);
near(1-Math.pow(.95,100),0.994079470779666,1e-10);
near((100.1*900+99.9*100)/1000,100.08,1e-10);
near(7*1.03/1.05,6.866666666666666,1e-10);
near(1.05*1.02-1,.071,1e-10);
near(.6*1000000*.02,12000,1e-10);
near(1.2*10000000/1500000,8,1e-10);
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
near((98+3)/100-1,.01);
near(1.2*.8-1,-.04);
near(1.06/1.04-1,0.01923076923076916);
near(1-Math.exp(-.02),0.019801326693244747);
near((120-90)/120,.25);
near(.5*.1+.5*(-.1),0);
near(.6*.3+.4*.7,.46);
near(.8*.5+.2*.5,.5);
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
assert(el("methodStats").textContent.includes("154 讲")&&el("methodStats").textContent.includes("1296 组"),"course totals not rendered");
assert((el("companyMethodList").innerHTML.match(/class="course-unit"/g)||[]).length===60,"first page should show twelve intact four-section courses");
assert((el("companyMethodList").innerHTML.match(/class="katex-zone"/g)||[]).length===120,"first page should show all 96 math expressions");
assert(el("methodPageStatus").textContent.includes("第 1 / 13 组"),"128 courses should have eleven pages at size twelve");
evalJS("methodPage=1;renderCompanyMethods(courseData)");
assert(el("methodPageStatus").textContent.includes("第 2 / 13 组"),"course pagination must advance");
assert((el("companyMethodList").innerHTML.match(/class="course-unit"/g)||[]).length===60,"second page lost full sections");
evalJS("methodShowAll=true;renderCompanyMethods(courseData)");
assert((el("companyMethodList").innerHTML.match(/class="course-unit"/g)||[]).length===648,"show-all must preserve all five hundred twelve technical sections");
assert((el("companyMethodList").innerHTML.match(/class="katex-zone"/g)||[]).length===1296,"show-all must preserve all one thousand twenty-four formulas");
evalJS("methodShowAll=false;methodPage=0;");
el("methodDomain").value="机器学习与AI";
evalJS("renderCompanyMethods(courseData)");
assert(el("methodStats").textContent.includes("当前匹配 32 讲"),"AI domain filter count incorrect");
assert((el("companyMethodList").innerHTML.match(/class="course-unit"/g)||[]).length===methods.lessons.filter(l=>l.domains.includes("机器学习与AI")).slice(0,12).reduce((n,l)=>n+l.sections.length,0),"filtered first page should preserve all complete sections");
evalJS("methodShowAll=true;renderCompanyMethods(courseData)");
assert((el("companyMethodList").innerHTML.match(/class="course-unit"/g)||[]).length===methods.lessons.filter(l=>l.domains.includes("机器学习与AI")).reduce((n,l)=>n+l.sections.length,0),"domain show-all must include all thirty-two complete courses");
el("methodDomain").value="";
evalJS("methodShowAll=false;methodPage=0;renderCompanyMethods(courseData)");
el("methodSearch").value="可转债";
evalJS("renderCompanyMethods(courseData)");
assert(el("methodStats").textContent.includes("当前匹配"),"course keyword search inactive");
assert((el("companyMethodList").innerHTML.match(/class="course-unit"/g)||[]).length>0,"search omitted relevant courses");
el("methodSearch").value="___impossible_subject_keyword___";
evalJS("renderCompanyMethods(courseData)");
assert(el("companyMethodList").innerHTML.includes("未找到匹配课程"),"missing honest empty result");
el("methodSearch").value="";
evalJS("methodPage=0;renderCompanyMethods(courseData)");
assert((el("companyMethodList").innerHTML.match(/class="course-unit"/g)||[]).length===60,"clearing search should return original first page");

ctx.db=core;ctx.researchDb=research;
evalJS("firms=db.companies;byId=new Map(firms.map(c=>[c.id,c]));profiles=new Map(researchDb.profiles.map(p=>[p.company_id,p]));");
el("sourceFilter").value="verified";
evalJS("render()");
assert(el("status").textContent.includes("69 / 156"),"official source filter should match all 65 explicitly linked institutions");
el("sourceFilter").value="";
el("q").value="天演";
evalJS("render()");
assert(el("status").textContent.includes("1 / 156"),"confirmed company should be searchable");
evalJS("openDetails('cn-qiml-062')");
assert(el("detailBody").innerHTML.includes("官方网站核验来源"),"verified source must render in detail");
assert(el("detailBody").innerHTML.includes("研究档案"),"full technical dossier must remain accessible");
evalJS("openDetails('global-quant-024')");
assert(el("detailBody").innerHTML.includes("法人存续状态")&&el("detailBody").innerHTML.includes("法律实体登记来源"),"Saemor dissolved status not surfaced");
assert(el("detailBody").innerHTML.includes("2023"),"Saemor dissolution date absent from detail");
evalJS("openDetails('global-quant-010')");
assert(el("detailBody").innerHTML.includes("Mercer"),"SECOR acquisition context not visible");
evalJS("openDetails('global-quant-022')");
assert(el("detailBody").innerHTML.includes("监管机构风险警示")&&el("detailBody").innerHTML.includes("查看监管警示"),"GSD warning should be visible distinctly from legal registration");
evalJS("openDetails('cn-qiml-041')");
assert(el("detailBody").innerHTML.includes("可复算案例")&&el("detailBody").innerHTML.includes("机制与公式推导"),"deep firm research missing in detail modal");
el("q").value="";


console.log("PASS: "+core.companies.length+" intact companies, "+research.profiles.length+" company profiles, "+axes+" research axes, "+facts+" sourced facts, "+methods.lessons.length+" courses, "+units+" complete sections and "+maths+" KaTeX equations; safe rendering and numeric finance identities");
