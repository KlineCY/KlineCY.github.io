const sum=a=>a.reduce((s,z)=>s+z,0), mean=a=>sum(a)/a.length;
function variance(a){let m=mean(a);return mean(a.map(z=>(z-m)**2));}
function combinations(n,k,start=0,chosen=[]){if(k===0)return [chosen];let out=[];for(let j=start;j<=n-k;j++)out.push(...combinations(n,k-1,j+1,[...chosen,j]));return out;}
function paths(n,m,visit,p=[]){if(p.length===m){visit(p);return;}for(let j=0;j<n;j++)paths(n,m,visit,[...p,j]);}
export const columns={
 budget:"RMSE,Theta,Alpha,Beta,Gamma,Level,Bias,Variance,MSE,Cost,ContinuousCost,SampleCount",
 allocation:"Budget,VarianceMultiplier,Upper0,Variance,ContinuousBound,Gap,TotalSamples",
 noise:"NoiseSD,Lag,Window,LatentXX,LatentYY,LatentXY,NoiseXX,NoiseYY,NoiseXY,UniformRatio,SineRatio,MeanUniformNoise,MeanSineNoise,Paths",
 validation:"Groups,TestGroups,LabelLength,Embargo,Split,Paths,Test,Train,Purged,Embargoed",
 cluster:"AssetScale,DateScale,IdioScale,MissingCell,N,Alpha,Beta,RSS,AssetVariance,DateVariance,IntersectionVariance,TwoWayVariance,TruncatedSE",
 blocks:"Dates,Block,Effect,Persistence,SharpePaths,ValidPaths,UndefinedMass,SharpeMean,SharpeSD,Sharpe025,SharpeMedian,Sharpe975,ObservedMax,RealityP,NullMean,NullSD"
};
export const projects={
 budget(){let out=[];for(let eps of [.25,.125,.0625])for(let t of [.25,.5,.75])for(let a of [1,2])for(let b of [1,2,3])for(let g of [1,2]){
  let L=0;while(2**(-a*L)>eps*Math.sqrt(1-t))L++;
  let V=Array.from({length:L+1},(_,j)=>2**(-b*j)),C=V.map((_,j)=>2**(g*j)),h=sum(V.map((v,j)=>Math.sqrt(v*C[j]))),n=V.map((v,j)=>Math.max(2,Math.ceil(h*Math.sqrt(v/C[j])/(eps*Math.sqrt(t))**2)));
  let bias=2**(-a*L),v=sum(V.map((z,j)=>z/n[j])),cost=sum(C.map((z,j)=>z*n[j])),lb=h*h/(eps*Math.sqrt(t))**2;
  out.push([eps,t,a,b,g,L,bias,v,bias*bias+v,cost,lb,sum(n)]);
 }return out;},
 allocation(){let out=[];for(let N of [8,12,16,24,32])for(let a of [.25,1,4,16])for(let u of [3,5,8]){
  let A=[.2*.2*a,.3*.3*2,.5*.5*.5],best=Infinity;
  for(let i=1;i<=Math.min(N-2,u);i++)for(let j=1;j<N-i;j++)best=Math.min(best,A[0]/i+A[1]/j+A[2]/(N-i-j));
  let lb=sum(A.map(Math.sqrt))**2/N;out.push([N,a,u,best,lb,best-lb,N]);
 }return out;},
 noise(){let out=[];for(let e of [0,.005,.01,.02])for(let k of [2,3,4])for(let w of [2,3,4]){
  let u=2/w,s=2-2*Math.cos(Math.PI/(w+1)),v=e*e;
  out.push([e,k,w,.0004,.0009,.0006,v,.25*v,-.5*v,u,s,v*u,v*s,512]);
 }return out;},
 validation(){let out=[];for(let N of [3,4,5,6])for(let K=1;K<N;K++)for(let l of [1,3,5])for(let gap of [0,2]){
  let f=combinations(N,K),phi=combinations(N-1,K-1).length;
  for(let i=0;i<f.length;i++){let test=[],train=[];for(let j=0;j<3*N;j++)(f[i].includes(Math.floor(j/3))?test:train).push(j);
   let overlap=train.filter(j=>test.some(t=>Math.max(j,t)<Math.min(j+l,t+l)));
   let embargo=train.filter(j=>!overlap.includes(j)&&test.some(t=>t+l<=j&&j<t+l+gap));
   out.push([N,K,l,gap,i,phi,test.length,train.length-overlap.length-embargo.length,overlap.length,embargo.length]);
  }
 }return out;},
 cluster(){let out=[];for(let a of [0,.2,.5])for(let b of [0,.2,.5])for(let e of [.1,.3,.6])for(let missing of [0,1]){
  let rows=[];for(let i=0;i<5;i++)for(let t=0;t<6;t++){if(missing&&i===1&&t===2)continue;let z=(i-2)*.4+(t-2.5)*.2+.1*((i+t)%3);rows.push({i,t,x:z,y:.05+.7*z+a*(i-2)+b*(t-2.5)+e*((3*i+2*t)%5-2)});}
  let xm=mean(rows.map(r=>r.x)),ym=mean(rows.map(r=>r.y)),xx=sum(rows.map(r=>(r.x-xm)**2)),beta=sum(rows.map(r=>(r.x-xm)*(r.y-ym)))/xx,alpha=ym-beta*xm;
  let resid=rows.map(r=>r.y-alpha-beta*r.x),u=rows.map((r,j)=>(r.x-xm)*resid[j]);
  function meat(test){let s=0;for(let i=0;i<rows.length;i++)for(let j=0;j<rows.length;j++)if(test(rows[i],rows[j]))s+=u[i]*u[j];return s/(xx*xx);}
  let v1=meat((r,s)=>r.i===s.i),v2=meat((r,s)=>r.t===s.t),v12=meat((r,s)=>r.i===s.i&&r.t===s.t),v=v1+v2-v12;
  out.push([a,b,e,missing,rows.length,alpha,beta,sum(resid.map(z=>z*z)),v1,v2,v12,v,Math.sqrt(Math.max(0,v))]);
 }return out;},
 blocks(){let out=[];for(let n of [6,8,12])for(let k of [2,3,4,6])if(n%k===0&&n/k<=3)for(let e of [0,.2,.5])for(let p of [0,1]){
  let state=0,rows=[];for(let t=0;t<n;t++){state=(p?.7:0)*state+((5*t+1)%7-3)*.1;rows.push([state+e,state+.5*e+.1*((t+2)%3-1),-.5*state+e*.25+.05*((2*t)%3-1)]);}
  let mu=[0,1,2].map(j=>mean(rows.map(r=>r[j]))),obs=Math.max(...mu),ss=[],nulls=[];
  paths(n,n/k,starts=>{let r=starts.flatMap(s=>Array.from({length:k},(_,j)=>rows[(s+j)%n]));let a=r.map(z=>z[0]);if(Math.max(...a)>Math.min(...a)){let v=variance(a);ss.push(mean(a)/Math.sqrt(v));}nulls.push(Math.max(...[0,1,2].map(j=>mean(r.map(z=>z[j]-mu[j])))));});
  ss.sort((a,b)=>a-b);let q=x=>ss[Math.max(1,Math.ceil(x*ss.length))-1];
  out.push([n,k,e,p,nulls.length,ss.length,1-ss.length/nulls.length,mean(ss),Math.sqrt(variance(ss)),q(.025),q(.5),q(.975),obs,nulls.filter(v=>v+1e-12>=obs).length/nulls.length,mean(nulls),Math.sqrt(variance(nulls))]);
 }return out;}
};
