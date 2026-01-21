import os
import subprocess
import re

class AmiBridge:
    def __init__(self, binary_path):
        self.binary_path = binary_path

    def run_reasoning(self, input_concepts):
        try:
            result = subprocess.run([self.binary_path] + input_concepts, capture_output=True, text=True)
            return result.stdout
        except Exception as e:
            return f"Error: {str(e)}"

    def run_reasoning_file(self, stream_path, extras=None):
        print(f"Feeding Mass Data Stream to AmI Brain Core...")
        try:
            cmd = [self.binary_path, "--file", stream_path]
            if extras:
                cmd.extend(extras)
            result = subprocess.run(cmd, capture_output=True, text=True)
            return result.stdout
        except Exception as e:
            return f"Error: {str(e)}"

class Parser:
    @staticmethod
    def extract_structured_data(text):
        data = {
            "concepts": [],
            "relationships": [],
            "rules": [],
            "constraints": [],
            "properties": []
        }
        # Concepts
        data["concepts"] = list(set(re.findall(r'\b[A-Z][a-zA-Z]+\b', text)))
        # Properties
        prop_matches = re.findall(r'(\b\w+\b)\s+(\w+)\s*=\s*(\d+\.?\d*)', text)
        for concept, prop, val in prop_matches:
            data["properties"].append({"concept": concept, "prop": prop, "value": val})
        # Rules
        math_matches = re.findall(r'(\b\w+\b)\s*=\s*([\w\s\*\+\-\/]+)', text)
        for target, formula in math_matches:
            if target != "value": # Avoid confusion with property values
                data["rules"].append({"target": target, "formula": formula.strip()})
        # Relationships
        rel_matches = re.findall(r'(\b\w+\b)\s+is\s+(?:a|an)?\s*(\b\w+\b)', text, re.I)
        for subject, obj in rel_matches:
            data["relationships"].append({"subject": subject, "object": obj})
        return data

    @staticmethod
    def batch_process_file(file_path):
        stream = []
        with open(file_path, "r", encoding="utf-8") as f:
            for line in f:
                data = Parser.extract_structured_data(line)
                for c in data["concepts"]: stream.append(f"CON:{c}")
                for p in data["properties"]: stream.append(f"PROP:{p['concept']}:{p['prop']}:{p['value']}")
                for r in data["relationships"]: stream.append(f"REL:{r['subject']}:{r['object']}")
        return list(set(stream))

def main():
    print("========================================")
    print("   AmI Algorithmal Intelligence UI")
    print("   Commands: 'ingest <file>', 'query <concept>', 'link <A> <B>', 'exit'")
    print("========================================")
    
    bridge = AmiBridge(os.path.join("build", "ami_brain_v1.exe"))
    
    while True:
        try:
            user_input = input("\n> ")
        except EOFError:
            break
            
        if user_input.lower() == 'exit':
            break

        if user_input.lower().startswith('ingest '):
            file_path = user_input.split(' ', 1)[1]
            if not os.path.exists(file_path):
                print(f"File not found: {file_path}")
                continue
            
            print(f"Distilling: {file_path}...")
            stream = Parser.batch_process_file(file_path)
            with open("brain_stream.txt", "w") as f:
                f.write("\n".join(stream))
            
            print(bridge.run_reasoning_file("brain_stream.txt"))
            continue

        if user_input.lower().startswith('query '):
            concept = user_input.split(' ', 1)[1]
            if not os.path.exists("brain_stream.txt"):
                print("Brain is empty. Please 'ingest' a file first.")
                continue
            print(f"Querying Brain about: {concept}...")
            print(bridge.run_reasoning_file("brain_stream.txt", extras=[f"QUERY:{concept}"]))
            continue

        if user_input.lower().startswith('link '):
            parts = user_input.split(' ')
            if len(parts) >= 3:
                c1, c2 = parts[1], parts[2]
                if not os.path.exists("brain_stream.txt"):
                    print("Brain is empty. Please 'ingest' a file first.")
                    continue
                print(f"Searching for hidden links: {c1} -> {c2}...")
                print(bridge.run_reasoning_file("brain_stream.txt", extras=[f"LINK:{c1}:{c2}"]))
            else:
                print("Usage: link <concept1> <concept2>")
            continue

        # Single line processing
        data = Parser.extract_structured_data(user_input)
        args = []
        for c in data["concepts"]: args.append(f"CON:{c}")
        for p in data["properties"]: args.append(f"PROP:{p['concept']}:{p['prop']}:{p['value']}")
        for r in data["relationships"]: args.append(f"REL:{r['subject']}:{r['object']}")
        
        if args:
            print(bridge.run_reasoning(args))
        else:
            print("No structured intelligence found.")

if __name__ == "__main__":
    main()
