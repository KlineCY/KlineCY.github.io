import {methods as kernel} from './predictive_kernel_reference.mjs';
import {methods as online} from './predictive_online_reference.mjs';
import {methods as neural} from './predictive_neural_reference.mjs';
import {methods as portfolio} from './predictive_portfolio_reference.mjs';
import {methods as interpretation} from './predictive_interpretation_reference.mjs';
import {methods as sequence} from './predictive_sequence_reference.mjs';
const M={...kernel,...online,...neural,...portfolio,...interpretation,...sequence};
const sum=x=>x.reduce((s,z)=>s+z,0),mean=x=>sum(x)/x.length,sq=x=>x*x,dot=(a,b)=>sum(a.map((z,i)=>z*b[i]));
export const columns={
 kernel:"Gamma,Landmarks,Lambda,Shift,LOOMSE,FullTestMSE,NystromTestMSE,PredictionGap,MeanDerivativeVariance,RFFGramRMSE",
 graph:"Labels,Gamma,Ridge,Noise,GraphMSE,LocalMSE,MeanMSE,Energy,Stationarity",
 delayed:"Length,Switch,Delay,Eta,Loss,ImmediateLoss,FirstRegimeLoss,SecondRegimeLoss,Pending,FinalWeight,PoisonPredictionGap",
 experts:"Length,Switch,Eta,Share,CumulativeLoss,BestFixed,FixedRegret,DynamicOracle,SecondRegimeMass,FinalMass",
 attention:"Length,Dilation,Cut,Shock,PrefixDifference,FutureDifference,AttentionLoss,MaskedMass,GradientNorm",
 recurrence:"Length,Decay,GateBias,ScanFinal,ScanFirstInputGradient,ScanInitialGradient,GRUFinal,GRUFirstInputGradient,GRUInitialGradient",
 decision:"Regime,Eta,Epochs,Model,Parameter,TrainMSE,TrainSPO,TestMSE,TestSPO,TestSPOPlus",
 utility:"Temperature,RiskPenalty,Fee,Shift,TrainStartUtility,TrainFinalUtility,TestUtility,TestMean,TestVariance,Turnover,W0,W1,W2,GradientSum"
};
function kernelProject(){let rows=[],n=12,m=6,p=Array.from({length:n},(_,i)=>-.9+.18*i),y=p.map((v,i)=>Math.sin(2*v)+.1*((3*i)%5-2)),data=p.flatMap((v,i)=>[v,y[i]]);
 for(let g of [.2,1,3])for(let k of [2,4,8,12])for(let l of [.05,.2,.8])for(let shift of [0,.4]){
  let loo=M.ridge_loo([n,g,l,...data]),freq=Array.from({length:k},(_,j)=>Math.sqrt(2*g)*(2*j+1-k)/k*2),gram=M.fourier_gram([n,k,g,...p,...freq]),full=0,low=0,gap=0,dvar=0;
  for(let j=0;j<m;j++){let q=-.8+.34*j+shift,z=Math.sin(2*q)+.1*((2*j)%5-2),b=M.gp_derivative([n,g,l,q,...data]),c=M.nystrom_ridge([n,k,g,.001,l,q,...data,...Array.from({length:k},(_,i)=>Math.floor(i*(n-1)/(k-1)))]);full+=sq(b[0]-z)/m;low+=sq(c[0]-z)/m;gap+=sq(b[0]-c[0])/m;dvar+=b[3]/m;}
  rows.push([g,k,l,shift,loo[0],full,low,gap,dvar,gram[0]]);
 }return rows;}
function graphProject(){let out=[],n=15,p=Array.from({length:n},(_,i)=>i/7-1),y=p.map(v=>Math.sin(2*v));for(let k of [3,5,8])for(let g of [.5,2,8])for(let l of [.02,.2])for(let noise of [0,.2]){
 let ids=Array.from({length:k},(_,j)=>Math.floor(j*(n-1)/(k-1))),labels=ids.map((i,j)=>y[i]+noise*((j*3)%5-2)),x=[n,k,l];for(let i=0;i<n;i++)for(let j=i+1;j<n;j++)x.push(Math.exp(-g*sq(p[i]-p[j])));x.push(...ids.flatMap((i,j)=>[i,labels[j]]));let f=M.harmonic_graph(x),train=ids.flatMap((i,j)=>[p[i],labels[j]]),gm=0,lm=0,mm=0;for(let i=0;i<n;i++)if(!ids.includes(i)){let a=M.local_linear_smoother([p[i],.5,...train]);gm+=sq(f[2+i]-y[i])/(n-k);lm+=sq(a[0]-y[i])/(n-k);mm+=sq(mean(labels)-y[i])/(n-k);}out.push([k,g,l,noise,gm,lm,mm,f[0],f[1]]);
 }return out;}
