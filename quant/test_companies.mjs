import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
const here=new URL("./",import.meta.url);
const read=path=>JSON.parse(fs.readFileSync(new URL(path,here),"utf8"));
const core=read("companies.json");
const research=read("company-research.json");
const html=fs.readFileSync(new URL("index.html",here),"utf8");
assert.equal(core.schema_version,1);
assert.equal(research.schema_version,1);
assert(core.companies.length>=156,"historical company records removed");
assert.equal(new Set(core.companies.map(c=>c.id)).size,core.companies.length);
assert(research.profiles.length>=25,"company research coverage shrank");
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
   assert(a.context.length>=45&&a.question.length>=15&&a.verification.length>=25&&a.methods.length>=3,"thin methods "+p.company_id+" "+a.title);
   axes++;
 }
 if(p.careers_url)assert(/^https:\/\//.test(p.careers_url));
}
assert(axes>=75&&facts>=38,"research coverage below required quality");
for(const term of ['id="profileCount"','id="profileFilter"','id="currencyFilter"','company-research.json','function researchHTML','function compareWithinCurrency','data-explanation']){
 assert(html.includes(term),"missing company atlas UI "+term);
}
assert(!/学习进度|标记已学|已学\s*\/\s*总/.test(html),"unrequested learning tracker");
assert(html.includes("跨货币数值不进行排名"),"AUM currency boundary missing");
const inline=html.match(/<script>\s*([\s\S]*?)<\/script>/)?.[1];
assert(inline&&inline.length>3000,"company reader JS missing");
new vm.Script(inline,{filename:"company-atlas-inline.js"});
const nodes=new Map();
function el(id){
 if(!nodes.has(id))nodes.set(id,{
  value:"",textContent:"",innerHTML:"",hidden:false,disabled:false,style:{},dataset:{},
  addEventListener(){},onclick:null,appendChild(){},setAttribute(){},removeAttribute(){},getAttribute(){return "false"},
  lastElementChild:{textContent:""},classList:{toggle(){},add(){},remove(){}},showModal(){},close(){}
 });
 return nodes.get(id);
}
const dom={
 getElementById:el,querySelectorAll(){return []},
 createElement(){return {value:"",textContent:"",dataset:{}}}
};
const ctx=vm.createContext({document:dom,location:{search:""},URLSearchParams,fetch(){return new Promise(()=>{})},console});
vm.runInContext(inline,ctx,{timeout:2500});
const evalJS=code=>vm.runInContext(code,ctx,{timeout:2500});
assert.equal(evalJS("qualified({scale:{currency:'USD',metric:'集团AUM',amount_billion:0.02}})"),true);
assert.equal(evalJS("qualified({scale:{currency:'USD',metric:'集团AUM',amount_billion:0.005}})"),false);
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
console.log("PASS: "+core.companies.length+" intact companies, "+research.profiles.length+" grounded profiles, "+axes+" study axes, "+facts+" sourced facts; currency-safe sorting, AUM classification, DOM escaping, no learning trackers");
