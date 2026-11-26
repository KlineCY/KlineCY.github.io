#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

struct Input {
    double balance, gross, net;
    int months;
    double cpr, yield, bump, cprDown, cprUp, senior;
};
struct Row {
    int month;
    double begin, scheduled, prepay, end, interest, principal, discount,
           seniorBegin, seniorPrincipal, seniorInterest,
           juniorBegin, juniorPrincipal, juniorInterest, seniorEnd, juniorEnd;
};
struct Result {
    std::vector<Row> rows;
    double io=0, po=0, wal=0, seniorPrice=0, juniorPrice=0,
           seniorWal=0, juniorWal=0;
    double price() const { return io+po; }
};
double parse(const char* s) {
    std::size_t pos=0;
    double x=std::stod(s,&pos);
    if (pos!=std::string(s).size() || !std::isfinite(x))
        throw std::invalid_argument("finite numeric input required");
    return x;
}
double smm(double cpr) {
    return cpr==1 ? 1 : -std::expm1(std::log1p(-cpr)/12);
}
double annuity(double r,int n) {
    return r==0 ? 1.0/n : r/(-std::expm1(-n*std::log1p(r)));
}
void validate(const Input& a) {
    if (!(a.balance>0 && a.balance<=1e12 && a.gross>=0 && a.gross<=1
          && a.net>=0 && a.net<=a.gross && a.months>=1 && a.months<=600
          && a.cpr>=0 && a.cpr<=1 && a.cprDown>=0 && a.cprDown<=1
          && a.cprUp>=0 && a.cprUp<=1 && a.bump>0 && a.bump<=0.5
          && a.yield-a.bump>=-0.1 && a.yield+a.bump<=1
          && a.senior>0 && a.senior<a.balance))
        throw std::invalid_argument("input outside documented domain");
}
Result evaluate(const Input& a,double cpr,double y) {
    Result out;
    double b=a.balance, senior=a.senior, junior=b-senior;
    const double r=a.gross/12, net=a.net/12, q=smm(cpr);
    for(int t=1;t<=a.months;++t) {
        int left=a.months-t+1;
        double sp=left==1 ? b : std::clamp(b*(annuity(r,left)-r),0.0,b);
        double pp=(b-sp)*q;
        double p=sp+pp;
        double end=std::max(0.0,b-p);
        double i=b*net;
        double d=std::exp(-t*std::log1p(y/12));
        double ps=std::min(senior,p), pj=std::min(junior,std::max(0.0,p-ps));
        double is=senior*net, ij=junior*net;
        double se=std::max(0.0,senior-ps), je=std::max(0.0,junior-pj);
        out.rows.push_back({t,b,sp,pp,end,i,p,d,senior,ps,is,junior,pj,ij,se,je});
        out.io+=i*d; out.po+=p*d;
        out.wal+=t*p/(12*a.balance);
        out.seniorPrice+=(ps+is)*d; out.juniorPrice+=(pj+ij)*d;
        out.seniorWal+=t*ps/(12*a.senior);
        out.juniorWal+=t*pj/(12*(a.balance-a.senior));
        b=end; senior=se; junior=je;
    }
    return out;
}
int main(int argc,char** argv) {
    try {
        if(argc!=11) throw std::invalid_argument("balance gross net months cpr yield bump cpr_down cpr_up senior");
        double n=parse(argv[4]);
        if(n<1 || n>600 || n!=std::floor(n)) throw std::invalid_argument("integer months required");
        Input a{parse(argv[1]),parse(argv[2]),parse(argv[3]),static_cast<int>(n),
                parse(argv[5]),parse(argv[6]),parse(argv[7]),parse(argv[8]),parse(argv[9]),parse(argv[10])};
        validate(a);
        Result base=evaluate(a,a.cpr,a.yield);
        double p=base.price(), down=evaluate(a,a.cprDown,a.yield-a.bump).price(),
               up=evaluate(a,a.cprUp,a.yield+a.bump).price();
        double duration=(down-up)/(2*p*a.bump);
        double convexity=(down+up-2*p)/(p*a.bump*a.bump);
        std::cout<<std::setprecision(17);
        std::cout<<"S,"<<p<<','<<base.io<<','<<base.po<<','<<base.wal<<','
                 <<base.seniorPrice<<','<<base.juniorPrice<<','<<base.seniorWal<<','
                 <<base.juniorWal<<','<<down<<','<<up<<','<<duration<<','<<convexity<<'\n';
        for(const Row& x:base.rows)
            std::cout<<"M,"<<x.month<<','<<x.begin<<','<<x.scheduled<<','<<x.prepay<<','
                     <<x.end<<','<<x.interest<<','<<x.principal<<','<<x.discount<<','
                     <<x.seniorBegin<<','<<x.seniorPrincipal<<','<<x.seniorInterest<<','
                     <<x.juniorBegin<<','<<x.juniorPrincipal<<','<<x.juniorInterest<<','
                     <<x.seniorEnd<<','<<x.juniorEnd<<'\n';
    } catch(const std::exception& e) {
        std::cerr<<e.what()<<'\n';
        return 1;
    }
}
