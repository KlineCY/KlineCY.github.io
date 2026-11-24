import json
import subprocess
import sys
from pathlib import Path
root=Path(__file__).resolve().parent/"applied-examples"
binary=Path(sys.argv[1]).resolve()
out=Path(sys.argv[2])
out.mkdir(parents=True,exist_ok=True)
catalog=json.loads((root/"catalog.json").read_text(encoding="utf-8"))
for case in catalog:
    result=subprocess.run([str(binary),case["command"],str(root/case["file"]),*map(str,case["params"])],capture_output=True,text=True,check=True,timeout=30)
    (out/case["file"]).write_text(result.stdout,encoding="utf-8")
print(f"Completed {len(catalog)} research workflows")
