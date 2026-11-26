#pragma once
#include "predictive_optimization.hpp"
namespace pdq {
inline Vec causal_conv_vjp(const Vec& x){
    require(x.size()>=7);int n=integer(x[0],2,10000),k=integer(x[1],1,1000),d=integer(x[2],1,10000);double bias=x[3];require(x.size()==size_t(4+k+2*n));
    Vec w(x.begin()+4,x.begin()+4+k),a(x.begin()+4+k,x.begin()+4+k+n),g(x.begin()+4+k+n,x.end()),y(n,bias),dx(n),dw(k);
    for(int t=0;t<n;++t)for(int j=0;j<k;++j){int i=t-j*d;if(i<0)break;y[t]+=w[j]*a[i];dx[i]+=w[j]*g[t];dw[j]+=a[i]*g[t];}
    Vec out{dot(y,g),sum(g)};for(const auto& v:{y,dx,dw})out.insert(out.end(),v.begin(),v.end());return out;
}
inline Vec diagonal_scan_vjp(const Vec& x){
    require(x.size()>=8);int n=integer(x[0],1,10000),d=integer(x[1],1,100);require(x.size()==size_t(2+4*d+2*n*d));
    Vec a(x.begin()+2,x.begin()+2+d),b(x.begin()+2+d,x.begin()+2+2*d),c(x.begin()+2+2*d,x.begin()+2+3*d),h0(x.begin()+2+3*d,x.begin()+2+4*d);
    for(double z:a){require(std::abs(z)<=1);}
    Mat X=zeros(n,d),G=X,H=X,Y=X,dx=X;size_t at=2+4*d;for(Mat* m:{&X,&G})for(auto& row:*m)for(double& z:row)z=x[at++];
    double loss=0;Vec da(d),db(d),dc(d),carry(d);
    for(int t=0;t<n;++t)for(int j=0;j<d;++j){double prev=t?H[t-1][j]:h0[j];H[t][j]=a[j]*prev+b[j]*X[t][j];Y[t][j]=c[j]*H[t][j];loss+=Y[t][j]*G[t][j];}
    for(int t=n-1;t>=0;--t)for(int j=0;j<d;++j){double prev=t?H[t-1][j]:h0[j],z=carry[j]+G[t][j]*c[j];dc[j]+=G[t][j]*H[t][j];da[j]+=z*prev;db[j]+=z*X[t][j];dx[t][j]=z*b[j];carry[j]=z*a[j];}
    Vec out{loss};for(const Mat* m:{&H,&Y})for(const auto& row:*m)out.insert(out.end(),row.begin(),row.end());for(const auto& v:{da,db,dc,carry})out.insert(out.end(),v.begin(),v.end());for(const auto& row:dx)out.insert(out.end(),row.begin(),row.end());return out;
}
inline Vec gru_scalar_vjp(const Vec& x){
    require(x.size()>=13&&(x.size()-10)%2==0);Vec w(x.begin(),x.begin()+9);double h0=x[9];int n=(x.size()-10)/2;require(n<=100000);
    Vec a(n),g(n),h(n),z(n),r(n),v(n),dw(9),dx(n);for(int t=0;t<n;++t){a[t]=x[10+2*t];g[t]=x[11+2*t];}
    for(int t=0;t<n;++t){double prev=t?h[t-1]:h0;z[t]=sigmoid(w[0]*a[t]+w[1]*prev+w[2]);r[t]=sigmoid(w[3]*a[t]+w[4]*prev+w[5]);v[t]=std::tanh(w[6]*a[t]+w[7]*r[t]*prev+w[8]);h[t]=z[t]*prev+(1-z[t])*v[t];}
    double carry=0;for(int t=n-1;t>=0;--t){
        double prev=t?h[t-1]:h0,G=g[t]+carry,dz=G*(prev-v[t])*z[t]*(1-z[t]),dv=G*(1-z[t])*(1-v[t]*v[t]),dr=dv*w[7]*prev*r[t]*(1-r[t]);
        dw[0]+=dz*a[t];dw[1]+=dz*prev;dw[2]+=dz;dw[3]+=dr*a[t];dw[4]+=dr*prev;dw[5]+=dr;dw[6]+=dv*a[t];dw[7]+=dv*r[t]*prev;dw[8]+=dv;
        dx[t]=dz*w[0]+dr*w[3]+dv*w[6];carry=G*z[t]+dz*w[1]+dr*w[4]+dv*w[7]*r[t];
    }
    Vec out{dot(h,g),carry};for(const auto& a0:{h,z,r,v,dw,dx})out.insert(out.end(),a0.begin(),a0.end());return out;
}
inline Vec patch_embedding_vjp(const Vec& x){
    require(x.size()>=10);int n=integer(x[0],2,200),c=integer(x[1],1,20),p=integer(x[2],1,30),d=integer(x[3],1,30),m=(n+p-1)/p,k=p*c;
    require(x.size()==size_t(4+n+n*c+d*k+d+m*d));
    size_t at=4;Vec valid(n);for(double& v:valid)v=integer(x[at++],0,1);
    Mat X=zeros(n,c),W=zeros(d,k),G=zeros(m,d),Y=G,dX=X,dW=W;Vec bias(d),db(d),mask(m);
    for(Mat* M:{&X,&W})for(auto& row:*M)for(double& v:row)v=x[at++];
    for(double& v:bias)v=x[at++];
    for(auto& row:G)for(double& v:row)v=x[at++];
    double loss=0;
    for(int j=0;j<m;++j){Vec z(k);for(int t=0;t<p;++t){int i=j*p+t;if(i<n&&valid[i]){mask[j]=1;for(int q=0;q<c;++q)z[t*c+q]=X[i][q];}}
        if(mask[j]==0)continue;
        for(int a=0;a<d;++a){Y[j][a]=dot(W[a],z)+bias[a];double g=G[j][a];loss+=g*Y[j][a];db[a]+=g;for(int t=0;t<k;++t)dW[a][t]+=g*z[t];}
        for(int t=0;t<p;++t){int i=j*p+t;if(i<n&&valid[i])for(int q=0;q<c;++q)for(int a=0;a<d;++a)dX[i][q]+=G[j][a]*W[a][t*c+q];}
    }
    Vec out{loss,double(m),sum(mask)};out.insert(out.end(),mask.begin(),mask.end());for(const Mat* M:{&Y,&dX,&dW})for(const auto& row:*M)out.insert(out.end(),row.begin(),row.end());out.insert(out.end(),db.begin(),db.end());return out;
}
inline Vec ohlc_raster(const Vec& x){
    require(x.size()>=12);int n=integer(x[0],2,200),h=integer(x[1],2,200);require(x.size()==size_t(2+5*n));int width=3*n;
    double lo=std::numeric_limits<double>::infinity(),hi=0,vmax=0;for(int i=0;i<n;++i){size_t j=2+5*i;double o=x[j],u=x[j+1],l=x[j+2],c=x[j+3],v=x[j+4];positive(l);require(l<=o&&l<=c&&u>=o&&u>=c&&v>=0);lo=std::min(lo,l);hi=std::max(hi,u);vmax=std::max(vmax,v);}
    Vec price(h*width),volume(h*width);auto y=[&](double p){return hi==lo?(h-1)/2:std::clamp(int(std::floor((p-lo)/(hi-lo)*(h-1)+.5)),0,h-1);};
    for(int i=0;i<n;++i){size_t j=2+5*i;int o=y(x[j]),u=y(x[j+1]),l=y(x[j+2]),c=y(x[j+3]);price[o*width+3*i]=1;price[c*width+3*i+2]=1;for(int r=l;r<=u;++r)price[r*width+3*i+1]=1;
        int k=vmax>0?std::clamp(int(std::floor(h*x[j+4]/vmax+.5)),0,h):0;for(int r=0;r<k;++r)volume[r*width+3*i+1]=1;}
    Vec out{lo,hi,vmax,sum(price),sum(volume),double(h),double(width)};out.insert(out.end(),price.begin(),price.end());out.insert(out.end(),volume.begin(),volume.end());return out;
}
inline Vec matured_gaussian_loss(const Vec& x){
    require(x.size()>=8);double now=x[0];int n=integer(x[1],1,100000);require(x.size()==size_t(2+6*n));
    Vec dmu(n),dl(n),mask(n);double loss=0,ws=0,count=0;
    for(int i=0;i<n;++i){size_t j=2+6*i;double t=x[j],a=x[j+1],y=x[j+2],mu=x[j+3],l=x[j+4],w=x[j+5];require(a>=t&&w>=0&&l>=-50&&l<=50);
        if(t>now||a>now||w==0)continue;
        mask[i]=1;++count;ws+=w;double e=mu-y,v=std::exp(-l);loss+=w*.5*(std::log(2*std::acos(-1.))+l+e*e*v);dmu[i]=w*e*v;dl[i]=w*.5*(1-e*e*v);}
    positive(ws);for(int i=0;i<n;++i){dmu[i]/=ws;dl[i]/=ws;}
    Vec out{loss/ws,ws,count};for(const auto& v:{mask,dmu,dl})out.insert(out.end(),v.begin(),v.end());return out;
}
}
