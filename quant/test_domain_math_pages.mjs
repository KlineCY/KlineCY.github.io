import fs from "node:fs";
import path from "node:path";
import assert from "node:assert/strict";
import katex from "katex";

const root=process.cwd();
const read=f=>fs.readFileSync(path.join(root,f),"utf8");
const catalog=[
 ["math","asynchronous-financial-variation.html","high_frequency_inference_lab.hpp",
  ["hy-cov","realized-quarticity","kernel-1","truncated-rv"]],
 ["audit","doubly-robust-causal-analysis.html","causal_robustness_lab.hpp",
  ["ht-ate","aipw-ate","smd-covariate","ipw-ess"]],
 ["feeds","orderbook-liquidity-structure.html","market_quality_lab.hpp",
  ["quoted-spread","replenishment","kyle-lambda","sign-autocorr"]],
 ["assets","discount-curve-dv01-convexity.html","fixed_income_sensitivity_lab.hpp",
  ["forward-zero","zero-dv01","cash-duration","cash-convexity"]],
 ["ml","imbalanced-classification-evidence.html","classification_audit_lab.hpp",
  ["mcc","macro-f1","balanced-acc","multiclass-brier3"]],
 ["research","euler-risk-tail-attribution.html","scenario_risk_attribution_lab.hpp",
  ["euler-vol2","tail-es2","quad-factor-pnl2","scenario-expectation"]],
 ["feeds","point-in-time-vintage-diagnostics.html","vintage_data_audit_lab.hpp",
  ["vintage-rmse","publish-delay","quote-stale","crossed-quotes"]],
 ["assets","volatility-surface-greeks.html","options_surface_diagnostics_lab.hpp",
  ["digital-call","vanna-call","volga-call","forward-variance"]]
];
const cli=read("quant/cpp/domain_lab_cli.hpp");
const sidebar=read("quant/sidebar-v2.js");
const index=read("quant/index.html");
const cpp=read("quant/cpp/index.html");
const linked=new Set(),commands=new Set();
let formulas=0;
for(const [area,file,header,recipes] of catalog){
 const html=read("quant/"+area+"/"+file);
 assert(!linked.has(file),"Duplicate research page: "+file);linked.add(file);
 const chapterIds=[...html.matchAll(/<article class="chapter" id="(chapter\d+)"/g)].map(m=>m[1]);
 assert.deepEqual(chapterIds,["chapter1","chapter2","chapter3","chapter4"],
                  "Invalid independent lesson IDs: "+file);
 assert(html.includes('data-page="'+area+'"')&&html.includes("../sidebar-v2.js"));
 assert(html.includes('"Times New Roman","SimSun"'),"Typography not bilingual: "+file);
 assert(html.includes("参考来源"),"Academic source section missing: "+file);
 assert(html.includes("数值计算与经济解释")&&html.includes("数据口径与边界"),
        "Mathematical or economic exposition missing: "+file);
 const matches=[...html.matchAll(/<div class="formula">\$\$([\s\S]*?)\$\$<\/div>/g)];
 assert.equal(matches.length,4,"Expected four separately typeset models: "+file);
 for(const [,raw] of matches){
  const formula=raw.replaceAll("&lt;","<").replaceAll("&gt;",">").replaceAll("&amp;","&");
  katex.renderToString(formula,{throwOnError:true,strict:"error"});
  formulas++;
 }
 const rawSource=read("quant/cpp/"+header)
  .replaceAll("&","&amp;").replaceAll("<","&lt;").replaceAll(">","&gt;");
 assert(html.includes(rawSource),"Course code differs from the actual C++ header: "+file);
 for(const cmd of recipes){
  assert(!commands.has(cmd),"Duplicate quantitative research command: "+cmd);
  commands.add(cmd);
  assert(html.includes("./kline_quant "+cmd+" "),"Missing reproducible CLI recipe: "+cmd);
  assert(cli.includes('c=="'+cmd+'"'),"CLI dispatcher misses command: "+cmd);
 }
 assert(read("quant/"+area+"/index.html").includes(file),"Category home missing "+file);
 assert(index.includes(file),"Quant portal missing "+file);
 assert(sidebar.includes(file),"Shared navigation missing "+file);
 assert(cpp.includes(file),"C++ portal missing "+file);
}
assert.equal(formulas,32);
assert.equal(commands.size,32);
assert(cpp.includes("<strong>321</strong>")&&cpp.includes("<strong>941</strong>"),
       "C++ header and command totals disagree with the site");
const approx=(a,b)=>Math.abs(a-b)<1e-10;
assert(approx(Math.log(1/Math.exp(-.05)),.05));
assert(approx((.25*2+.75*4),3.5));
assert(approx((1-Math.exp(-.0001))*100,.00999950001666));
console.log("PASS: 32 independent lectures, valid KaTeX, exact embedded C++17 source, CLI and site navigation");
