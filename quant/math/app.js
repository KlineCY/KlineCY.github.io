/* Quant Mathematics — an incremental, inspectable proof-oriented curriculum. */
(()=>{
"use strict";
const $=id=>document.getElementById(id);
const esc=s=>String(s??"").replace(/[&<>"']/g,x=>({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[x]));
const alpha=(a,b)=>String(a).localeCompare(String(b),"en",{sensitivity:"base"});
const link=(u,label)=>/^https:\/\//.test(u||"")?'<a href="'+esc(u)+'" target="_blank" rel="noopener noreferrer">'+esc(label)+'</a>':"";
let curriculum=null,depts=[],chapters=[],nodes=[],lessons=[],lessonMap=new Map();
let nodeMap=new Map(),chapMap=new Map(),deptMap=new Map(),activeId="",filterDept="",filterLevel="",filterStatus="",query="";
const progressKey="kline-quant-math-progress-v1";
let studied=new Set();
try{const saved=JSON.parse(localStorage.getItem(progressKey)||"[]");if(Array.isArray(saved))studied=new Set(saved)}catch(_){}
const saveProgress=()=>{try{localStorage.setItem(progressKey,JSON.stringify([...studied]))}catch(_){}};
const getDept=id=>deptMap.get(id),getChap=id=>chapMap.get(id),getNode=id=>nodeMap.get(id);
const humanStatus=node=>lessonMap.has(node.id)?"完整讲义":"知识目录";
const isComplete=node=>lessonMap.has(node.id);
function setURL(updates){
 const url=new URL(location.href);
 for(const [key,value] of Object.entries(updates))value?url.searchParams.set(key,value):url.searchParams.delete(key);
 history.replaceState(null,"",url.pathname+url.search+url.hash);
}
function mathReady(){return typeof window.katex!=="undefined"}
function typeset(el){
 if(!el)return;
 if(typeof window.renderMathInElement==="function"){
   try{window.renderMathInElement(el,{delimiters:[
    {left:"$$",right:"$$",display:true},
    {left:"\\(",right:"\\)",display:false},
    {left:"$",right:"$",display:false}
   ],throwOnError:false,strict:"ignore"})}catch(error){console.warn("Math parsing",error)}
 }
}
function equation(tex,label=""){
 const div=document.createElement("div");div.className="formula-block";div.tabIndex=0;div.setAttribute("role","region");div.setAttribute("aria-label",label?"数学公式："+label:"数学公式");
 if(label){const h=document.createElement("strong");h.textContent=label;div.appendChild(h)}
 const math=document.createElement("div");math.className="math";div.appendChild(math);
 if(mathReady()){
   try{window.katex.render(tex,math,{displayMode:true,throwOnError:false,strict:"ignore"})}
   catch(error){math.textContent=tex}
 }else{const c=document.createElement("code");c.textContent=tex;math.appendChild(c)}
 return div;
}
function makeSection(heading){
 const section=document.createElement("section"),h=document.createElement("h3");
 h.textContent=heading;section.appendChild(h);return section;
}
function paragraph(section,text,classname){
 const p=document.createElement("p");if(classname)p.className=classname;p.textContent=text;section.appendChild(p);typeset(p);return p;
}
function resourceArea(dept){
const section=makeSection("优质学习资源 · 官方课程 / 教材 / 视频");
const resources=document.createElement("div");resources.className="learnlinks";
for(const r of dept.resources||[]){
if(!/^https:\/\//.test(r.url))continue;
const a=document.createElement("a");a.className="learnlink";a.href=r.url;a.target="_blank";a.rel="noopener noreferrer";a.textContent=r.title;
const small=document.createElement("small");small.textContent=r.type||"公开教学资源";a.appendChild(small);resources.appendChild(a)}
section.appendChild(resources);
paragraph(section,"外部链接以高校官方课程、作者公开教材、课程视频或讲义入口为主。PDF/视频通过原始网站访问，不在本站未经授权复制。");
return section;
}
function isMatch(n){
 if(filterDept&&n.discipline_id!==filterDept)return false;
 if(filterLevel&&n.level!==filterLevel)return false;
 if(filterStatus&&(filterStatus==="complete")!==isComplete(n))return false;
 if(!query.trim())return true;
 const dept=getDept(n.discipline_id),chap=getChap(n.chapter_id);
 return [n.title,dept?.title,dept?.english,chap?.title].join(" ").toLocaleLowerCase().includes(query.trim().toLocaleLowerCase());
}
function nodeButton(n){
const b=document.createElement("button");b.className="node"+(n.id===activeId?" active":"");b.type="button";b.dataset.node=n.id;
const ok=isComplete(n);b.innerHTML='<span class="complete-mark" aria-hidden="true">'+(studied.has(n.id)?"✓":ok?"●":"·")+'</span><span class="node-name">'+esc(n.title)+'</span><span class="node-label'+(ok?" complete":"")+'">'+(ok?"讲义":"目录")+'</span>';
b.addEventListener("click",()=>selectNode(n.id));
return b;
}
function makeChapter(c,visible,open){
const out=document.createElement("details");out.className="chapter";out.open=open;
const summary=document.createElement("summary");
summary.innerHTML='<span class="chapter-label">'+String(c.order).padStart(2,"0")+'. '+esc(c.title)+'</span><span class="count">'+visible.length+' 点</span><span class="chev" aria-hidden="true">▸</span>';out.appendChild(summary);
const ul=document.createElement("div");ul.className="nodes";for(const n of visible)ul.appendChild(nodeButton(n));out.appendChild(ul);return out;
}
function renderTree(){
const container=$("tree");container.replaceChildren();
let visibleCount=0,departmentCount=0;const totalComplete=nodes.filter(isComplete).length;
for(const d of depts){
 const groups=[];
 for(const cid of d.chapter_ids){
   const c=getChap(cid);if(!c)continue;
   const relevant=c.node_ids.map(getNode).filter(Boolean).filter(isMatch);
   if(relevant.length)groups.push({chapter:c,nodes:relevant});
 }
 if(!groups.length)continue;departmentCount++;visibleCount+=groups.reduce((a,g)=>a+g.nodes.length,0);
 const whole=document.createElement("details");whole.className="department";whole.dataset.department=d.id;
 whole.open=d.id===(filterDept||getNode(activeId)?.discipline_id)||!!query;
 const summary=document.createElement("summary");
 const n=groups.reduce((acc,g)=>acc+g.nodes.length,0);
 summary.innerHTML='<span class="number">'+String(d.order).padStart(2,"0")+'</span><span class="dept-label"><strong>'+esc(d.title)+'</strong><span>'+esc(d.english)+'</span></span><span class="count">'+n+' 知识点</span><span class="chev" aria-hidden="true">▸</span>';
 whole.appendChild(summary);
 const chapterBox=document.createElement("div");chapterBox.className="chapters";
 for(const [i,g] of groups.entries())chapterBox.appendChild(makeChapter(g.chapter,g.nodes,g.chapter.id===getNode(activeId)?.chapter_id||!!query||Boolean(filterDept)));
 whole.appendChild(chapterBox);
 const resourceLine=document.createElement("div");resourceLine.className="dept-resources";
 for(const r of d.resources||[]){
  if(!/^https:\/\//.test(r.url))continue;
  const a=document.createElement("a");a.href=r.url;a.target="_blank";a.rel="noopener noreferrer";a.textContent="学习资料 · "+r.type;resourceLine.appendChild(a);
 }whole.appendChild(resourceLine);
 container.appendChild(whole);
}
$("status").textContent="显示 "+visibleCount+" / "+nodes.length+" 个知识点 · "+departmentCount+" 个学科 · "+totalComplete+" 个完整讲义";
if(!visibleCount){const p=document.createElement("p");p.className="notice";p.textContent="当前条件无匹配知识点。请清除搜索或筛选。";container.appendChild(p)}
$("mastered").textContent=studied.size;
document.querySelectorAll(".roadmap-chip").forEach(b=>b.classList.toggle("active",b.dataset.dept===filterDept));
}
function makeControls(node,wrap){
const section=document.createElement("div");section.className="tool-actions";
const done=document.createElement("button");done.className="action"+(studied.has(node.id)?" done":"");done.type="button";
done.textContent=studied.has(node.id)?"✓ 已标记学会 · 点击取消":"标记我已学会";
done.onclick=()=>{
 if(studied.has(node.id))studied.delete(node.id);else studied.add(node.id);
 saveProgress();done.classList.toggle("done",studied.has(node.id));
 done.textContent=studied.has(node.id)?"✓ 已标记学会 · 点击取消":"标记我已学会";
 renderTree();
};section.appendChild(done);
const copy=document.createElement("button");copy.className="action";copy.textContent="复制本节链接";copy.type="button";
copy.onclick=async()=>{const url=new URL(location.href);url.searchParams.set("topic",node.id);try{await navigator.clipboard.writeText(url.href);copy.textContent="链接已复制"}catch(e){copy.textContent="请复制浏览器地址"}};section.appendChild(copy);
const next=nodes.find(n=>n.discipline_id===node.discipline_id&&nodes.indexOf(n)>nodes.indexOf(node));
if(next){const b=document.createElement("button");b.className="action primary";b.textContent="下一知识点 →";b.type="button";b.onclick=()=>selectNode(next.id);section.appendChild(b)}
wrap.appendChild(section);
}
function renderMathDeepSections(node,lesson,container){
 const sections=lesson.deep_sections;
 if(!sections?.length)return;
 const nav=makeSection("深入阅读路径：每一步回答一个数学问题");
 const ol=document.createElement("ol");ol.className="math-deep-toc";
 for(const [i,part] of sections.entries()){
  const li=document.createElement("li"),button=document.createElement("button");
  button.type="button";button.textContent=part.title;
  button.addEventListener("click",()=>container.querySelector("#math-deep-"+node.id+"-"+i)?.scrollIntoView({behavior:"smooth",block:"start"}));
  li.appendChild(button);ol.appendChild(li);
 }
 nav.appendChild(ol);container.appendChild(nav);
 for(const [i,part] of sections.entries()){
  const box=makeSection((i+1)+". "+part.title);
  box.classList.add("math-deep-unit");box.id="math-deep-"+node.id+"-"+i;
  paragraph(box,part.question,"math-deep-question");
  for(const prose of part.paragraphs)paragraph(box,prose);
  for(const formula of part.formulas)box.appendChild(equation(formula.tex,formula.label));
  for(const [label,value] of [["推理与成立机制",part.proof],["逐项计算",part.worked],["不成立的情形",part.limitation]]){
   const h=document.createElement("h4");h.textContent=label;box.appendChild(h);
   paragraph(box,value,label==="不成立的情形"?"warn":undefined);
  }
  container.appendChild(box);
 }
 if(lesson.links?.length){
  const part=makeSection("知识链：先修与延伸");const row=document.createElement("div");row.className="math-deep-related";
  for(const ref of lesson.links){
   const a=document.createElement("a");a.textContent=ref.label;
   a.href=ref.kind==="ml"?"../ml/#lesson/"+encodeURIComponent(ref.id):"?topic="+encodeURIComponent(ref.id);
   row.appendChild(a);
  }
  part.appendChild(row);container.appendChild(part);
 }
 if(lesson.sources?.length){
  const part=makeSection("参考来源（原站阅读）");
  for(const source of lesson.sources){
   const a=document.createElement("a");a.textContent=source.title;a.href=source.url;
   a.target="_blank";a.rel="noopener noreferrer";a.className="math-deep-source";
   part.appendChild(a);
  }
  container.appendChild(part);
 }
}
function lessonArticle(n,lesson,container){
if(n.prerequisites?.length){
 const pre=makeSection("课程先修知识");const links=document.createElement("div");links.className="math-prerequisites";
 for(const id of n.prerequisites){
  const item=getNode(id);if(!item)continue;
  const button=document.createElement("button");button.type="button";button.className="action";button.textContent=item.title;
  button.addEventListener("click",()=>selectNode(id));links.appendChild(button);
 }
 pre.appendChild(links);container.appendChild(pre);
}
const lead=makeSection("一、研究对象与严格定义");
paragraph(lead,lesson.definition);container.appendChild(lead);
const assumptions=makeSection("二、前提条件与符号口径");paragraph(assumptions,lesson.assumptions);container.appendChild(assumptions);
const property=makeSection("三、重要性质及其成立机制");
const list=document.createElement("ol");list.className="property-list";
for(const item of lesson.properties||[]){const li=document.createElement("li");li.textContent=item;list.appendChild(li);typeset(li)}property.appendChild(list);container.appendChild(property);
if(lesson.formulas?.length){
const sec=makeSection("四、核心数学关系");for(const f of lesson.formulas)sec.appendChild(equation(f.tex,f.label));
container.appendChild(sec);}
const proof=makeSection("五、逐步推导与证明");
for(const [i,step] of (lesson.derivation||[]).entries()){
 const box=document.createElement("div");box.className="proof-step";
 const badge=document.createElement("span");badge.className="step-index";badge.textContent=i+1;
 const text=document.createElement("p");text.textContent=step.explanation;box.append(badge,text);
 typeset(text);
 const math=equation(step.tex);math.className="step-equation";box.appendChild(math);
 proof.appendChild(box)}
container.appendChild(proof);
const example=makeSection("六、独立数值例题");paragraph(example,lesson.example);container.appendChild(example);
const application=makeSection("七、量化研究与实际应用");paragraph(application,lesson.application);container.appendChild(application);
const pitfall=makeSection("八、失效边界与易错口径");
paragraph(pitfall,lesson.caveat,"warn");container.appendChild(pitfall);
renderMathDeepSections(n,lesson,container);
}
function outlineArticle(n,container){
const ch=getChap(n.chapter_id),dept=getDept(n.discipline_id);
const section=makeSection("本知识点的学习位置");
const c=document.createElement("p");c.textContent="此知识点位于「"+dept.title+" → "+ch.title+"」第 "+n.order+" 节，难度为 "+n.level+"。当前已纳入完整学习目录，但尚未编写独立的定义—性质—证明—例题讲义。";section.appendChild(c);
if(n.summary){
 const intro=makeSection("研究范围与学习目标");paragraph(intro,n.summary);container.appendChild(intro);
 const pre=makeSection("先修知识");const links=document.createElement("div");links.className="math-prerequisites";
 for(const id of n.prerequisites||[]){const item=getNode(id);if(!item)continue;
  const b=document.createElement("button");b.className="action";b.type="button";b.textContent=item.title;
  b.addEventListener("click",()=>selectNode(id));links.appendChild(b)}
 pre.appendChild(links);container.appendChild(pre);
}
const guide=document.createElement("div");guide.className="outline-note";
guide.textContent="当前状态：知识目录（未标记为完整讲义）。建议按本章节列示的顺序学习，先掌握前置章节，再阅读以下官方课件与教材。后续可以逐节增加严格定义、条件、LaTeX公式和完整推导。";
section.appendChild(guide);
const list=document.createElement("ol");list.className="outline-list";
const chapterNodes=ch.node_ids.map(getNode).filter(Boolean);
for(const topic of chapterNodes){const li=document.createElement("li");li.textContent=topic.title+(topic.id===n.id?"（当前知识点）":"");list.appendChild(li)}
section.appendChild(list);container.appendChild(section);
}
function appendLearningSequence(node,container){
 const chapter=getChap(node.chapter_id);
 if(!chapter)return;
 const position=chapter.node_ids.indexOf(node.id);
 if(position<0)return;
 const nextId=chapter.node_ids[position+1];
 const prevId=chapter.node_ids[position-1];
 const later=nodes.filter(n=>(n.prerequisites||[]).includes(node.id)&&n.id!==nextId).slice(0,4);
 const navigation=makeSection("学习衔接：从本节走向下一结论");
 navigation.classList.add("math-learning-sequence");
 const explain=document.createElement("p");
 explain.textContent="先掌握本节的定义与条件，再沿所在章节顺序继续；其他直接依赖本节的专题可作为延伸阅读。未完成的知识点会明确保留目录状态。";
 navigation.appendChild(explain);
 const row=document.createElement("div");row.className="math-learning-sequence-links";
 function add(target,label){
  const point=getNode(target);if(!point)return;
  const button=document.createElement("button");button.type="button";button.className="action";
  button.textContent=label+" · "+point.title+(lessonMap.has(target)?"":"（待扩写）");
  button.addEventListener("click",()=>selectNode(target));row.appendChild(button);
 }
 if(prevId)add(prevId,"上一节");
 if(nextId)add(nextId,"下一节");
 for(const topic of later)add(topic.id,"关联延伸");
 if(!row.childElementCount)return;
 navigation.appendChild(row);container.appendChild(navigation);
}
function selectNode(id,update=true){
 const node=getNode(id);if(!node)return;
 activeId=id;
 const dept=getDept(node.discipline_id),chapter=getChap(node.chapter_id);
 const article=$("article");article.replaceChildren();
 const heading=document.createElement("div");heading.className="breadcrumb";
 heading.textContent=dept.title+" / "+chapter.title+" / "+String(node.order).padStart(2,"0");article.appendChild(heading);
 const title=document.createElement("h2");title.textContent=node.title;article.appendChild(title);
 const badges=document.createElement("div");badges.className="lesson-state";
 badges.innerHTML='<span class="status-pill'+(isComplete(node)?"":" outline")+'">'+humanStatus(node)+'</span><span class="level-pill">'+esc(node.level)+'</span>';
 article.appendChild(badges);
 const lesson=lessonMap.get(id);
 if(lesson)lessonArticle(node,lesson,article);
 else outlineArticle(node,article);
 appendLearningSequence(node,article);
 article.appendChild(resourceArea(dept));
 makeControls(node,article);
 typeset(article);
 renderTree();
 if(update)setURL({topic:node.id});
 // No automatic window.scrollIntoView to avoid sidebar interaction bounce.
 $("articlePane").scrollTop=0;
}
window.mathSelectDiscipline=(id)=>{
 filterDept=deptMap.has(id)?id:"";
 $("disciplineFilter").value=filterDept;
 let target=getNode(activeId);
 if(filterDept&&target?.discipline_id!==filterDept){
   target=nodes.find(n=>n.discipline_id===filterDept&&lessonMap.has(n.id))||
          nodes.find(n=>n.discipline_id===filterDept);
 }
 if(target)selectNode(target.id,false);
 else renderTree();
 setURL({discipline:filterDept,topic:target?.id||null});
};
window.mathSelectTopic=(id)=>selectNode(id);
$("search").addEventListener("input",e=>{query=e.target.value;renderTree()});
$("disciplineFilter").addEventListener("change",e=>{window.mathSelectDiscipline(e.target.value)});
$("levelFilter").addEventListener("change",e=>{filterLevel=e.target.value;renderTree()});
$("coverageFilter").addEventListener("change",e=>{filterStatus=e.target.value;renderTree()});
$("resetFilters").onclick=()=>{
filterDept=filterLevel=filterStatus=query="";
$("disciplineFilter").value=$("levelFilter").value=$("coverageFilter").value=$("search").value="";
renderTree();setURL({discipline:null})
};
$("expandAll").onclick=()=>document.querySelectorAll("#tree details").forEach(d=>d.open=true);
$("collapseAll").onclick=()=>document.querySelectorAll("#tree details").forEach(d=>d.open=false);
async function fetchMathJson(path){
 let last=null;
 for(let attempt=0;attempt<3;attempt++){
  try{
   const res=await fetch(path,{cache:"no-cache"});
   if(!res.ok){
    const error=Error("HTTP "+res.status+"："+path);
    if(res.status!==429&&res.status<500)throw error;
    last=error;
   }else return await res.json();
  }catch(e){
   if(/^HTTP 4\d\d/.test(e.message)&&!e.message.startsWith("HTTP 429"))throw e;
   last=e;
  }
  if(attempt<2)await new Promise(done=>setTimeout(done,160*(attempt+1)));
 }
 throw Error("数学讲义下载失败："+path+"；"+last?.message);
}
async function loadMathAdditions(){
 const m=await fetchMathJson("./lessons-manifest.json");
 if(m.version!==1||!Array.isArray(m.modules)||m.modules.length<2||
   !Number.isInteger(m.expected_lessons)||m.expected_lessons<6)throw Error("数学讲义模块清单异常");
 const names=m.modules.map(x=>x.path);
 if(new Set(names).size!==names.length||
    m.modules.some(x=>!/^\.\/lessons-phase-\d\d-[a-z]\.json$/.test(x.path)||!Number.isInteger(x.lessons)||x.lessons<1)||
    m.modules.reduce((sum,x)=>sum+x.lessons,0)!==m.expected_lessons)throw Error("数学讲义模块路径或数量错误");
 const results=new Array(names.length);let cursor=0;
 await Promise.all(Array.from({length:Math.min(4,names.length)},async()=>{
   while(cursor<names.length){
    const i=cursor++,doc=await fetchMathJson(names[i]);
    if(doc.version!==1||!Array.isArray(doc.lessons)||doc.lessons.length!==m.modules[i].lessons)
     throw Error("数学讲义模块损坏："+names[i]);
    results[i]=doc.lessons;
   }
 }));
 const added=results.flat();
 if(added.length!==m.expected_lessons||new Set(added.map(x=>x.node_id)).size!==added.length)
  throw Error("数学讲义模块出现重复或缺页");
 return added;
}
async function loadV89Foundations(){
 const doc=await fetchMathJson("./lessons-foundations-v89.json");
 if(doc.version!==1||!Array.isArray(doc.lessons)||doc.lessons.length!==12||new Set(doc.lessons.map(l=>l.node_id)).size!==12)
  throw Error("V89基础数学完整讲义数量、版本或唯一ID异常");
 return doc.lessons;
}
async function loadV91FoundationDeepening(){
 const doc=await fetchMathJson("./lessons-foundations-v91.json");
 if(doc.version!==1||doc.revision!=="V91"||!Array.isArray(doc.extensions)||doc.extensions.length!==12||
    !Array.isArray(doc.patches)||doc.patches.length!==2||
    new Set(doc.extensions.map(x=>x.node_id)).size!==12||
    doc.extensions.some(x=>!Array.isArray(x.deep_sections)||x.deep_sections.length!==2||
       x.deep_sections.some(s=>!s.title||!s.proof||!s.worked||!s.limitation||
          !Array.isArray(s.formulas)||s.formulas.length<1||!Array.isArray(s.paragraphs)||s.paragraphs.length<2)))
  throw Error("V91深化讲义结构损坏或缺失");
 return doc;
}
function applyV91FoundationDeepening(v89,v91){
 const byId=new Map(v89.map(l=>[l.node_id,l]));
 if(byId.size!==12||v91.extensions.some(x=>!byId.has(x.node_id)||byId.get(x.node_id).title!==x.title))
  throw Error("V91扩展与V89正式课程ID/标题不一致");
 for(const ext of v91.extensions){
  const lesson=byId.get(ext.node_id);
  if(lesson.deep_sections.length!==4||
     ext.deep_sections.some(s=>lesson.deep_sections.some(original=>original.title===s.title)))
   throw Error("V91扩展基线或章节标题不一致："+ext.node_id);
  lesson.deep_sections.push(...ext.deep_sections);
 }
 const seenPatches=new Set();
 for(const patch of v91.patches){
  const lesson=byId.get(patch.node_id);
  if(!lesson||seenPatches.has(patch.node_id))throw Error("V91修订目标重复或缺失");
  seenPatches.add(patch.node_id);
  if(patch.kind==="formula"&&patch.node_id==="foundation-n0003"&&patch.index===3&&
     lesson.formulas[patch.index]?.label===patch.expected_label&&patch.formula?.tex){
    lesson.formulas[patch.index]=patch.formula;
  }else if(patch.kind==="deep_section"&&patch.node_id==="foundation-n0015"&&patch.index===1&&
     lesson.deep_sections[patch.index]?.title===patch.expected_title&&patch.section?.formulas?.length){
    lesson.deep_sections[patch.index]=patch.section;
  }else throw Error("V91补丁位置与历史讲义不符："+patch.node_id);
 }
 if([...byId.values()].some(l=>l.deep_sections.length!==6))throw Error("V91章节数量不一致");
}
Promise.all([
 fetch("./curriculum.json",{cache:"no-cache"}).then(r=>{if(!r.ok)throw Error("知识树加载失败 "+r.status);return r.json()}),
 fetch("./lessons-foundations.json",{cache:"no-cache"}).then(r=>{if(!r.ok)throw Error("基础讲义加载失败 "+r.status);return r.json()}),
 fetch("./lessons-advanced.json",{cache:"no-cache"}).then(r=>{if(!r.ok)throw Error("进阶讲义加载失败 "+r.status);return r.json()}),
 fetch("./lessons-more.json",{cache:"no-cache"}).then(r=>{if(!r.ok)throw Error("专题讲义加载失败 "+r.status);return r.json()}),
 loadMathAdditions(),
 loadV89Foundations(),
 loadV91FoundationDeepening()
]).then(([c,l1,l2,l3,extra,v89,v91])=>{
applyV91FoundationDeepening(v89,v91);
curriculum=c;depts=c.disciplines;chapters=c.chapters;nodes=c.nodes;lessons=[...l1.lessons,...l2.lessons,...l3.lessons,...extra,...v89];
nodeMap=new Map(nodes.map(n=>[n.id,n]));chapMap=new Map(chapters.map(c=>[c.id,c]));deptMap=new Map(depts.map(d=>[d.id,d]));lessonMap=new Map(lessons.map(l=>[l.node_id,l]));
if(lessonMap.size!==lessons.length||lessons.some(l=>!nodeMap.has(l.node_id)||nodeMap.get(l.node_id).title!==l.title))
 throw Error("数学讲义ID重复、目标缺失或标题不匹配");
$("disciplines").textContent=depts.length;$("chapters").textContent=chapters.length;$("nodes").textContent=nodes.length;$("written").textContent=lessonMap.size;
for(const d of depts){
 const option=document.createElement("option");option.value=d.id;option.textContent=String(d.order).padStart(2,"0")+" · "+d.title;$("disciplineFilter").appendChild(option);
 const chip=document.createElement("button");chip.type="button";chip.className="roadmap-chip";chip.dataset.dept=d.id;chip.textContent=d.order+" · "+d.title;chip.onclick=()=>window.mathSelectDiscipline(filterDept===d.id?"":d.id);$("roadmap").appendChild(chip);
}
const params=new URLSearchParams(location.search);
const d=params.get("discipline"),requested=params.get("topic"),status=params.get("status");
if(d&&deptMap.has(d)){filterDept=d;$("disciplineFilter").value=d}
if(["complete","outline"].includes(status)){filterStatus=status;$("coverageFilter").value=status}
const defaultNode=nodes.find(n=>n.discipline_id==="calculus"&&n.title==="链式法则")||nodes.find(n=>lessonMap.has(n.id))||nodes[0];
const initial=(nodeMap.has(requested)?getNode(requested):null) ||
  (filterDept?nodes.find(n=>n.discipline_id===filterDept&&lessonMap.has(n.id))||nodes.find(n=>n.discipline_id===filterDept):null) ||
  defaultNode;
selectNode(initial.id,false);
}).catch(error=>{$("status").textContent="加载失败："+error.message;const div=document.createElement("p");div.className="notice";div.textContent="检查网络或站点数据文件： "+error.message;$("tree").appendChild(div)});
})();
