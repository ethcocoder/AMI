import os
import subprocess
import json

class AmiBridge:
    def __init__(self, binary_path):
        self.binary_path = binary_path

    def run_reasoning(self, input_concepts):
        print(f"Sending {len(input_concepts)} concepts to AmI Brain Core...")
        try:
            # Pass concepts as space-separated arguments
            result = subprocess.run([self.binary_path] + input_concepts, capture_output=True, text=True)
            return result.stdout
        except Exception as e:
            return f"Error: {str(e)}"

import re

class Parser:
    @staticmethod
    def extract_structured_data(text):
        """
        Extracts structured intelligence from natural language.
        Rules:
        - Relationships: 'A is a B', 'X relates to Y'
        - Math Rules: 'A = B * C'
        - Constraints: 'A > 0'
        """
        data = {
            "concepts": [],
            "relationships": [],
            "rules": [],
            "constraints": [],
            "properties": []
        }

        # 1. Extract Capitalized Concepts
        data["concepts"] = list(set(re.findall(r'\b[A-Z][a-zA-Z]+\b', text)))

        # 2. Extract Properties (e.g., Mass value = 10)
        prop_matches = re.findall(r'(\b\w+\b)\s+(\w+)\s*=\s*(\d+\.?\d*)', text)
        for concept, prop, val in prop_matches:
            data["properties"].append({"concept": concept, "prop": prop, "value": val})

        # 3. Extract Math Rules (e.g., Force = Mass * Acceleration)
        math_matches = re.findall(r'(\b\w+\b)\s*=\s*(.+)', text)
        for target, formula in math_matches:
            data["rules"].append({"target": target, "formula": formula})

        # 3. Extract Relationships (e.g., Cat is a Mammal)
        rel_matches = re.findall(r'(\b\w+\b)\s+is\s+(?:a|an)?\s*(\b\w+\b)', text, re.I)
        for subject, obj in rel_matches:
            data["relationships"].append({"subject": subject, "object": obj})

        # 4. Extract Constraints (e.g., Mass > 0)
        constraint_matches = re.findall(r'(\b\w+\b)\s*([><=]+)\s*(\d+)', text)
        for subject, op, val in constraint_matches:
            data["constraints"].append({"subject": subject, "operator": op, "value": val})

        return data

def main():
    print("========================================")
    print("   AmI Algorithmal Intelligence UI")
    print("========================================")
    
    bridge = AmiBridge(os.path.join("build", "ami_brain_v1.exe"))
    
    while True:
        try:
            user_input = input("\nEnter data to learn (or 'exit'): ")
        except EOFError:
            break
            
        if user_input.lower() == 'exit':
            break
            
        # Step 1: Parse
        data = Parser.extract_structured_data(user_input)
        print(f"Intelligence Extracted: {data}")
        
        # Flatten for CLI passing
        args = []
        for c in data["concepts"]: args.append(f"CON:{c}")
        for p in data["properties"]: args.append(f"PROP:{p['concept']}:{p['prop']}:{p['value']}")
        for r in data["relationships"]: args.append(f"REL:{r['subject']}:{r['object']}")
        for rule in data["rules"]: args.append(f"RULE:{rule['target']}:{rule['formula']}")
        for con in data["constraints"]: args.append(f"CONS:{con['subject']}:{con['operator']}:{con['value']}")

        # Step 2: Bridge to Brain Core
        if not args:
            print("No structured intelligence found in input.")
            continue

        print("Brain: Syncing Intelligence to Core...")
        output = bridge.run_reasoning(args)
        
        # Step 3: Display Reasoning
        print("\n--- AmI Reasoning Output ---")
        print(output)
        print("----------------------------")

if __name__ == "__main__":
    main()
