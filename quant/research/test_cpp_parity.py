import json
import math
import pathlib
import subprocess
import tempfile

here = pathlib.Path(__file__).resolve().parent
py = json.loads((here / "strategies.json").read_text(encoding="utf-8"))["strategies"]
cpp_data = json.loads((here / "strategies-cpp.json").read_text(encoding="utf-8"))
cpp = cpp_data["strategies"]
ids = [item["id"] for item in py]
assert len(ids) == 70 and len(set(ids)) == 70
assert set(ids) == set(cpp)
assert cpp_data["coverage_count"] == 70


def make_history(seed, days):
    history = {}
    for j, name in enumerate(["AAA", "BBB", "CCC", "DDD"]):
        bars = []
        previous = 90 + 17 * j
        for t in range(days):
            trend = (j - 1.3) * 0.0005
            fluctuation = math.sin(t / (8 + j) + seed + j) * 0.012
            gap = 0.001 * math.cos(t + j + seed)
            open_price = previous * (1 + gap)
            close_price = open_price * (1 + trend + fluctuation)
            volume = 200000 + 10000 * j + (t * 1907 + seed * 61) % 90000
            bars.append({"open": open_price, "close": close_price, "volume": float(volume)})
            previous = close_price
        history[name] = bars
    return history


def cpp_input(history):
    lines = [str(len(history))]
    for name, bars in history.items():
        lines.append(f"{name} {len(bars)}")
        for bar in bars:
            lines.append(" ".join(format(bar[field], ".17g") for field in ("open", "close", "volume")))
    return "\n".join(lines) + "\n"


def expected_weights(code, history):
    context = {}
    exec(code, context)
    return context["signals"](history)


def actual_weights(exe, history):
    run = subprocess.run([str(exe)], input=cpp_input(history), text=True, capture_output=True, timeout=10)
    if run.returncode != 0:
        raise AssertionError(f"C++ executable failed: {run.stderr}")
    result = {}
    for line in run.stdout.splitlines():
        key, number = line.split()
        result[key] = float(number)
    return result


with tempfile.TemporaryDirectory() as folder:
    for item in py:
        key = item["id"]
        source = cpp[key]
        assert "int main()" in source and "Weights signals(" in source
        assert "#define " not in source and "template<" not in source
        assert "using namespace std" in source
        file = pathlib.Path(folder) / (key + ".cpp")
        binary = pathlib.Path(folder) / key
        file.write_text(source, encoding="utf-8")
        compiled = subprocess.run(["g++", "-std=c++17", "-O0", "-Wall", "-Wextra", "-pedantic",
                                   str(file), "-o", str(binary)],
                                  text=True, capture_output=True, timeout=30)
        if compiled.returncode:
            raise AssertionError(f"Compile failed [{key}]:\n{compiled.stderr[:4000]}")
        for seed, days in [(1, 100), (2, 61), (3, 90), (4, 41), (5, 20)]:
            history = make_history(seed, days)
            expected = expected_weights(item["code"], history)
            actual = actual_weights(binary, history)
            if set(expected) != set(actual):
                raise AssertionError(f"{key} / {seed} / {days}: keys {expected} != {actual}")
            for name, value in expected.items():
                if not math.isclose(value, actual[name], abs_tol=1e-7, rel_tol=1e-7):
                    raise AssertionError(f"{key} / {seed} / {days} / {name}: "
                                         f"Python={value} C++={actual[name]}")
        print("PASS", key)
print("All 70 C++ strategy programs compile and match five Python scenarios each.")
