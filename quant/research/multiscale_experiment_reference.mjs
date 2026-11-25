const sum=a=>a.reduce((s,v)=>s+v,0),avg=a=>sum(a)/a.length,sq=x=>x*x;
function variance(a){const m=avg(a);return sum(a.map(x=>sq(x-m)))/(a.length-1)}

function covariance(a,b){const m=avg(a),n=avg(b);return sum(a.map((x,i)=>(x-m)*(b[i]-n)))/(a.length-1)}

function rng(seed){let state=seed;function uniform(){state=(48271*state)%2147483647;return state/2147483647}return ()=>Math.sqrt(-2*Math.log(uniform()))*Math.cos(2*Math.PI*uniform())}

function prices(n,sigma,random){let w=0;return Array.from({length:n},(_,i)=>{w+=random();return 100*Math.exp((.02-.5*sigma*sigma)*(i+1)/n+sigma/Math.sqrt(n)*w)})}

function pair(n,strike,sigma,random){let p=prices(n,sigma,random),a=avg(p),b=avg(p.filter((_,i)=>i%2===1)),df=Math.exp(-.02);return [df*Math.max(0,a-strike),df*Math.max(0,b-strike)]}

function mlmc(){
 let rows=[],id=0,cost=[2,2,4,8];
 for(const k of [90,100,110])for(const s of [.15,.3,.45])for(const e of [1,2,3]){
  let seed=100000+1000*++id,v=[];
  for(let l=0;l<4;l++){const random=rng(seed+10*l+1),z=Array.from({length:96},()=>{const [f,c]=pair(cost[l],k,s,random);return l===0?c:f-c});v.push(variance(z))}
  const h=sum(v.map((x,i)=>Math.sqrt(x*cost[i]))),n=v.map((x,i)=>Math.max(2,Math.ceil(h*Math.sqrt(x/cost[i])/(e*e)))),pv=sum(v.map((x,i)=>x/n[i])),c=sum(n.map((x,i)=>x*cost[i]));
  let m=[],vv=[];
  for(let l=0;l<4;l++){const random=rng(seed+10*l+2),z=Array.from({length:n[l]},()=>{const [f,c]=pair(cost[l],k,s,random);return l===0?c:f-c});m.push(avg(z));vv.push(variance(z))}
  const random=rng(seed+99),nd=Math.max(2,Math.floor(c/8)),direct=Array.from({length:nd},()=>pair(8,k,s,random)[0]);
  rows.push([k,s,e,sum(m),Math.sqrt(sum(vv.map((z,i)=>z/n[i]))),c,1536,pv,m[3],avg(direct),Math.sqrt(variance(direct)/nd),...n]);
 }
 return rows;
}

function control(){
 let rows=[],id=0;
 for(const n of [4,8,16,32])for(const s of [.1,.2,.4]){
  const seed=200000+1000*++id,t=Array.from({length:n},(_,i)=>(i+1)/n),m=t.map(z=>100*Math.exp(.02*z)),ea=avg(m),mt=avg(t),vlog=sum(t.flatMap(a=>t.map(b=>s*s*Math.min(a,b))))/(n*n),eg=100*Math.exp((.02-s*s/2)*mt+vlog/2);
  let va=sum(t.flatMap((a,i)=>t.map((b,j)=>m[i]*m[j]*Math.expm1(s*s*Math.min(a,b)))))/(n*n),vg=eg*eg*Math.expm1(vlog),co=avg(t.map((a,i)=>m[i]*eg*Math.expm1(avg(t.map(b=>s*s*Math.min(a,b))))));
  const pilot=rng(seed+1),external=rng(seed+2),draw=random=>{const p=prices(n,s,random);return [avg(p),Math.exp(avg(p.map(Math.log)))]};
  const p=Array.from({length:80},()=>draw(pilot)),y=Array.from({length:400},()=>draw(external));
  const a=p.map(z=>z[0]),g=p.map(z=>z[1]),b=covariance(a,g)/variance(g),raw=y.map(z=>z[0]),adjust=y.map(z=>z[0]-b*(z[1]-eg));
  rows.push([n,s,ea,va,eg,vg,co/vg,va-co*co/vg,b,avg(raw),avg(adjust),variance(raw),variance(adjust)])
 }
 return rows;
}

