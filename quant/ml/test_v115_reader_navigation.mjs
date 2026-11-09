import fs from "node:fs";
import assert from "node:assert/strict";
import vm from "node:vm";
const base=new URL("./",import.meta.url);
const read=p=>fs.readFileSync(new URL(p,base),"utf8");
const mlHtml=read("index.html"),mathHtml=read("../math/index.html");
const mlJs=read("atlas.js"),mathJs=read("../math/app.js");
const mlCss=read("atlas.css"),mathCss=read("../math/styles.css");
new vm.Script(mlJs,{filename:"atlas.js"});
new vm.Script(mathJs,{filename:"app.js"});
const expect=(source,fragment,reason)=>assert(source.includes(fragment),reason+": "+fragment);
function checkReader(html,id,old){
 const heroEnd=html.indexOf("</header>"),now=html.indexOf('id="'+id+'"'),then=html.indexOf(old);
 assert(heroEnd>0&&now>heroEnd&&now<heroEnd+900,id+" not immediately after hero");
 assert(then>now+1000,id+" still below previous long learning-path sections");
 assert.equal(html.split('id="'+id+'"').length,2,id+" missing or duplicated");
}
checkReader(mlHtml,"atlas-reader-section",'aria-label="系统学习路径"');
checkReader(mathHtml,"math-reader-section",'aria-label="数学知识路线"');
for(const html of [mlHtml,mathHtml]){
 assert.equal(html.split('id="course-site-nav-toggle"').length,2,"Missing mobile site navigation toggle");
 expect(html,'aria-controls="quantSidebar" aria-expanded="false"',"Mobile global menu disclosure a11y");
 expect(html,'../reader-site-nav-v115.js',"Shared mobile navigation behavior");
}
new vm.Script(read("../reader-site-nav-v115.js"),{filename:"reader-site-nav-v115.js"});
for(const css of [mlCss,mathCss]){
 expect(css,'#quantSidebar{display:none!important}',"Mobile reader must not be buried under the site menu");
 expect(css,'body.course-site-nav-open #quantSidebar{display:block!important',"Mobile global navigation must remain accessible");
}
for(const id of ["atlas-search","atlas-topic","atlas-toc","atlas-lesson","atlas-total-lessons","atlas-toggle-catalog"])
 assert.equal(mlHtml.split('id="'+id+'"').length,2,"ML reader element missing/duplicated "+id);
for(const id of ["tree","article","articlePane","roadmap","search","disciplineFilter","math-toggle-catalog"])
 assert.equal(mathHtml.split('id="'+id+'"').length,2,"Math reader element missing/duplicated "+id);
expect(mlHtml,"机器学习课程 · 目录与正文","ML readable title");
expect(mathHtml,"量化数学 · 分级目录与讲义","Math readable title");
expect(mlHtml,'aria-controls="atlas-toc" aria-expanded="false"',"ML mobile a11y");
expect(mathHtml,'aria-controls="tree" aria-expanded="false"',"Math mobile a11y");
expect(mlHtml,"atlas.css?v=20261109-v115-reader-layout","ML CSS cache");
expect(mlHtml,"atlas.js?v=20261109-v115-reader-layout","ML JS cache");
expect(mathHtml,"styles.css?v=20261104-probscore-v6","Math prior CSS cache retained");
expect(mathHtml,"v115-sidebar-first","Math CSS/JS cache suffix");
expect(mathHtml,"app.js?v=20261108-v105-control","Math prior JS cache retained");
expect(mlHtml,'class="atlas-paths"',"Keep former ML study paths");
expect(mathHtml,'id="roadmap"',"Keep former math study paths");
const a=mlJs.indexOf("function chapterShouldOpen("),b=mlJs.indexOf("function fixInlineMarkdown(",a);
assert(a>0&&b>a,"ML hierarchical directory code missing");
const directory=mlJs.slice(a,b);
for(const x of ['class="atlas-domain"','class="atlas-section"','class="atlas-topic-index"','class="atlas-topic-label"','aria-current="page"'])
 expect(directory,x,"Three-level directory");
