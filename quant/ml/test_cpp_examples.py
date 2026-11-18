import json
import pathlib
import subprocess
import tempfile

root = pathlib.Path(__file__).resolve().parents[1]
math = json.loads((root / "math/cpp-examples.json").read_text(encoding="utf-8"))
ml = json.loads((root / "ml/atlas-cpp-examples.json").read_text(encoding="utf-8"))
curriculum = json.loads((root / "math/curriculum.json").read_text(encoding="utf-8"))
math_nodes = {node["id"]: node["discipline_id"] for node in curriculum["nodes"]}
ml_sources = [
    root / "ml/atlas-concept-chapters.json",
    *sorted((root / "ml").glob("atlas-concept-bridge-v113*.json")),
    *sorted((root / "ml").glob("atlas-concept-bridge-v114*.json")),
]
ml_ids = set()
for file in ml_sources:
    ml_ids.update(json.loads(file.read_text(encoding="utf-8"))["lessons"])

assert math["schema"] == 1 and ml["schema"] == 1
assert len(math["examples"]) == 40
assert len(ml["lessons"]) == 44
assert set(math["examples"]) <= set(math_nodes)
assert set(ml["lessons"]) <= ml_ids
counts = {}
for node in math["examples"]:
    category = math_nodes[node]
    counts[category] = counts.get(category, 0) + 1
assert len(counts) == 20 and all(value >= 2 for value in counts.values())

with tempfile.TemporaryDirectory() as folder:
    for kind, examples in (("math", math["examples"]), ("ml", ml["lessons"])):
        for id, example in examples.items():
            code = example["code"]
            assert example["title"] and len(code) >= 130
            assert "int main()" in code
            assert "#define " not in code and "template<" not in code
            source = pathlib.Path(folder) / (id + ".cpp")
            binary = pathlib.Path(folder) / id
            source.write_text(code, encoding="utf-8")
            compile_run = subprocess.run(
                ["g++", "-std=c++17", "-O0", "-Wall", "-Wextra", "-pedantic", str(source), "-o", str(binary)],
                text=True, capture_output=True, timeout=30
            )
            if compile_run.returncode:
                raise AssertionError(f"{kind} {id}: C++ compile failed:\n{compile_run.stderr[:3000]}")
            executed = subprocess.run([str(binary)], text=True, capture_output=True, timeout=5)
            if executed.returncode:
                raise AssertionError(f"{kind} {id}: program returned {executed.returncode}")
            if not executed.stdout.strip():
                raise AssertionError(f"{kind} {id}: no numerical or algorithmic output")
            print("PASS", kind, id)
print("All 84 mathematical and ML C++17 examples compiled and ran.")
