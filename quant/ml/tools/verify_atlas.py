#!/usr/bin/env python3
"""Verify Kline Quant ML Atlas archive and editorial corrections (stdlib only).

Run from repository root:
    python quant/ml/tools/verify_atlas.py
"""
import ast
import gzip
import html
import json
import re
import sys
from html.parser import HTMLParser
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PARTS = [ROOT / "atlas" / f"data.part{i}" for i in range(1, 5)]
GENERIC_A = ("这里的随机对象、输入分布、样本索引和运算顺序都应在项目文档中写明。"
             "公式的作用是给出可审计的数量关系，并不自动提供统计一致性或金融收益保证。")
GENERIC_B = ("这些关系的成立还要求明确数据可得时间、参数单位、取样机制以及用于估计的训练区间。"
             "违反条件时不能由等式直接推断样本外效果。")
ID_RE = re.compile(r"^m\d{2}-c\d{2}-p\d{2}$")
ANCHOR_RE = re.compile(r'href="#lesson/(m\d{2}-c\d{2}-p\d{2})"')
CODE_RE = re.compile(r"<pre(?:\s[^>]*)?>\s*<code(?:\s[^>]*)?>(.*?)</code>\s*</pre>", re.S)
MATH_RE = re.compile(r'<span class="atlas-tex" data-tex="([^"]*)">')

class Structure(HTMLParser):
    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.bad = []
        self.stack = []
    def handle_starttag(self, tag, attrs):
        if tag not in {"meta", "link", "br", "hr", "img", "input", "source", "wbr", "area"}:
            self.stack.append(tag)
    def handle_endtag(self, tag):
        if tag not in self.stack:
            self.bad.append("unmatched closing " + tag)
        else:
            while self.stack:
                ended = self.stack.pop()
                if ended == tag:
                    break

