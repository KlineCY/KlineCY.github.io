import assert from "node:assert/strict";
import fs from "node:fs";
import vm from "node:vm";

const source=fs.readFileSync(new URL("./atlas.js",import.meta.url),"utf8");
function segment(start,end){
 const a=source.indexOf(start),b=source.indexOf(end,a);
 assert(a>=0&&b>a,"reader function must exist: "+start);
 return source.slice(a,b);
}
const state={
 index:[
  {id:"m01-c01-p01",domain_id:"m01",title:"线性回归"},
  {id:"m01-c01-p02",domain_id:"m01",title:"岭回归"},
  {id:"m02-c01-p01",domain_id:"m02",title:"时间序列"},
  {id:"m02-c01-p02",domain_id:"m02",title:"滚动验证"}
 ],
 byId:new Map(),searchText:new Map(),term:"",topic:"",chapterOpen:new Map(),domainOpen:new Map(),
 activeId:"m01-c01-p01"
};
state.byId=new Map(state.index.map(x=>[x.id,x]));
for(const x of state.index)state.searchText.set(x.id,(x.id+" "+x.title).toLocaleLowerCase());
const location={hash:""};
const reader=vm.runInNewContext(
 segment("function visibleLessons(){","function chapterShouldOpen(")+
 segment("function chapterShouldOpen(","function setCatalogOpen(")+
 segment("function jump(id){","function display(){")+
 "\n({visibleLessons,chapterShouldOpen,domainShouldOpen,jump});",
 {state,location}
);
const ids=()=>Array.from(reader.visibleLessons(),x=>x.id);
assert.deepEqual(ids(),["m01-c01-p01","m01-c01-p02","m02-c01-p01","m02-c01-p02"]);
state.topic="m02";
assert.deepEqual(ids(),["m02-c01-p01","m02-c01-p02"],"topic filter must restrict lessons");
state.term="时间";
assert.deepEqual(ids(),["m02-c01-p01"],"title search and topic filter must intersect");
state.topic="";state.term="m01-c01-p02";
assert.deepEqual(ids(),["m01-c01-p02"],"stable lesson IDs must be searchable");
state.term="";state.topic="";
const chapterKey="m01_第一章";
assert.equal(reader.chapterShouldOpen(chapterKey,true),true,"active chapter opens");
assert.equal(reader.chapterShouldOpen(chapterKey,false),false,"inactive chapter defaults closed");
state.chapterOpen.set(chapterKey,true);
assert.equal(reader.chapterShouldOpen(chapterKey,false),true,"manual chapter expansion persists");
state.chapterOpen.set(chapterKey,false);
assert.equal(reader.chapterShouldOpen(chapterKey,true),false,"manual collapse persists");
assert.equal(reader.domainShouldOpen("m01",true),true,"active domain opens");
state.domainOpen.set("m01",false);
assert.equal(reader.domainShouldOpen("m01",true),false,"manual domain collapse persists");
state.term="回归";
assert.equal(reader.chapterShouldOpen(chapterKey,false),true,"search reveals chapter");
assert.equal(reader.domainShouldOpen("m01",false),true,"search reveals domain");
state.term="";
reader.jump("m02-c01-p02");
assert.equal(location.hash,"lesson/m02-c01-p02","canonical deep links stay usable");
reader.jump("missing-course");
assert.equal(location.hash,"lesson/m02-c01-p02","invalid course IDs must not navigate");
console.log("PASS: canonical topic/search filtering, collapsible hierarchy and deep links without learning progress");
