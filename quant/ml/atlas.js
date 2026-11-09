(()=>{
"use strict";
const $=id=>document.getElementById(id);
const esc=x=>String(x??"").replace(/[&<>"']/g,c=>({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[c]));
const LESSON=/^#(?:lesson|ml)\/(m\d\d-c\d\d-p\d\d)$/;
const state={index:[],byId:new Map(),domains:{},searchText:new Map(),term:"",topic:"",activeId:"",enhanced:new Set(),conceptChapters:new Set(),chapterOpen:new Map(),domainOpen:new Map()};
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

const EDITORIAL_A="这里的随机对象、输入分布、样本索引和运算顺序都应在项目文档中写明。公式的作用是给出可审计的数量关系，并不自动提供统计一致性或金融收益保证。";
const EDITORIAL_B="这些关系的成立还要求明确数据可得时间、参数单位、取样机制以及用于估计的训练区间。违反条件时不能由等式直接推断样本外效果。";
function applyEditorial(data,patch){
 if(patch.schema!==1)throw Error("教材勘误文件版本不受支持");
 const sums={definitions:0,conditions:0,formulas:0};
 function record(id,oldText,newText,key){
  if(!/^m\d\d-c\d\d-p\d\d$/.test(id))throw Error("非法知识点勘误ID："+id);
  const row=data.domains[id.slice(0,3)]?.[id];
  if(!row)throw Error("勘误目标不存在："+id);
  const original=row.html;
  if(!original.includes(oldText))throw Error("勘误目标正文发生变化："+id+" / "+key);
  row.html=original.replace(oldText,newText);
  sums[key]++;
 }
 for(const [id,meaning] of Object.entries(patch.replacementA||{}))
  record(id,"<p>"+EDITORIAL_A+"</p>","<p>"+esc(meaning)+"</p>","definitions");
 for(const [id,meaning] of Object.entries(patch.replacementB||{}))
  record(id,"<p>"+EDITORIAL_B+"</p>","<p>"+esc(meaning)+"</p>","conditions");
 for(const fix of patch.math||[]){
  if(!fix.from||!fix.to)throw Error("公式勘误缺失原式或新式："+fix.id);
  const row=data.domains[fix.id.slice(0,3)]?.[fix.id];
  if(!row||!row.html.includes(fix.from))throw Error("公式勘误原式缺失："+fix.id);
  row.html=row.html.split(fix.from).join(fix.to);
  sums.formulas++;
 }
 if(sums.definitions!==20||sums.conditions!==31||sums.formulas!==2)throw Error("教材勘误应用数量不匹配");
 return sums;
}
function applyDepth(data,patches,manifest){
 if(!Array.isArray(patches)||patches.length!==manifest.modules.length)throw Error("深化讲义资源列表不完整");
 const seen=new Set(),allDomains=new Set();
 function items(rows){return "<ol>"+rows.map(s=>"<li>"+esc(s)+"</li>").join("")+"</ol>"}
 for(let i=0;i<patches.length;i++){
  const patch=patches[i];
  if(patch.schema!==1||!patch.lessons||typeof patch.lessons!=="object")throw Error("深化讲义资源格式错误");
  if(Object.keys(patch.lessons).length!==manifest.modules[i].lessons)throw Error("深化讲义资源数量不匹配："+manifest.modules[i].path);
  for(const [id,s] of Object.entries(patch.lessons)){
   if(seen.has(id))throw Error("重复的深化讲义ID："+id);
   seen.add(id);allDomains.add(id.slice(0,3));
  const row=data.domains[id.slice(0,3)]?.[id];
  if(!row)throw Error("深化讲义未找到目标："+id);
  if(typeof s.source_title!=="string"||s.source_title!==row.title)throw Error("深化讲义标题与原知识点不匹配："+id+" / "+s.source_title);
  if(typeof s.title!=="string"||s.title.length<8||typeof s.overview!=="string"||s.overview.length<60||
     !Array.isArray(s.equations)||s.equations.length<2||!Array.isArray(s.derivation)||s.derivation.length<3||
     !Array.isArray(s.protocol)||s.protocol.length<3||!Array.isArray(s.pitfalls))throw Error("深化讲义内容不完整："+id);
  if(typeof s.worked_example!=="string"||s.worked_example.length<50)
   throw Error("深化讲义数值例子不足："+id);
  if(/-(?:[456]|(?:0[789]|1\d)-[a-z])\.json$/.test(manifest.modules[i].path)&&
     (s.equations.length<3||!s.worked_example||s.worked_example.length<60))
   throw Error("进阶讲义缺少完整数学算例："+id);
  const formulas=s.equations.map(eq=>{
   if(!eq.tex||!eq.label)throw Error("深化讲义公式不完整："+id);
   return '<div class="atlas-depth-equation"><h4>'+esc(eq.label)+'</h4><div class="ml-math-block"><span class="atlas-tex" data-tex="'+esc(eq.tex)+'"></span></div></div>';
  }).join("");
  row.html+='<section class="atlas-depth" aria-label="进阶推导与研究实践">'+
   '<h2>进阶专题：'+esc(s.title)+'</h2><p class="atlas-depth-intro">'+esc(s.overview)+'</p>'+
   '<h3>核心数学关系</h3>'+formulas+
   '<h3>推导与适用边界</h3>'+items(s.derivation)+
   (s.worked_example?'<h3>数值或研究示例</h3><p class="atlas-depth-example">'+esc(s.worked_example)+'</p>':'')+
   '<h3>研究与实现流程</h3>'+items(s.protocol)+
   '<h3>常见误区</h3>'+items(s.pitfalls)+'</section>';
  }
 }
 if(seen.size!==manifest.expected_lessons||allDomains.size!==manifest.expected_domains)
  throw Error("深化讲义覆盖不完整："+seen.size+"篇，"+allDomains.size+"专题");
 const coverage=new Map();
 for(const id of seen)coverage.set(id.slice(0,3),(coverage.get(id.slice(0,3))||0)+1);
 if([...coverage.values()].some(n=>n<22))throw Error("某机器学习专题的进阶内容少于22篇");
 return seen;
}
function cleanCodeComments(root){
 root.querySelectorAll(".ml-point-code-wrap pre code").forEach(el=>{
  el.textContent=el.textContent.replace(/^([ \t]*#\s*)第\d+轮\s*[：:]?\s*/gm,"$1");
 });
}

async function fetchAtlasJson(url,options={}){
 let failure=null;
 for(let attempt=0;attempt<3;attempt++){
  try{
   const response=await fetch(url,options);
   if(!response.ok){
    const problem=Error("HTTP "+response.status+"："+url);
    if(response.status!==429&&response.status<500)throw problem;
    failure=problem;
   }else return await response.json();
  }catch(err){
   if(/^HTTP 4[0-9][0-9]/.test(err.message)&&!err.message.startsWith("HTTP 429"))throw err;
   failure=err;
  }
  if(attempt<2)await new Promise(resolve=>setTimeout(resolve,180*(attempt+1)));
 }
 throw Error("资源下载失败（已尝试3次）："+url+"；"+(failure?.message||"未知网络故障"));
}
async function fetchAtlasModulesBounded(files,concurrency=6,onProgress=()=>{}){
 const output=new Array(files.length);
 let cursor=0,completed=0;
 const workers=Array.from({length:Math.min(concurrency,files.length)},async()=>{
  while(cursor<files.length){
   const index=cursor++;
   output[index]=await fetchAtlasJson(files[index],{cache:"no-cache"});
    onProgress(++completed,files.length);
  }
 });
 await Promise.all(workers);
 return output;
}
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
 const words=state.term.split(/\s+/).filter(Boolean);
 return state.index.filter(x=>(!state.topic||x.domain_id===state.topic)&&
  (words.length===0||words.every(word=>state.searchText.get(x.id)?.includes(word))));
}
function chapterShouldOpen(key,active){
 const preference=state.chapterOpen.get(key);
 return state.term.length>0||(preference===undefined?active:preference);
}
function domainShouldOpen(id,active){
 const preference=state.domainOpen.get(id);
 return state.term.length>0||(preference===undefined?active:preference);
}
function setCatalogOpen(open){
 const section=$("atlas-reader-section"),button=$("atlas-toggle-catalog");
 if(!section||!button)return;
 section.classList.toggle("catalog-open",open);
 button.setAttribute("aria-expanded",String(open));
 button.textContent=open?"收起课程目录":"展开课程目录";
}
function renderDirectory(){
 const matches=visibleLessons();
 showStatus("显示 "+matches.length+" / "+state.index.length+" 篇讲义");
 const domains=new Map();
 for(const x of matches){
  if(!domains.has(x.domain_id))
   domains.set(x.domain_id,{id:x.domain_id,title:x.domain,chapters:new Map(),count:0});
  const group=domains.get(x.domain_id);
  group.count++;
  const key=x.domain_id+"_"+x.chapter;
  if(!group.chapters.has(key))group.chapters.set(key,{key,title:x.chapter,number:(x.id.match(/-c(\d+)-/)||[])[1]||"",rows:[]});
  group.chapters.get(key).rows.push(x);
 }
 const html=[...domains.values()].map(group=>{
  const active=[...group.chapters.values()].some(c=>c.rows.some(x=>x.id===state.activeId));
  const domainLabel=(group.id.match(/\d+/)||[])[0]||"";
  const chapters=[...group.chapters.values()].map(c=>{
   const open=chapterShouldOpen(c.key,c.rows.some(x=>x.id===state.activeId));
   const links=c.rows.map(x=>{
    const index=(x.id.match(/-p(\d+)$/)||[])[1]||"";
    return '<a href="#lesson/'+esc(x.id)+'" data-id="'+esc(x.id)+'" title="'+esc(x.title)+'"'+
     (x.id===state.activeId?' class="active" aria-current="page"':'')+'>'+
     '<span class="atlas-topic-index">'+esc(index)+'</span><span class="atlas-topic-label">'+
     esc(x.title)+
     (state.conceptChapters.has(x.id)?'<small class="atlas-concept-small"> · 精讲</small>':'')+'</span></a>';
   }).join("");
   return '<details class="atlas-section" data-key="'+esc(c.key)+'"'+(open?' open':'')+'>'+
    '<summary><span class="atlas-chapter-title"><span class="atlas-chapter-index">'+esc(c.number)+'</span>'+esc(c.title)+'</span>'+
    '<span class="atlas-chapter-count">'+c.rows.length+' 篇</span></summary>'+
    '<div class="atlas-chapter-items">'+links+'</div></details>';
  }).join("");
  return '<details class="atlas-domain" data-domain="'+esc(group.id)+'"'+(domainShouldOpen(group.id,active)?' open':'')+'>'+
    '<summary><span class="atlas-domain-heading"><span class="atlas-domain-index">'+esc(domainLabel)+'</span><strong>'+esc(group.title)+'</strong></span>'+
    '<span class="atlas-domain-count">'+group.count+' 篇</span></summary>'+
    '<div class="atlas-domain-chapters">'+chapters+'</div></details>';
 }).join("");
 const toc=$("atlas-toc"),previousScroll=toc.scrollTop;
 toc.innerHTML=html||'<p class="atlas-empty-directory">当前筛选没有匹配的知识点，请调整搜索关键词。</p>';
 toc.scrollTop=previousScroll;
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
 if(state.activeId!==id){
  state.chapterOpen.delete(meta.domain_id+"_"+meta.chapter);
  state.domainOpen.delete(meta.domain_id);
 }
 state.activeId=id;
 if(state.topic&&state.topic!==meta.domain_id){
  state.topic=meta.domain_id;$("atlas-topic").value=state.topic;
  const u=new URL(location.href);u.searchParams.set("discipline",state.topic);
  history.replaceState(null,"",u.pathname+u.search+u.hash);
 }
 if(state.term&&!state.term.split(/\s+/).filter(Boolean).every(word=>state.searchText.get(id)?.includes(word))){
  state.term="";$("atlas-search").value="";
 }
 const position=state.index.findIndex(x=>x.id===id);
 const prev=state.index[position-1],next=state.index[position+1];
 const actions='<div class="atlas-lesson-actions">'+
  '<button id="atlas-prev" type="button"'+(!prev?' disabled':'')+'>上一篇</button>'+
  '<button id="atlas-next" type="button"'+(!next?' disabled':'')+'>下一篇</button>'+
  '<button id="atlas-copy-link" type="button">复制本文链接</button>'+ 
   (state.enhanced.has(id)?'<button id="atlas-jump-depth" type="button">跳至进阶推导</button>':'')+'</div>';
 root.innerHTML='<div class="atlas-crumb">'+esc(meta.domain)+" / "+esc(meta.chapter)+" / 第"+esc((id.match(/-p(\d+)$/)||[])[1]||"")+"节"+
  '</div><h1>'+esc(data.title)+'</h1>'+actions+
  '<div class="article">'+data.html+'</div><p class="atlas-note">系统教材 · 公式、证明和代码请按对应条件使用；示例不构成投资建议。</p>';
 $("atlas-prev").addEventListener("click",()=>prev&&jump(prev.id));
 $("atlas-next").addEventListener("click",()=>next&&jump(next.id));
  $("atlas-jump-depth")?.addEventListener("click",()=>root.querySelector(".atlas-depth")?.scrollIntoView({block:"start",behavior:"smooth"}));
 $("atlas-copy-link").addEventListener("click",async()=>{
  const url=new URL(location.href);url.hash="lesson/"+id;
  await copyText(url.href,$("atlas-copy-link"),"已复制链接");
 });
 root.querySelectorAll(".atlas-concept-jump").forEach(button=>button.addEventListener("click",()=>{
  root.querySelector("#"+button.dataset.target)?.scrollIntoView({block:"start",behavior:"smooth"});
 }));
 fixInlineMarkdown(root.querySelector(".article"));
 cleanCodeComments(root.querySelector(".article"));
 renderMath(root.querySelector(".article"));
 renderDirectory();
 const toc=$("atlas-toc"),selected=toc.querySelector("a[aria-current='page']");
 if(selected){
  const outer=toc.getBoundingClientRect(),inner=selected.getBoundingClientRect();
  if(inner.top<outer.top)toc.scrollTop+=inner.top-outer.top-14;
  else if(inner.bottom>outer.bottom)toc.scrollTop+=inner.bottom-outer.bottom+14;
 }
 root.scrollTop=0;
 setCatalogOpen(false);
 if(location.hash)$("atlas-reader-section")?.scrollIntoView({block:"start",behavior:"auto"});
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
  showStatus("正在读取并解压720篇正式讲义…");
  const data=await loadArchive();
  showStatus("课程档案已校验，正在读取进阶模块清单…");
  const manifest=await fetchAtlasJson("./atlas-depth-manifest.json",{cache:"no-cache"});
  if(manifest.schema!==1||!Number.isSafeInteger(manifest.expected_lessons)||manifest.expected_lessons<80||
     manifest.expected_domains!==24||!Array.isArray(manifest.modules)||manifest.modules.length<10)
   throw Error("深化讲义目录版本或内容不完整");
  const depthFiles=manifest.modules.map(m=>m.path);
  if(new Set(depthFiles).size!==depthFiles.length||
     manifest.modules.some(m=>!/^\.\/atlas-depth(?:-[a-z0-9]+)*\.json$/.test(m.path)||
                                  m.path==="./atlas-depth-manifest.json"||
                                  !Number.isInteger(m.lessons)||m.lessons<1))
   throw Error("深化讲义目录包含非法路径或数量");
  if(manifest.modules.reduce((sum,m)=>sum+m.lessons,0)!==manifest.expected_lessons)
   throw Error("深化讲义目录数量不一致");
  const editorial=await fetchAtlasJson("./atlas-editorial.json",{cache:"no-cache"});
  const depthPatches=await fetchAtlasModulesBounded(depthFiles,6,(done,total)=>{
   if(done===total||done%8===0)showStatus("正在传输进阶正文 "+done+" / "+total+" 个模块…");
  });
  showStatus("进阶内容已传输，正在校验公式与构建全文索引…");
  applyEditorial(data,editorial);
  state.enhanced=applyDepth(data,depthPatches,manifest);
  const added=await fetchAtlasJson("./atlas-new-lessons.json",{cache:"no-cache"});
  if(!window.quantMlExpansion?.install)throw Error("新增专题阅读器未加载");
  const newIds=window.quantMlExpansion.install(data,added);
  const volatility=await fetchAtlasJson("./atlas-volatility-chapter.json",{cache:"no-cache"});
  const volatilityIds=window.quantMlExpansion.install(data,volatility);
  for(const id of volatilityIds){
   if(newIds.has(id))throw Error("新增专题ID重复："+id);
   newIds.add(id);
  }
  const regularization=await fetchAtlasJson("./atlas-linear-regularization-chapter.json",{cache:"no-cache"});
  const regularizationIds=window.quantMlExpansion.install(data,regularization);
  for(const id of regularizationIds){
   if(newIds.has(id))throw Error("新增专题ID重复："+id);
   newIds.add(id);
  }
  const evidenceBundle=await fetchAtlasJson("./atlas-evaluation-evidence-chapter.json",{cache:"no-cache"});
  const evidenceIds=window.quantMlExpansion.install(data,evidenceBundle);
  for(const id of evidenceIds){
   if(newIds.has(id))throw Error("新增专题ID重复："+id);
   newIds.add(id);
  }
  for(const path of ["./atlas-generalization-v2.json","./atlas-kernel-covariance-v2.json"]){
   const extension=await fetchAtlasJson(path,{cache:"no-cache"});
   const extensionIds=window.quantMlExpansion.install(data,extension);
   for(const id of extensionIds){
    if(newIds.has(id))throw Error("新增专题ID重复："+id);
    newIds.add(id);
   }
  }
  for(const path of ["./atlas-stability-kernel-v3.json","./atlas-robust-validation-v3.json"]){
   const bundle=await fetchAtlasJson(path,{cache:"no-cache"});
   const ids=window.quantMlExpansion.install(data,bundle);
   for(const id of ids){
    if(newIds.has(id))throw Error("新增专题ID重复："+id);
    newIds.add(id);
   }
  }
  for(const path of ["./atlas-quantile-calibration-v4.json","./atlas-tailrisk-models-v4.json"]){
   const bundle=await fetchAtlasJson(path,{cache:"no-cache"});
   const ids=window.quantMlExpansion.install(data,bundle);
   for(const id of ids){
    if(newIds.has(id))throw Error("新增专题ID重复："+id);
    newIds.add(id);
   }
  }
  for(const path of ["./atlas-proper-scoring-conformal-v5.json","./atlas-density-pit-finance-v5.json"]){
   const bundle=await fetchAtlasJson(path,{cache:"no-cache"});
   const ids=window.quantMlExpansion.install(data,bundle);
   for(const id of ids){
    if(newIds.has(id))throw Error("新增专题ID重复："+id);
    newIds.add(id);
   }
  }
  for(const path of ["./atlas-copula-v6.json","./atlas-multivariate-risk-v6.json"]){
   const bundle=await fetchAtlasJson(path,{cache:"no-cache"});
   const ids=window.quantMlExpansion.install(data,bundle);
   for(const id of ids){
    if(newIds.has(id))throw Error("新增专题ID重复："+id);
    newIds.add(id);
   }
  }
  const conceptBundle=await fetchAtlasJson("./atlas-concept-chapters.json",{cache:"no-cache"});
  if(!window.quantMlConcepts?.mount)throw Error("知识链分层渲染器未加载");
  state.conceptChapters=window.quantMlConcepts.mount(data,conceptBundle);
  const bridge=await fetchAtlasJson("./atlas-concept-bridge-01.json",{cache:"no-cache"});
  for(const id of window.quantMlConcepts.mount(data,bridge)){
   if(state.conceptChapters.has(id))throw Error("知识链ID重复："+id);
   state.conceptChapters.add(id);
  }
  for(const path of ["./atlas-concept-bridge-v111a.json","./atlas-concept-bridge-v111b.json","./atlas-concept-bridge-v111c.json"]){
   const extension=await fetchAtlasJson(path,{cache:"no-cache"});
   if(extension.schema!==1||extension.revision!=="V111"||
      Object.keys(extension.lessons||{}).length!==4)
    throw Error("V111数学与机器学习衔接文件不完整："+path);
   const ids=window.quantMlConcepts.mount(data,extension);
   if(ids.size!==4)throw Error("V111数学知识链实际挂载不足："+path);
   for(const id of ids){
    if(state.conceptChapters.has(id))throw Error("V111知识链重复覆盖已完成讲义："+id);
    state.conceptChapters.add(id);
   }
  }
  for(const path of ["./atlas-concept-bridge-v112a.json","./atlas-concept-bridge-v112b.json","./atlas-concept-bridge-v112c.json",
                    "./atlas-concept-bridge-v112d.json","./atlas-concept-bridge-v112e.json"]){
   const extension=await fetchAtlasJson(path,{cache:"no-cache"});
   const entries=Object.keys(extension.lessons||{});
   if(extension.schema!==1||extension.revision!=="V112"||entries.length!==4||
      entries.some(id=>state.conceptChapters.has(id)||newIds.has(id)))
    throw Error("V112正式机器学习教材重复或内容不完整："+path);
   const ids=window.quantMlConcepts.mount(data,extension);
   if(ids.size!==entries.length)throw Error("V112正式课程实际挂载不完整："+path);
   for(const id of ids)state.conceptChapters.add(id);
  }
  for(const path of ["./atlas-concept-bridge-v113a.json","./atlas-concept-bridge-v113b.json","./atlas-concept-bridge-v113c.json",
                    "./atlas-concept-bridge-v113d.json","./atlas-concept-bridge-v113e.json"]){
   const extension=await fetchAtlasJson(path,{cache:"no-cache"});
   const entries=Object.keys(extension.lessons||{});
   if(extension.schema!==1||extension.revision!=="V113"||entries.length!==4||
      entries.some(id=>state.conceptChapters.has(id)||newIds.has(id)))
    throw Error("V113知识链内容不完整或与已有课程重复："+path);
   const ids=window.quantMlConcepts.mount(data,extension);
   if(ids.size!==4)throw Error("V113正式教材挂载数错误："+path);
   for(const id of ids)state.conceptChapters.add(id);
  }
  for(const path of ["./atlas-concept-bridge-v114a.json","./atlas-concept-bridge-v114b.json","./atlas-concept-bridge-v114c.json",
                    "./atlas-concept-bridge-v114d.json","./atlas-concept-bridge-v114e.json"]){
   const extension=await fetchAtlasJson(path,{cache:"no-cache"});
   const entries=Object.keys(extension.lessons||{});
   if(extension.schema!==1||extension.revision!=="V114"||entries.length!==4||
      entries.some(id=>state.conceptChapters.has(id)||newIds.has(id)))
    throw Error("V114课程正文包不完整或课程编号重复："+path);
   const ids=window.quantMlConcepts.mount(data,extension);
   if(ids.size!==4)throw Error("V114深入讲义实际挂载不完整："+path);
   for(const id of ids)state.conceptChapters.add(id);
  }
  for(const newId of newIds)state.conceptChapters.add(newId);
  state.index=data.catalog.items;state.byId=new Map(state.index.map(x=>[x.id,x]));state.domains=data.domains;
  const total=$("atlas-total-lessons");if(total)total.textContent=state.index.length+" 篇系统讲义（含新增专题）";
  for(const x of state.index){
   const page=data.domains[x.domain_id][x.id].html;
   const plain=page.replace(/<[^>]*>/g," ").replace(/&(lt|gt|amp|quot|nbsp);/g,(_,entity)=>({lt:"<",gt:">",amp:"&",quot:"\"",nbsp:" "}[entity]));
   state.searchText.set(x.id,[x.id,x.title,x.domain,x.chapter,plain].join(" ").toLocaleLowerCase());
  }
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
  $("atlas-toc").addEventListener("click",e=>{
   const summary=e.target.closest("summary");
   const details=summary?.parentElement;
   if(details?.matches(".atlas-domain")&&details.dataset.domain)
    state.domainOpen.set(details.dataset.domain,!details.open);
   else if(details?.matches(".atlas-section")&&details.dataset.key)
    state.chapterOpen.set(details.dataset.key,!details.open);
  });
  $("atlas-toggle-catalog")?.addEventListener("click",()=>setCatalogOpen(!$("atlas-reader-section").classList.contains("catalog-open")));
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
   '<p>请刷新页面重试；若问题持续，请检查课程数据是否已完整部署。旧版备份不作为正式课程入口。</p>';
 }
}
start();
})();