function delayedProject(){let out=[];for(let n of [30,60,90])for(let sw of [10,20])for(let d of [1,3,6])for(let eta of [.05,.2,.5]){let data=Array.from({length:n},(_,t)=>{let x=((t*5)%13-6)/6,y=(t<sw?.8:-.8)*x+.04*((t*3)%7-3);return [x,y,d+t%2,1];}),run=rows=>M.delayed_ogd([eta,-2,2,0,...rows.flat()]),a=run(data),b=run(data.map(r=>[r[0],r[1],1,1])),c=run(data.map((r,t)=>[r[0],r[1]+(t+r[2]>=n?10000:0),r[2],1])),l=Array.from({length:n},(_,t)=>a[5+4*t]),gap=Math.max(...Array.from({length:n},(_,t)=>Math.abs(a[4+4*t]-c[4+4*t])));out.push([n,sw,d,eta,a[0],b[0],sum(l.slice(0,sw)),sum(l.slice(sw)),a[2],a[1],gap]);}return out;}
function expertsProject(){let out=[];for(let n of [30,60,90])for(let sw of [10,20])for(let eta of [.5,2,4])for(let share of [0,.05,.2]){let l=Array.from({length:n},(_,t)=>[(t<sw?.1:.7)+.02*(t%4),.35+.02*((t+1)%4),(t<sw?.7:.1)+.02*((t+2)%4)]),a=M.fixed_share([3,eta,share,...l.flat()]);out.push([n,sw,eta,share,a[0],a[1],a[2],sum(l.map(r=>Math.min(...r))),a[3+4*sw+3],a.at(-1)]);}return out;}
function attentionChain(x,d){let n=x.length,a=M.causal_conv_vjp([n,3,d,.1,.5,-.2,.3,...x,...Array(n).fill(1)]),y=a.slice(2,2+n),q=[],k=[],v=[],g=[];for(let t=0;t<n;t++){q.push(y[t],x[t]);k.push(.7*y[t],.2*x[t]);v.push(y[t],.5*x[t]);g.push(1,-.3);}return M.causal_attention_vjp([n,2,...q,...k,...v,...g]);}
function attentionProject(){let out=[];for(let n of [6,8,12])for(let d of [1,2,3])for(let cut of [2,4])for(let shock of [1,4]){let x=Array.from({length:n},(_,t)=>Math.sin(.3*t)+.1*((t*3)%5-2)),z=x.map((v,t)=>v+(t>=cut?shock:0)),a=attentionChain(x,d),b=attentionChain(z,d),off=1+n*n,e=a.slice(off,off+2*n).map((v,i)=>Math.abs(v-b[off+i])),masked=0;for(let i=0;i<n;i++)for(let j=i+1;j<n;j++)masked+=a[1+i*n+j];out.push([n,d,cut,shock,Math.max(...e.slice(0,2*cut)),Math.max(...e.slice(2*cut)),a[0],masked,Math.sqrt(sum(a.slice(off+2*n).map(sq)))]);}return out;}
function recurrenceProject(){let out=[];for(let n of [6,12,24])for(let decay of [.2,.6,.95])for(let gate of [-1,0,2]){let x=Array.from({length:n},(_,t)=>.2*Math.sin(.3*t)+.1*((2*t)%5-2)),g=Array.from({length:n},(_,i)=>i===n-1?1:0),u=M.diagonal_scan_vjp([n,1,decay,.7,1,.1,...x,...g]),v=M.gru_scalar_vjp([.2,.1,gate,.2,.1,0,.7,.4,0,.1,...x.flatMap((z,i)=>[z,g[i]])]);out.push([n,decay,gate,u[0],u[1+2*n+4],u[1+2*n+3],v[0],v[2+4*n+9],v[1]]);}return out;}
function costs(t,regime){let z=((t*5)%17-8)/8;return [.02+.35*z+regime*.1*((t+1)%4-1.5),-.03-.25*z+.1*Math.sin(.4*t),.08+.12*z-.15*Math.sin(.2*t)];}
function pred(t,beta){let z=((t*5)%17-8)/8;return [.02+beta*z,-.03-beta*z,.08+.2*beta*z];}
function decisionProject(){let out=[],cap=[.55,.65,.6];for(let regime of [0,.5,1])for(let eta of [.01,.03,.08])for(let epochs of [5,15,40])for(let method of [0,1]){let beta=.15;for(let ep=0;ep<epochs;ep++){let g=0;for(let t=0;t<24;t++){let c=costs(t,regime),p=pred(t,beta),d=M.spo_plus([...c,...p,...cap]),z=((t*5)%17-8)/8,f=[z,-z,.2*z];g+=method?dot(d.slice(13),f)/24:2*dot(p.map((v,i)=>v-c[i]),f)/(24*3);}beta=Math.max(-1,Math.min(1,beta-eta*g));}let row=[regime,eta,epochs,method,beta];for(let part of [0,1]){let lo=part?24:0,hi=part?40:24,mse=0,spo=0,plus=0;for(let t=lo;t<hi;t++){let c=costs(t,regime),p=pred(t,beta),d=M.spo_plus([...c,...p,...cap]);mse+=mean(p.map((v,i)=>sq(v-c[i])))/(hi-lo);spo+=d[0]/(hi-lo);plus+=d[1]/(hi-lo);}row.push(mse,spo);if(part)row.push(plus);}out.push(row);}return out;}
function portfolioData(start,m,shift){return Array.from({length:m},(_,i)=>{let t=start+i,common=.015*((5*t)%7-3);return [.004+common+.01*Math.sin(.2*t),.002+shift+common*.3+.015*Math.cos(.3*t),-.001-shift*.4-common*.5+.012*Math.sin(.5*t)];}).flat();}
function utilityProject(){let out=[];for(let T of [.3,.7,1])for(let gamma of [0,5,20])for(let fee of [0,.001,.005])for(let shift of [0,.01]){let tr=portfolioData(0,24,0),te=portfolioData(24,16,shift),s=[0,0,0],f=(s,r)=>M.soft_portfolio_vjp([3,r.length/3,T,gamma,fee,.001,...s,1/3,1/3,1/3,...r]),init=f(s,tr);for(let i=0;i<60;i++){let a=f(s,tr);s=s.map((v,j)=>v+5*a[8+j]);}let a=f(s,tr),b=f(s,te);out.push([T,gamma,fee,shift,init[0],a[0],b[0],b[1],b[2],b[3],b[5],b[6],b[7],b[4]]);}return out;}
export const projects={kernel:kernelProject,graph:graphProject,delayed:delayedProject,experts:expertsProject,attention:attentionProject,recurrence:recurrenceProject,decision:decisionProject,utility:utilityProject};
