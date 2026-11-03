/* Stable-ID add-on lessons: append a new chapter after a canonical domain without mutating the gzip archive. */
(()=>{
"use strict";
const esc=s=>String(s??"").replace(/[&<>"']/g,c=>({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[c]));
function install(data,bundle){
 if(bundle.schema!==1||!Array.isArray(bundle.chapters)||!bundle.chapters.length)throw Error("新增专题课程格式错误");
 const existing=new Set(data.catalog.items.map(x=>x.id)),result=new Set();
 for(const chapter of bundle.chapters){
  if(!/^m\d\d$/.test(chapter.domain_id)||!data.domains[chapter.domain_id]||
    !chapter.chapter||!Array.isArray(chapter.lessons)||!chapter.lessons.length)throw Error("新增专题章节信息不完整");
  const prototype=data.catalog.items.find(x=>x.domain_id===chapter.domain_id);
  if(!prototype)throw Error("新增专题无法对应现有专题："+chapter.domain_id);
  const newRows=[];
  for(const item of chapter.lessons){
   const id=item.id;
   if(!/^m\d\d-c\d\d-p\d\d$/.test(id)||!id.startsWith(chapter.domain_id+"-")||existing.has(id)||result.has(id))
    throw Error("新增课程ID冲突："+id);
   if(!item.title||!item.goal||!Array.isArray(item.sections)||item.sections.length<3||!Array.isArray(item.protocol))throw Error("新增课程正文不足："+id);
   const nav=item.sections.map((section,i)=>'<li><button type="button" class="atlas-concept-jump" data-target="atlas-new-'+id+'-'+i+'">'+esc(section.title)+'</button></li>').join("");
   const concepts=item.sections.map((section,i)=>{
    if(!section.title||!section.question||!Array.isArray(section.paragraphs)||section.paragraphs.length<2||
       !Array.isArray(section.equations)||!section.equations.length||!section.worked||!section.boundary)
     throw Error("新增课程内部小节不完整："+id+" "+i);
    const math=section.equations.map(f=>{
     if(!f.tex||!f.label)throw Error("新增课程数学关系不完整："+id);
     return '<div class="atlas-depth-equation"><h4>'+esc(f.label)+'</h4><div class="ml-math-block"><span class="atlas-tex" data-tex="'+esc(f.tex)+'"></span></div></div>';
    }).join("");
    return '<section class="atlas-concept-unit" id="atlas-new-'+id+'-'+i+'"><h3>'+(i+1)+'. '+esc(section.title)+'</h3>'+
     '<p class="atlas-concept-question">'+esc(section.question)+'</p>'+
     section.paragraphs.map(p=>'<p>'+esc(p)+'</p>').join("")+math+
     '<h4>数值或研究核算</h4><p>'+esc(section.worked)+'</p>'+
     '<h4>成立边界</h4><p>'+esc(section.boundary)+'</p></section>';
   }).join("");
   const links=(item.prerequisites||[]).map(ref=>{
    if(ref.kind==="ml"&&!existing.has(ref.id)&&!result.has(ref.id))throw Error("新增课程先修机器学习ID不存在："+ref.id);
    if(ref.kind!=="ml"&&ref.kind!=="math")throw Error("新增课程先修类型非法："+ref.kind);
    if(!ref.label||!ref.id)throw Error("新增课程先修标签缺失："+id);
    return '<a class="atlas-concept-related" href="'+(ref.kind==="math"?"../math/?topic="+encodeURIComponent(ref.id):"#lesson/"+ref.id)+'">'+esc(ref.label)+'</a>';
   }).join("");
   const sources=(item.sources||[]).map(ref=>{
    if(!/^https:\/\//.test(ref.url)||!ref.title)throw Error("新增课程来源链接无效："+id);
    return '<a target="_blank" rel="noopener noreferrer" href="'+esc(ref.url)+'">'+esc(ref.title)+'</a>';
   }).join(" · ");
   const html='<div class="atlas-added-lesson atlas-concept-chain">'+
     '<h2>本节研究对象与学习路线</h2><p>'+esc(item.goal)+'</p>'+
     '<nav class="atlas-concept-toc" aria-label="知识点分层导航"><strong>按原理顺序阅读</strong><ol>'+nav+'</ol></nav>'+
     concepts+'<section class="atlas-concept-next"><h3>实施与验证顺序</h3><ol>'+
     item.protocol.map(p=>'<li>'+esc(p)+'</li>').join("")+
     '</ol><h3>先修与衔接</h3><div>'+links+'</div>'+
     (sources?'<h3>资料与开源参考</h3><p>'+sources+'</p>':'')+'</section></div>';
   data.domains[chapter.domain_id][id]={title:item.title,html};
   newRows.push({id,title:item.title,domain_id:chapter.domain_id,domain:prototype.domain,chapter:chapter.chapter});
   result.add(id);
  }
  let last=data.catalog.items.findLastIndex(x=>x.domain_id===chapter.domain_id);
  if(last<0)throw Error("新增专题无插入位置");
  data.catalog.items.splice(last+1,0,...newRows);
  for(const row of newRows)existing.add(row.id);
 }
 return result;
}
window.quantMlExpansion={install};
})();
