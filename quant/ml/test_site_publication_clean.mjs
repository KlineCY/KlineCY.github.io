import assert from "node:assert/strict";
import fs from "node:fs";
import vm from "node:vm";
const root=new URL("./",import.meta.url);
const read=(file)=>fs.readFileSync(new URL(file,root),"utf8");
const ml=read("index.html"),math=read("../math/index.html");
const atlas=read("atlas.js"),mathRuntime=read("../math/app.js");
new vm.Script(atlas);
new vm.Script(mathRuntime);
const required=[
 [ml,'id="atlas-reader-section"'],[ml,'id="atlas-search"'],
 [ml,'id="atlas-toc"'],[ml,'aria-label="系统学习路径"'],
 [math,'id="math-reader-section"'],[math,'id="tree"'],
 [math,'id="articlePane"'],[math,'id="roadmap"']
];
for(const [html,token]of required)assert(html.includes(token),"Public course reader or navigation missing: "+token);
for(const [name,html] of [["ML",ml],["math",math]]){
 assert(!/<details class="(?:atlas|math)-spectral-path/.test(html),
  name+" still shows historical per-release navigation panels");
 assert(!/本轮|此次|本次|原有课程|独立稿|待审|待扩|施工|编写进度说明|V\\d{2,3}\\s*[·：]/.test(html),
  name+" page still includes revision diary or incomplete-work messaging");
}
for(const old of ["CURRICULUM_V2.md","ml/atlas/README.md"]){
 assert(!fs.existsSync(new URL("../"+old,root)),"Legacy workflow journal remains: "+old);
}
const entries=fs.readdirSync(root,{withFileTypes:true}).filter(e=>e.isFile()).map(e=>e.name);
assert(entries.every(name=>!/^notes-.*\\.json$|^lessons-.*\\.json$/.test(name)),
 "A retired draft or duplicate textbook bundle remains alongside the production Atlas");
for(const p of ["legacy.html","legacy-before-atlas.html","manifest.json","curriculum.json","app.js","styles.css"])
 assert(!entries.includes(p),"Obsolete ML app or index remains: "+p);
assert(JSON.parse(read("atlas-depth-manifest.json")).expected_lessons===720,"Canonical Atlas manifest missing");
assert(JSON.parse(read("../math/curriculum.json")).nodes.length===656,"Math curriculum damaged");
console.log("PASS: public course pages have no revision diaries or abandoned draft bundles; canonical readers and data intact");
