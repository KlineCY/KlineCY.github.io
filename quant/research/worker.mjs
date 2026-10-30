/* Browser-only Python execution. No server, no credential storage.
   Strategy code runs in a dedicated Web Worker; stop terminates the worker. */
import { loadPyodide } from "https://cdn.jsdelivr.net/pyodide/v314.0.7/full/pyodide.mjs";
const pyReady = loadPyodide({indexURL:"https://cdn.jsdelivr.net/pyodide/v314.0.7/full/"});
const engineReady = fetch("./engine.py").then(r=>{
  if(!r.ok) throw new Error("Python engine HTTP "+r.status);
  return r.text();
});
self.onmessage=async ({data})=>{
  const id=data?.id;
  if(data?.type!=="run")return;
  try{
    self.postMessage({id,type:"status",message:"加载浏览器内 Python 运行时…"});
    const [py,engine] = await Promise.all([pyReady,engineReady]);
    py.globals.set("KLINE_CONFIG_JSON",JSON.stringify(data.config));
    self.postMessage({id,type:"status",message:"正在执行策略与回测分析…"});
    const result=await py.runPythonAsync(engine+"\nmain(KLINE_CONFIG_JSON)");
    const payload=JSON.parse(String(result));
    self.postMessage({id,type:"result",data:payload});
  }catch(error){
    self.postMessage({id,type:"error",message:String(error?.message||error)});
  }
};
