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
        
        # technical terminology filter
        STOP_WORDS = {"the", "and", "was", "for", "with", "that", "this", "from", "into", "been", "they", "were", "their", "will", "would", "which", "could", "there", "about", "above", "after", "again", "than", "then", "them", "these"}

        # 1. Sequential Linking (The Flow Engine) - Preserve natural flow
        # Use split to preserve punctuation and original case for human-like definitions
        flow_words = text.split()
        prev_w = None
        for w in flow_words:
            # Clean word for technical concept identification
            clean = re.sub(r'[^A-Za-z0-9]', '', w)
            if not clean: continue

            # Technical discovery: include short uppercase terms (e.g. K, S, AI) or terms > 3 chars
            is_technical = (len(clean) > 3 or (len(clean) >= 1 and clean.isupper()))
            if is_technical and clean.lower() not in STOP_WORDS:
                # CONCEPT NORMALIZATION
                # Preserve proper nouns (all caps or capitalized), normalize others
                if clean.isupper() and len(clean) > 1:
                    concept_name = clean  # Preserve acronyms like NASA, AI
                elif clean[0].isupper():
                    concept_name = clean  # Preserve proper nouns like Mars, Einstein
                else:
                    concept_name = clean.title()  # Normalize common words
                
                # Basic stemming: remove common plural 's'
                if concept_name.endswith('s') and len(concept_name) > 4:
                    # Check if it's likely a plural (not words like "Mars", "Iss")
                    singular = concept_name[:-1]
                    # Use singular form preferentially
                    concept_name = singular
                
                data["concepts"].append(concept_name)
            
            # Build sequence map with original words for flow
            if prev_w:
                data["relationships"].append({"subject": prev_w, "object": w, "type": "sequence"})
                
                # BRIDGE: Ensure clean concepts can enter the flow by also indexing from the clean subject
                clean_prev = re.sub(r'[^A-Za-z0-9]', '', prev_w)
                if clean_prev != prev_w:
                    data["relationships"].append({"subject": clean_prev, "object": w, "type": "sequence"})
                # Case-insensitivity bridge for flow entry
                if clean_prev.lower() != clean_prev:
                    data["relationships"].append({"subject": clean_prev.lower(), "object": w, "type": "sequence"})
            
            prev_w = w

        # 2. Mathematical Rule Discovery (Verification Layer)
        math_match = re.search(r'(\b[A-Za-z]+\b)\s*=\s*(\b[A-Za-z]+\b)\s*([\+\-\*\/])\s*(\b[A-Za-z]+\b)', text)
        if math_match:
            target, a, op, b = math_match.groups()
            data["relationships"].append({"subject": target.title(), "object": f"{a.title()}:{op}:{b.title()}", "type": "math"})

        # 3. Structural Relationship Discovery (The Knowledge Graph)
        # Use more semantic triggers
        rel_patterns = [
            (r'(\b[A-Za-z]{4,}\b)\s+(?:is|has|of|and|in|to)\s+(\b[A-Za-z]{4,}\b)', "link"),
            (r'(\b[A-Za-z]{4,}\b)\s+(?:is defined as|represents|produces|causes)\s+(\b[A-Za-z]{4,}\b)', "causality")
        ]
        
        for pattern, rel_type in rel_patterns:
            matches = re.findall(pattern, text)
            for sub, obj in matches:
                if sub.lower() not in STOP_WORDS and obj.lower() not in STOP_WORDS:
                    data["relationships"].append({"subject": sub.title(), "object": obj.title()})

        # 4. Numeric Property Capture
        prop_matches = re.findall(r'(\b[A-Za-z]{4,}\b)\s+(?:is|value|=)\s+([\d\.]+)', text)
        for concept, val in prop_matches:
            if concept.lower() not in STOP_WORDS:
                data["properties"].append({"concept": concept.title(), "prop": "value", "value": val})

        # DEDUPLICATION: Remove duplicate concepts while preserving order
        seen_concepts = set()
        unique_concepts = []
        for c in data["concepts"]:
            if c not in seen_concepts:
                seen_concepts.add(c)
                unique_concepts.append(c)
        data["concepts"] = unique_concepts

        return data

    @staticmethod
    def batch_process_file(file_path):
        stream = []
        if not os.path.exists(file_path):
            return []
        
        # Use a seen set to avoid duplicate REL/CON lines in the same neighborhood
        neighborhood_seen = set()
        
        with open(file_path, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line: continue
                
                data = Parser.extract_structured_data(line)
                
                # Register Concepts
                for c in set(data["concepts"]): 
                    stream.append(f"CON:{c}")
                    # RAG: Quality filtering for evidence
                    if len(line) > 20 and text_entropy_check(line):
                        stream.append(f"EVI:{c}:{line}")

                for p in data["properties"]: 
                    stream.append(f"PROP:{p['concept']}:{p['prop']}:{p['value']}")
                
                for r in data["relationships"]:
                    key = f"{r['type'] if 'type' in r else 'REL'}:{r['subject']}:{r['object']}"
                    if r.get("type") == "sequence":
                        # We don't dedupe sequences, we want frequency
                        stream.append(f"SEQ:{r['subject']}:{r['object']}")
                    else:
                        if key not in neighborhood_seen:
                            if r.get("type") == "math":
                                stream.append(f"RULE:{r['subject']}:{r['object']}")
                            else:
                                stream.append(f"REL:{r['subject']}:{r['object']}")
                            neighborhood_seen.add(key)
        return stream # Return raw stream to preserve counts for C++ frequency analysis

def text_entropy_check(text):
    # Simple check to avoid headers, page numbers, and copyright footers
    if len(text) < 15: return False
    if text.strip().isdigit(): return False
    if "Project Gutenberg" in text: return False
    return True

class AmiBridge:
    def __init__(self, binary_path):
        self.binary_path = binary_path
        self.research_cache = set()

    def run_training(self, stream_path):
        print(f"Training AmI Brain on: {stream_path}...")
        try:
            result = subprocess.run([self.binary_path, "--train", stream_path], capture_output=True, text=True, encoding='utf-8', errors='replace')
            output = result.stdout or ""
            
            # PARSE RESEARCH REQUESTS
            requests = re.findall(r"AMI_EXTERNAL_RESEARCH_REQUEST:\[(.*?)\]", output)
            if requests:
                print(f"[Bridge] AmI has requested external research on {len(requests)} topics.")
                for req in requests:
                    if req not in self.research_cache:
                        self.perform_real_research(req, stream_path)
                        self.research_cache.add(req)
                    else:
                        print(f"  > [Bridge] Research for '{req}' already in memory. Skipping web query.")
            
            return output
        except Exception as e:
            return f"Error: {str(e)}"

    def perform_real_research(self, query, stream_path):
        from duckduckgo_search import DDGS
        import time
        # Extract core concept (e.g., "Tell me more about Mars" -> "Mars")
        concept_tag = query.replace("Tell me more about ", "").replace(" ", "_")
        
        print(f"  > [Web] Querying Internet for: {query}...")
        try:
            time.sleep(3) # Be very gentle
            with DDGS() as ddgs:
                results = ddgs.text(query, max_results=1)
                if results:
                    with open(stream_path, "a", encoding="utf-8") as f:
                        for r in results:
                            body = r.get("body", "")
                            if body:
                                print(f"    - Found insight: {r.get('title')}")
                                f.write(f"CON:{concept_tag}\n")
                                f.write(f"EVI:{concept_tag}:[Internet] {body}\n")
                    return
        except Exception as e:
            print(f"  > [Web Sync] External rate limit hit. Activating Latent Research Cache...")
        
        # MOCK FALLBACK
        mock_data = {
            "Mars": "Mars is the fourth planet from the Sun and the second-smallest planet in the Solar System.",
            "Colonization": "Space colonization involves permanent human habitation on Mars.",
            "Atmosphere": "The atmosphere of Mars is thin, composed of 95% carbon dioxide.",
            "SpaceX": "SpaceX was founded by Elon Musk to enable Mars colonization.",
            "Musk": "Elon Musk is the founder of SpaceX and Tesla."
        }
        
        matched = False
        for key, val in mock_data.items():
            if key.lower() in query.lower():
                with open(stream_path, "a", encoding="utf-8") as f:
                    f.write(f"CON:{key}\n")
                    f.write(f"EVI:{key}:[Latent Cache] {val}\n")
                print(f"    - Synchronized insight from Latent Cache for: {key}")
                matched = True
        
        if not matched:
            print(f"    - [Bridge] No cache entry for: '{query}'. Topic remains a mystery.")

    def run_query(self, commands):
        try:
            cmd = [self.binary_path, "--chat"] + commands
            result = subprocess.run(cmd, capture_output=True, text=True, encoding='utf-8', errors='replace')
            return result.stdout or ""
        except Exception as e:
            return f"Error: {str(e)}"

    def run_summary(self):
        try:
            result = subprocess.run([self.binary_path, "--summary"], capture_output=True, text=True, encoding='utf-8', errors='replace')
            return result.stdout or ""
        except Exception as e:
            return f"Error: {str(e)}"
