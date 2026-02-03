import argparse
import os
import json
import csv
import bz2
import re
import xml.etree.ElementTree as ET
from ami_utils import Parser, AmiBridge


def iter_files(root, exts):
    for dirpath, _, filenames in os.walk(root):
        for name in filenames:
            if any(name.lower().endswith(ext) for ext in exts):
                yield os.path.join(dirpath, name)


def iter_text_units(path):
    ext = os.path.splitext(path)[1].lower()
    if ext in [".txt", ".md"]:
        with open(path, "r", encoding="utf-8", errors="ignore") as f:
            for line in f:
                yield line
        return
    if ext == ".csv":
        with open(path, "r", encoding="utf-8", errors="ignore", newline="") as f:
            reader = csv.reader(f)
            for row in reader:
                if row:
                    yield " ".join(row)
        return
    if ext == ".jsonl":
        with open(path, "r", encoding="utf-8", errors="ignore") as f:
            for raw in f:
                raw = raw.strip()
                if not raw:
                    continue
                try:
                    obj = json.loads(raw)
                except json.JSONDecodeError:
                    continue
                if isinstance(obj, dict):
                    for key in ["text", "content", "body", "message"]:
                        if key in obj and isinstance(obj[key], str):
                            yield obj[key]
                            break


def strip_wiki_markup(text):
    if not text:
        return ""
    text = re.sub(r"<ref[^>]*>.*?</ref>", " ", text, flags=re.DOTALL)
    text = re.sub(r"<!--.*?-->", " ", text, flags=re.DOTALL)
    text = re.sub(r"\{\{[^}]*\}\}", " ", text)
    text = re.sub(r"\[\[(?:[^\]|]*\|)?([^\]]+)\]\]", r"\1", text)
    text = re.sub(r"<[^>]+>", " ", text)
    text = re.sub(r"'{2,}", " ", text)
    text = re.sub(r"=+", " ", text)
    text = re.sub(r"\s+", " ", text).strip()
    return text


def chunk_text(text, chunk_size):
    if chunk_size <= 0 or len(text) <= chunk_size:
        return [text]
    chunks = []
    start = 0
    while start < len(text):
        end = min(start + chunk_size, len(text))
        chunks.append(text[start:end])
        start = end
    return chunks


def iter_wiki_texts(path, max_articles=None, chunk_size=1200):
    opener = bz2.open if path.lower().endswith(".bz2") else open
    with opener(path, "rb") as f:
        context = ET.iterparse(f, events=("end",))
        count = 0
        for _, elem in context:
            if not elem.tag.endswith("page"):
                continue
            if elem.find("{*}redirect") is not None:
                elem.clear()
                continue
            title_elem = elem.find("{*}title")
            text_elem = elem.find(".//{*}text")
            title = title_elem.text if title_elem is not None else ""
            text = text_elem.text if text_elem is not None else ""
            merged = (title + ". " + text).strip()
            merged = strip_wiki_markup(merged)
            if merged:
                for chunk in chunk_text(merged, chunk_size):
                    yield chunk
            count += 1
            elem.clear()
            if max_articles and count >= max_articles:
                break


def stream_from_lines(lines_iter, source_tag, out_f, dedupe_set=None, evidence=True, line_limit=None):
    count = 0
    for raw in lines_iter:
        if line_limit and count >= line_limit:
            break
        line = str(raw).strip()
        if not line:
            continue
        count += 1

        data = Parser.extract_structured_data(line)

        def emit(item):
            if not item:
                return 0
            if dedupe_set is not None:
                if item in dedupe_set:
                    return 0
                dedupe_set.add(item)
            out_f.write(item + "\n")
            return 1

        for c in data["concepts"]:
            emit(f"CON:{c}")
            if evidence and len(line) > 10:
                if source_tag:
                    emit(f"EVI:{c}:[{source_tag}] {line}")
                else:
                    emit(f"EVI:{c}:{line}")

        for p in data["properties"]:
            emit(f"PROP:{p['concept']}:{p['prop']}:{p['value']}")
        for r in data["relationships"]:
            if r.get("type") == "sequence":
                emit(f"SEQ:{r['subject']}:{r['object']}")
            elif r.get("type") == "math":
                emit(f"RULE:{r['subject']}:{r['object']}")
            else:
                emit(f"REL:{r['subject']}:{r['object']}")


def is_wiki_dump(path):
    name = os.path.basename(path).lower()
    if name.endswith(".xml") or name.endswith(".xml.bz2"):
        return True
    if "pages-articles" in name or "pages-meta" in name:
        return True
    return False


def main():
    parser = argparse.ArgumentParser(description="Ingest datasets into AmI brain stream.")
    parser.add_argument("path", help="File or directory to ingest")
    parser.add_argument("--out", default="brain_stream.txt", help="Output stream file")
    parser.add_argument("--limit", type=int, default=0, help="Max lines per file (0 = no limit)")
    parser.add_argument("--train", action="store_true", help="Run training after stream build")
    parser.add_argument("--dedupe", action="store_true", help="Dedupe stream items (memory heavy)")
    parser.add_argument("--no-evidence", action="store_true", help="Disable evidence (EVI) output")
    parser.add_argument("--wiki-max-articles", type=int, default=0, help="Max articles per wiki dump (0 = no limit)")
    parser.add_argument("--wiki-chunk-size", type=int, default=1200, help="Chunk size for wiki text")
    args = parser.parse_args()

    target = args.path
    if not os.path.exists(target):
        print(f"Error: path not found: {target}")
        return

    if os.path.isdir(target):
        files = list(iter_files(target, [".txt", ".md", ".csv", ".jsonl", ".xml", ".bz2"]))
    else:
        files = [target]

    if not files:
        print("No compatible files found.")
        return

    dedupe_set = set() if args.dedupe else None
    out_dir = os.path.dirname(os.path.abspath(args.out))
    if out_dir and not os.path.exists(out_dir):
        os.makedirs(out_dir, exist_ok=True)

    written_any = False
    with open(args.out, "w", encoding="utf-8") as out_f:
        for fpath in files:
            source_tag = os.path.basename(fpath)
            if is_wiki_dump(fpath):
                max_articles = args.wiki_max_articles if args.wiki_max_articles > 0 else None
                lines_iter = iter_wiki_texts(fpath, max_articles=max_articles, chunk_size=args.wiki_chunk_size)
            else:
                lines_iter = iter_text_units(fpath)
            if lines_iter is None:
                continue
            stream_from_lines(
                lines_iter,
                source_tag=source_tag,
                out_f=out_f,
                dedupe_set=dedupe_set,
                evidence=not args.no_evidence,
                line_limit=args.limit if args.limit > 0 else None,
            )
            written_any = True

    if not written_any:
        print("No stream output generated.")
        return

    print(f"Stream saved to {args.out} ({len(files)} files).")

    if args.train:
        bridge = AmiBridge(os.path.join("build", "ami_brain_v1.exe"))
        output = bridge.run_training(args.out)
        print(output)


if __name__ == "__main__":
    main()
