/* Kline Quant Wiki — locally browsable data; no server-side dependencies. */
(() => {
"use strict";
const $=id=>document.getElementById(id);
const esc=s=>String(s??"").replace(/[&<>"']/g,c=>({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[c]));
const alpha=(a,b)=>String(a).localeCompare(String(b),"en",{sensitivity:"base"});
let terms=[],categories=[],paths=[],byId=new Map(),filterCategory="",filterPath="",search="",filterLevel="";
const summaryItems=[],categoryState=new Map();
const modal=$("termDialog"),body=$("termBody");
const catBlock=(cat,subset,open)=>{
const root=document.createElement("details");root.className="cat";root.dataset.category=cat.id;root.open=open;
const title=document.createElement("summary");title.innerHTML='<div class="catname"><b>'+esc(cat.name)+'</b><small>'+esc(cat.english)+'</small></div><span class="count">'+subset.length+' 词条</span><span class="chev" aria-hidden="true">▸</span>';root.appendChild(title);
const inner=document.createElement("div");inner.className="cat-content";const grid=document.createElement("div");grid.className="entries";
for(const term of subset){
const card=document.createElement("article");card.className="entry";
card.innerHTML='<div><h3>'+esc(term.title)+'</h3><div class="en">'+esc(term.english)+'</div></div><p>'+esc(term.definition)+'</p><div class="bottom"><span class="level">'+esc(term.level)+'</span><button type="button" data-term="'+esc(term.id)+'">查看解释、示例与关联 →</button></div>';
grid.appendChild(card);
}
inner.appendChild(grid);root.appendChild(inner);
root.addEventListener("toggle",()=>{categoryState.set(cat.id,root.open)});
return root;
};
function updateURL(changes){
const url=new URL(location.href);
for(const [key,value] of Object.entries(changes)){
if(value)url.searchParams.set(key,value);else url.searchParams.delete(key);
}
history.replaceState(null,"",url.pathname+url.search+url.hash);
}
function updateActive(){
const sidebar=$("quantSidebar");
if(!sidebar)return;
for(const link of sidebar.querySelectorAll("#wikiCategories a,#wikiPaths a")){
const href=new URL(link.href,location.href),cid=href.searchParams.get("category"),pid=href.searchParams.get("path");
link.classList.toggle("chosen",!!((cid&&cid===filterCategory)||(pid&&pid===filterPath)));
}
}
function countSummary(list){
$("visibleCount").textContent=list.length;
$("status").textContent="显示 "+list.length+" / "+terms.length+" 个词条 · 各主题内按英文名称 A–Z 排列";
}
function render(){
const q=search.trim().toLocaleLowerCase();
let list=terms.filter(t=>{
if(filterCategory&&t.category_id!==filterCategory)return false;
if(filterPath){const path=paths.find(p=>p.id===filterPath);if(!path?.term_ids?.includes(t.id))return false}
if(filterLevel&&t.level!==filterLevel)return false;
if(q&&!([t.title,t.english,t.definition,t.example,t.pitfall,t.formula,categoryName(t.category_id)].join(" ").toLocaleLowerCase().includes(q)))return false;
return true;
});
countSummary(list);
const index=new Map();for(const t of list){const group=index.get(t.category_id)||[];group.push(t);index.set(t.category_id,group)}
const root=$("catalog");root.replaceChildren();
const arranged=[...categories].sort((a,b)=>alpha(a.english,b.english));
let first=true;
for(const cat of arranged){
const arr=index.get(cat.id);if(!arr?.length)continue;
arr.sort((a,b)=>alpha(a.english,b.english));
const open=search.trim()||filterCategory||filterPath?true:(categoryState.has(cat.id)?categoryState.get(cat.id):first);
root.appendChild(catBlock(cat,arr,!!open));first=false;
}
if(!list.length){const el=document.createElement("p");el.className="empty";el.textContent="没有匹配的术语。请清除关键词或筛选条件。";root.appendChild(el)}
for(const p of $("pathCards").querySelectorAll("[data-path]"))p.classList.toggle("active",!!filterPath&&p.dataset.path===filterPath);
updateActive();
}
function categoryName(id){return categories.find(c=>c.id===id)?.name||""}
function setCategory(value,update=true){
filterCategory=categories.some(c=>c.id===value)?value:"";
filterPath="";
$("categorySelect").value=filterCategory;
$("levelSelect").value=filterLevel;
if(update)updateURL({category:filterCategory,path:null});
render();
}
function setPath(value,update=true){
const p=paths.find(x=>x.id===value);filterPath=p?.id||"";filterCategory="";
$("categorySelect").value="";
if(update)updateURL({path:filterPath,category:null});
render();
}
window.wikiSetCategory=(id)=>setCategory(id);
window.wikiSetPath=(id)=>setPath(id);
function closeModal(){if(modal.open)modal.close();updateURL({term:null})}
function openTerm(id,update=true){
const item=byId.get(id);if(!item)return;
const cat=categories.find(c=>c.id===item.category_id);
const related=item.related_ids.map(i=>byId.get(i)).filter(Boolean).filter(t=>t.id!==id);
body.innerHTML=
'<div class="modal-top"><div class="modal-title"><div class="eyebrow">'+esc(cat?.name||"量化百科")+" · "+esc(item.level)+'</div><h2>'+esc(item.title)+'</h2><div class="modal-en">'+esc(item.english)+'</div></div><button class="close" id="modalClose" type="button">关闭 ×</button></div>'+
'<section class="detail"><h3>概念定义</h3><p>'+esc(item.definition)+'</p></section>'+
(item.formula?'<section class="detail"><h3>常用公式 / 数学表达</h3><div class="formula">'+esc(item.formula)+'</div></section>':"")+
'<section class="detail"><h3>量化研究中的例子</h3><p>'+esc(item.example)+'</p></section>'+
'<section class="detail"><h3>常见误区与注意事项</h3><p>'+esc(item.pitfall)+'</p></section>'+
(related.length?'<section class="detail"><h3>关联术语</h3><div class="related">'+related.map(x=>'<button data-related="'+esc(x.id)+'" type="button">'+esc(x.title)+'</button>').join("")+'</div></section>':"")+
'<section class="detail"><h3>扩展阅读</h3><p><a href="'+esc(/^https:\/\//.test(item.reference_url)?item.reference_url:"#")+'" target="_blank" rel="noopener noreferrer">查看相关官方文档或学术参考 ↗</a></p><p class="footnote">链接指向该主题的参考资源，不表示该外部机构逐条审阅了本站原创解释。</p></section>'+
'<section class="detail"><button class="copy" id="copyTerm" type="button">复制此词条链接</button></section>';
$("modalClose").onclick=closeModal;
$("copyTerm").onclick=async()=>{
const link=new URL(location.href);link.searchParams.set("term",id);
try{await navigator.clipboard.writeText(link.href);$("copyTerm").textContent="链接已复制";}catch(e){$("copyTerm").textContent="复制不可用，可复制浏览器地址";}
};
if(!modal.open)modal.showModal();
if(update)updateURL({term:id});
}
body.addEventListener("click",event=>{const b=event.target.closest("[data-related]");if(b)openTerm(b.dataset.related)});
$("catalog").addEventListener("click",event=>{const b=event.target.closest("[data-term]");if(b)openTerm(b.dataset.term)});
modal.addEventListener("close",()=>{if(new URL(location.href).searchParams.has("term"))updateURL({term:null})});
$("search").addEventListener("input",event=>{search=event.target.value;render()});
$("categorySelect").addEventListener("change",event=>setCategory(event.target.value));
$("levelSelect").addEventListener("change",event=>{filterLevel=event.target.value;render()});
$("reset").onclick=()=>{search="";filterLevel="";filterPath="";filterCategory="";$("search").value="";$("levelSelect").value="";$("categorySelect").value="";updateURL({path:null,category:null,term:null});render()};
$("expandAll").onclick=()=>$("catalog").querySelectorAll(".cat").forEach(d=>d.open=true);
$("collapseAll").onclick=()=>$("catalog").querySelectorAll(".cat").forEach(d=>d.open=false);
$("pathCards").addEventListener("click",e=>{const b=e.target.closest("[data-path]");if(b)setPath(b.dataset.path===filterPath?"":b.dataset.path)});
Promise.all(["manifest.json","part1.json","part2.json"].map(f=>fetch("./"+f).then(r=>{if(!r.ok)throw Error(f+": HTTP "+r.status);return r.json()}))).then(([manifest,...volumes])=>{
categories=volumes.flatMap(v=>v.categories);
terms=volumes.flatMap(v=>v.terms);
paths=manifest.learning_paths||[];
byId=new Map(terms.map(t=>[t.id,t]));
$("total").textContent=terms.length;$("topics").textContent=categories.length;$("pathN").textContent=paths.length;
const order=[...categories].sort((a,b)=>alpha(a.english,b.english));
for(const c of order)$("categorySelect").add(new Option(c.name,c.id));
$("pathCards").innerHTML=paths.map(p=>'<button type="button" class="path-card" data-path="'+esc(p.id)+'"><div class="count">'+p.term_ids.length+' 个核心概念</div><b>'+esc(p.title)+'</b><small>'+esc(p.description)+'</small></button>').join("");
const params=new URLSearchParams(location.search),path=params.get("path"),cat=params.get("category");
filterLevel=["初级","中级","高级"].includes(params.get("level"))?params.get("level"):"";
$("levelSelect").value=filterLevel;
if(path&&paths.some(x=>x.id===path))setPath(path,false);
else setCategory(cat||"",false);
const initial=params.get("term");if(initial&&byId.has(initial))openTerm(initial,false);
}).catch(error=>{$("status").textContent="量化百科数据读取失败："+error.message});
})();