def main():
    payload = b"".join(p.read_bytes() for p in PARTS)
    assert len(payload) == 1969862, f"unexpected archive size: {len(payload)}"
    data = json.loads(gzip.decompress(payload))
    items = data["catalog"]["items"]
    assert len(items) == 720
    assert len(data["domains"]) == 24
    by_id = {}
    for item in items:
        lid = item["id"]
        assert ID_RE.fullmatch(lid) and lid not in by_id, lid
        body = data["domains"][item["domain_id"]][lid]
        assert body["title"] and body["html"], lid
        by_id[lid] = body

    editorial = json.loads((ROOT / "atlas-editorial.json").read_text(encoding="utf-8"))
    assert editorial["schema"] == 1
    fixed_a = fixed_b = fixed_math = 0
    for key, marker, field in [
        ("replacementA", GENERIC_A, "definitions"),
        ("replacementB", GENERIC_B, "conditions"),
    ]:
        for lid, replacement in editorial[key].items():
            assert lid in by_id, lid
            find = "<p>" + marker + "</p>"
            assert by_id[lid]["html"].count(find) == 1, (lid, key)
            assert len(replacement) >= 80, (lid, "too short", key)
            by_id[lid]["html"] = by_id[lid]["html"].replace(
                find, "<p>" + html.escape(replacement, quote=True) + "</p>"
            )
            if key == "replacementA":
                fixed_a += 1
            else:
                fixed_b += 1

    for patch in editorial["math"]:
        lid, before, after = patch["id"], patch["from"], patch["to"]
        assert lid in by_id and before in by_id[lid]["html"]
        by_id[lid]["html"] = by_id[lid]["html"].replace(before, after)
        assert before not in by_id[lid]["html"]
        fixed_math += 1
    assert (fixed_a, fixed_b, fixed_math) == (20, 31, 2)

    depth = json.loads((ROOT / "atlas-depth.json").read_text(encoding="utf-8"))
    assert depth["schema"] == 1
    supplements = depth["lessons"]
    assert len(supplements) == 8, "expected eight in-lesson deep dives"
    assert not any(GENERIC_A in b["html"] or GENERIC_B in b["html"] for b in by_id.values()), "generic editorial placeholders remain"
    for lid, lesson in supplements.items():
        assert lid in by_id, ("orphan in-lesson supplement", lid)
        assert len(lesson["title"]) >= 8 and len(lesson["overview"]) >= 60, lid
        assert len(lesson["equations"]) >= 2, lid
        assert len(lesson["derivation"]) >= 3, lid
        assert len(lesson["protocol"]) >= 3, lid
        assert len(lesson["pitfalls"]) >= 2, lid
        for eq in lesson["equations"]:
            assert eq["label"] and eq["tex"], ("empty equation", lid)
        for group in ("derivation", "protocol", "pitfalls"):
            threshold = 15 if group == "pitfalls" else 30
            assert all(isinstance(s, str) and len(s) > threshold for s in lesson[group]), (lid, group)

    refs = math_count = python_count = 0
    legacy_comments = 0
    invalid = []
    for lid, body in by_id.items():
        article = body["html"]
        parser = Structure()
        parser.feed(article)
        if parser.bad:
            invalid.append((lid, "HTML", parser.bad[:3]))
        blocks = CODE_RE.findall(article)
        if len(blocks) != 1:
            invalid.append((lid, "code block count", len(blocks)))
            continue
        source = html.unescape(blocks[0])
        try:
            ast.parse(source, filename=lid + ".py")
        except SyntaxError as err:
            invalid.append((lid, "Python syntax", str(err)))
        python_count += 1
        math_count += len(MATH_RE.findall(article))
        for target in ANCHOR_RE.findall(article):
            if target not in by_id:
                invalid.append((lid, "broken link", target))
            refs += 1
        legacy_comments += len(re.findall(r"(?m)^\s*#\s*第\d+轮", source))

    assert not invalid, invalid[:10]
    assert python_count == 720
    assert math_count >= 20655
    assert refs >= 148
    assert sorted({item["domain_id"] for item in items}) == [
        f"m{i:02d}" for i in range(1, 25)
    ]
    generic_a_left = [lid for lid, body in by_id.items() if GENERIC_A in body["html"]]
    generic_b_left = [lid for lid, body in by_id.items() if GENERIC_B in body["html"]]
    generic_by_domain = {
        f"m{i:02d}": {
            "definition": sum(lid.startswith(f"m{i:02d}") for lid in generic_a_left),
            "boundary": sum(lid.startswith(f"m{i:02d}") for lid in generic_b_left),
        } for i in range(1, 25)
    }
    shortest = sorted(
        ((len(re.sub(r"<[^>]+>", "", body["html"])), lid) for lid, body in by_id.items())
    )[:10]
    shortest_by_domain = {
        f"m{i:02d}": [
            {
                "id": lid,
                "title": by_id[lid]["title"],
                "text_characters": n,
            }
            for n, lid in sorted(
                (len(re.sub(r"<[^>]+>", "", by_id[lid]["html"])), lid)
                for lid in by_id if lid.startswith(f"m{i:02d}-")
            )[:2]
        ] for i in range(1, 25)
    }
    report = {
        "result": "PASS",
        "lessons": len(items),
        "domains": len(data["domains"]),
        "python_syntax": python_count,
        "math_markers": math_count,
        "internal_links_checked": refs,
        "formal_definitions_revised": fixed_a,
        "assumption_paragraphs_revised": fixed_b,
        "equations_corrected": fixed_math,
        "in_lesson_advanced_supplements": len(supplements),
        "historic_code_comment_labels_retained_in_source": legacy_comments,
        "remaining_generic_definition_paragraphs": len(generic_a_left),
        "remaining_generic_boundary_paragraphs": len(generic_b_left),
        "generic_boundary_targets": [
            {
                "id": lid,
                "title": by_id[lid]["title"],
                "preceding_context": html.unescape(re.sub(r"<[^>]+>", " ", by_id[lid]["html"].split(GENERIC_B)[0])[-500:]).strip(),
            } for lid in generic_b_left
        ],
        "generic_paragraphs_by_domain": generic_by_domain,
        "shortest_extracted_lesson_lengths": shortest,
        "shortest_by_domain": shortest_by_domain,
        "note": "Checks archive and in-memory overlays; remaining generic text is measured, not automatically rewritten. Does not run Python examples or a real browser.",
    }
    print(json.dumps(report, ensure_ascii=False, indent=2))
    return 0

if __name__ == "__main__":
    sys.exit(main())
