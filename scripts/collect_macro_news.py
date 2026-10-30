#!/usr/bin/env python3
"""Build the public macro policy news archive from a curated list of official RSS/Atom feeds.

The resulting JSON is a 365-day *collected* index, never a claim to exhaustive coverage.
Uses Python standard library only; each source can fail without deleting existing news.
"""
import datetime as dt
import email.utils
import hashlib
import json
import pathlib
import re
import sys
import urllib.request
import urllib.error
import xml.etree.ElementTree as ET

ROOT = pathlib.Path(__file__).resolve().parents[1]
NEWS = ROOT / "quant/news/news.json"
SOURCES = ROOT / "quant/news/sources.json"
MAX_ITEMS = 12000
NOW = dt.datetime.now(dt.timezone.utc)
START = NOW - dt.timedelta(days=365)
UA = "MemoryAtlas-Macro-News/1.0 (+https://github.com/KlineCY/KlineCY.github.io)"
DATE_FIELDS = ("pubDate", "published", "updated", "date", "{http://purl.org/dc/elements/1.1/}date")
ATOM_NS = "{http://www.w3.org/2005/Atom}"

def normalize(text):
    if text is None:
        return ""
    return re.sub(r"\s+", " ", str(text)).strip()

def date_parse(s):
    if not s:
        return None
    s = normalize(s)
    try:
        parsed = email.utils.parsedate_to_datetime(s)
        if parsed:
            return parsed.replace(tzinfo=dt.timezone.utc) if parsed.tzinfo is None else parsed.astimezone(dt.timezone.utc)
    except (ValueError, TypeError, IndexError):
        pass
    try:
        parsed = dt.datetime.fromisoformat(s.replace("Z", "+00:00"))
        return parsed.replace(tzinfo=dt.timezone.utc) if parsed.tzinfo is None else parsed.astimezone(dt.timezone.utc)
    except ValueError:
        return None

def node_text(node, tag):
    child = node.find(tag)
    return normalize("".join(child.itertext())) if child is not None else ""

def parse_entries(raw):
    root = ET.fromstring(raw)
    if root.tag.lower().endswith("rss") or root.find("channel") is not None:
        nodes = root.findall("./channel/item")
        for node in nodes:
            title = node_text(node, "title")
            url = node_text(node, "link") or node_text(node, "guid")
            published = next((date_parse(node_text(node, f)) for f in DATE_FIELDS if node_text(node, f)), None)
            if title and url and published:
                yield title, url, published
    else:
        # Atom 1.0
        nodes = root.findall(".//" + ATOM_NS + "entry")
        for node in nodes:
            title = node_text(node, ATOM_NS + "title")
            alt = [e.attrib.get("href", "") for e in node.findall(ATOM_NS + "link") if e.attrib.get("rel", "alternate") == "alternate"]
            url = next(iter(alt), "")
            published = date_parse(node_text(node, ATOM_NS + "published") or node_text(node, ATOM_NS + "updated"))
            if title and url and published:
                yield title, url, published

def acceptable(url):
    return isinstance(url, str) and url.startswith(("https://", "http://"))

def main():
    source_data = json.loads(SOURCES.read_text(encoding="utf-8"))
    old = json.loads(NEWS.read_text(encoding="utf-8")) if NEWS.exists() else {"items": []}
    items = {}
    for a in old.get("items", []):
        date = date_parse(a.get("date"))
        if date and START <= date <= NOW + dt.timedelta(days=1) and acceptable(a.get("url")):
            items[a["url"].split("#")[0]] = a
    status = []
    added = 0
    for region in source_data.get("regions", []):
        for source in region.get("institutions", []):
            rss = source.get("feed_url")
            if not rss:
                continue
            entry = {"region": region["name"], "institution": source["name"], "feed": rss, "result": "error", "fetched": 0}
            try:
                request = urllib.request.Request(rss, headers={"User-Agent": UA, "Accept": "application/rss+xml, application/atom+xml, text/xml, application/xml, */*"})
                with urllib.request.urlopen(request, timeout=22) as response:
                    data = response.read(3_000_000)
                n = 0
                for title, url, published in parse_entries(data):
                    if not acceptable(url) or not (START <= published <= NOW + dt.timedelta(days=1)):
                        continue
                    n += 1
                    key = url.split("#")[0]
                    if key not in items:
                        items[key] = {"id": hashlib.sha256(key.encode()).hexdigest()[:20], "date": published.date().isoformat(),
                            "region": region["name"], "category": source["category"], "institution": source["name"],
                            "title": title[:500], "summary": "", "url": url, "origin": "official-rss"}
                        added += 1
                entry.update(result="ok", fetched=n)
            except Exception as exc:
                entry["error"] = str(exc)[:180]
                print("Feed failed:", region["name"], source["name"], str(exc)[:150], file=sys.stderr)
            status.append(entry)
    all_items = sorted(items.values(), key=lambda x: (x.get("date", ""), x.get("title", "")), reverse=True)[:MAX_ITEMS]
    good = sum(s["result"] == "ok" for s in status)
    output = {"schema_version": 1, "updated_at": NOW.date().isoformat(), "window_days": 365,
        "coverage_note": (f"近365天的已收集官方公告：本次RSS成功 {good}/{len(status)} 个、增量新增 {added} 条。"
            "RSS仅返回部分近期项目；未覆盖的旧公告需通过机构历史归档查阅，不能将此库视为全部消息。"),
        "items": all_items, "feed_status": status}
    NEWS.write_text(json.dumps(output, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"RSS success {good}/{len(status)}, newly collected {added}, total retained {len(all_items)}")

if __name__ == "__main__":
    main()
