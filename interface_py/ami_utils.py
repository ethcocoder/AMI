import re
import os
import subprocess

class Parser:
    @staticmethod
    def extract_structured_data(text):
        data = {
            "concepts": [],
            "properties": [],
            "relationships": []
        }
        
        # Stop words to filter out grammatical noise
        STOP_WORDS = {"the", "and", "was", "for", "with", "that", "this", "from", "into", "been", "they", "were", "their", "will", "would", "which", "could", "there"}

        # 1. Concept Discovery and SEQUENTIAL LINKING (The LLM Engine)
        words = re.findall(r'\b[A-Za-z]{4,}\b', text)
        prev_w = None
        for w in words:
            curr_w = w.title()
            if w.lower() not in STOP_WORDS:
                data["concepts"].append(curr_w)
                if prev_w:
                    # Create a sequential link (Word A follows Word B)
                    data["relationships"].append({"subject": prev_w, "object": curr_w, "type": "sequence"})
                prev_w = curr_w

        # 2. Mathematical Rule Discovery
        # Pattern: Target = A (+/-/*) B
        math_match = re.search(r'(\b[A-Za-z]+\b)\s*=\s*(\b[A-Za-z]+\b)\s*([\+\-\*\/])\s*(\b[A-Za-z]+\b)', text)
        if math_match:
            target, a, op, b = math_match.groups()
            data["relationships"].append({"subject": target.title(), "object": f"{a.title()}:{op}:{b.title()}", "type": "math"})

        # 3. Structural Relationship Discovery
        rel_patterns = [
            (r'(\b[A-Za-z]{4,}\b)\s+(?:is|has|of|and|in|to)\s+(\b[A-Za-z]{4,}\b)', "link"),
            (r'(\b[A-Za-z]{4,}\b)\s+(?:produces|creates|determines)\s+(\b[A-Za-z]{4,}\b)', "causality")
        ]
        
        for pattern, rel_type in rel_patterns:
            matches = re.findall(pattern, text)
            for sub, obj in matches:
                if sub.lower() not in STOP_WORDS and obj.lower() not in STOP_WORDS:
                    data["relationships"].append({"subject": sub.title(), "object": obj.title()})

        # 3. Numeric Property Capture
        prop_matches = re.findall(r'(\b[A-Za-z]{4,}\b)\s+(?:is|value|=)\s+([\d\.]+)', text)
        for concept, val in prop_matches:
            if concept.lower() not in STOP_WORDS:
                data["properties"].append({"concept": concept.title(), "prop": "value", "value": val})

        return data

    @staticmethod
    def batch_process_file(file_path):
        stream = []
        if not os.path.exists(file_path):
            return []
        with open(file_path, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line: continue
                
                data = Parser.extract_structured_data(line)
                
                # Register Concepts
                for c in data["concepts"]: 
                    stream.append(f"CON:{c}")
                    # RAG: Associate this line (Evidence) with the concept
                    if len(line) > 10: # Only store meaningful sentences
                        stream.append(f"EVI:{c}:{line}")

                for p in data["properties"]: stream.append(f"PROP:{p['concept']}:{p['prop']}:{p['value']}")
                for r in data["relationships"]:
                    if r.get("type") == "sequence":
                        stream.append(f"SEQ:{r['subject']}:{r['object']}")
                    elif r.get("type") == "math":
                        stream.append(f"RULE:{r['subject']}:{r['object']}")
                    else:
                        stream.append(f"REL:{r['subject']}:{r['object']}")
        return list(set(stream))

class AmiBridge:
    def __init__(self, binary_path):
        self.binary_path = binary_path

    def run_training(self, stream_path):
        print(f"Training AmI Brain on: {stream_path}...")
        try:
            result = subprocess.run([self.binary_path, "--train", stream_path], capture_output=True, text=True)
            return result.stdout
        except Exception as e:
            return f"Error: {str(e)}"

    def run_query(self, commands):
        try:
            cmd = [self.binary_path, "--chat"] + commands
            result = subprocess.run(cmd, capture_output=True, text=True)
            return result.stdout
        except Exception as e:
            return f"Error: {str(e)}"

    def run_summary(self):
        try:
            result = subprocess.run([self.binary_path, "--summary"], capture_output=True, text=True)
            return result.stdout
        except Exception as e:
            return f"Error: {str(e)}"
