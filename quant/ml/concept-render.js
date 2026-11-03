/* Structured mathematical learning chains: append to the canonical article, never replace source lessons. */
(()=>{
"use strict";
const esc=s=>String(s??"").replace(/[&<>"']/g,c=>({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[c]));
function mount(data,bundle){
 if(bundle.schema!==1||!bundle.lessons||typeof bundle.lessons!=="object")throw Error("知识链目录格式不正确");
 const ids=new Set();
 for(const [id,unit] of Object.entries(bundle.lessons)){
  if(!/^m\d\d-c\d\d-p\d\d$/.test(id)||ids.has(id))throw Error("知识链ID重复或非法："+id);
  const article=data.domains[id.slice(0,3)]?.[id];
  if(!article||article.title!==unit.source_title)throw Error("知识链未对应原有讲义："+id);
  if(!unit.goal||!Array.isArray(unit.sections)||unit.sections.length<3||!Array.isArray(unit.links))throw Error("知识链章节不完整："+id);
  for(const rel of unit.links){
   if(rel.kind==="ml"&&!data.domains[rel.id.slice(0,3)]?.[rel.id])throw Error("知识链机器学习先修引用无效："+rel.id);
   if(rel.kind!=="math"&&rel.kind!=="ml")throw Error("知识链引用类型无效："+id);
   if(!rel.id||!rel.label||rel.kind==="math"&&!/^[a-z]+-(?:n\d{4}|x\d{3})$/.test(rel.id))throw Error("知识链引用格式无效："+id);
  }
  const nav=unit.sections.map((sec,i)=>'<li><button type="button" class="atlas-concept-jump" data-target="atlas-concept-'+id+'-'+(i+1)+'">'+esc(sec.title)+'</button></li>').join("");
  const links=unit.links.map(r=>'<a class="atlas-concept-related" href="'+(r.kind==="math"?"../math/?topic="+encodeURIComponent(r.id):"#lesson/"+r.id)+'">'+esc(r.label)+(r.kind==="math"?" · 数学":" · 机器学习")+'</a>').join("");
  const sections=unit.sections.map((sec,i)=>{
   if(!sec.title||!sec.question||!Array.isArray(sec.paragraphs)||sec.paragraphs.length<2||
      !Array.isArray(sec.equations)||!sec.equations.length||!sec.worked||!sec.boundary)
    throw Error("知识链小节不完整："+id+" / "+i);
   const prose=sec.paragraphs.map(p=>'<p>'+esc(p)+'</p>').join("");
   const math=sec.equations.map(eq=>{
    if(!eq.label||!eq.tex)throw Error("知识链公式缺失："+id);
    return '<div class="atlas-depth-equation"><h4>'+esc(eq.label)+'</h4><div class="ml-math-block"><span class="atlas-tex" data-tex="'+esc(eq.tex)+'"></span></div></div>';
   }).join("");
   return '<section class="atlas-concept-unit" id="atlas-concept-'+id+'-'+(i+1)+'">'+
     '<h3>'+(i+1)+'. '+esc(sec.title)+'</h3><p class="atlas-concept-question">'+esc(sec.question)+'</p>'+prose+
     math+'<h4>逐项核算</h4><p>'+esc(sec.worked)+'</p><h4>结论的边界</h4><p>'+esc(sec.boundary)+'</p></section>';
  }).join("");
  const sources=(unit.sources||[]).map(source=>{
   if(!/^https:\/\//.test(source.url)||!source.title)throw Error("知识链引用来源无效："+id);
   return '<a href="'+esc(source.url)+'" target="_blank" rel="noopener noreferrer">'+esc(source.title)+'</a>';
  }).join(" · ");
  article.html+='<section class="atlas-concept-chain" aria-label="系统化知识链与分层推导">'+
   '<h2>分层精讲：从核心原理到验证与应用</h2><p>'+esc(unit.goal)+'</p>'+
   '<nav class="atlas-concept-toc" aria-label="本知识点的细分章节"><strong>本节学习顺序</strong><ol>'+nav+'</ol></nav>'+
   sections+'<section class="atlas-concept-next"><h3>先修与延伸知识</h3><div>'+links+'</div>'+
   (sources?'<h3>参考资料（原站访问）</h3><p>'+sources+'</p>':'')+'</section></section>';
  ids.add(id);
 }
 return ids;
}
window.quantMlConcepts={mount};
})();
