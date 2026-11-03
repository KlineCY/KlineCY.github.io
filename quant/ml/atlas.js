(()=>{
"use strict";
const $=id=>document.getElementById(id);
const esc=x=>String(x??"").replace(/[&<>"']/g,c=>({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[c]));
const LESSON=/^#(?:lesson|ml)\/(m\d\d-c\d\d-p\d\d)$/;
const KEY="kline-quant-atlas-read-v1";
const state={index:[],byId:new Map(),domains:{},term:"",topic:"",activeId:"",read:new Set()};
try {const saved=JSON.parse(localStorage.getItem(KEY)||"[]");if(Array.isArray(saved))state.read=new Set(saved.filter(v=>typeof v==="string"))}catch(_){}
function saveRead(){try{localStorage.setItem(KEY,JSON.stringify([...state.read]))}catch(_){}}
function showStatus(message,error=false){const el=$("atlas-status");el.textContent=message;el.classList.toggle("atlas-error",error)}
function parseID(){return (location.hash.match(LESSON)||[])[1]||""}
function setTopic(v,updateUrl=true){
 state.topic=state.index.some(x=>x.domain_id===v)?v:"";
 $("atlas-topic").value=state.topic;
 if(updateUrl){
  const u=new URL(location.href);
  if(state.topic)u.searchParams.set("discipline",state.topic);
  else u.searchParams.delete("discipline");
  history.replaceState(null,"",u.pathname+u.search+u.hash);
 }
 renderDirectory();
}
window.mlSelectDiscipline=id=>setTopic(id,false);
async function loadArchive(){
 if(typeof DecompressionStream==="undefined")throw Error("当前浏览器不支持gzip解压，请使用新版Chrome、Edge、Safari或Firefox");
 const buffers=await Promise.all([1,2,3,4].map(async n=>{
  const r=await fetch("./atlas/data.part"+n);
  if(!r.ok)throw Error("讲义数据块"+n+"加载失败（HTTP "+r.status+"）");
  return new Uint8Array(await r.arrayBuffer());
 }));
 const total=buffers.reduce((s,b)=>s+b.length,0);
 if(total!==1969862)throw Error("讲义数据长度不匹配："+total);
 const data=new Uint8Array(total);let pos=0;
 for(const b of buffers){data.set(b,pos);pos+=b.length}
 const unzipped=new Blob([data]).stream().pipeThrough(new DecompressionStream("gzip"));
 const parsed=JSON.parse(await new Response(unzipped).text());
 if(parsed.catalog?.items?.length!==720||Object.keys(parsed.domains||{}).length!==24)
  throw Error("课程数量不完整：应有720篇讲义和24专题");
 const ids=new Set();
 for(const row of parsed.catalog.items){
  if(ids.has(row.id)||!parsed.domains[row.domain_id]?.[row.id])throw Error("讲义ID重复或正文缺失："+row.id);
  ids.add(row.id);
 }
 return parsed;
}
function visibleLessons(){
 return state.index.filter(x=>(!state.topic||x.domain_id===state.topic)&&
 (!state.term||[x.id,x.title,x.domain,x.chapter].some(v=>String(v||"").toLocaleLowerCase().includes(state.term))));
}
function updateReadStats(){
 const n=state.index.filter(x=>state.read.has(x.id)).length;
 const el=$("atlas-read-count");if(el)el.textContent=n+" / 720 篇已标记";
 const b=$("atlas-read-toggle");
 if(b&&state.activeId){const done=state.read.has(state.activeId);b.setAttribute("aria-pressed",String(done));b.textContent=done?"已学 · 点击取消":"标记已学";}
}
function renderDirectory(){
 const matches=visibleLessons();
 showStatus("显示 "+matches.length+" / "+state.index.length+" 篇讲义 · "+matches.filter(x=>state.read.has(x.id)).length+" 篇已学");
 const grouped=new Map();
 for(const x of matches){
  const key=x.domain_id+"_"+x.chapter;
  if(!grouped.has(key))grouped.set(key,[]);
  grouped.get(key).push(x);
 }
 const html=[...grouped.values()].map(rows=>{
  const active=rows.some(x=>x.id===state.activeId);
  const open=state.term.length>0||active;
  const count=rows.filter(x=>state.read.has(x.id)).length;
  return '<details class="atlas-section"'+(open?' open':'')+'><summary>'+esc(rows[0].domain)+" / "+esc(rows[0].chapter)+
   '<span class="atlas-chapter-count">'+count+"/"+rows.length+'</span></summary><div class="atlas-chapter-items">'+
   rows.map(x=>'<a href="#lesson/'+x.id+'" data-id="'+x.id+'"'+(x.id===state.activeId?' class="active" aria-current="page"':'')+'>'+
    (state.read.has(x.id)?'<span class="atlas-done-mark" aria-label="已学">✓ </span>':'')+esc(x.title)+'</a>').join("")+
   "</div></details>";
 }).join("");
 $("atlas-toc").innerHTML=html||"<p>未找到匹配的讲义。可尝试其他关键词。</p>";
 updateReadStats();
}
function fixInlineMarkdown(root){
 const nodes=[];
 const walk=document.createTreeWalker(root,NodeFilter.SHOW_TEXT);
 while(walk.nextNode()){
  const node=walk.currentNode;
  if(node.textContent.includes("**")&&!node.parentElement?.closest("pre,code,.atlas-tex,script,style"))nodes.push(node);
 }
 for(const node of nodes){
  const s=node.textContent,rx=/\*\*([^*\n]{1,180})\*\*/g;
  let m,last=0,changed=false;const frag=document.createDocumentFragment();
  while((m=rx.exec(s))){
   if(m.index>last)frag.appendChild(document.createTextNode(s.slice(last,m.index)));
   const strong=document.createElement("strong");strong.textContent=m[1];frag.appendChild(strong);
   last=rx.lastIndex;changed=true;
  }
  if(changed){if(last<s.length)frag.appendChild(document.createTextNode(s.slice(last)));node.replaceWith(frag)}
 }
}
function renderMath(root){
 root.querySelectorAll(".atlas-tex").forEach(el=>{
  const tex=el.getAttribute("data-tex")||"";
  if(window.katex){
   try{window.katex.render(tex,el,{displayMode:!!el.closest(".ml-math-block"),throwOnError:false,strict:"ignore"});return}
   catch(e){console.warn("KaTeX error",e)}
  }
  el.textContent=tex;
 });
}
function jump(id){if(state.byId.has(id))location.hash="lesson/"+id}
function display(){
 const id=parseID()||state.index[0]?.id;
 if(!id)return;
 const root=$("atlas-lesson"),meta=state.byId.get(id);
 if(!meta){root.innerHTML='<p class="atlas-error">找不到该讲义：'+esc(id)+'</p>';return}
 const data=state.domains[meta.domain_id]?.[id];
 if(!data){root.innerHTML='<p class="atlas-error">讲义正文缺失：'+esc(id)+'</p>';return}
 state.activeId=id;
 const position=state.index.findIndex(x=>x.id===id);
 const prev=state.index[position-1],next=state.index[position+1];
 const actions='<div class="atlas-lesson-actions">'+
  '<button id="atlas-prev" type="button"'+(!prev?' disabled':'')+'>上一篇</button>'+
  '<button id="atlas-next" type="button"'+(!next?' disabled':'')+'>下一篇</button>'+
  '<button id="atlas-copy-link" type="button">复制本文链接</button>'+
  '<button id="atlas-read-toggle" type="button" aria-pressed="false">标记已学</button></div>';
 root.innerHTML='<div class="atlas-crumb">'+esc(meta.domain)+" / "+esc(meta.chapter)+" / "+esc(id)+
  '</div><h1>'+esc(data.title)+'</h1>'+actions+
  '<div class="article">'+data.html+'</div><p class="atlas-note">系统教材 · 公式、证明和代码请按对应条件使用；示例不构成投资建议。</p>';
 $("atlas-prev").addEventListener("click",()=>prev&&jump(prev.id));
 $("atlas-next").addEventListener("click",()=>next&&jump(next.id));
 $("atlas-copy-link").addEventListener("click",async()=>{
  const url=new URL(location.href);url.hash="lesson/"+id;
  await copyText(url.href,$("atlas-copy-link"),"已复制链接");
 });
 $("atlas-read-toggle").addEventListener("click",()=>{
  if(state.read.has(id))state.read.delete(id);else state.read.add(id);
  saveRead();renderDirectory();
 });
 fixInlineMarkdown(root.querySelector(".article"));
 renderMath(root.querySelector(".article"));
 renderDirectory();
 if(location.hash)root.scrollIntoView({block:"start",behavior:"auto"});
}
async function copyText(value,button,label){
 const old=button.textContent;
 try{
  if(!navigator.clipboard?.writeText)throw Error("Clipboard API unavailable");
  await navigator.clipboard.writeText(value);
  button.textContent=label;
 }catch(e){
  const text=document.createElement("textarea");text.value=value;
  document.body.appendChild(text);text.select();
  const ok=document.execCommand?.("copy");text.remove();
  button.textContent=ok?label:"复制失败：请手动选中复制";
 }
 setTimeout(()=>{if(button.isConnected)button.textContent=old},1600);
}
async function start(){
 try{
  const data=await loadArchive();
  state.index=data.catalog.items;state.byId=new Map(state.index.map(x=>[x.id,x]));state.domains=data.domains;
  const groups=new Set();
  for(const x of state.index){
   if(groups.has(x.domain_id))continue;
   groups.add(x.domain_id);
   const o=document.createElement("option");o.value=x.domain_id;o.textContent=x.domain;
   $("atlas-topic").appendChild(o);
  }
  const topic=new URL(location.href).searchParams.get("discipline")||"";
  state.topic=groups.has(topic)?topic:"";$("atlas-topic").value=state.topic;
  $("atlas-search").addEventListener("input",e=>{state.term=e.target.value.trim().toLocaleLowerCase();renderDirectory()});
  $("atlas-topic").addEventListener("change",e=>setTopic(e.target.value));
  $("atlas-clear").addEventListener("click",()=>{state.term="";$("atlas-search").value="";setTopic("");});
  $("atlas-lesson").addEventListener("click",e=>{
   const button=e.target.closest("[data-ml-copy-point-code]");
   if(!button)return;
   const el=button.closest(".ml-point-code-wrap")?.querySelector("pre code");
   if(el)copyText(el.textContent,button,"代码已复制");
  });
  window.addEventListener("hashchange",display);
  display();
 }catch(e){
  console.error("ML Atlas:",e);
  showStatus("课程加载失败："+e.message,true);
  $("atlas-lesson").innerHTML='<h2>讲义暂时无法加载</h2><p>'+esc(e.message)+'</p>'+
   '<p><a href="./legacy-before-atlas.html">打开保留的旧版页面</a></p>';
 }
}
start();
})();