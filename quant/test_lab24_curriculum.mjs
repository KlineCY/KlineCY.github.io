import fs from "node:fs";
import path from "node:path";
import assert from "node:assert/strict";
import katex from "katex";

const read=f=>fs.readFileSync(path.join(process.cwd(),f),"utf8");
const esc=s=>s.replaceAll("&","&amp;").replaceAll("<","&lt;").replaceAll(">","&gt;");
const unesc=s=>s.replaceAll("&lt;","<").replaceAll("&gt;",">").replaceAll("&amp;","&");
const catalog=[
  {
    "area": "math",
    "slug": "linear",
    "functions": [
      {
        "name": "qr_solve",
        "command": "lab24-linear-qr-solve",
        "csv": "X1,X2,Y\n1,0,2\n0,1,3\n1,1,5",
        "params": [],
        "result": [
          2,
          3,
          0
        ]
      },
      {
        "name": "spectral2",
        "command": "lab24-linear-spectral2",
        "csv": "A,B\n2,1\n1,2",
        "params": [],
        "result": [
          3,
          1,
          0.7071067811865476,
          0.7071067811865476,
          2
        ]
      },
      {
        "name": "ridge2",
        "command": "lab24-linear-ridge2",
        "csv": "X1,X2,Y\n-1,-1,-6\n-1,1,2\n1,-1,-2\n1,1,6",
        "params": [
          4
        ],
        "result": [
          0,
          1,
          2,
          2
        ]
      },
      {
        "name": "woodbury_rank1",
        "command": "lab24-linear-woodbury-rank1",
        "csv": "U,B\n1,3\n1,1",
        "params": [
          1
        ],
        "result": [
          1.6666666666666667,
          -0.3333333333333333
        ]
      },
      {
        "name": "power_iteration",
        "command": "lab24-linear-power-iteration",
        "csv": "C1,C2,C3\n4,0,0\n0,2,0\n0,0,1",
        "params": [
          40
        ],
        "result": [
          4,
          0
        ]
      }
    ]
  },
  {
    "area": "math",
    "slug": "optim",
    "functions": [
      {
        "name": "simplex_projection",
        "command": "lab24-optim-simplex-projection",
        "csv": "Z\n0.2\n1.2\n-0.3",
        "params": [],
        "result": [
          0,
          1,
          0
        ]
      },
      {
        "name": "l1_ball",
        "command": "lab24-optim-l1-ball",
        "csv": "Z\n2\n-1\n0.5",
        "params": [
          2
        ],
        "result": [
          1.5,
          -0.5,
          0
        ]
      },
      {
        "name": "box_budget",
        "command": "lab24-optim-box-budget",
        "csv": "Z,Lower,Upper\n0.8,0,0.7\n0.6,0,0.7\n0.2,0,0.7",
        "params": [
          1
        ],
        "result": [
          0.6,
          0.4,
          0
        ]
      },
      {
        "name": "elastic_coordinate",
        "command": "lab24-optim-elastic-coordinate",
        "csv": "CrossProduct,GramDiagonal\n3,2\n-1,2\n0.2,2",
        "params": [
          1,
          1
        ],
        "result": [
          0.6666666666666666,
          0,
          0
        ]
      },
      {
        "name": "entropy_tilt",
        "command": "lab24-optim-entropy-tilt",
        "csv": "Prior,Feature\n0.5,0\n0.5,1",
        "params": [
          1.0986122886681098
        ],
        "result": [
          0.75,
          0.13081203594113697
        ]
      }
    ]
  },
  {
    "area": "math",
    "slug": "network",
    "functions": [
      {
        "name": "laplacian_energy",
        "command": "lab24-network-laplacian-energy",
        "csv": "From,To,Weight,ValueFrom,ValueTo\n0,1,2,1,2\n1,2,1,2,4",
        "params": [],
        "result": [
          6
        ]
      },
      {
        "name": "pagerank",
        "command": "lab24-network-pagerank",
        "csv": "From,To,Weight\n0,1,1\n1,0,1",
        "params": [
          2,
          0.85,
          50
        ],
        "result": [
          0.5,
          0.5
        ]
      },
      {
        "name": "diffusion",
        "command": "lab24-network-diffusion",
        "csv": "From,To,Weight\n0,1,1",
        "params": [
          0.25,
          1,
          1,
          0
        ],
        "result": [
          0.75,
          0.25
        ]
      },
      {
        "name": "minimax_path",
        "command": "lab24-network-minimax-path",
        "csv": "From,To,Distance\n0,1,2\n1,2,3\n0,2,5",
        "params": [
          3,
          0,
          2
        ],
        "result": [
          3
        ]
      },
      {
        "name": "minimum_spanning_tree",
        "command": "lab24-network-minimum-spanning-tree",
        "csv": "From,To,Distance\n0,1,2\n1,2,3\n0,2,5",
        "params": [
          3
        ],
        "result": [
          5,
          3
        ]
      }
    ]
  },
  {
    "area": "math",
    "slug": "state",
    "functions": [
      {
        "name": "scalar_kalman",
        "command": "lab24-state-scalar-kalman",
        "csv": "Observation,MeasurementVariance\n2,1",
        "params": [
          0,
          1,
          0
        ],
        "result": [
          1,
          0.5,
          -2.2655121234846454
        ]
      },
      {
        "name": "hmm_forward",
        "command": "lab24-state-hmm-forward",
        "csv": "Likelihood0,Likelihood1\n0.8,0.2",
        "params": [
          0.5,
          0.9,
          0.8
        ],
        "result": [
          0.8,
          0.2,
          -0.6931471805599453
        ]
      },
      {
        "name": "viterbi",
        "command": "lab24-state-viterbi",
        "csv": "Likelihood0,Likelihood1\n0.9,0.1\n0.8,0.2",
        "params": [
          0.5,
          0.9,
          0.9
        ],
        "result": [
          0,
          0
        ]
      },
      {
        "name": "ou_transition_fit",
        "command": "lab24-state-ou-transition-fit",
        "csv": "State\n1\n0.5\n0.25\n0.125",
        "params": [
          1
        ],
        "result": [
          0.6931471805599453,
          0,
          0
        ]
      },
      {
        "name": "markov_hitting",
        "command": "lab24-state-markov-hitting",
        "csv": "StayProbability\n0.8",
        "params": [
          2
        ],
        "result": [
          0.36,
          5
        ]
      }
    ]
  },
  {
    "area": "audit",
    "slug": "panel",
    "functions": [
      {
        "name": "within_panel",
        "command": "lab24-panel-within-panel",
        "csv": "Entity,X,Y\n0,0,1\n0,1,3\n1,0,10\n1,1,12",
        "params": [],
        "result": [
          2,
          1
        ]
      },
      {
        "name": "cluster_ols",
        "command": "lab24-panel-cluster-ols",
        "csv": "Cluster,X,Y\n0,0,1\n0,1,3\n1,0,5\n1,1,7",
        "params": [],
        "result": [
          3,
          2,
          0
        ]
      },
      {
        "name": "paired_t",
        "command": "lab24-panel-paired-t",
        "csv": "Before,After\n0,1\n0,2\n0,3",
        "params": [],
        "result": [
          2,
          0.5773502691896258,
          3.4641016151377544
        ]
      },
      {
        "name": "randomization_exact",
        "command": "lab24-panel-randomization-exact",
        "csv": "Difference\n1\n1",
        "params": [],
        "result": [
          1,
          0.5
        ]
      },
      {
        "name": "iv_first_stage",
        "command": "lab24-panel-iv-first-stage",
        "csv": "Instrument,Treatment,Outcome\n-1,-2,-4\n-1,0,0\n1,0,0\n1,2,4",
        "params": [],
        "result": [
          1,
          2,
          2
        ]
      }
    ]
  },
  {
    "area": "ml",
    "slug": "learning",
    "functions": [
      {
        "name": "online_logistic",
        "command": "lab24-learning-online-logistic",
        "csv": "Feature1,Feature2,Label\n1,0,1",
        "params": [
          0.2
        ],
        "result": [
          0.1,
          0,
          0.6931471805599453
        ]
      },
      {
        "name": "adagrad_ridge",
        "command": "lab24-learning-adagrad-ridge",
        "csv": "Feature1,Feature2,Target\n1,0,2",
        "params": [
          0.1,
          1,
          0.001
        ],
        "result": [
          0.09995002498750626,
          0,
          4
        ]
      },
      {
        "name": "brier_decomposition",
        "command": "lab24-learning-brier-decomposition",
        "csv": "Probability,Label\n0.25,0\n0.75,1",
        "params": [],
        "result": [
          0.0625,
          0.0625,
          0.25,
          0.25
        ]
      },
      {
        "name": "isotonic_pav",
        "command": "lab24-learning-isotonic-pav",
        "csv": "Score,Label\n1,0\n2,1\n3,0\n4,1",
        "params": [],
        "result": [
          0,
          0.5,
          0.5,
          1
        ]
      },
      {
        "name": "stacking_two",
        "command": "lab24-learning-stacking-two",
        "csv": "Model1,Model2,Target\n0,2,1\n2,0,1",
        "params": [],
        "result": [
          0.5,
          0.5,
          0
        ]
      }
    ]
  },
  {
    "area": "assets",
    "slug": "rates",
    "functions": [
      {
        "name": "discount_bootstrap",
        "command": "lab24-rates-discount-bootstrap",
        "csv": "ParCoupon\n0.05\n0.05",
        "params": [],
        "result": [
          0.9523809523809523,
          0.9070294784580498
        ]
      },
      {
        "name": "forward_curve",
        "command": "lab24-rates-forward-curve",
        "csv": "Time,Discount\n1,0.9801986733067553\n2,0.9417645335842487",
        "params": [],
        "result": [
          0.02,
          0.04
        ]
      },
      {
        "name": "curve_keyrate",
        "command": "lab24-rates-curve-keyrate",
        "csv": "Time,Cash\n1,10\n2,100",
        "params": [
          1,
          2,
          0.02,
          0.04
        ],
        "result": [
          102.11362137173113,
          0.0009801496650067136,
          0.01846048081811597
        ]
      },
      {
        "name": "duration_immunize",
        "command": "lab24-rates-duration-immunize",
        "csv": "Maturity\n1\n3",
        "params": [
          100,
          2,
          0
        ],
        "result": [
          50,
          50,
          50,
          50
        ]
      },
      {
        "name": "yield_newton",
        "command": "lab24-rates-yield-newton",
        "csv": "Time,Cash\n1,110",
        "params": [
          100,
          -0.5,
          0.5
        ],
        "result": [
          0.09531017980432493,
          0
        ]
      }
    ]
  },
  {
    "area": "assets",
    "slug": "derivative",
    "functions": [
      {
        "name": "crr_american",
        "command": "lab24-derivative-crr-american",
        "csv": "Input\n1",
        "params": [
          100,
          100,
          0,
          0,
          0.6931471805599453,
          1,
          1,
          0
        ],
        "result": [
          33.333333333333336,
          33.333333333333336
        ]
      },
      {
        "name": "asian_path",
        "command": "lab24-derivative-asian-path",
        "csv": "S1,S2,S3\n110,110,110\n90,90,90",
        "params": [
          100,
          0,
          1,
          1
        ],
        "result": [
          5,
          5
        ]
      },
      {
        "name": "butterfly_density",
        "command": "lab24-derivative-butterfly-density",
        "csv": "Strike,Call\n90,15\n100,8\n120,2",
        "params": [
          0,
          1
        ],
        "result": [
          0.02666666666666667,
          0
        ]
      },
      {
        "name": "variance_strip",
        "command": "lab24-derivative-variance-strip",
        "csv": "Strike,OTMPrice\n90,1\n100,1\n110,1",
        "params": [
          100,
          100,
          0,
          1
        ],
        "result": [
          0.006122028364452608,
          0.07824339182609998
        ]
      },
      {
        "name": "discrete_delta_pnl",
        "command": "lab24-derivative-discrete-delta-pnl",
        "csv": "Time,Spot,Delta\n0,100,0.5\n1,110,0.5",
        "params": [
          5,
          10,
          0,
          0
        ],
        "result": [
          0,
          0
        ]
      }
    ]
  },
  {
    "area": "assets",
    "slug": "credit",
    "functions": [
      {
        "name": "migration_two",
        "command": "lab24-credit-migration-two",
        "csv": "Good,Weak,Default\n0.8,0.1,0.1\n0,0.8,0.2\n0,0,1",
        "params": [
          2,
          1,
          0,
          0
        ],
        "result": [
          0.64,
          0.16,
          0.2
        ]
      },
      {
        "name": "poisson_binomial",
        "command": "lab24-credit-poisson-binomial",
        "csv": "PD\n0.1\n0.2",
        "params": [],
        "result": [
          0.72,
          0.26,
          0.02
        ]
      },
      {
        "name": "loss_convolution",
        "command": "lab24-credit-loss-convolution",
        "csv": "PD,LossUnits\n0.1,1\n0.2,1",
        "params": [
          0.9
        ],
        "result": [
          0.3,
          1,
          1.2
        ]
      },
      {
        "name": "survival_hazard",
        "command": "lab24-credit-survival-hazard",
        "csv": "EndTime,Hazard\n1,0.6931471805599453\n2,0.6931471805599453",
        "params": [],
        "result": [
          0.5,
          0.5,
          0.25,
          0.25
        ]
      },
      {
        "name": "discrete_cds_spread",
        "command": "lab24-credit-discrete-cds-spread",
        "csv": "EndTime,Survival,Discount\n1,0.9,1",
        "params": [
          0.4
        ],
        "result": [
          0.06,
          0.9,
          0.06666666666666667
        ]
      }
    ]
  },
  {
    "area": "research",
    "slug": "portfolio",
    "functions": [
      {
        "name": "factor_neutral_two",
        "command": "lab24-portfolio-factor-neutral-two",
        "csv": "TargetWeight,Factor\n1,-1\n1,0\n1,1",
        "params": [
          0,
          0
        ],
        "result": [
          0,
          0,
          0
        ]
      },
      {
        "name": "risk_budget_diag",
        "command": "lab24-portfolio-risk-budget-diag",
        "csv": "Volatility,RiskBudget\n1,0.5\n2,0.5",
        "params": [],
        "result": [
          0.6666666666666666,
          0.3333333333333333
        ]
      },
      {
        "name": "factor_cov_risk",
        "command": "lab24-portfolio-factor-cov-risk",
        "csv": "Weight,Beta,IdioVariance\n0.5,1,1\n0.5,1,1",
        "params": [
          1
        ],
        "result": [
          1,
          1,
          0.5,
          1.224744871391589
        ]
      },
      {
        "name": "turnover_prox",
        "command": "lab24-portfolio-turnover-prox",
        "csv": "Old,Target,Curvature,Cost\n0.2,0.5,1,0.1\n0.4,0.35,1,0.1",
        "params": [],
        "result": [
          0.4,
          0.4
        ]
      },
      {
        "name": "rebalance_drift",
        "command": "lab24-portfolio-rebalance-drift",
        "csv": "OldWeight,Return,TargetWeight\n0.5,0.2,0.5\n0.5,0,0.5",
        "params": [],
        "result": [
          0.1,
          0.045454545454545456,
          0.5454545454545454,
          0.45454545454545453
        ]
      }
    ]
  },
  {
    "area": "feeds",
    "slug": "execution",
    "functions": [
      {
        "name": "ac_schedule",
        "command": "lab24-execution-ac-schedule",
        "csv": "Quantity\n100",
        "params": [
          4,
          1,
          0
        ],
        "result": [
          25,
          25,
          25,
          25
        ]
      },
      {
        "name": "participation_schedule",
        "command": "lab24-execution-participation-schedule",
        "csv": "MarketVolume\n100\n200\n100",
        "params": [
          50,
          0.2
        ],
        "result": [
          20,
          30,
          0,
          0
        ]
      },
      {
        "name": "queue_fifo",
        "command": "lab24-execution-queue-fifo",
        "csv": "AheadCancellation,AggressiveQuantity\n2,5\n0,6",
        "params": [
          10,
          5
        ],
        "result": [
          3,
          2,
          0
        ]
      },
      {
        "name": "multi_venue_sweep",
        "command": "lab24-execution-multi-venue-sweep",
        "csv": "Ask,Available,UnitFee\n10,2,0\n11,4,0",
        "params": [
          5
        ],
        "result": [
          53,
          5,
          0,
          2,
          3
        ]
      },
      {
        "name": "impact_decay_fit",
        "command": "lab24-execution-impact-decay-fit",
        "csv": "Elapsed,Impact\n0,4\n1,2\n2,1",
        "params": [],
        "result": [
          4,
          0.6931471805599453,
          1
        ]
      }
    ]
  },
  {
    "area": "audit",
    "slug": "data",
    "functions": [
      {
        "name": "asof_age",
        "command": "lab24-data-asof-age",
        "csv": "AvailableTime,Value,DecisionTime\n1,10,2\n3,30,3\n5,50,8",
        "params": [
          2
        ],
        "result": [
          1,
          10,
          1,
          1,
          30,
          0,
          0,
          0,
          0
        ]
      },
      {
        "name": "purged_mask",
        "command": "lab24-data-purged-mask",
        "csv": "LabelStart,LabelEnd\n0,1\n1,3\n4,5\n6,7",
        "params": [
          2,
          3,
          1
        ],
        "result": [
          1,
          0,
          0,
          1
        ]
      },
      {
        "name": "label_uniqueness",
        "command": "lab24-data-label-uniqueness",
        "csv": "Start,End\n0,1\n1,2",
        "params": [],
        "result": [
          0.75,
          0.75
        ]
      },
      {
        "name": "revision_latest",
        "command": "lab24-data-revision-latest",
        "csv": "Reference,Available,Value\n1,2,10\n1,4,12\n2,3,20",
        "params": [
          3
        ],
        "result": [
          1,
          2,
          10,
          2,
          3,
          20
        ]
      },
      {
        "name": "event_time_bars",
        "command": "lab24-data-event-time-bars",
        "csv": "Price,Quantity\n10,2\n12,3\n11,1",
        "params": [
          3
        ],
        "result": [
          10,
          12,
          10,
          12,
          3,
          10.666666666666666,
          12,
          12,
          11,
          11,
          3,
          11.666666666666666
        ]
      }
    ]
  }
];
const cli=read("quant/cpp/lab24_cli.hpp");
const root=read("quant/index.html"),sidebar=read("quant/sidebar-v2.js"),cpp=read("quant/cpp/index.html");
const common=esc(read("quant/cpp/lab24_common.hpp"));
let chapters=0,equations=0,values=0;
const commands=new Set();
for(const entry of catalog){
 const file="lab24-"+entry.slug+".html";
 const page="quant/"+entry.area+"/"+file;
 const html=read(page);
 const header=read("quant/cpp/lab24_"+entry.slug+".hpp");
 assert(html.includes(esc(header))&&html.includes(common),"Published source must match repository: "+file);
 assert(html.includes('"Times New Roman","SimSun"')&&html.includes("background:white"));
 assert(html.includes('data-page="'+entry.area+'"')&&html.includes("参考文献"));
 assert(!/(学习进度|修订规则|本轮修改|待补充|TODO)/.test(html),"Internal material in "+file);
 for(const d of [root,sidebar,cpp,read("quant/"+entry.area+"/index.html")])assert(d.includes(file),"Missing navigation: "+file);
 const articles=[...html.matchAll(/<article class="lesson" id="(lesson\d+)">([\s\S]*?)<\/article>/g)];
 assert.equal(articles.length,5,file);
 assert.deepEqual(articles.map(a=>a[1]),["lesson1","lesson2","lesson3","lesson4","lesson5"]);
 for(let j=0;j<articles.length;++j){
  const article=articles[j][2],f=entry.functions[j];
  assert(article.includes("推导、最优性条件与数值结构")&&article.includes("估计假设与应用边界"));
  assert(article.includes("输出顺序：")&&article.includes(esc(f.csv)),"Incorrect reproducible input: "+f.command);
  const cmd="./kline_quant "+f.command+" study.csv"+(f.params.length?" "+f.params.join(" "):"");
  assert(article.includes(esc(cmd)),"Incorrect reproducible command: "+f.command);
  assert(header.includes("inline Vec "+f.name+"(")&&cli.includes('command=="'+f.command+'"'));
  assert(!commands.has(f.command));commands.add(f.command);
  const forms=[...article.matchAll(/<div class="equation">\$\$([\s\S]*?)\$\$<\/div>/g)];
  assert.equal(forms.length,3,"Model, derivation, and numerical output required");
  for(const form of forms){katex.renderToString(unesc(form[1]),{throwOnError:true,strict:"error"});++equations;}
  const numerical=unesc(forms[2][1]);
  const vector=numerical.match(/\\left\(([\s\S]+)\\right\)/);
  assert(vector,"Missing numerical output");
  const observed=vector[1].split(",\\ ").map(Number);
  assert.equal(observed.length,f.result.length,"Incorrect output dimension: "+f.command);
  for(let i=0;i<observed.length;++i){
   assert(Number.isFinite(observed[i])&&Math.abs(observed[i]-f.result[i])<=1e-7*Math.max(1,Math.abs(f.result[i])),"Published output differs from independent reference: "+f.command);
   ++values;
  }
  ++chapters;
 }
}
assert.equal(chapters,60);assert.equal(equations,180);assert.equal(commands.size,60);
assert(cpp.includes("<strong>313</strong>")&&cpp.includes("<strong>675</strong>"));
console.log("PASS: "+chapters+" substantive chapters, "+equations+" strict KaTeX equations, "+values+" verified vector coordinates, "+commands.size+" executable recipes and twelve exact source-linked pages");
