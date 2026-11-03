import assert from "node:assert/strict";
import fs from "node:fs";
import vm from "node:vm";

const atlas=fs.readFileSync(new URL("./atlas.js",import.meta.url),"utf8");
const begin=atlas.indexOf("async function fetchAtlasJson(");
const end=atlas.indexOf("async function start(){",begin);
assert(begin>=0&&end>begin,"bounded loader functions must exist");
const loader=atlas.slice(begin,end);
let active=0,maxActive=0;
const attempts=new Map();
const fetch=async(url,options={})=>{
 if(url.startsWith("./chapter-"))assert.equal(options.cache,"no-cache","module request must revalidate cached content");
 active++;maxActive=Math.max(maxActive,active);
 const trial=(attempts.get(url)||0)+1;
 attempts.set(url,trial);
 await Promise.resolve();
 active--;
 if(url==="./notfound")return {ok:false,status:404};
 if(url==="./always500")return {ok:false,status:503};
 if(url==="./rate-limit"&&trial<=2)return {ok:false,status:429};
 if(url==="./transient"&&trial===1)return {ok:false,status:503};
 return {ok:true,status:200,json:async()=>({url,trial})};
};
const {fetchAtlasJson,fetchAtlasModulesBounded}=vm.runInNewContext(
 loader+"\n({fetchAtlasJson,fetchAtlasModulesBounded});",
 {fetch,setTimeout:(callback)=>callback(),console}
);
const urls=Array.from({length:20},(_,i)=>"./chapter-"+i);
const progress=[];
const data=await fetchAtlasModulesBounded(urls,6,(done,total)=>progress.push([done,total]));
assert.equal(data.length,20);
assert.equal(progress.length,20,"progress must report each completed module");
assert(progress.every(([done,total],i)=>done===i+1&&total===20),"progress counters must be complete and monotonic");
for(let i=0;i<20;i++)assert.equal(data[i].url,urls[i],"worker results must preserve manifest order");
assert(maxActive<=6,"must never exceed six in-flight module fetches");
assert.equal(attempts.size,20);
assert.deepEqual(await fetchAtlasJson("./transient"),{url:"./transient",trial:2});
assert.equal(attempts.get("./transient"),2);
assert.deepEqual(await fetchAtlasJson("./rate-limit"),{url:"./rate-limit",trial:3});
assert.equal(attempts.get("./rate-limit"),3);
await assert.rejects(fetchAtlasJson("./notfound"),/HTTP 404/);
assert.equal(attempts.get("./notfound"),1,"permanent 404 must fail without retries");
await assert.rejects(fetchAtlasJson("./always500"),/已尝试3次/);
assert.equal(attempts.get("./always500"),3,"transient failures must have finite retries");
console.log("PASS: bounded atlas transfer, ordered JSON assembly, retry 503/429, fast 404 and limited failure attempts");