expect(directory,"state.domainOpen.get(id)","Folded subject preference");
expect(directory,"state.chapterOpen.get(key)","Folded chapter preference");
expect(mlJs,"root.scrollTop=0","Article independent scroll reset");
expect(mlJs,'setCatalogOpen(false);',"Close mobile catalog after topic selection");
expect(mlJs,'$("atlas-reader-section")?.scrollIntoView',"Deep links return to reader");
assert(!mlJs.includes('if(location.hash)root.scrollIntoView'),"Old body-scrolling bug remains");
expect(mathJs,'setMathCatalogOpen(false);',"Math mobile navigation");
expect(mathJs,'$("articlePane").scrollTop=0',"Math right-pane scroll reset");
expect(mathJs,"container.scrollTop=previousScroll","Math left-pane scroll retained");
expect(mathJs,'b.setAttribute("aria-current","page")',"Accessible math title selection");
expect(mathJs,'String(n.order).padStart(2,"0")',"Math knowledge numbering");
// Evaluate the real directory renderer, not a rewritten approximation.
const sample=[
 {id:"m01-c01-p01",domain_id:"m01",domain:"统计学习基础",chapter:"学习范式",title:"监督学习"},
 {id:"m01-c01-p02",domain_id:"m01",domain:"统计学习基础",chapter:"学习范式",title:"无监督学习"},
 {id:"m01-c02-p01",domain_id:"m01",domain:"统计学习基础",chapter:"经验风险",title:"经验风险最小化"},
 {id:"m02-c01-p01",domain_id:"m02",domain:"线性代数",chapter:"矩阵基础",title:"线性变换"}
];
const toc={innerHTML:"",scrollTop:27};
const state={index:sample,activeId:"m01-c01-p02",term:"",chapterOpen:new Map(),domainOpen:new Map(),conceptChapters:new Set(["m01-c02-p01"])};
const ctx={state,toc,esc:s=>String(s).replaceAll("&","&amp;").replaceAll('"',"&quot;")};
const prelude="const $=id=>toc; const visibleLessons=()=>state.index;const showStatus=()=>{};\n";
vm.runInNewContext(prelude+directory+"\nrenderDirectory();",ctx,{timeout:1500});
let rendered=toc.innerHTML;
assert.equal((rendered.match(/class="atlas-domain"/g)||[]).length,2,"Two subjects");
assert.equal((rendered.match(/class="atlas-section"/g)||[]).length,3,"Three chapters");
assert.equal((rendered.match(/class="atlas-topic-label"/g)||[]).length,4,"Four knowledge points");
expect(rendered,'data-domain="m01" open',"Current subject expanded");
expect(rendered,'data-key="m01_学习范式" open',"Current chapter expanded");
assert(!rendered.includes('data-domain="m02" open'),"Unselected subject must be folded");
expect(rendered,'title="无监督学习"',"Canonical titles retained");
expect(rendered,'href="#lesson/m01-c01-p02"',"Deep link preserved");
expect(rendered,'aria-current="page"',"Selected knowledge point marked");
assert.equal(toc.scrollTop,27,"Independent navigation scroll was reset");
ctx.state.activeId="m02-c01-p01";ctx.state.domainOpen.set("m01",false);
vm.runInNewContext("renderDirectory();",ctx,{timeout:1500});
expect(toc.innerHTML,'data-domain="m02" open',"External selected subject must expand");
for(const [label,css,parts] of [
 ["ML",mlCss,[".atlas-layout{grid-template-columns:minmax(250px,315px) minmax(0,1fr)","overscroll-behavior:contain",".atlas-domain>summary","line-break:strict",".atlas-reader-section.catalog-open .atlas-toc{display:block}","@media(max-width:900px)","@media print"]],
 ["math",mathCss,[".math-grid{grid-template-columns:minmax(255px,335px) minmax(0,1fr)",".knowledge-tree{min-width:0;position:sticky","height:calc(100dvh - 28px)",".math-reader-section.catalog-open .knowledge-tree{display:block}","@media(max-width:900px)","@media print"]]
])for(const part of parts)expect(css,part,label+" responsive and scroll behavior");
for(const [name,css]of [["ML",mlCss],["math",mathCss]]){
 expect(css,"overflow-x:auto",name+" KaTeX overflow");
 expect(css,":focus-visible",name+" keyboard focus");
}
assert.equal(JSON.parse(read("atlas-depth-manifest.json")).expected_lessons,720,"Atlas canonical curriculum size changed");
assert.equal(JSON.parse(read("../math/curriculum.json")).nodes.length,656,"Math 656 syllabus changed");
const marker="node quant/ml/test_v115_reader_navigation.mjs";
expect(read("../../.github/workflows/test-quant-ml.yml"),marker,"ML CI integration");
expect(read("../../.github/workflows/test-quant-math.yml"),marker,"Math CI integration");
console.log("V115 PASS: both readers first, canonical title hierarchy, executable 3-level ML directory, active topic routing, sticky twin panes, mobile catalog, print/CJK/KaTeX, original 720/656 curricula and dual CI.");
