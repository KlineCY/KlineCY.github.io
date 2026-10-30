/* Quant Machine Learning — proof, code, limitations and practical ML topics. */
(()=>{
"use strict";
const $=id=>document.getElementById(id);
const esc=s=>String(s??"").replace(/[&<>"']/g,x=>({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[x]));
const alpha=(a,b)=>String(a).localeCompare(String(b),"en",{sensitivity:"base"});
const link=(u,label)=>/^https:\/\//.test(u||"")?'<a href="'+esc(u)+'" target="_blank" rel="noopener noreferrer">'+esc(label)+'</a>':"";
let curriculum=null,depts=[],chapters=[],nodes=[],lessons=[],lessonMap=new Map(),noteMap=new Map();
let nodeMap=new Map(),chapMap=new Map(),deptMap=new Map(),activeId="",filterDept="",filterLevel="",filterStatus="",query="";
const progressKey="kline-quant-ml-progress-v1";
let studied=new Set();
try{const saved=JSON.parse(localStorage.getItem(progressKey)||"[]");if(Array.isArray(saved))studied=new Set(saved)}catch(_){}
const saveProgress=()=>{try{localStorage.setItem(progressKey,JSON.stringify([...studied]))}catch(_){}};
const getDept=id=>deptMap.get(id),getChap=id=>chapMap.get(id),getNode=id=>nodeMap.get(id);
const humanStatus=node=>lessonMap.has(node.id)?"完整算法讲义":noteMap.has(node.id)?"专题扩展笔记 · 待补全":"课程目录";
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
 const div=document.createElement("div");div.className="formula-block";
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
b.addEventListener("click",()=>selectNode(n.id,true,true));
return b;
}
function makeChapter(c,visible,open){
const out=document.createElement("details");out.className="chapter";out.open=open;
const summary=document.createElement("summary");
summary.innerHTML='<span class="chapter-label">'+String(c.order).padStart(2,"0")+'. '+esc(c.title)+'</span><span class="count">'+visible.filter(isComplete).length+'/'+visible.length+' 讲义</span><span class="chev" aria-hidden="true">▸</span>';out.appendChild(summary);
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
 const finished=groups.reduce((acc,g)=>acc+g.nodes.filter(isComplete).length,0);
 summary.innerHTML='<span class="number">'+String(d.order).padStart(2,"0")+'</span><span class="dept-label"><strong>'+esc(d.title)+'</strong><span>'+esc(d.english)+'</span></span><span class="count">'+finished+'/'+n+' 讲义</span><span class="chev" aria-hidden="true">▸</span>';
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
$("status").textContent="显示 "+visibleCount+" / "+nodes.length+" 个知识点 · "+departmentCount+" 个学科 · "+totalComplete+" 个算法讲义";
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
function lessonArticle(n,lesson,container){
 const intro=makeSection("一、概念与工作原理");
 paragraph(intro,lesson.definition);container.appendChild(intro);
 const assumptions=makeSection("二、前置条件与适用假设");
 paragraph(assumptions,lesson.assumptions);container.appendChild(assumptions);
 const property=makeSection("三、算法性质与适用范围");
 const list=document.createElement("ol");list.className="property-list";
 for(const item of lesson.properties||[]){const li=document.createElement("li");li.textContent=item;list.appendChild(li);typeset(li)}
 property.appendChild(list);container.appendChild(property);
 if(lesson.formulas?.length){
  const sec=makeSection("四、数学公式");
  for(const f of lesson.formulas)sec.appendChild(equation(f.tex,f.label));
  container.appendChild(sec);
 }
 const proof=makeSection("五、原理推导");
 for(const [i,step] of (lesson.derivation||[]).entries()){
  const box=document.createElement("div");box.className="proof-step";
  const badge=document.createElement("span");badge.className="step-index";badge.textContent=i+1;
  const txt=document.createElement("p");txt.textContent=step.explanation;box.append(badge,txt);
  typeset(txt);
  const math=equation(step.tex);math.className="step-equation";box.appendChild(math);
  proof.appendChild(box);
 }
 container.appendChild(proof);
 const codeSection=makeSection("六、基础 Python 代码");
 const pkg=document.createElement("p");pkg.className="code-caption";
 pkg.textContent="示例依赖："+(lesson.packages||"numpy")+" · 最小教学示例，使用合成数据或固定数值；仅在具备相应依赖的Python环境运行。";
 codeSection.appendChild(pkg);
 const wrap=document.createElement("div");wrap.className="code-wrap";
 const pre=document.createElement("pre"),code=document.createElement("code");
 code.className="language-python";code.textContent=lesson.code||"# 暂无示例";
 pre.appendChild(code);wrap.appendChild(pre);
 const copy=document.createElement("button");copy.className="copy-code";copy.type="button";copy.textContent="复制代码";
 copy.onclick=async()=>{
   try{await navigator.clipboard.writeText(lesson.code);copy.textContent="✓ 已复制"}
   catch(e){copy.textContent="复制不可用，请手动选择代码"}
 };
 wrap.appendChild(copy);codeSection.appendChild(wrap);
 const footer=document.createElement("p");footer.className="code-caption";
 footer.textContent="此站为静态GitHub Pages，代码不会在网页或后台自动执行；可复制后在本地Python或Colab运行。";
 codeSection.appendChild(footer);container.appendChild(codeSection);
 const quant=makeSection("七、量化金融应用");
 paragraph(quant,lesson.application);container.appendChild(quant);
 const limitations=makeSection("八、局限、失效条件与常见误区");
 paragraph(limitations,lesson.limits,"warn");container.appendChild(limitations);
}
function outlineArticle(n,container){
 const ch=getChap(n.chapter_id),dept=getDept(n.discipline_id);
 const section=makeSection("课程位置与前置学习");
 paragraph(section,"该知识点位于「"+dept.title+" → "+ch.title+"」第"+n.order+"节。目前尚未完成该知识点的独立算法讲义；目录或相邻讲义不能替代它的完整证明。");
 const note=document.createElement("div");note.className="outline-note";
 note.textContent="待补充：严格定义、适用条件、算法步骤、LaTeX公式与推导、Python最小实现、量化应用和局限。以下列出章节学习次序及已完成的关联讲义。";
 section.appendChild(note);
 const list=document.createElement("ol");list.className="outline-list";
 for(const id of ch.node_ids){
  const topic=getNode(id);if(!topic)continue;
  const li=document.createElement("li");
  if(topic.id===n.id)li.textContent=topic.title+"（当前）";
  else if(lessonMap.has(topic.id)){
   const button=document.createElement("button");button.type="button";button.className="action";
   button.textContent=topic.title+" · 已有讲义";
   button.onclick=()=>selectNode(topic.id,true,false);
   li.appendChild(button);
  }else li.textContent=topic.title+"（课程目录）";
  list.appendChild(li);
 }
 section.appendChild(list);container.appendChild(section);
 const pos=nodes.findIndex(x=>x.id===n.id);
 const near=nodes.filter(x=>x.discipline_id===n.discipline_id&&lessonMap.has(x.id)&&x.id!==n.id)
 .sort((x,y)=>Math.abs(nodes.indexOf(x)-pos)-Math.abs(nodes.indexOf(y)-pos)).slice(0,5);
 if(near.length){
   const related=makeSection("相关的已完成讲义");
   paragraph(related,"可先学习以下邻近主题，再回到该知识点阅读官方课程资料。");
   const links=document.createElement("div");links.className="learnlinks";
   for(const item of near){
     const btn=document.createElement("button");btn.className="action";btn.type="button";
     btn.textContent=item.title;btn.onclick=()=>selectNode(item.id,true,false);
     links.appendChild(btn);
   }related.appendChild(links);container.appendChild(related);
 }
}
function selectNode(id,update=true,preserveTree=false){
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
 else if(noteMap.has(id))lessonArticle(node,noteMap.get(id),article);
 else outlineArticle(node,article);
 article.appendChild(resourceArea(dept));
 makeControls(node,article);
 typeset(article);
 if(preserveTree){
   $("tree").querySelectorAll(".node").forEach(el=>el.classList.toggle("active",el.dataset.node===node.id));
 }else renderTree();
 if(update)setURL({topic:node.id});
 // No automatic window.scrollIntoView to avoid sidebar interaction bounce.
 $("articlePane").scrollTop=0;
}
window.mlSelectDiscipline=(id)=>{
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
window.mlSelectTopic=(id)=>selectNode(id);
$("search").addEventListener("input",e=>{query=e.target.value;renderTree()});
$("disciplineFilter").addEventListener("change",e=>{window.mlSelectDiscipline(e.target.value)});
$("levelFilter").addEventListener("change",e=>{filterLevel=e.target.value;renderTree()});
$("coverageFilter").addEventListener("change",e=>{filterStatus=e.target.value;renderTree()});
$("resetFilters").onclick=()=>{
filterDept=filterLevel=filterStatus=query="";
$("disciplineFilter").value=$("levelFilter").value=$("coverageFilter").value=$("search").value="";
renderTree();setURL({discipline:null})
};
$("expandAll").onclick=()=>document.querySelectorAll("#tree details").forEach(d=>d.open=true);
$("collapseAll").onclick=()=>document.querySelectorAll("#tree details").forEach(d=>d.open=false);
Promise.all([
  fetch("./curriculum.json").then(r=>{if(!r.ok)throw Error("课程目录: HTTP "+r.status);return r.json()}),
  fetch("./manifest.json").then(r=>{if(!r.ok)throw Error("讲义目录: HTTP "+r.status);return r.json()})
]).then(async([c,manifest])=>{
  const results=await Promise.all(manifest.lesson_files.map(async file=>{
    const r=await fetch("./"+file);
    if(!r.ok)throw Error(file+": HTTP "+r.status);
    return r.json();
  }));
  curriculum=c;depts=c.disciplines;chapters=c.chapters;nodes=c.nodes;
  lessons=results.flatMap(x=>x.lessons);
  if(lessons.length!==manifest.total_lessons)throw Error("讲义清单与实际数据数量不一致");
  const draftResults=await Promise.all((manifest.note_files||[]).map(async file=>{
    const r=await fetch("./"+file);
    if(!r.ok)throw Error(file+": HTTP "+r.status);
    return r.json();
  }));
  const drafts=draftResults.flatMap(x=>x.notes||[]);
  if(drafts.length!==manifest.notes_count)throw Error("扩展笔记清单数量不一致");
  noteMap=new Map(drafts.filter(n=>!lessons.some(l=>l.node_id===n.node_id)).map(n=>[n.node_id,n]));

nodeMap=new Map(nodes.map(n=>[n.id,n]));chapMap=new Map(chapters.map(c=>[c.id,c]));deptMap=new Map(depts.map(d=>[d.id,d]));lessonMap=new Map(lessons.map(l=>[l.node_id,l]));
$("disciplines").textContent=depts.length;$("chapters").textContent=chapters.length;$("nodes").textContent=nodes.length;$("written").textContent=lessonMap.size;
for(const d of depts){
 const option=document.createElement("option");option.value=d.id;option.textContent=String(d.order).padStart(2,"0")+" · "+d.title;$("disciplineFilter").appendChild(option);
 const chip=document.createElement("button");chip.type="button";chip.className="roadmap-chip";chip.dataset.dept=d.id;chip.textContent=d.order+" · "+d.title;chip.onclick=()=>window.mlSelectDiscipline(filterDept===d.id?"":d.id);$("roadmap").appendChild(chip);
}
const params=new URLSearchParams(location.search);
const d=params.get("discipline"),requested=params.get("topic"),status=params.get("status");
if(d&&deptMap.has(d)){filterDept=d;$("disciplineFilter").value=d}
if(["complete","outline"].includes(status)){filterStatus=status;$("coverageFilter").value=status}
const defaultNode=nodes.find(n=>n.discipline_id==="roadmap"&&n.title==="经验风险最小化ERM")||nodes.find(n=>lessonMap.has(n.id))||nodes[0];
const initial=(nodeMap.has(requested)?getNode(requested):null) ||
  (filterDept?nodes.find(n=>n.discipline_id===filterDept&&lessonMap.has(n.id))||nodes.find(n=>n.discipline_id===filterDept):null) ||
  defaultNode;
selectNode(initial.id,false);
}).catch(error=>{$("status").textContent="加载失败："+error.message;const div=document.createElement("p");div.className="notice";div.textContent="检查网络或站点数据文件： "+error.message;$("tree").appendChild(div)});
})();
