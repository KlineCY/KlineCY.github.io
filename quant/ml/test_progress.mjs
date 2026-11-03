import assert from "node:assert/strict";
import fs from "node:fs";
import vm from "node:vm";

const src=fs.readFileSync(new URL("./atlas.js",import.meta.url),"utf8");
const from=src.indexOf("function serializeReadProgress("),to=src.indexOf("function showStatus(",from);
assert(from>=0&&to>from,"portable progress helpers must be present");
const {serializeReadProgress,parseReadProgress}=vm.runInNewContext(
 src.slice(from,to)+"\n({serializeReadProgress,parseReadProgress});",{}
);
const known=new Set(["m01-c01-p01","m01-c01-p02","m02-c01-p01"]);
const serialized=serializeReadProgress(["m02-c01-p01","m01-c01-p01","m02-c01-p01","unknown"],known);
const decoded=JSON.parse(serialized);
assert.equal(decoded.format,"kline-quant-ml-progress");
assert.equal(decoded.version,1);
assert.deepEqual(decoded.read_ids,["m01-c01-p01","m02-c01-p01"],"export removes duplicates and unknowns");
assert.deepEqual([...parseReadProgress(serialized,known)],decoded.read_ids,"progress roundtrip");
const invalid=[
 "{}",
 "{broken json",
 JSON.stringify({format:"kline-quant-ml-progress",version:2,read_ids:[]}),
 JSON.stringify({format:"kline-quant-ml-progress",version:1,read_ids:["unknown"]}),
 JSON.stringify({format:"kline-quant-ml-progress",version:1,read_ids:["m01-c01-p01","m01-c01-p01"]}),
 JSON.stringify({format:"kline-quant-ml-progress",version:1,read_ids:["__proto__"]}),
 " ".repeat(65537)
];
for(const source of invalid)assert.throws(()=>parseReadProgress(source,known),"invalid progress must be rejected");
const merged=new Set(["m01-c01-p02"]);
for(const id of parseReadProgress(serialized,known))merged.add(id);
assert.deepEqual([...merged].sort(),[...known].sort(),"import must merge without deleting existing records");
console.log("PASS: portable progress export, strict import and non-destructive merge");
