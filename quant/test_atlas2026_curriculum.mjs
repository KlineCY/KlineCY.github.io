import fs from "node:fs";
import path from "node:path";
import assert from "node:assert/strict";
import katex from "katex";

const read=file=>fs.readFileSync(path.join(process.cwd(),file),"utf8");
const catalog=[
  {
    "slug": "tail",
    "area": "math",
    "header": "atlas2026_tail.hpp",
    "commands": [
      "atlas-tail-tail-frequency",
      "atlas-tail-mean-excess",
      "atlas-tail-conditional-tail-mean",
      "atlas-tail-exceedance-second-moment",
      "atlas-tail-weighted-expected-shortfall"
    ]
  },
  {
    "slug": "robust",
    "area": "math",
    "header": "atlas2026_robust.hpp",
    "commands": [
      "atlas-robust-huber-objective",
      "atlas-robust-pseudo-huber",
      "atlas-robust-cauchy-objective",
      "atlas-robust-scaled-log-cosh",
      "atlas-robust-tukey-biweight"
    ]
  },
  {
    "slug": "macro",
    "area": "news",
    "header": "atlas2026_macro.hpp",
    "commands": [
      "atlas-macro-after-tax-real-growth",
      "atlas-macro-real-rate-gap",
      "atlas-macro-taylor-zero-target",
      "atlas-macro-gdp-deflator-level",
      "atlas-macro-cpi-deflated-wage"
    ]
  },
  {
    "slug": "forecast",
    "area": "ml",
    "header": "atlas2026_forecast.hpp",
    "commands": [
      "atlas-forecast-wape-guarded",
      "atlas-forecast-symmetric-mape",
      "atlas-forecast-mean-log-cosh-error",
      "atlas-forecast-epsilon-tube-mae",
      "atlas-forecast-threshold-class-hit"
    ]
  },
  {
    "slug": "dependence",
    "area": "math",
    "header": "atlas2026_dependence.hpp",
    "commands": [
      "atlas-dependence-downside-cohit",
      "atlas-dependence-conditional-downside",
      "atlas-dependence-upside-cohit",
      "atlas-dependence-conditional-downside-beta",
      "atlas-dependence-downside-relative-spread"
    ]
  },
  {
    "slug": "execution",
    "area": "feeds",
    "header": "atlas2026_execution.hpp",
    "commands": [
      "atlas-execution-participation-headroom",
      "atlas-execution-overcap-window-share",
      "atlas-execution-exceeding-market-volume",
      "atlas-execution-peak-participation-utilization",
      "atlas-execution-participation-target-deviation"
    ]
  },
  {
    "slug": "signals",
    "area": "research",
    "header": "atlas2026_signals.hpp",
    "commands": [
      "atlas-signals-signal-hit-rate",
      "atlas-signals-signal-conditional-payoff",
      "atlas-signals-signal-signed-return",
      "atlas-signals-signal-top-bottom-spread",
      "atlas-signals-signal-ic-excess"
    ]
  },
  {
    "slug": "cashflows",
    "area": "assets",
    "header": "atlas2026_cashflows.hpp",
    "commands": [
      "atlas-cashflows-cashflow-present-value",
      "atlas-cashflows-cashflow-discounted-duration",
      "atlas-cashflows-cashflow-time-dispersion",
      "atlas-cashflows-cashflow-coverage-ratio",
      "atlas-cashflows-cashflow-discounted-payback"
    ]
  },
  {
    "slug": "wealth",
    "area": "research",
    "header": "atlas2026_wealth.hpp",
    "commands": [
      "atlas-wealth-relative-log-wealth",
      "atlas-wealth-relative-wealth-max-drawdown",
      "atlas-wealth-excess-return-hurdle-hit",
      "atlas-wealth-relative-volatility-drag",
      "atlas-wealth-relative-terminal-wealth"
    ]
  },
  {
    "slug": "credit",
    "area": "assets",
    "header": "atlas2026_credit.hpp",
    "commands": [
      "atlas-credit-portfolio-expected-credit-loss",
      "atlas-credit-independent-credit-loss-variance",
      "atlas-credit-high-lgd-expected-defaults",
      "atlas-credit-expected-credit-recovery",
      "atlas-credit-independent-credit-loss-sd"
    ]
  }
];
const sidebar=read("quant/sidebar-v2.js");
const root=read("quant/index.html");
const cpp=read("quant/cpp/index.html");
const cli=read("quant/cpp/atlas2026_cli.hpp");
const cases=new Set();
let chapters=0,formulas=0;
for(const entry of catalog){
 const pathname="quant/"+entry.area+"/atlas2026-"+entry.slug+".html";
 const html=read(pathname);
 const header=read("quant/cpp/"+entry.header);
 const escaped=header.replaceAll("&","&amp;").replaceAll("<","&lt;").replaceAll(">","&gt;");
 assert(html.includes(escaped),"The published C++ source does not match "+pathname);
 assert(html.includes("atlas2026_common.hpp"),"Missing shared input validation");
 assert(html.includes('"Times New Roman","SimSun"'),"Typography not compliant");
 assert(html.includes('data-page="'+entry.area+'"'),"Wrong menu category");
 assert(html.includes("参考文献")&&html.includes('href="https://'),"Reference document omitted");
 assert(read("quant/"+entry.area+"/index.html").includes("atlas2026-"+entry.slug+".html"),
        "Area directory missing page");
 assert(sidebar.includes("atlas2026-"+entry.slug+".html"),"Sidebar missing page");
 assert(root.includes("atlas2026-"+entry.slug+".html"),"Quant root missing page");
 assert(cpp.includes("atlas2026-"+entry.slug+".html"),"CPP directory missing page");
 const list=[...html.matchAll(/<article class="lesson" id="(lesson\d+)"/g)];
 assert.equal(list.length,5,"Five substantive lessons required in "+pathname);
 assert.deepEqual(list.map(x=>x[1]),["lesson1","lesson2","lesson3","lesson4","lesson5"]);
 chapters+=list.length;
 const display=[...html.matchAll(/<div class="equation">\$\$([\s\S]+?)\$\$<\/div>/g)];
 assert.equal(display.length,10,"Each lesson needs a definition and verified numerical equation");
 for(const match of display){
  const latex=match[1].replaceAll("&lt;","<").replaceAll("&gt;",">").replaceAll("&amp;","&");
  katex.renderToString(latex,{throwOnError:true,strict:"error"});
  formulas++;
 }
 for(const command of entry.commands){
  assert(!cases.has(command),"Duplicate 2026 research command");
  cases.add(command);
  assert(cli.includes('command=="'+command+'"'),"Missing C++ command dispatcher");
  assert(html.includes("./kline_quant "+command+" study.csv X Y "),
         "Unreproducible mathematical calculation");
 }
 assert(!/(学习进度|修订规则|本轮修改|待补充|TODO)/.test(html),
        "Internal workflow content in public site");
}
assert.equal(chapters,50);
assert.equal(formulas,100);
assert.equal(cases.size,50);
assert(cpp.includes("<strong>321</strong>")&&cpp.includes("<strong>707</strong>"),
       "C++ documentation counts conflict with executable models");
console.log("PASS: 50 complete quantitative chapters, 100 strict KaTeX equations, 50 CLI recipes and 10 exact code-linked directories");
