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
 byId:new Map(),searchText:new Map(),term:"",topic:"",depthOnly:false,unreadOnly:false,
 enhanced:new Set(["m01-c01-p01","m01-c01-p02","m02-c01-p01","m02-c01-p02"]),worked:new Set(["m01-c01-p02","m02-c01-p02"]),read:new Set(["m01-c01-p01"]),activeId:"m01-c01-p01"
};
state.byId=new Map(state.index.map(x=>[x.id,x]));
for(const x of state.index)state.searchText.set(x.id,x.title.toLowerCase());
const location={hash:""};
const messages=[];
const reader=vm.runInNewContext(
 segment("function visibleLessons(){","function updateReadStats(){")+
 segment("function jump(id){","function display(){")+
 "\n({visibleLessons,continueUnread});",
 {state,location,showStatus:s=>messages.push(s)}
);
assert.equal(reader.visibleLessons().length,4,"default must show all lessons");
state.unreadOnly=true;
assert.deepEqual(Array.from(reader.visibleLessons(),x=>x.id),["m01-c01-p02","m02-c01-p01","m02-c01-p02"]);
state.depthOnly=true;
assert.deepEqual(Array.from(reader.visibleLessons(),x=>x.id),["m01-c01-p02","m02-c01-p02"],"unread and worked-example filters intersect");
state.topic="m01";
assert.deepEqual(Array.from(reader.visibleLessons(),x=>x.id),["m01-c01-p02"],"topic and progress filters intersect");
state.topic="";state.depthOnly=false;state.unreadOnly=false;
state.term="时间";
assert.deepEqual(Array.from(reader.visibleLessons(),x=>x.id),["m02-c01-p01"],"full-text term still filters lessons");
state.term="";
reader.continueUnread();
assert.equal(location.hash,"lesson/m01-c01-p02","resume finds next unread after current");
state.activeId="m02-c01-p02";
reader.continueUnread();
assert.equal(location.hash,"lesson/m01-c01-p02","resume wraps to beginning");
state.read=new Set(state.index.map(x=>x.id));
location.hash="";
reader.continueUnread();
assert.equal(location.hash,"","all-complete progress must not navigate");
assert(messages.at(-1).includes("全部 4 篇"),"all-complete status should be explanatory");
console.log("PASS: unread and worked-example filters, topic/search intersection, next-unread wrapping and complete state");
