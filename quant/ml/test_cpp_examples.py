import argparse
import json
import pathlib
import re
import subprocess
import tempfile

root = pathlib.Path(__file__).resolve().parents[1]


def read(path):
    return json.loads((root / path).read_text(encoding="utf-8"))


def collect(area):
    key = "examples" if area == "math" else "lessons"
    base = read(area + ("/cpp-examples.json" if area == "math" else "/atlas-cpp-examples.json"))
    manifest = read(area + "/cpp-topic-manifest.json")
    assert manifest["schema"] == base["schema"] == 1
    examples = dict(base[key])
    origin = {id: "core" for id in examples}
    for m in manifest["modules"]:
        assert re.fullmatch(r"\./cpp-topic-pack-\d\d\.json", m["path"])
        package = read(area + "/" + m["path"][2:])
        assert package["schema"] == 1 and len(package[key]) == m["count"]
        for id, item in package[key].items():
            assert id not in examples, f"Duplicate lesson: {id}"
            examples[id] = item
            origin[id] = m["path"]
    assert len(examples) == manifest["expected_total"]
    return examples, origin


math, math_origin = collect("math")
ml, ml_origin = collect("ml")
assert len(math) == 656 and len(ml) == 566
curriculum = read("math/curriculum.json")
math_nodes = {node["id"]: node for node in curriculum["nodes"]}
assert set(math) == set(math_nodes), 'Every mathematics curriculum node needs runnable C++17'

ml_paths = [
    root / "ml/atlas-concept-chapters.json",
    *sorted((root / "ml").glob("atlas-concept-bridge-v113*.json")),
    *sorted((root / "ml").glob("atlas-concept-bridge-v114*.json")),
    *sorted((root / "ml").glob("atlas-depth*.json")),
]
ml_sources = {}
for p in ml_paths:
    for id, item in json.loads(p.read_text(encoding="utf-8"))["lessons"].items():
        ml_sources[id] = item.get("source_title") or item.get("title")
assert set(ml) <= set(ml_sources)

counts = {}
for id in math:
    category = math_nodes[id]["discipline_id"]
    counts[category] = counts.get(category, 0) + 1
assert len(counts) == 20 and all(n >= 4 for n in counts.values())

programs = []
seen = {}
for area, entries, origin in (("math", math, math_origin), ("ml", ml, ml_origin)):
    for id, entry in sorted(entries.items()):
        code = entry["code"]
        assert entry["title"]
        if origin[id] != "core":
            assert len(entry.get("description", "")) >= 8
        assert len(code) >= 130 and "int main()" in code
        assert "#define " not in code and "template<" not in code
        if origin[id] != "core":
            expected = math_nodes[id]["title"] if area == "math" else ml_sources[id]
            assert entry["title"] == expected, f"Wrong lesson title: {id}"
        normalized = re.sub(r"\s+", "", code.split("int main()", 1)[1])
        assert normalized not in seen, f"Cloned C++ example: {seen.get(normalized)} and {id}"
        seen[normalized] = id
        programs.append((area, id, code))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--shard", type=int, default=0)
    parser.add_argument("--shards", type=int, default=1)
    args = parser.parse_args()
    assert args.shards >= 1 and 0 <= args.shard < args.shards
    chosen = [p for i, p in enumerate(programs) if i % args.shards == args.shard]

    with tempfile.TemporaryDirectory() as directory:
        for area, id, code in chosen:
            cpp = pathlib.Path(directory) / (id + ".cpp")
            exe = pathlib.Path(directory) / id
            cpp.write_text(code, encoding="utf-8")
            compiled = subprocess.run(
                ["g++", "-std=c++17", "-O0", "-Wall", "-Wextra", "-pedantic", str(cpp), "-o", str(exe)],
                capture_output=True, text=True, timeout=30
            )
            if compiled.returncode:
                raise AssertionError(f"{area}/{id} compilation failed:\n{compiled.stderr[:3500]}")
            run = subprocess.run([str(exe)], capture_output=True, text=True, timeout=8)
            if run.returncode or not run.stdout.strip():
                raise AssertionError(f"{area}/{id}: execution failed ({run.returncode}) {run.stderr[:500]}")
            if re.search(r"(?<![a-z])(?:-?nan|-?inf)(?![a-z])", run.stdout.lower()):
                raise AssertionError(f"{area}/{id}: invalid numeric output {run.stdout[:100]}")
            print("PASS", area, id)

    print(f"Validated {len(chosen)} / {len(programs)} independent educational C++17 programs")


if __name__ == '__main__':
    main()
