import os
import re
from ami_utils import AmiBridge

def main():
    bridge = AmiBridge(os.path.join("build", "ami_brain_v1.exe"))
    
    print("========================================")
    print("   AmI Artificial Intelligence Chat")
    print("   Commands: 'query <concept>', 'link <A> <B>', 'exit'")
    print("========================================")

    # Show Knowledge Summary
    print(bridge.run_summary())

    while True:
        try:
            user_input = input("\nYou > ").strip()
            if not user_input: continue
            if user_input.lower() in ['exit', 'quit', 'bye']:
                break
            
            # Simple Natural Language Processing (NLP) Layer
            args = []
            
            # Pattern 1: What is / Tell me about [Concept]
            what_is_match = re.search(r"(?:what is|tell me about|who is|show me)\s+([\b\w\s]+)", user_input, re.IGNORECASE)
            # Pattern 2: How are [A] and [B] related / link [A] [B]
            link_match = re.search(r"(?:how are|relation between|link)\s+(\w+)\s+(?:and|to|with)\s+(\w+)", user_input, re.IGNORECASE)
            # Pattern 3: Direct command fallback
            query_match = re.match(r"query\s+([\w\s]+)", user_input, re.IGNORECASE)

            if link_match:
                a, b = link_match.groups()
                args = [f"LINK:{a.title()}:{b.title()}"]
            elif what_is_match:
                concept = what_is_match.group(1).strip().title()
                args = [f"QUERY:{concept}"]
            elif query_match:
                concept = query_match.group(1).strip().title()
                args = [f"QUERY:{concept}"]
            else:
                # Greedy search: Just take the first capitalized word as a concept
                words = re.findall(r'\b[A-Z]\w+\b', user_input)
                if not words: # Fallback to any word
                    words = user_input.split()
                
                if words:
                    concept = words[0].title()
                    args = [f"QUERY:{concept}"]

            if args:
                output = bridge.run_query(args)
                # Cleanup output to look more 'conversational'
                clean_output = output.replace("[Brain] Query Concept:", "Knowledge entry for")
                clean_output = clean_output.replace("[Brain] Searching for path between", "Checking connection between")
                print(f"AmI > {clean_output.strip()}")
            else:
                print("AmI > I'm not sure what you're asking. Try 'What is Gravity?'")
        
        except KeyboardInterrupt:
            break

    print("\nGoodbye.")

if __name__ == "__main__":
    main()
