/* Mobile-only global site navigation disclosure for quant/math and quant/ml. */
(()=>{
 "use strict";
 const button=document.getElementById("course-site-nav-toggle");
 const nav=document.getElementById("quantSidebar");
 if(!button||!nav)return;
 let open=false;
 const mobile=window.matchMedia("(max-width:820px)");
 function update(value){
  open=Boolean(value);
  document.body.classList.toggle("course-site-nav-open",open);
  button.setAttribute("aria-expanded",String(open));
  button.textContent=open?"收起站点导航":"展开站点导航";
 }
 button.addEventListener("click",()=>update(!open));
 window.addEventListener("resize",()=>{if(!mobile.matches&&open)update(false)});
 update(false);
})();
