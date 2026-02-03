import os
import re
import json
from typing import Dict, Any


class ToolDispatcher:
    def __init__(self, base_dir=None):
        self.base_dir = base_dir or os.getcwd()

    def run(self, name: str, args: str) -> Dict[str, Any]:
        name = name.lower().strip()
        if name == "calc":
            return self._tool_calc(args)
        if name == "list_dir":
            return self._tool_list_dir(args)
        if name == "read_file":
            return self._tool_read_file(args)
        if name == "find":
            return self._tool_find(args)
        return {"ok": False, "error": f"Unknown tool: {name}"}

    def _tool_calc(self, expr: str) -> Dict[str, Any]:
        expr = expr.strip()
        if not re.match(r"^[0-9\.\+\-\*\/\(\)\s]+$", expr):
            return {"ok": False, "error": "calc: invalid characters in expression"}
        try:
            value = eval(expr, {"__builtins__": {}}, {})
            return {"ok": True, "result": value}
        except Exception as exc:
            return {"ok": False, "error": f"calc: {exc}"}

    def _tool_list_dir(self, path: str) -> Dict[str, Any]:
        path = path.strip() or "."
        target = os.path.join(self.base_dir, path)
        if not os.path.isdir(target):
            return {"ok": False, "error": f"list_dir: not a directory: {path}"}
        items = []
        for name in sorted(os.listdir(target)):
            full = os.path.join(target, name)
            items.append({"name": name, "type": "dir" if os.path.isdir(full) else "file"})
        return {"ok": True, "items": items}

    def _tool_read_file(self, args: str) -> Dict[str, Any]:
        parts = args.split()
        if not parts:
            return {"ok": False, "error": "read_file: missing path"}
        path = parts[0]
        limit = 40
        if len(parts) > 1 and parts[1].isdigit():
            limit = min(int(parts[1]), 200)
        target = os.path.join(self.base_dir, path)
        if not os.path.isfile(target):
            return {"ok": False, "error": f"read_file: not a file: {path}"}
        with open(target, "r", encoding="utf-8", errors="ignore") as f:
            lines = [next(f, "") for _ in range(limit)]
        return {"ok": True, "path": path, "lines": [l.rstrip("\n") for l in lines]}

    def _tool_find(self, args: str) -> Dict[str, Any]:
        parts = args.split()
        if len(parts) < 2:
            return {"ok": False, "error": "find: usage find <pattern> <file>"}
        pattern, path = parts[0], parts[1]
        target = os.path.join(self.base_dir, path)
        if not os.path.isfile(target):
            return {"ok": False, "error": f"find: not a file: {path}"}
        regex = re.compile(pattern)
        matches = []
        with open(target, "r", encoding="utf-8", errors="ignore") as f:
            for idx, line in enumerate(f, 1):
                if regex.search(line):
                    matches.append({"line": idx, "text": line.rstrip("\n")})
                if len(matches) >= 20:
                    break
        return {"ok": True, "matches": matches}
