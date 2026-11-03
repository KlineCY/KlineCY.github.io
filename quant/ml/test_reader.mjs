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
 byId:new Map(),searchText:new Map(),term:"",topic:"",chapterOpen:new Map(),learnedOnly:false,unreadOnly:false,
 enhanced:new Set(["m01-c01-p01","m01-c01-p02","m02-c01-p01","m02-c01-p02"]),
 read:new Set(["m01-c01-p01"]),activeId:"m01-c01-p01"
};
state.byId=new Map(state.index.map(x=>[x.id,x]));
for(const x of state.index)state.searchText.set(x.id,x.title.toLowerCase());
const location={hash:""};
const input={"atlas-learned-only":{checked:false},"atlas-unread-only":{checked:false}};
const messages=[];
let renders=0;
const reader=vm.runInNewContext(
 segment("function setProgressFilter(","function visibleLessons(){")+
 segment("function visibleLessons(){","function updateReadStats(){")+
 segment("function chapterShouldOpen(","function renderDirectory(){")+
 segment("function jump(id){","function display(){")+
 "\n({setProgressFilter,visibleLessons,chapterShouldOpen,continueUnread});",
 {state,location,$:id=>input[id],renderDirectory:()=>{renders++},showStatus:s=>messages.push(s)}
);
assert.equal(reader.visibleLessons().length,4);
const chapterKey="m01_第一章";
assert.equal(reader.chapterShouldOpen(chapterKey,true),true,"active chapter auto opens");
assert.equal(reader.chapterShouldOpen(chapterKey,false),false,"nonactive chapter defaults closed");
state.chapterOpen.set(chapterKey,true);
assert.equal(reader.chapterShouldOpen(chapterKey,false),true,"manual expansion survives navigation");
state.chapterOpen.set(chapterKey,false);
assert.equal(reader.chapterShouldOpen(chapterKey,true),false,"manual collapse survives directory re-render");
state.term="检索";
assert.equal(reader.chapterShouldOpen(chapterKey,false),true,"search can expand matching chapters");
state.term="";
reader.setProgressFilter("unread",true);
assert.deepEqual(Array.from(reader.visibleLessons(),x=>x.id),["m01-c01-p02","m02-c01-p01","m02-c01-p02"]);
reader.setProgressFilter("learned",true);
assert.deepEqual(Array.from(reader.visibleLessons(),x=>x.id),["m01-c01-p01"]);
assert.equal(input["atlas-unread-only"].checked,false,"learned and unread filters must be mutually exclusive");
assert.equal(input["atlas-learned-only"].checked,true);
state.read.add("m02-c01-p02");state.topic="m02";
assert.deepEqual(Array.from(reader.visibleLessons(),x=>x.id),["m02-c01-p02"],"learned and topic filters intersect");
reader.setProgressFilter("learned",false);
assert.deepEqual(Array.from(reader.visibleLessons(),x=>x.id),["m02-c01-p01","m02-c01-p02"]);
state.topic="";state.term="时间";
assert.deepEqual(Array.from(reader.visibleLessons(),x=>x.id),["m02-c01-p01"],"search filters");
state.term="";reader.continueUnread();
assert.equal(location.hash,"lesson/m01-c01-p02","resume next unread");
state.activeId="m02-c01-p02";reader.continueUnread();
assert.equal(location.hash,"lesson/m01-c01-p02","resume wraps");
state.read=new Set(state.index.map(x=>x.id));location.hash="";
reader.continueUnread();
assert.equal(location.hash,"","all complete must not navigate");
assert(messages.at(-1).includes("全部 4 篇"));
assert(renders>=3);
assert.throws(()=>reader.setProgressFilter("bad",true),/未知学习进度筛选/);
console.log("PASS: learned/unread filters mutually exclusive, topic/search, resume and completion");
