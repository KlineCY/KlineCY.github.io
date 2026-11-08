import fs from "node:fs";
import assert from "node:assert/strict";
const root=new URL("./",import.meta.url),json=f=>JSON.parse(fs.readFileSync(new URL(f,root),"utf8"));
const tree=json("curriculum.json");
const files=fs.readdirSync(root).filter(f=>/^lessons-.*\.json$/.test(f));
const written=files.flatMap(f=>{const d=json(f);return Array.isArray(d.lessons)?d.lessons:[]});
const registered=new Set(tree.nodes.map(n=>n.id));
assert.equal(registered.size,tree.nodes.length,"Duplicated syllabus IDs");
assert.equal(new Set(written.map(l=>l.node_id)).size,written.length,"Duplicated authored lessons");
assert(written.every(l=>registered.has(l.node_id)),"Authored lesson absent from syllabus");
const authored=new Set(written.map(l=>l.node_id));
const remaining=tree.nodes.filter(n=>!authored.has(n.id));
console.log("V108 COVERAGE REPORT: "+written.length+" completed / "+tree.nodes.length+" total; "+remaining.length+" missing");
for(const d of tree.disciplines){
 const group=tree.nodes.filter(n=>n.discipline_id===d.id);
 const missing=group.filter(n=>!authored.has(n.id));
 console.log(d.id+": "+(group.length-missing.length)+"/"+group.length+" | remaining: "+
  JSON.stringify(missing.map(n=>({id:n.id,title:n.title}))));
}
assert.equal(tree.nodes.length,656);
assert(written.length>=586,"V107 previously completed lessons disappeared");
