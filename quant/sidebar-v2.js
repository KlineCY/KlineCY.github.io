/* Kline Quant: single navigation tree used across every quant section.
   Each top-level category is its own rounded, collapsible parent. */
(function () {
  "use strict";
  const host=document.getElementById("quantSidebar");
  if(!host)return;
  const page=host.dataset.page||"companies";
  const prefix=page==="companies"?"./":"../";
  const route=name=>prefix+(name==="companies"?"":name+"/");
  const home=page==="companies"?"../":"../../";
  const groups=[
    ["companies","量化公司"],
    ["assets","金融资产分类"],
    ["feeds","核心数据源 Data Feeds"],
    ["research","策略研究与回测"],
    ["wiki","量化百科 Wiki"],
    ["papers","每周机器学习论文"],
    ["rankings","学术排名与资源"],
    ["news","全球宏观 News"]
  ];
  const esc=s=>String(s??"").replace(/[&<>"']/g,c=>({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[c]));
  const alpha=(a,b)=>String(a).localeCompare(String(b),"en",{sensitivity:"base"});
  const cnOrder={"中国":"China","美国":"United States","英国":"United Kingdom","法国":"France","荷兰":"Netherlands","德国":"Germany","加拿大":"Canada","瑞士":"Switzerland","中国香港":"Hong Kong","欧元区":"Euro Area","日本":"Japan","韩国":"South Korea","新加坡":"Singapore","印度":"India","国际机构":"International","巴西":"Brazil","澳大利亚":"Australia","新西兰":"New Zealand"};
  const sortGeo=(a,b)=>alpha(cnOrder[a]||a,cnOrder[b]||b);
  const subsection=(id,label,content,open=false)=>'<details class="q-nav-subgroup" id="'+id+'" '+(open?"open":"")+'><summary><span>'+label+'</span><span class="q-sub-chevron" aria-hidden="true">▸</span></summary><div class="q-nav-leaves" id="'+content+'"></div></details>';
  const staticMenu={
    companies:
      (page==="companies"
        ?'<button id="all" class="q-menu-link active" data-filter="all" type="button">全部公司 <span class="q-menu-count" id="allN"></span></button>'
        :'<a class="q-menu-link" href="'+route("companies")+'">全部公司</a>')+
      (page==="companies" ? 
        '<button type="button" class="q-sub-toggle" id="typeToggle" aria-expanded="false" aria-controls="typeSub"><span>按类型划分</span><span class="q-sub-chevron" aria-hidden="true">▸</span></button><div class="q-nav-leaves" id="typeSub" hidden></div>'+
        '<button type="button" class="q-sub-toggle" id="regionToggle" aria-expanded="false" aria-controls="regionSub"><span>按地区划分</span><span class="q-sub-chevron" aria-hidden="true">▸</span></button><div class="q-nav-leaves" id="regionSub" hidden></div>'
      :subsection("typeNavMenu","按类型划分","typeSub")+
       subsection("regionNavMenu","按地区划分","regionSub")),
    assets:
      '<a class="q-menu-link" href="'+route("assets")+'">全部金融资产</a>'+
      subsection("marketNav","按市场 / 地区","marketChoices",page==="assets")+
      subsection("typeNav","按金融工具类型","typeChoices"),
    feeds:
      '<a class="q-menu-link" href="'+route("feeds")+'">全部数据字段</a>'+
      '<a class="q-menu-link" href="'+route("feeds")+'?mode=providers">数据供应商目录</a>'+
      subsection("feedsCategoryNav","按数据类别","feedChoices",page==="feeds"),
    research:
      '<a class="q-menu-link" href="'+route("research")+'">在线 Python 回测</a>'+
      '<a class="q-menu-link" href="'+route("research")+'?strategy=ma">策略超市 / Python 示例</a>'+
      subsection("researchStrategyNav","按策略选择","strategyChoices",page==="research"),
    wiki:
      '<a class="q-menu-link" href="'+route("wiki")+'">全部词条</a>'+
      subsection("wikiCategoryNav","按知识主题","wikiCategories",page==="wiki")+
      subsection("wikiLearningNav","系统学习路径","wikiPaths"),
    papers:
      '<a class="q-menu-link" href="'+route("papers")+'">全部论文</a>'+
      subsection("weekNav","按周浏览","weeks",page==="papers"),
    rankings:
      '<a class="q-menu-link" href="'+route("rankings")+'">全部学术网站</a>'+
      '<details class="q-nav-subgroup academic-fold" id="academicFold" '+(page==="rankings"?"open":"")+'><summary><span>按资源类别</span><span class="q-sub-chevron" aria-hidden="true">▸</span></summary><div class="q-nav-leaves academic-fold-items" id="catsNav"></div></details>',
    news:
      '<a class="q-menu-link" href="'+route("news")+'">全部新闻 / 官方发布源</a>'+
      subsection("regionNewsNav","按国家 / 地区","newsRegionChoices",page==="news")+
      subsection("topicNewsNav","按消息类型","newsTopicChoices")
  };
  host.innerHTML=
    '<a class="q-brand" href="'+home+'">Kline<span>.</span></a>'+
    '<nav class="q-nav-tree" aria-label="量化研究一级目录">'+
       groups.map(([id,label])=>
         '<details class="q-nav-group" data-group="'+id+'" '+(id===page?"open":"")+'>'+
           '<summary><a class="q-nav-title'+(id===page?" is-active":"")+'" href="'+route(id)+'"'+(id===page?' aria-current="page"':'')+'>'+label+'</a>'+
            '<span class="q-chevron" aria-hidden="true">▸</span></summary>'+
           '<div class="q-nav-items">'+staticMenu[id]+'</div>'+
         '</details>').join("")+
    '</nav>'+
    '<a href="'+home+'" class="q-home-link">返回个人主页 <span aria-hidden="true">↗</span></a>';
  // A summary opens/closes its own subtree; clicking the title navigates.
  host.querySelectorAll(".q-nav-group > summary > a").forEach(a=>{
    a.addEventListener("click",e=>e.stopPropagation());
  });
  // Keep navigation within the current document. Reloading the same page for
  // a category was causing a visible flash and a scroll position reset.
  const cleanPath=p=>p.replace(/\/index\.html$/,"/").replace(/\/+$/,"/");
  host.addEventListener("click",event=>{
    const a=event.target.closest("a[href]");
    if(!a||!host.contains(a))return;
    const dest=new URL(a.href,location.href);
    if(dest.origin!==location.origin || cleanPath(dest.pathname)!==cleanPath(location.pathname))return;
    const params=dest.searchParams;
    if(a.classList.contains("q-nav-title")){
      event.preventDefault();
      const group=a.closest(".q-nav-group");
      if(group)group.open=!group.open;
      return;
    }
    let handled=false;
    if(page==="companies"&&typeof window.apply==="function"){
      const type=params.get("kind"),value=params.get("value");
      if((type==="type"||type==="region")&&value){
        window.apply(type,value);
        const toggle=document.getElementById(type+"Toggle");
        if(toggle&&toggle.getAttribute("aria-expanded")!=="true")toggle.click();
        handled=true;
      }else if(!dest.search){window.apply("all","");handled=true;}
    }
    if(page==="assets"&&typeof window.selectView==="function"){
      const v=params.get("view")==="type"?"type":"market",focus=params.get("focus")||"";
      window.selectView(v,focus,false);
      handled=true;
    }
    if(page==="papers"&&typeof window.setIssue==="function"){
      window.setIssue(params.get("issue")||"");handled=true;
    }
    if(page==="rankings"){
      const category=document.getElementById("cat");
      if(category){category.value=params.get("category")||"";category.dispatchEvent(new Event("change",{bubbles:true}));handled=true;}
    }
    if(page==="news"){
      const region=document.getElementById("region"),topic=document.getElementById("category");
      if(region&&topic){
        region.value=params.get("region")||"";
        topic.value=params.get("category")||"";
        topic.dispatchEvent(new Event("change",{bubbles:true}));
        handled=true;
      }
    }
    if(page==="research"&&typeof window.selectStrategy==="function"){
      const id=params.get("strategy");
      if(id)window.selectStrategy(id);
      handled=true;
    }
    if(page==="wiki"&&typeof window.wikiSetCategory==="function"&&typeof window.wikiSetPath==="function"){
      const c=params.get("category"),p=params.get("path");
      if(p)window.wikiSetPath(p);
      else window.wikiSetCategory(c||"");
      handled=true;
    }
    if(page==="feeds"){
      const mode=params.get("mode");
      if(mode==="providers"){
        document.getElementById("modeProviders")?.click();
        handled=true;
      }else{
        document.getElementById("modeFields")?.click();
        const category=document.getElementById("category");
        if(category){
          category.value=params.get("category")||"";
          category.dispatchEvent(new Event("change",{bubbles:true}));
          handled=true;
        }
      }
    }
    if(handled){
      event.preventDefault();
      history.replaceState(null,"",dest.pathname+dest.search+dest.hash);
    }
  },true);
  // Prevent a scrollbar reanchor inside the independent sidebar when a tree
  // node expands or contracts. The document itself must not move.
  let lastNavTop=null;
  host.addEventListener("pointerdown",event=>{
    if(window.innerWidth<=820)return;
    if(event.target.closest(".q-nav-group > summary,.q-nav-subgroup > summary,.q-sub-toggle"))
      lastNavTop=host.scrollTop;
  },true);
  host.addEventListener("toggle",()=>{
    if(lastNavTop===null||window.innerWidth<=820)return;
    const top=lastNavTop;
    lastNavTop=null;
    requestAnimationFrame(()=>{host.scrollTop=top});
  },true);

  // For an inactive module, turn its data-backed subcategories into true links.
  function fill(id,values,onClick){
    const target=document.getElementById(id);if(!target)return;
    for(const row of values){
      const link=document.createElement("a");
      link.className="q-menu-link q-subitem";
      link.href=onClick(row);
      link.textContent=row.label||row.name||row;
      target.appendChild(link);
    }
  }
  function getJson(url){return fetch(url).then(r=>{if(!r.ok)throw Error("HTTP "+r.status);return r.json()}).catch(()=>null)}
  if(page!=="companies"){
    getJson(route("companies")+"companies.json").then(d=>{
      if(!d?.companies)return;
      const rows=d.companies;
      const labels={"主观基金":"Discretionary","做市商":"Market Maker","对冲基金":"Hedge Fund","综合资管":"Multi-Asset","自营交易":"Proprietary Trading","量化资管":"Quant Manager"};
      const count=(kind,value)=>rows.filter(x=>x[kind]===value).length;
      fill("typeSub",[...new Set(rows.map(x=>x.category))].sort((a,b)=>alpha(labels[a]||a,labels[b]||b)).map(v=>({name:v,label:v+" ("+count("category",v)+")"})),r=>route("companies")+"?kind=type&value="+encodeURIComponent(r.name));
      fill("regionSub",[...new Set(rows.map(x=>x.country))].sort(sortGeo).map(v=>({name:v,label:v+" ("+count("country",v)+")"})),r=>route("companies")+"?kind=region&value="+encodeURIComponent(r.name));
    });
  }
  if(page!=="assets"){
    getJson(route("assets")+"data.json").then(d=>{
      if(!d?.assets)return;
      const arr=d.assets;
      const names=(key)=>[...new Set(arr.map(x=>x[key]))].sort((a,b)=>alpha(cnOrder[a]||a,cnOrder[b]||b));
      fill("marketChoices",names("market").map(n=>({name:n,label:n+" ("+arr.filter(x=>x.market===n).length+")"})),r=>route("assets")+"?view=market&focus="+encodeURIComponent(r.name));
      fill("typeChoices",names("instrument_group").map(n=>({name:n,label:n+" ("+arr.filter(x=>x.instrument_group===n).length+")"})),r=>route("assets")+"?view=type&focus="+encodeURIComponent(r.name));
    });
  }
  // Every module can browse the core data feed taxonomy without losing its parent tree.
  getJson(route("feeds")+"catalog.json").then(d=>{
    if(!d?.categories)return;
    fill("feedChoices",d.categories.map(c=>({name:c.id,label:c.title+" ("+c.subgroups.reduce((n,g)=>n+g.item_ids.length,0)+")"})),r=>route("feeds")+"?category="+encodeURIComponent(r.name));
  });
  getJson(route("research")+"strategies.json").then(d=>{
    if(!d?.strategies)return;
    fill("strategyChoices",[...d.strategies].sort((a,b)=>alpha(a.name,b.name)).map(t=>({name:t.id,label:t.name})),r=>route("research")+"?strategy="+encodeURIComponent(r.name));
  });
  getJson(route("wiki")+"manifest.json").then(d=>{
    if(!d?.categories)return;
    fill("wikiCategories",[...d.categories].sort((a,b)=>alpha(a.english,b.english)).map(c=>({name:c.id,label:c.name+" ("+c.count+")"})),r=>route("wiki")+"?category="+encodeURIComponent(r.name));
    fill("wikiPaths",d.learning_paths.map(p=>({name:p.id,label:p.title})),r=>route("wiki")+"?path="+encodeURIComponent(r.name));
  });
  if(page!=="papers"){
    getJson(route("papers")+"data.json").then(d=>{
      if(!d?.issues)return;
      fill("weeks",[...d.issues].sort((a,b)=>b.date.localeCompare(a.date)).map(w=>({name:w.date,label:w.date+" ("+w.papers.length+")"})),r=>route("papers")+"?issue="+encodeURIComponent(r.name));
    });
  }
  if(page!=="rankings"){
    getJson(route("rankings")+"sites.json").then(d=>{
      if(!d?.sites)return;
      const cats=[...new Set(d.sites.flatMap(s=>s.categories))].sort(alpha);
      fill("catsNav",cats.map(x=>({name:x,label:x})),r=>route("rankings")+"?category="+encodeURIComponent(r.name));
    });
  }
  // News uses links rather than global mutable callbacks, so each option works from any page.
  getJson(route("news")+"sources.json").then(d=>{
    if(!d?.regions)return;
    fill("newsRegionChoices",[...d.regions].sort((a,b)=>sortGeo(a.name,b.name)).map(r=>({name:r.name,label:r.name})),r=>route("news")+"?region="+encodeURIComponent(r.name));
    fill("newsTopicChoices",["央行","财政","宏观数据"].map(name=>({name,label:name})),r=>route("news")+"?category="+encodeURIComponent(r.name));
  });
})();
