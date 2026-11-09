import fs from "node:fs";
import assert from "node:assert/strict";

const repo=new URL("../",import.meta.url);
const read=path=>fs.readFileSync(new URL(path,repo),"utf8");
const files=["quant/ml/index.html","quant/math/index.html","quant/index.html"];
const retired=[
 "quant/CURRICULUM_V2.md","quant/COMPANY_DATA_README.md",
 "quant/ml/atlas/README.md","quant/research/README.md",
 "quant/ml/legacy.html","quant/ml/legacy-before-atlas.html",
 "quant/ml/app.js","quant/ml/styles.css",
 "quant/ml/curriculum.json","quant/ml/manifest.json"
];
for(const name of retired)
 assert(!fs.existsSync(new URL(name,repo)),"Obsolete publication or maintenance file remains: "+name);
for(const path of files){
 const html=read(path),documentUrl=new URL(path,repo);
 const text=html.replace(/<!--[\s\S]*?-->/g,"")
  .replace(/<script\b[^>]*>[\s\S]*?<\/script>/gi,"")
  .replace(/<style\b[^>]*>[\s\S]*?<\/style>/gi,"")
  .replace(/<[^>]+>/g," ").replace(/\s+/g," ");
 assert(!/(?:本轮|本次新增|旧版归档|独立稿|待审校|待扩写|版本修订|质量审计)/.test(text),
  "Published page contains internal iteration notes: "+path);
 assert(!/\bV\d{1,3}\b/.test(text),"Published page shows historical version labels: "+path);
 for(const match of html.matchAll(/<(?:script|link)\b[^>]*?(?:src|href)="([^"]+)"/g)){
  const url=match[1].split("?")[0];
  if(!/^(?:\.\.?\/|[^/:#]+\.(?:js|css)$)/.test(url)||url.startsWith("https:"))continue;
  assert(fs.existsSync(new URL(url,documentUrl)),"Missing referenced runtime asset: "+path+" -> "+url);
 }
 for(const match of html.matchAll(/<a\b[^>]*href="([^"]+)"/g))
  assert(!/\.md(?:[?#]|$)/i.test(match[1]),"Published page exposes maintenance markdown: "+path);
}
const ml=read("quant/ml/index.html"),math=read("quant/math/index.html");
assert(ml.includes('class="atlas-study-library"')&&math.includes('class="math-study-library"'),
 "Topical study indexes must replace revision histories");
assert((ml.match(/<details class="atlas-spectral-path/g)||[]).length===42);
assert((math.match(/<details class="math-spectral-path/g)||[]).length===50);
assert(ml.indexOf('id="atlas-reader-section"')<ml.indexOf('aria-label="系统学习路径"'));
assert(math.indexOf('id="math-reader-section"')<math.indexOf('class="math-study-library"'));
assert(!fs.existsSync(new URL("quant/ml/test_examples.py",repo)));
const depthManifest=JSON.parse(read("quant/ml/atlas-depth-manifest.json"));
assert.equal(depthManifest.expected_lessons,720,"ML canonical depth must cover all lessons");
for(const module of depthManifest.modules){
 const name="quant/ml/"+module.path.replace(/^\.\//,"");
 const data=JSON.parse(read(name));
 assert.equal(Object.keys(data.lessons).length,module.lessons,"Incomplete ML lesson bundle: "+name);
 for(const [id,lesson] of Object.entries(data.lessons)){
  assert(!("revision_note" in lesson),"Internal editorial notes included in "+name+" / "+id);
  assert(!Object.keys(lesson).some(k=>/^v\d+_note$/.test(k)),"Historical release notes included in "+name+" / "+id);
 }
}
console.log("PASS: published routes are topical, maintenance records and legacy readers are absent, runtime assets resolve");