function hyRef(a,b,shift=0){
 let c=0,overlap=0,n=0;
 for(let i=1;i<a.length;i++)for(let j=1;j<b.length;j++){let len=Math.min(a[i][0],b[j][0]+shift)-Math.max(a[i-1][0],b[j-1][0]+shift);if(len>0){c+=(Math.log(a[i][1])-Math.log(a[i-1][1]))*(Math.log(b[j][1])-Math.log(b[j-1][1]));overlap+=len;n++}}
 return [c,n,overlap];
}

function refreshRef(a,b){
 const t=[Math.max(a[0][0],b[0][0])];
 while(true){let u=a.find(z=>z[0]>t.at(-1)),v=b.find(z=>z[0]>t.at(-1));if(!u||!v)break;t.push(Math.max(u[0],v[0]))}
 let x=t.map(s=>Math.log(a.filter(z=>z[0]<=s).at(-1)[1])),y=t.map(s=>Math.log(b.filter(z=>z[0]<=s).at(-1)[1]));
 let dx=x.slice(1).map((z,i)=>z-x[i]),dy=y.slice(1).map((z,i)=>z-y[i]),c=sum(dx.map((z,i)=>z*dy[i]));
 return [c,t.length,sum(dx.map(sq))*sum(dy.map(sq))-c*c];
}

function asynchronous(){
 let rows=[],id=0;
 for(const a of [2,3,5])for(const b of [3,4,6])for(const delay of [0,2,4])for(const noise of [0,.002]){
  let seed=300000+1000*++id,latent=rng(seed+1),eps=rng(seed+2),x=[0],y=[0],truth=0;
  for(let i=1;i<=120;i++){let z=latent(),w=latent(),u=.006*z,v=.004*(.6*z+.8*w);x.push(x.at(-1)+u);y.push(y.at(-1)+v);truth+=u*v}
  let p=[],q=[];for(let j=0;j<=120;j+=a)p.push([j,Math.exp(x[j]+noise*eps())]);for(let j=0;j<=120;j+=b)q.push([j+delay,Math.exp(y[j]+noise*eps())]);
  let h=hyRef(p,q),r=refreshRef(p,q),aligned=hyRef(p,q,-delay),grid=[-4,-2,0,2,4],c=grid.map(g=>hyRef(p,q,g)[0]),best=0;
  for(let j=1;j<c.length;j++)if(Math.abs(c[j])>Math.abs(c[best]))best=j;
  rows.push([a,b,delay,noise,truth,h[0],r[0],aligned[0],Math.abs(c[best]),grid[best],h[2],r[1],r[2]])
 }
 return rows;
}

function noise(){
 const rows=[];
 for(const eta of [0,.005,.01,.02])for(const k of [1,2,3,4]){
  let u=eta*eta,v=.25*u,c=-.5*u,base=.0004;
  rows.push([eta,k,base+8*u/k,2.25*base+8*v/k,1.5*base+8*c/k,base,2.25*base,1.5*base,base+4*u*1.5/1.25,base,512])
 }
 return rows;
}

function feeRoot(cash,fee,h,w){
 const total=cash+sum(h),cuts=[0,total,...h.map((a,i)=>w[i]>0?a/w[i]:0).filter(x=>x>0&&x<total)].sort((a,b)=>a-b);
 for(let j=1;j<cuts.length;j++){let lo=cuts[j-1],hi=cuts[j];if(hi===lo)continue;let signs=h.map((z,i)=>w[i]*(lo+hi)/2>=z?1:-1);
  let post=(total+fee*sum(h.map((x,i)=>x*signs[i])))/(1+fee*sum(w.map((x,i)=>x*signs[i])));
  if(post>=lo-1e-8&&post<=hi+1e-8)return post;
 }throw Error("fee root");
}

