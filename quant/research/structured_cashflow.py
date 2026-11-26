"""Homogeneous fixed-rate mortgage pool; closed-form survivor reference."""
import math
import sys

def annuity_factor(rate, months):
    if months == 0:
        return 0.0
    if rate == 0:
        return float(months)
    return -math.expm1(-months * math.log1p(rate)) / rate

def validate(a):
    b, gross, net, n, cpr, y, h, cd, cu, senior = a
    if not all(math.isfinite(x) for x in a):
        raise ValueError("finite numeric input required")
    if not (0 < b <= 1e12 and 0 <= net <= gross <= 1 and
            1 <= n <= 600 and n == int(n) and
            all(0 <= x <= 1 for x in (cpr, cd, cu)) and
            0 < h <= .5 and -.1 <= y-h and y+h <= 1 and 0 < senior < b):
        raise ValueError("input outside documented domain")

def evaluate(a, cpr, y):
    b, gross, net, n, _, _, _, _, _, senior = a
    n = int(n)
    r = gross / 12
    installment = b / annuity_factor(r, n)
    survival = 0.0 if cpr == 1 else math.exp(math.log1p(-cpr)/12)
    def contract_balance(t):
        return installment * annuity_factor(r, n-t)
    rows = []
    cumulative = 0.0
    for t in range(1, n+1):
        previous = survival ** (t-1)
        before = contract_balance(t-1)*previous
        end = contract_balance(t)*survival**t
        scheduled = (contract_balance(t-1)-contract_balance(t))*previous
        prepay = contract_balance(t)*previous*(1-survival)
        principal = scheduled+prepay
        interest = before*net/12
        discount = (1+y/12)**(-t)
        # Sequential principal allocation follows cumulative principal, rather
        # than recursing over each tranche as in the native implementation.
        sb = max(0.0, senior-cumulative)
        jb = max(0.0, b-senior-max(0.0,cumulative-senior))
        cumulative += principal
        se = max(0.0, senior-cumulative)
        je = max(0.0, b-senior-max(0.0,cumulative-senior))
        rows.append((t,before,scheduled,prepay,end,interest,principal,discount,
                     sb,sb-se,sb*net/12,jb,jb-je,jb*net/12,se,je))
    io = math.fsum(x[5]*x[7] for x in rows)
    po = math.fsum(x[6]*x[7] for x in rows)
    wal = math.fsum(x[0]*x[6] for x in rows)/(12*b)
    sp = math.fsum((x[9]+x[10])*x[7] for x in rows)
    jp = math.fsum((x[12]+x[13])*x[7] for x in rows)
    sw = math.fsum(x[0]*x[9] for x in rows)/(12*senior)
    jw = math.fsum(x[0]*x[12] for x in rows)/(12*(b-senior))
    return (io+po,io,po,wal,sp,jp,sw,jw),rows

def model(a):
    validate(a)
    summary, rows = evaluate(a,a[4],a[5])
    p = summary[0]
    down = evaluate(a,a[7],a[5]-a[6])[0][0]
    up = evaluate(a,a[8],a[5]+a[6])[0][0]
    h = a[6]
    return summary+(down,up,(down-up)/(2*p*h),(down+up-2*p)/(p*h*h)),rows

if __name__ == "__main__":
    try:
        a = tuple(map(float,sys.argv[1:]))
        if len(a) != 10:
            raise ValueError("expected 10 inputs")
        summary, rows = model(a)
        print("S,"+",".join(format(x,".17g") for x in summary))
        for row in rows:
            print("M,"+",".join(format(x,".17g") for x in row))
    except (ValueError, OverflowError) as exc:
        print(exc,file=sys.stderr)
        sys.exit(1)
