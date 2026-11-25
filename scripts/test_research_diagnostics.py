import csv,json,math,pathlib,subprocess,sys
root=pathlib.Path(__file__).resolve().parents[1]
refs=json.loads((root/'quant/research/diagnostic-cases.json').read_text(encoding='utf-8'))
expected={(c['id'],str(c['scenario']),f):v for c in refs for f,v in zip(c['fields'],c['expected'])}
raw=subprocess.check_output([sys.argv[1]],text=True)
actual={}
for r in csv.DictReader(raw.splitlines()):
    k=(r['id'],r['scenario'],r['field'])
    assert k not in actual,k
    actual[k]=float(r['value'])
assert actual.keys()==expected.keys()
for k,v in expected.items():
    assert math.isclose(actual[k],v,rel_tol=1e-10,abs_tol=1e-12),(k,actual[k],v)
print(f'{len(refs)} scenarios; {len(expected)} independent numerical checks')
