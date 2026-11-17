import fs from "node:fs";
import assert from "node:assert/strict";

const repo=new URL("../",import.meta.url);
const read=path=>fs.readFileSync(new URL(path,repo),"utf8");
const files=["quant/ml/index.html","quant/math/index.html","quant/index.html","quant/assets/index.html","quant/feeds/index.html","quant/research/index.html","quant/wiki/index.html","quant/papers/index.html","quant/rankings/index.html","quant/news/index.html"];
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

const json=path=>JSON.parse(read(path));
const assetData=json("quant/assets/data.json");
assert.equal(assetData.assets.length,113,"Asset coverage changed unexpectedly");
assert.equal(new Set(assetData.assets.map(a=>a.id)).size,113,"Duplicate asset identifier");
for(const a of assetData.assets){
 assert(a.notes?.length>=30,"Asset lacks specific pricing/data risk notes: "+a.id);
 assert(/^https:\/\//.test(a.data_source?.url||""),"Asset missing original data-provider URL: "+a.id);
}
const siteData=json("quant/rankings/sites.json");
assert.equal(siteData.sites.length,51,"Academic portal coverage changed");
for(const p of siteData.sites){
 assert(p.description?.length>=30,"Academic site purpose is too generic: "+p.id);
 assert(/^https:\/\//.test(p.url),"Invalid academic site URL: "+p.id);
}
const wikiMeta=json("quant/wiki/manifest.json");
const wikiTerms=wikiMeta.data_files.flatMap(name=>json("quant/wiki/"+name).terms);
assert.equal(wikiTerms.length,wikiMeta.total_terms,"Wiki index and data files disagree");
const expandedWiki=wikiTerms.filter(t=>t.example?.includes("数值例题："));
assert(expandedWiki.length>=64,"Encyclopedia lost independent numerical case coverage");
const catalog=json("quant/feeds/catalog.json"),providers=json("quant/feeds/providers.json");
assert.equal(catalog.categories.length,7,"Data feed category coverage changed");
assert.equal(catalog.items.length,497,"Data source field coverage changed");
const providerIds=new Set(providers.providers.map(p=>p.id));
assert.equal(new Set(catalog.items.map(x=>x.id)).size,497);
for(const c of catalog.categories){
 assert(c.research_protocol?.length>=30&&c.failure_modes?.length>=20,"Data-family research contract missing: "+c.id);
}
const researched=catalog.items.filter(x=>x.research_protocol&&x.failure_modes);
assert(researched.length>=35,"Field-level PIT research protocols missing");
for(const item of catalog.items)for(const id of item.provider_ids)
 assert(providerIds.has(id),"Unknown data provider: "+item.id+" -> "+id);
const strategyData=json("quant/research/strategies.json");
assert.equal(strategyData.strategies.length,70,"Strategy template coverage changed");
for(const strategy of strategyData.strategies){
 assert(strategy.code?.includes("def signals("),"Strategy code missing: "+strategy.id);
 assert(strategy.research?.validation&&strategy.research?.derivation,"Strategy proof or validation missing: "+strategy.id);
}
const paperData=json("quant/papers/data.json");
const paperIds=new Set(paperData.papers.map(p=>p.id));
assert.equal(paperIds.size,paperData.papers.length,"Duplicate research paper");
for(const issue of paperData.issues)for(const id of issue.papers)
 assert(paperIds.has(id),"Paper archive references nonexistent paper "+id);
const macroNews=json("quant/news/news.json");
assert.equal(new Set(macroNews.items.map(item=>item.id)).size,macroNews.items.length,"Duplicate macro news ID");
assert(macroNews.items.every(item=>item.date&&item.url&&item.title),"Macro release lacks essential source metadata");

console.log("PASS: published routes are topical, maintenance records and legacy readers are absent, runtime assets resolve");