function universe(){
 let rows=[];
 for(const delay of [0,1,2])for(const fee of [0,.001,.005]){
  let q=Array(6).fill(0),p=[10,12,14,16,18,20],cash=1000;
  for(let t=0;t<8;t++){
   let eligible=Array.from({length:6},(_,i)=>i===0?(t<2+delay||t>=6+delay):i===1?t<4+delay:true);
   let scores=Array.from({length:6},(_,i)=>(3*i+2*t)%7),mx=Math.max(...scores.filter((_,i)=>eligible[i])),mn=Math.min(...scores.filter((_,i)=>eligible[i]));
   let top=scores.map((s,i)=>eligible[i]&&s===mx?1:0),bottom=scores.map((s,i)=>eligible[i]&&s===mn?1:0),w=top.map(s=>s/sum(top)),short=bottom.map(s=>s/sum(bottom)),h=q.map((z,i)=>z*p[i]);
   const v=cash+sum(h),post=feeRoot(cash,fee,h,w),tr=sum(h.map((z,i)=>Math.abs(w[i]*post-z))),f=fee*tr;
   q=w.map((z,i)=>z*post/p[i]);cash=post*(1-sum(w));p=p.map((z,i)=>z*(1+.01*((5*i+3*t)%7-3)));
   const end=cash+sum(q.map((z,i)=>z*p[i]));
   rows.push([delay,fee,t,sum(eligible.map(Number)),sum(w.map((z,i)=>z*scores[i])),v,post,f,tr,end,cash,sum(w.map((z,i)=>Math.abs(z-short[i]))),sum(w.map((z,i)=>z-short[i]))]);
  }
 }
 return rows;
}

function ridge(rows,x,lambda){
 const a=rows.map(z=>z[1]),b=rows.map(z=>z[2]),mu=avg(a),ym=avg(b),v=sum(a.map(z=>sq(z-mu))),coef=sum(a.map((z,i)=>(z-mu)*(b[i]-ym)))/(v+lambda*v/a.length);return ym+coef*(x-mu);
}

function maturity(){
 let rows=[],data=Array.from({length:45},(_,t)=>{let x=((7*t)%13-6)/5;return [t,x,.04+.3*x+.01*((t*3)%7-3),t+3+t%2]});
 for(const l of [0,1,5])for(const b of [0,2])for(let t=8;t<=40;t++){
  const x=data[t][1],y=data[t][2],train=data.filter(r=>r[0]<t&&r[3]<=t-b),naive=data.filter(r=>r[0]<t&&r[0]<=t-b);
  const p=ridge(train,x,l),q=ridge(naive,x,l);
  rows.push([l,b,t,train.length,p,q,y,sq(p-y),sq(q-y),0]);
 }
 return rows;
}

function signRef(data){
 let n=data.length,k=data[0].length,cols=Array.from({length:k},(_,j)=>data.map(r=>r[j])),den=cols.map(a=>Math.sqrt(sum(a.map(sq)))),obs=cols.map((a,j)=>Math.abs(sum(a))/den[j]),order=Array.from({length:k},(_,j)=>j).sort((a,b)=>obs[b]-obs[a]);
 let perm=[];
 for(let mask=0;mask<(1<<n);mask++){let signs=Array.from({length:n},(_,i)=>(mask&(1<<i))?1:-1);perm.push(cols.map((a,j)=>Math.abs(sum(a.map((x,i)=>x*signs[i])))/den[j]));}
 let raw=obs.map((z,j)=>perm.filter(r=>r[j]+1e-12>=z).length/perm.length),single=obs.map(z=>perm.filter(r=>Math.max(...r)+1e-12>=z).length/perm.length),step=Array(k).fill(0),prev=0;
 for(let i=0;i<k;i++){let j=order[i],p=perm.filter(r=>Math.max(...order.slice(i).map(l=>r[l]))+1e-12>=obs[j]).length/perm.length;prev=Math.max(prev,p);step[j]=prev}
 let sort=[...raw].sort((a,b)=>a-b),h=sum(Array.from({length:k},(_,i)=>1/(i+1))),bh=raw.map(p=>Math.min(1,...sort.map((z,i)=>z>=p?k*z/(i+1):1))),by=bh.map(z=>Math.min(1,z*h));
 return {obs,raw,single,step,bh,by};
}

