import fs from "node:fs";
import path from "node:path";
import assert from "node:assert/strict";

const root=process.cwd();
const pages=[
  ["quant/math/return-series-diagnostics.html",48,"quant/math/index.html"],
  ["quant/math/inference-and-survival.html",40,"quant/math/index.html"],
  ["quant/research/fund-portfolio-economics.html",48,"quant/research/index.html"],
  ["quant/institutional-quant-economics.html",40,"quant/index.html"],
  ["quant/assets/derivatives-pricing-lab.html",48,"quant/assets/index.html"],
  ["quant/research/credit-derivatives-lab.html",40,"quant/research/index.html"],
  ["quant/ml/predictive-risk-calibration.html",48,"quant/ml/index.html"],
  ["quant/feeds/market-microstructure-lab.html",48,"quant/feeds/index.html"],
  ["quant/news/macro-asset-transmission.html",48,"quant/news/index.html"],
  ["quant/papers/financial-ml-replication.html",56,"quant/papers/index.html"],
  ["quant/rankings/research-source-quality.html",56,"quant/rankings/index.html"]
];
const read=file=>fs.readFileSync(path.join(root,file),"utf8");
const sidebar=read("quant/sidebar-v2.js");
const cppIndex=read("quant/cpp/index.html");
let lessons=0;
const algorithmFiles=new Set();
for(const [file,expected,index] of pages){
    const html=read(file);
    const articleIds=[...html.matchAll(/<article\b[^>]*id="([^"]+)"/g)].map(match=>match[1]);
    assert.equal(articleIds.length,expected,file+" lesson count");
    lessons+=articleIds.length;
    assert.equal(new Set(articleIds).size,expected,file+" repeated lesson IDs");
    assert(html.includes("katex.min.js"),file+" missing KaTeX");
    for(const match of html.matchAll(/<div class="(?:math|formula)">\$\$([^$]*)\$\$<\/div>/g)){
      const expr=match[1];
      assert(!(/[\u3400-\u9fff]/u.test(expr)&&!/[\\=^_]/.test(expr)),
             file+" display math contains an unrenderable natural-language label");
    }
    assert(!/<p>\\(?:mathrm|tau|lambda|widehat|frac|operatorname|sum|hat|mathbb)\b/.test(html),
           file+" raw TeX is displayed outside a KaTeX delimiter");
    if(file.startsWith("quant/math/")){
      assert(html.includes("left:'\\\\('")&&html.includes("right:'\\\\)'"),
             file+" missing inline mathematics renderer");
      assert.equal((html.match(/<p class="math-topic">/g)||[]).length,expected,
                   file+" missing preserved descriptive course labels");
    }
    assert(html.includes("../sidebar-v2.js")||html.includes("./sidebar-v2.js"),
           file+" missing shared site navigation");
    assert(html.includes("Times New Roman")&&html.includes("SimSun"),
           file+" textbook typography");
    const formulaDelimiters=(html.match(/\$\$/g)||[]).length;
    assert(formulaDelimiters%2===0,file+" unpaired mathematics delimiters");
    if(file.startsWith("quant/math/")){
      assert(formulaDelimiters>=expected*2+2+4*(expected/8),file+" independent worked formulas missing");
      assert.equal((html.match(/<div class="worked-example">/g)||[]).length,expected/8,
                   file+" topic-specific mathematical derivation missing");
    }else{
      assert.equal(formulaDelimiters,expected*2+2,file+" unpaired math formulas");
    }
    const fullSourceCount=(html.match(/<pre><code>#pragma once/g)||[]).length;
    assert.equal(fullSourceCount,expected/8,file+" incomplete C++17 source modules");
    const headers=[...html.matchAll(/https:\/\/github\.com\/KlineCY\/KlineCY\.github\.io\/blob\/main\/quant\/cpp\/([a-z0-9_]+\.hpp)/g)].map(match=>match[1]);
    assert(headers.length>=expected/8,file+" C++17 links absent");
    for(const header of headers){
        assert(fs.existsSync(path.join(root,"quant/cpp",header)),file+" broken source: "+header);
        algorithmFiles.add(header);
    }
    assert(read(index).includes(path.basename(file)),file+" section home missing course link");
    assert(sidebar.includes(path.basename(file)),file+" shared sidebar missing course link");
}
assert.equal(lessons,520,"Expected 520 actual full-length course units");
assert(algorithmFiles.size>=45,"Expected 45+ distinct algorithm source links");
assert(cppIndex.includes("<strong>278</strong>"),"Algorithm total inconsistent");
assert(cppIndex.includes("<strong>500</strong>"),"CLI workflow total inconsistent");
for(const [file] of pages){
    const relative=path.relative(path.join(root,"quant/cpp"),
                                 path.join(root,file)).replaceAll(path.sep,"/");
    assert(cppIndex.includes(relative),"C++17 course index missing "+file);
}
console.log("PASS: "+lessons+" complete mathematics, finance and ML lessons across "+pages.length+" deep courses");
console.log("PASS: "+algorithmFiles.size+" linked C++17 headers, full embedded source, formulas, navigation and site index");
