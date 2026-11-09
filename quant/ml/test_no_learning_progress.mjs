import assert from "node:assert/strict";
import fs from "node:fs";
import vm from "node:vm";
const root=new URL("./",import.meta.url);
const read=name=>fs.readFileSync(new URL(name,root),"utf8");
const html=read("index.html"),js=read("atlas.js"),css=read("atlas.css");
new vm.Script(js);
const forbiddenIds=[
 "atlas-read-count","atlas-learned-only","atlas-learned-count","atlas-unread-only",
 "atlas-read-toggle","atlas-resume","atlas-export","atlas-import","atlas-import-file"
];
for(const id of forbiddenIds){
 assert(!html.includes('id="'+id+'"'),"Retired learning progress control is still rendered: "+id);
 assert(!js.includes('"'+id+'"'),"Retired learning progress control is still referenced: "+id);
 assert(!css.includes("#"+id),"Retired learning progress CSS is still present: "+id);
}
for(const name of [
 "learnedOnly","unreadOnly","continueUnread","setProgressFilter",
 "serializeReadProgress","parseReadProgress","saveRead","kline-quant-atlas-read-v1",
 "atlas-done-mark","atlas-progress-filter"
])assert(!js.includes(name)&&!html.includes(name)&&!css.includes(name),"Learning progress feature reintroduced: "+name);
assert(!/\b(?:localStorage|sessionStorage|indexedDB)\b/.test(js),
 "Published Atlas must not store learning completion or reading progress");
assert(!/(?:已学|未学|标记已学|学习进度|进度导入|进度导出)/.test(html),
 "Published page still describes learning completion or progress");
assert(!fs.existsSync(new URL("test_progress.mjs",root)),"Obsolete progress-specific test was not removed");
for(const id of ["atlas-search","atlas-topic","atlas-clear","atlas-toc","atlas-lesson",
                  "atlas-reader-section","atlas-toggle-catalog"])
 assert(html.includes('id="'+id+'"'),"Core navigation control missing: "+id);
assert(js.includes("function visibleLessons(){")&&js.includes("function renderDirectory(){"),
 "Course search and hierarchical directory missing");
assert(js.includes("const prev=state.index[position-1],next=state.index[position+1]")&&
 js.includes("atlas-copy-link")&&js.includes("atlas-jump-depth"),
 "Previous/next, link copy or advanced derivations were lost");
assert(js.includes('c.rows.length+\' 篇\'')&&js.includes('group.count+\' 篇\''),
 "Directory must show curriculum size, not completed/total ratios");
assert(html.indexOf('id="atlas-reader-section"')<html.indexOf('aria-label="系统学习路径"'),
 "The V115 reader-first layout must be preserved");
assert(html.includes("atlas.js?v=20261109-v115-reader-layout-v116-ml-bridges-v117-no-learning-progress"),
 "JS cache must be refreshed after removing retired controls");
assert(html.includes("atlas.css?v=20261109-v115-reader-layout-v117-no-learning-progress"),
 "CSS cache must be refreshed after removing retired controls");
console.log("PASS: no learner tracking, local storage, completion controls or progress ratios; core reading navigation retained");
