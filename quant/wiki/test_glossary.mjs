import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
const read = p => JSON.parse(fs.readFileSync(new URL(p, import.meta.url),"utf8"));
const manifest=read("./manifest.json");
const volumes=manifest.data_files.map(p=>read("./"+p));
const categories=volumes.flatMap(v=>v.categories);
const terms=volumes.flatMap(v=>v.terms);
const ids=new Set();
const names=new Set();
const catIds=new Set(categories.map(c=>c.id));
for(const t of terms){
  assert(!ids.has(t.id),"Duplicate ID "+t.id);
  ids.add(t.id);
  assert(!names.has(t.title),"Duplicate Chinese title "+t.title);
  names.add(t.title);
  assert(catIds.has(t.category_id),"Missing category "+t.id);
  for(const key of ["title","english","definition","example","pitfall","level","reference_url"]){
    assert(typeof t[key]==="string"&&t[key].trim(),t.id+": missing "+key);
  }
  assert(["初级","中级","高级"].includes(t.level),t.id+": invalid level");
  assert(t.reference_url.startsWith("https://"),t.id+": invalid source URL");
}
assert.equal(manifest.total_terms,terms.length,"Manifest count out of date");
for(const c of categories){
  assert(c.term_ids.length>0,c.id+": empty category");
  for(const id of c.term_ids){
    assert(ids.has(id),"Unknown term "+id);
    assert(terms.find(t=>t.id===id)?.category_id===c.id,"Cross-category mismatch "+id);
  }
}
for(const t of terms)for(const id of t.related_ids){
  assert(ids.has(id),"Unknown related term "+id);
  assert.notEqual(id,t.id,"Self-reference "+t.id);
}
for(const path of manifest.learning_paths){
  assert(path.term_ids.length>=5,path.id+": incomplete path");
  for(const id of path.term_ids)assert(ids.has(id),"Unknown learning step "+id);
}
const app=fs.readFileSync(new URL("./app.js",import.meta.url),"utf8");
new vm.Script(app,{filename:"app.js"});
const sidebar=fs.readFileSync(new URL("../sidebar-v2.js",import.meta.url),"utf8");
new vm.Script(sidebar,{filename:"sidebar-v2.js"});
assert(sidebar.includes('["wiki","量化百科 Wiki"]'),"Wiki missing from global navigation");
console.log("Wiki validation passed: "+terms.length+" terms, "+categories.length+" categories, "+manifest.learning_paths.length+" learning paths.");
