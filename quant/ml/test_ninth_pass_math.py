"""Mathematical sanity checks for 122 ninth-wave in-lesson enhancements.

The 48 cases independently recompute two published numerical examples
per domain. They do not replace complete academic proof review.
"""
import json
import math
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parent
LESSONS={}
for part in "abcdefghijkl":
    data=json.loads((ROOT/f"atlas-depth-09-{part}.json").read_text(encoding="utf-8"))
    assert data["schema"]==1
    assert len(data["lessons"])==(12 if part=="a" else 10)
    for uid,lesson in data["lessons"].items():
        assert uid not in LESSONS,uid
        LESSONS[uid]=lesson

class NinthPassMath(unittest.TestCase):
    def test_all_122_chapters_and_protocols(self):
        self.assertEqual(len(LESSONS),122)
        topics={}
        for uid,chapter in LESSONS.items():
            topics[uid[:3]]=topics.get(uid[:3],0)+1
            with self.subTest(uid=uid):
                self.assertRegex(uid,r"^m\d\d-c\d\d-p\d\d$")
                self.assertTrue(chapter["source_title"])
                self.assertGreaterEqual(len(chapter["title"]),8)
                self.assertGreaterEqual(len(chapter["overview"]),60)
                self.assertGreaterEqual(len(chapter["worked_example"]),60)
                self.assertEqual(len(chapter["equations"]),3)
                self.assertEqual(len(chapter["derivation"]),3)
                self.assertEqual(len(chapter["protocol"]),3)
                self.assertEqual(len(chapter["pitfalls"]),2)
                for q in chapter["equations"]:
                    self.assertTrue(q["label"] and q["tex"])
                for key in ("derivation","protocol","pitfalls"):
                    threshold=15 if key=="pitfalls" else 30
                    self.assertTrue(all(len(t)>threshold for t in chapter[key]))
        expected={f"m{i:02d}":(6 if i<=2 else 5) for i in range(1,25)}
        self.assertEqual(topics,expected)

    def test_48_independent_numeric_examples(self):
        # ID, mathematical computation, expected result, exact evidence substring, absolute tolerance
        checks=[
          ("m01-c05-p05",(.5*1.1)/(.5*1.1+.5),.5238095,"52.38%",.00001),
          ("m01-c04-p06",20*10,200,"200bp",0),
          ("m02-c05-p06",9/(9+1),.9,"90%",0),
          ("m02-c03-p01",4*3/5+3*4/5,4.8,"4.8",1e-9),
          ("m03-c05-p04",(.8-1)**2,.04,"0.04",1e-9),
          ("m03-c02-p03",1+(.5*.5)*(2-(-2))**2,5,"=5",1e-9),
          ("m04-c03-p04",math.ceil((math.log(100)+math.log(20))/.1),77,"77个",0),
          ("m04-c03-p05",2*.03,.06,"0.06",1e-9),
          ("m05-c04-p02",2,2,"λ=2",0),
          ("m05-c03-p06",2*(1-.1*.05),1.99,"1.99",1e-9),
          ("m06-c04-p01",math.comb(5,2),10,"10项",0),
          ("m06-c05-p01",10-8,2,"均值为8",0),
          ("m07-c04-p01",.2*(1-.2),.16,"0.16",1e-9),
          ("m07-c03-p02",-math.log(.8*.7),.579818,"0.580",.001),
          ("m08-c03-p05",[50,100,150][[.15,.12,.14].index(min([.15,.12,.14]))],100,"100",0),
          ("m08-c02-p02",math.exp(-1),.368,"0.368",.001),
          ("m09-c01-p02",3/math.sqrt(3**2+4**2),.6,"0.6",0),
          ("m09-c04-p04",1-.5**2/(1+1),.875,"0.875",1e-9),
          ("m10-c03-p01",.8*math.log(.8/.5)+.2*math.log(.2/.5),.193,"0.193",.001),
          ("m10-c01-p04",(0/1+2/1)/(1+1),1,"均值为1",0),
          ("m11-c04-p02",.12/(.12+.08),.6,"0.6",1e-9),
          ("m11-c05-p05",sum((1-1)**2 for _ in range(4)),0,"方差",0),
          ("m12-c01-p01",990/1000,.99,"99%",1e-9),
          ("m12-c03-p04",100*.05,5,"假阳性数为5",0),
          ("m13-c02-p04",.9*(1-.9),.09,"0.09",1e-9),
          ("m13-c02-p01",8*3+1,25,"25",0),
          ("m14-c05-p04",64*10+10,650,"650",0),
          ("m14-c04-p05",1000/(1000*10000),.0001,"1000个参数",1e-9),
          ("m15-c03-p02",.8*2+.25*.4,1.7,"1.7",1e-9),
          ("m15-c05-p03",.5**3,.125,"0.125",1e-9),
          ("m16-c05-p02",.2*1+.8*3,2.6,"2.6",1e-9),
          ("m16-c03-p05",2*512*2048,2097152,"2097152",0),
          ("m17-c03-p06",8/10,.8,"80%",1e-9),
          ("m17-c01-p02",math.log(10),2.303,"2.303",.001),
          ("m18-c01-p02",1/math.sqrt(2),.707,"0.707",.001),
          ("m18-c02-p06",10**3,1000,"1000",0),
          ("m19-c05-p01",1.1*.9-1,-.01,"−1%",1e-9),
          ("m19-c04-p02",2+.5*(-1),1.5,"1.5",1e-9),
          ("m20-c05-p04",1-.9**2,.19,"0.19",1e-9),
          ("m20-c01-p02",.7+.3,1,"概率为1",0),
          ("m21-c01-p04",.25*2+.75*6,5,"=5",0),
          ("m21-c01-p05",math.log(math.exp(5)+math.exp(1))-5,.018,"0.018",.001),
          ("m22-c01-p02",.5**3,.125,"0.125",1e-9),
          ("m22-c02-p02",3*4,12,"12",0),
          ("m23-c04-p04",1-90/120,.25,"25%",1e-9),
          ("m23-c03-p03",(.05+.09)/2,.07,"7%",1e-9),
          ("m24-c04-p03",.2+.01,.21,"0.21",1e-9),
          ("m24-c03-p02",abs(1-0),1,"W1=1",0),
        ]
        self.assertEqual(len(checks),48)
        topics={}
        for uid,value,expected,marker,tolerance in checks:
            topics[uid[:3]]=topics.get(uid[:3],0)+1
            with self.subTest(uid=uid):
                self.assertIn(uid,LESSONS)
                self.assertIn(marker,LESSONS[uid]["worked_example"])
                self.assertAlmostEqual(value,expected,delta=tolerance)
        self.assertEqual(topics,{f"m{i:02d}":2 for i in range(1,25)})

if __name__=="__main__":
    unittest.main()
