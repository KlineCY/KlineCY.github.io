(()=>{"use strict";
const $=id=>document.getElementById(id);
const esc=x=>String(x??"").replace(/[&<>"']/g,c=>({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[c]));
const state={index:[],byId:new Map(),domains:{},term:"",topic:""};
async function readArchive(){
 if(typeof DecompressionStream==="undefined")throw Error("此浏览器不支持gzip解压，请使用新版Chrome、Edge、Safari或Firefox");
 const fragments=await Promise.all([1,2,3,4].map(async n=>{
  const r=await fetch("./atlas/data.part"+n,{cache:"force-cache"});if(!r.ok)throw Error("讲义数据块 "+n+" 不存在 (HTTP "+r.status+")");
  return new Uint8Array(await r.arrayBuffer());
 }));
 const n=fragments.reduce((sum,x)=>sum+x.byteLength,0);
 if(n!==1969862)throw Error("讲义压缩数据长度不匹配："+n);
 const binary=new Uint8Array(n);let off=0;for(const p of fragments){binary.set(p,off);off+=p.length}
 const stream=new Blob([binary]).stream().pipeThrough(new DecompressionStream("gzip"));
 const data=JSON.parse(await new Response(stream).text());
 if(data.catalog?.items?.length!==720 || Object.keys(data.domains||{}).length!==24)throw Error("讲义数据不完整（期待720篇、24专题）");
 for(const x of data.catalog.items)if(!data.domains[x.domain_id]?.[x.id])throw Error("缺少讲义："+x.id);
 return data;
}
function active(){
 const id=(location.hash.match(/^#lesson\/(m\d\d-c\d\d-p\d\d)$/)||[])[1];
 $("atlas-toc").querySelectorAll("a[data-id]").forEach(a=>a.classList.toggle("active",a.dataset.id===id));
}
function listing(){
 const matches=state.index.filter(x=>(!state.topic||x.domain_id===state.topic)&&(!state.term||[x.id,x.title,x.domain,x.chapter].some(v=>String(v).toLocaleLowerCase().includes(state.term))));
 $("atlas-status").textContent="显示 "+matches.length+" / "+state.index.length+" 篇讲义";
 const groups=new Map();for(const x of matches){const key=x.domain_id+"_"+x.chapter;if(!groups.has(key))groups.set(key,[]);groups.get(key).push(x)}
 $("atlas-toc").innerHTML=[...groups.values()].map(rows=>"<section class='atlas-section'><h3>"+esc(rows[0].domain)+" · "+esc(rows[0].chapter)+"</h3>"+rows.map(x=>'<a href="#lesson/'+x.id+'" data-id="'+x.id+'">'+esc(x.title)+"</a>").join("")+"</section>").join("")||"<p>没有匹配的知识点。</p>";
 active();
}
function math(root){root.querySelectorAll(".atlas-tex").forEach(el=>{
 const tex=el.getAttribute("data-tex")||"";
 if(window.katex){try{window.katex.render(tex,el,{displayMode:!!el.closest(".ml-math-block"),throwOnError:false,strict:"ignore"});return}catch(e){console.warn(e)}}
 el.textContent=tex;
})}
function route(){
 let id=(location.hash.match(/^#lesson\/(m\d\d-c\d\d-p\d\d)$/)||[])[1]||state.index[0]?.id;if(!id)return;
 const meta=state.byId.get(id),root=$("atlas-lesson");
 if(!meta){root.innerHTML="<p class='atlas-error'>该知识点不存在，请从目录选择。</p>";return}
 const data=state.domains[meta.domain_id]?.[id];
 if(!data){root.innerHTML="<p class='atlas-error'>讲义数据包缺失："+esc(id)+"</p>";return}
 root.innerHTML='<div class="atlas-crumb">'+esc(meta.domain)+" / "+esc(meta.chapter)+" / "+esc(id)+"</div><h1>"+esc(data.title)+'</h1><div class="article">'+data.html+'</div><p class="atlas-note">第119轮系统讲义 · 数学结论与Python示例请依据正文假设使用</p>';
 math(root);active();if(location.hash)root.scrollIntoView({block:"start",behavior:"auto"});
}
async function start(){
 try{
 const data=await readArchive();state.index=data.catalog.items;state.byId=new Map(state.index.map(x=>[x.id,x]));state.domains=data.domains;
 const seen=new Set();for(const x of state.index){if(seen.has(x.domain_id))continue;seen.add(x.domain_id);const o=document.createElement("option");o.value=x.domain_id;o.textContent=x.domain;$("atlas-topic").appendChild(o)}
 $("atlas-search").addEventListener("input",e=>{state.term=e.target.value.trim().toLocaleLowerCase();listing()});
 $("atlas-topic").addEventListener("change",e=>{state.topic=e.target.value;listing()});
 $("atlas-clear").addEventListener("click",()=>{state.term="";state.topic="";$("atlas-search").value="";$("atlas-topic").value="";listing()});
 window.addEventListener("hashchange",route);listing();route();
 }catch(e){console.error(e);$("atlas-status").textContent="课程加载失败："+e.message;$("atlas-lesson").innerHTML="<h2>讲义资源暂不可用</h2><p>"+esc(e.message)+"</p><p><a href='legacy.html'>阅读已保留的旧版机器学习页面</a></p>"}
}
start();
})();