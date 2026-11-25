import fs from "node:fs";
import assert from "node:assert/strict";
import path from "node:path";
import katex from "katex";

const volumes=[
 ["math","ohlc-realized-volatility.html","range_volatility_lab.hpp",
  ["range-parkinson","range-gk","range-rs","overnight-gap"]],
 ["ml","probabilistic-forecast-evaluation.html","forecast_verification_lab.hpp",
  ["pinball-loss","gaussian-crps","average-precision","ranked-score3"]],
 ["audit","causal-inference-estimators.html","causal_inference_lab.hpp",
  ["rd-local","wald-iv","meta-fixed","synthetic-simplex"]],
 ["assets","credit-cashflow-models.html","credit_cashflow_lab.hpp",
  ["coupon-accrual","ois-discount","merton-default","credit-zero"]],
 ["feeds","execution-microstructure-dynamics.html","execution_dynamics_lab.hpp",
  ["queue-poisson","transient-impact","rv-signature","volume-bucket"]],
 ["research","portfolio-path-and-margin.html","portfolio_path_lab.hpp",
  ["leveraged-nav","drawdown-tail","margin-call","diversification-2"]]
];
const read=file=>fs.readFileSync(path.join(process.cwd(),file),"utf8");
const sidebar=read("quant/sidebar-v2.js");
const cppIndex=read("quant/cpp/index.html");
const cli=read("quant/cpp/empirical_lab_cli.hpp");
let chapters=0, rendered=0;
for(const [folder,file,header,commands] of volumes){
 const html=read("quant/"+folder+"/"+file);
 const chaptersOnPage=[...html.matchAll(/<article class="chapter" id="([^"]+)">/g)].map(m=>m[1]);
 assert.equal(chaptersOnPage.length,4,"Four independent topics: "+file);
 assert.equal(new Set(chaptersOnPage).size,4,"Duplicate chapter anchors: "+file);
 chapters+=chaptersOnPage.length;
 assert(html.includes('left:\'\\\\(\''),"Inline KaTeX delimiter: "+file);
 assert(html.includes('right:\'\\\\)\''),"Inline KaTeX delimiter: "+file);
 assert(html.includes('font:16px/1.95 "Times New Roman","SimSun"'),
        "Bilingual text typography: "+file);
 assert(html.includes('<pre><code>#pragma once')&&
        html.includes("github.com/KlineCY/KlineCY.github.io/blob/main/quant/cpp/"+header),
        "Executable C++ header not fully embedded: "+file);
 assert(read("quant/"+folder+"/index.html").includes(file),"Category directory link missing: "+file);
 assert(read("quant/index.html").includes(file),"Main directory link missing: "+file);
 assert(cppIndex.includes(file),"CPP discovery link missing: "+file);
 assert(sidebar.includes(file),"Shared navigation link missing: "+file);
 assert(fs.existsSync(path.join("quant/cpp",header)),"C++ source missing: "+header);
 const equations=[...html.matchAll(/<div class="formula">\$\$([\s\S]+?)\$\$<\/div>/g)];
 assert.equal(equations.length,4,"Four independent displayed models: "+file);
 for(const [,formula] of equations){
  const expr=formula.replaceAll("&lt;","<").replaceAll("&gt;",">").replaceAll("&amp;","&");
  katex.renderToString(expr,{throwOnError:true,strict:"error"});
  rendered++;
 }
 for(const command of commands){
  assert(html.includes("./kline_quant "+command+" "),"Missing runnable CSV recipe: "+command);
  assert(cli.includes('c=="'+command+'"'),"CLI dispatch missing: "+command);
 }
 assert(html.includes("数值复算")&&html.includes("输入口径与适用范围")&&
        html.includes("学术与技术资料"),"Insufficient empirical context: "+file);
 assert(!/(学习进度|本轮修订|修改规则|版本修订)/.test(html),
        "Internal administration content in published page: "+file);
}
assert(cppIndex.includes("<strong>321</strong>")&&cppIndex.includes("<strong>1043</strong>"),
       "C++ catalog totals do not include empirical labs");
const near=(a,b)=>assert(Math.abs(a-b)<1e-6);
near(Math.pow(Math.log(110/90),2)/(4*Math.log(2)),.014523873553353111);
near((Math.sqrt(2)-1)/Math.sqrt(Math.PI),.23369497725510913);
near((.5*1+.5*2/3),5/6);
near(1/(1.025*1.02),1/1.0455);
near(1-Math.exp(-1),.6321205588285577);
near(1.2*.8,.96);
console.log("PASS: "+chapters+" complete independent research chapters, "+
            rendered+" parsed KaTeX formulas and 24 CLI recipes across 6 site areas");