function selection(){
 let rows=[];
 for(const n of [6,8,10,12])for(const effect of [0,.2,.5,1]){
  let data=Array.from({length:n},(_,t)=>Array.from({length:4},(_,j)=>((5*t+1)%7-3)*.3+((3*t+2*j+1)%5-2)*.2+effect*(j+1)/4)),a=signRef(data);
  for(let j=0;j<4;j++)rows.push([n,effect,j,a.obs[j],a.raw[j],a.single[j],a.step[j],a.bh[j],a.by[j],a.raw[j]<=.1?1:0,a.step[j]<=.1?1:0]);
 }
 return rows;
}

function spanning(){
 let rows=[];
 for(const alpha of [0,.01,.03,.05])for(const s of [.02,.05,.1,.2])for(const beta of [0,.5,1.5]){
  let base=.04,add=sq(alpha/s),both=base+add,sd=Math.sqrt(both),m=[-1,1].flatMap(a=>[-1,1].map(b=>1-.2*a-alpha/s*b));
  rows.push([alpha,s,beta,base,both,add,0,sd,sd,m.filter(z=>z<0).length/4,0]);
 }
 return rows;
}
export const projects={mlmc,control,async:asynchronous,noise,universe,maturity,selection,spanning};
export const columns={
  "mlmc": "Strike,Sigma,SEBudget,Estimate,EstimatedSE,ProductionCost,PilotCost,PlannedVariance,FinestCorrection,DirectFineEstimate,DirectFineSE,N0,N1,N2,N3",
  "control": "Monitoring,Sigma,EA,VarA,EG,VarG,AnalyticBeta,AnalyticResidualVariance,PilotBeta,RawExternalMean,AdjustedExternalMean,RawExternalVariance,AdjustedExternalVariance",
  "async": "SpacingX,SpacingY,TimestampDelay,Noise,LatentPathCov,HY,Refresh,ShiftAlignedHY,PeakAbsHY,PeakShift,Overlap,RefreshNodes,RefreshPSDDet",
  "noise": "NoiseSD,Lag,MeanRawVarX,MeanRawVarY,MeanRawCov,MeanCorrectedVarX,MeanCorrectedVarY,MeanCorrectedCov,MeanPreavgRaw,MeanPreavgCorrected,EnumeratedPaths",
  "universe": "AvailabilityDelay,Fee,Date,EligibleCount,LongScore,PretradeWealth,PostFeeWealth,Fees,TradeNotional,EndWealth,Cash,RankGross,RankNet",
  "maturity": "Ridge,Embargo,Decision,TrainingCount,Forecast,NaiveUnavailableForecast,ActualLabel,ProperLoss,NaiveLoss,UnavailableLabelPerturbation",
  "selection": "Dates,Effect,Strategy,Statistic,RawP,MaxTP,StepdownP,BH,BY,RawReject,StepdownReject",
  "spanning": "Alpha,ResidualSD,Beta,BaseSR2,ExpandedSR2,Increment,SpanningClosure,HJLowerBound,SDFDistance,NegativeSDFShare,MaxPricingError"
};
if(typeof process!=="undefined"&&process.argv[1]?.endsWith("multiscale_experiment_reference.mjs")){const id=process.argv[2];if(!projects[id])throw Error("unknown project");console.log(columns[id]);for(const r of projects[id]())console.log(r.join(","));}
