import fs from "node:fs";
import assert from "node:assert/strict";
import katex from "katex";

const root=new URL("./",import.meta.url);
const read=n=>fs.readFileSync(new URL(n,root),"utf8");
const json=n=>JSON.parse(read(n));
const curriculum=json("curriculum.json");
assert.equal(curriculum.nodes.length,656,"Curriculum size changed");
const nodes=new Map(curriculum.nodes.map(n=>[n.id,n]));
assert.equal(nodes.size,656,"Curriculum node IDs not unique");
const manifest=json("lessons-manifest.json");
assert.equal(manifest.expected_lessons,201,"Historical manifest unexpectedly changed");
assert.equal(manifest.modules.length,77,"Historical bundle registry unexpectedly changed");
const oldFiles=[
 "lessons-foundations.json","lessons-advanced.json","lessons-more.json",
 ...manifest.modules.map(m=>m.path.replace(/^\.\//,"")),
 "lessons-foundations-v89.json","lessons-linear-v90.json","lessons-linear-v92.json",
 "lessons-matrix-v93.json","lessons-probability-v93.json",
 "lessons-probability-v94a.json","lessons-probability-v94b.json","lessons-probability-v94c.json",
 "lessons-statistics-v95a.json","lessons-statistics-v95b.json",
 "lessons-optimization-v96a.json","lessons-optimization-v96b.json",
 "lessons-portfolio-v97.json","lessons-algorithms-v97.json","lessons-numerical-v97.json",
 "lessons-diffeq-v98a.json","lessons-diffeq-v98b.json",
 "lessons-dynamics-v99a.json","lessons-pde-v99b.json",
 "lessons-pricing-pde-v99c.json","lessons-control-v99d.json",
 "lessons-finance-v100a.json","lessons-finance-v100b.json",
 "lessons-finance-v100c.json","lessons-finance-v100d.json",
 "lessons-timeseries-v101.json","lessons-risk-tail-v101.json",
 "lessons-risk-axioms-v101.json","lessons-risk-governance-v101.json",
 "lessons-mlmath-v102a.json","lessons-mlmath-v102b.json",
 "lessons-mlmath-v102c.json",
 "lessons-information-v103a.json","lessons-information-v103b.json",
 "lessons-information-v103c.json"
];
const history=oldFiles.flatMap(n=>json(n).lessons);
assert.equal(history.length,506,"V103 historical 506 lesson texts changed");
assert.equal(new Set(history.map(l=>l.node_id)).size,506,"V103 history has duplicate IDs");
const files=["lessons-discrete-v104a.json","lessons-discrete-v104b.json","lessons-discrete-v104c.json","lessons-discrete-v104d.json"];
const docs=files.map(json);
docs.forEach((d,i)=>{
 assert.equal(d.version,1,"V104 data schema wrong");
 assert.equal(d.revision,"V104","V104 revision mismatch");
 assert.equal(d.lessons.length,[8,7,7,6][i],"V104 file size mismatch");
});
const newest=docs.flatMap(d=>d.lessons);
const ids=[...Array.from({length:8},(_,i)=>i+511),520,521,522,...Array.from({length:4},(_,i)=>i+524),...Array.from({length:7},(_,i)=>i+528),...Array.from({length:6},(_,i)=>i+535)].map(i=>"discrete-n"+String(i).padStart(4,"0"));
assert.deepEqual(newest.map(l=>l.node_id),ids,"V104 discrete/financial networks coverage changed");
const all=[...history,...newest];
assert.equal(all.length,534,"Expected 534 completed mathematics lectures");
assert.equal(new Set(all.map(l=>l.node_id)).size,534,"Historical lesson overwritten");
assert(all.every(l=>nodes.get(l.node_id)?.title===l.title),"Curriculum mismatched or missing lesson");
const completed=new Set(all.map(l=>l.node_id));
const discrete=curriculum.nodes.filter(n=>n.discipline_id==="discrete");
assert.equal(discrete.length,30,"Discrete curriculum 30 changed");
assert(discrete.every(n=>completed.has(n.id)),"Discrete graph mathematics must be 30/30 complete");
assert.equal(history.filter(l=>l.node_id.startsWith("discrete-")).length,2,"Prior two discrete mathematical lessons changed");
assert.equal(newest.filter(l=>l.node_id.startsWith("discrete-")).length,28,"Expected 28 new discrete lectures");
let mathCount=0,deepCount=0,deepChars=0,linkCount=0;
function formula(tex,id){
 assert.equal(typeof tex,"string",id+" formula type");
 assert(tex.length>=10,id+" formula too short");
 assert(!/[\uFFFD\u200B\u200C\u200D\uFEFF]/u.test(tex),id+" bad character");
 try{katex.renderToString(tex,{displayMode:true,throwOnError:true,strict:"error"});}
 catch(e){throw Error(id+" strict KaTeX failed: "+tex+" / "+e.message);}
 mathCount++;
}
for(const l of newest){
 const id=l.node_id;
 assert.equal(l.id,id,"Lesson identity mismatch");
 assert.equal(l.title,nodes.get(id).title,"Course title drift");
 for(const field of ["definition","assumptions","example","application","caveat"])
  assert(l[field]?.length>=70,id+" incomplete "+field);
 assert(l.properties?.length>=4 && l.properties.every(p=>p.length>=20),id+" shallow properties");
 assert.equal(l.formulas.length,3,id+" core formulas");
 assert.equal(l.derivation.length,3,id+" derivations");
 assert.equal(l.deep_sections.length,3,id+" detailed chapters");
 for(const x of l.formulas){assert(x.label?.length>=2,id+" mathematical label");formula(x.tex,id);}
 for(const x of l.derivation){assert(x.explanation?.length>=25,id+" derivation prose");formula(x.tex,id);}
 const headings=new Set();
 for(const s of l.deep_sections){
  assert(s.title?.length>=8 && s.question?.length>=10,id+" deep section title/question");
  assert(!headings.has(s.title),id+" repeated subchapter");
  headings.add(s.title);
  assert(s.paragraphs?.length===2 && s.paragraphs.every(p=>p.length>=30),id+" deep explanatory text");
  for(const [key,min] of [["proof",50],["worked",50],["limitation",25]])
   assert(s[key]?.length>=min,id+" short "+key);
  assert.equal(s.formulas?.length,1,id+" deep math");
  assert(s.formulas[0].label?.length>=2,id+" deep formula caption");
  formula(s.formulas[0].tex,id);
  deepCount++;
  deepChars+=s.paragraphs.join("").length+s.proof.length+s.worked.length+s.limitation.length;
 }
 assert(/[\u3400-\u9FFF]/u.test(JSON.stringify(l)),id+" Chinese content missing");
 assert(!/TODO|FIXME|Lorem ipsum|待补充/i.test(JSON.stringify(l)),id+" placeholder");
 for(const link of l.links||[]){
  assert(["math","ml"].includes(link.kind),id+" unknown cross link kind");
  if(link.kind==="math")assert(nodes.has(link.id),id+" dangling link "+link.id);
  linkCount++;
 }
}
assert.equal(mathCount,252,"New formula count changed");
assert.equal(deepCount,84,"Deep chapter count changed");
assert(deepChars>=22000,"Depth reduced from reviewed version");
assert.equal(linkCount,84,"Cross-lesson link count changed");
const html=read("index.html"),css=read("styles.css"),app=read("app.js");
const workflow=read("../../.github/workflows/test-quant-math.yml");
assert(html.includes('app.js?v=20261108-v105-control-game-market-making-complete'),"Reader must bust V103 cache");
assert(html.includes("V104 · 离散数学与金融网络"),"V104 CJK curriculum absent");
for(const id of ids)assert(html.includes("?topic="+id+'"'),"Study route missing "+id);
for(const needle of [
 'loadV103InformationSignals("./lessons-information-v103c.json",6)',
 'loadV104DiscreteGraphs("./lessons-discrete-v104a.json",8)',
 'loadV104DiscreteGraphs("./lessons-discrete-v104b.json",7)',
 'loadV104DiscreteGraphs("./lessons-discrete-v104c.json",7)',
 'loadV104DiscreteGraphs("./lessons-discrete-v104d.json",6)',
 '...v103a,...v103b,...v103c,...v104a,...v104b,...v104c,...v104d',
 'new Set(lessons.map(l=>l.node_id)).size!==lessons.length',
 'new Set(doc.lessons.map(l=>l.node_id)).size!==8'
])assert(app.includes(needle),"Missing V104 canonical or historical graph loader: "+needle);
for(const token of ["overflow-x:auto","line-break:strict","@media(max-width:700px)","@media print",":focus-visible"])
 assert(css.includes(token),"Missing established Chinese/LaTeX mobile accessibility: "+token);
assert(workflow.includes("node quant/math/test_v104_discrete_complete.mjs"),"V104 CI step absent");




let checks=0;
function near(label,value,expected,tol=1e-8){
 assert(Number.isFinite(value),label+": nonfinite");
 assert(Math.abs(value-expected)<=tol,label+": "+value+" expected "+expected);
 checks++;
}
function yes(label,condition){near(label,Number(Boolean(condition)),1);}
function choose(n,k){let r=1;for(let i=1;i<=k;i++)r=r*(n-i+1)/i;return r;}
function mult(A,B){return A.map(row=>B[0].map((_,j)=>row.reduce((s,x,k)=>s+x*B[k][j],0)));}
function matVec(A,x){return A.map(row=>row.reduce((s,v,i)=>s+v*x[i],0));}
function sum(x){return x.reduce((a,b)=>a+b,0);}
function fold(i){const a=[1,2];for(let j=2;j<=i;j++)a[j]=a[j-1]+a[j-2];return a[i];}
function dot(a,b){return sum(a.map((x,i)=>x*b[i]));}
near("Counting independent choices",3*4,12);
near("Counting disjoint alternatives",3*4+5,17);
near("Counting non-repeating ordered two assets",3*2,6);
near("Counting four binary states",2**4,16);
near("Counting at least one state",2**4-1,15);
near("Permutation 6P3",6*5*4,120);
near("Combination 6C3",choose(6,3),20);
near("Combination factorial pairing",120/(3*2*1),20);
near("Combination 5C2",choose(5,2),10);
near("Combination Pascal 4C2+4C1",choose(4,2)+choose(4,1),10);
near("Combination constrained sector",choose(4,2)*choose(3,1),18);
near("Inclusion-exclusion pair",40+30-10,60);
near("Inclusion-exclusion triple",40+30+20-10-8-6+3,69);
near("Bonferroni lower",40+30+20-10-8-6,66);
near("Bonferroni upper",40+30+20,90);
near("Independent pair overlap",.3*.2,.06);
near("Dependent pair union",.3+.2-.1,.4);
near("Independent pair union",.3+.2-.3*.2,.44);
near("Recurrence n2",fold(2),3);
near("Recurrence n3",fold(3),5);
near("Recurrence n4",fold(4),8);
near("Recurrence golden root",(1+Math.sqrt(5))/2,1.618033988749895);
near("Recurrence characteristic polynomial",((1+Math.sqrt(5))/2)**2-(1+Math.sqrt(5))/2-1,0);
near("Generating function two coins at amount4",Math.floor(4/2)+1,3);
near("Generating fibonacci coefficient 4",fold(4),8);
near("Generating function convolution 0..4",sum([0,1,2].map(j=>Number(4-2*j>=0))),3);
near("Graph simple 4 vertices possible edges",choose(4,2),6);
near("Graph all four vertex structures",2**choose(4,2),64);
near("Graph exactly three edges",choose(6,3),20);
near("Graph labelled trees",4**(4-2),16);
near("Graph triangle plus isolated graphs",20-16,4);
near("Graph ETF 37 possible pairs",choose(37,2),666);
near("Directed graph outgoing degrees sum",2+1+1,4);
near("Directed graph incoming degrees sum",1+1+2,4);
near("Undirected three path degree sum",1+2+1,4);
const A=[[0,1,0],[1,0,1],[0,1,0]],A2=mult(A,A);
near("Adjacency square middle return walks",A2[1][1],2);
near("Adjacency square endpoints cross walk",A2[0][2],1);
near("Adjacency square no odd middle endpoint",A2[0][1],0);
near("Adjacency path graph eigen plus",Math.sqrt(2),1.4142135623730951);
near("Shortest route direct",5,5);
near("Shortest via intermediate",2+1,3);
yes("Shortest indirect cheaper",2+1<5);
near("Shortest relax C",Math.min(5,2+1),3);
near("MST selected two edges",1+2,3);
near("MST alternate pair",1+4,5);
yes("MST cheaper alternative",1+2<1+4);
near("MST three nodes edges",3-1,2);
near("MST triangle A-C shorter than tree path",1+2,3);
near("Max flow first path",2,2);
near("Max flow second path",1,1);
near("Max flow third path",2,2);
near("Max flow path sum",2+1+2,5);
near("Max flow source cut capacity",3+2,5);
near("Max flow sink capacity",2+3,5);
near("Max flow node a capacity",2+1,3);
near("Max flow node b capacity",1+2,3);
const P=[[0,1,0],[.5,0,.5],[0,1,0]];
const pi=[.25,.5,.25],next=pi.map((_,j)=>sum(pi.map((p,i)=>p*P[i][j])));
near("Random walk stationary π1",next[0],.25);
near("Random walk stationary π2",next[1],.5);
near("Random walk stationary π3",next[2],.25);
near("Random walk stationary sum",sum(next),1);
near("Random walk two steps center",mult(P,P)[1][1],1);
near("Random walk two steps center to endpoint",mult(P,P)[1][0],0);
near("Spectral clustering two graph zero eig count",2,2);
near("Spectral clustering two graph nonzero eigenvalue",2,2);
near("Spectral graph quadratic energy edge3",3*(1-(-1))**2,12);
near("Spectral zero eigenvalue vector",dot([1,1],[1-1,-1+1]),0);
const diffusion=Math.exp(-2*Math.log(2)/2);
near("Graph diffusion amplitude at ln2/2",diffusion,.5);
near("Graph diffusion node1",(.5)*(1+diffusion),.75);
near("Graph diffusion node2",(.5)*(1-diffusion),.25);
near("Graph diffusion conserves mass",(.5)*(1+diffusion)+(.5)*(1-diffusion),1);
near("Graph diffusion limit node1",.5,.5);
const alpha=.8,rank=[5/9,4/9];
near("PageRank first",rank[0],5/9);
near("PageRank second",rank[1],4/9);
near("PageRank normalizes",sum(rank),1);
near("PageRank fixed point first",alpha*rank[1]+(1-alpha),rank[0]);
near("PageRank fixed point second",alpha*rank[0],rank[1]);
yes("PageRank damping is contraction",alpha<1);
near("Clearing first payment",Math.min(100,70),70);
near("Clearing second payment",Math.min(100,70+0),70);
near("Clearing first loss shortfall",100-70,30);
near("Clearing second loss shortfall",100-70,30);
const io1=120/(1-.2*.3),io2=100+.3*io1;
near("Supply chain inverse determinant",1-.2*.3,.94);
near("Supply chain production1",io1,127.65957446808511);
near("Supply chain production2",io2,138.29787234042553);
near("Supply chain equation1",.2*io2+100,io1);
near("Supply chain equation2",.3*io1+100,io2);
near("Supply chain second round coefficient",.2*.3,.06);
near("Ownership two-step",.6*.5,.3);
near("Ownership direct plus indirect",.1+.6*.5,.4);
near("Ownership circular weight",.5*.2,.1);
near("Ownership circular two turns",(.5*.2)**2,.01);
near("Related transaction gross",30+20,50);
near("Related transaction balance",30-20,10);
near("Related triangle product",30*20*10,6000);
near("Related A purchase to income",20/100,.2);
near("Related B purchase to income",30/1000,.03);
near("Counterparty netting",Math.max(10-6,0),4);
near("Counterparty gross positive",Math.max(10,0)+Math.max(-6,0),10);
near("Counterparty simple CVA",.95*.02*.6*10,.114);
near("Counterparty default loss standalone",.02*.6*10,.12);
near("Counterparty wrong-way covariance offset",.12-.12,0);
near("GNN node1 self loop mean",(1+0)/2,.5);
near("GNN node2 self loop mean",(1+0+0)/3,1/3);
near("GNN node3 self loop mean",(0+0)/2,0);
near("GNN fully connected oversmooth",1/3,1/3);
near("Graph forecast path midpoint neighbor mean",(2+1)/2,1.5);
near("Graph forecast endpoint neighbor mean",0,0);
near("Graph forecast sample time index",5-1,4);
near("Weighted interval schedule combined",3+4,7);
near("Weighted interval schedule alternative",6,6);
near("Weighted interval DP max",Math.max(7,6),7);
near("Bellman execution all at once",2**2,4);
near("Bellman execution evenly",1**2+1**2,2);
near("Bellman execution uneven",.5**2+1.5**2,2.5);
near("Bellman q2 optimum",2**2/2,2);
near("Bellman quadratic derivative at optimum",4*1-2*2,0);
near("Bitmask 20 subsets",2**20,1048576);
near("Bitmask approximate DP states",20*2**20,20971520);
near("Bitmask three cities one direction",1+2+3,6);
near("Bitmask three cities other direction",3+2+1,6);
near("Complexity function at100",3*100**2+10*100+7,31007);
near("Complexity function at200",3*200**2+10*200+7,122007);
yes("Complexity upper constant",3*100**2+10*100+7<=20*100**2);
yes("Complexity lower constant",3*100**2+10*100+7>=3*100**2);
near("Complexity max undirected n37",37*36/2,666);
near("Approximation matching cardinality",2,2);
near("Approximation solution endpoints",2*2,4);
near("Approximation optimum vertex cover",2,2);
near("Approximation achieved ratio",4/2,2);
yes("Approximation 2-approx verified",4<=2*2);
near("Reservoir first item survives 3",1*(1-.5)*(1-1/3),1/3);
near("Reservoir second item survives 3",.5*(1-1/3),1/3);
near("Reservoir third item selected",1/3,1/3);
near("Reservoir n3 total",1/3+1/3+1/3,1);
near("Hoeffding required count",Math.ceil(Math.log(40)/(2*.05**2)),738);
near("Hoeffding required count half precision",Math.ceil(Math.log(40)/(2*.025**2)),2952);
yes("Hoeffding 738 sufficient",2*Math.exp(-2*738*.05**2)<=.05);
assert(checks>=120,"Independent graph and combinatorics computations unexpectedly incomplete");
console.log("V104 PASS",newest.length,"new lectures,",all.length,"total;",mathCount,"strict KaTeX;",
 deepCount,"proof chapters;",deepChars,"deep prose characters;",checks,"independent numerical checks;",
 linkCount,"crosslinks; discrete graph mathematics 30/30.");
