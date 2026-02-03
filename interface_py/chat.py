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
            
            # NLP Layer: Subject Extraction
            STOP_WORDS = {"how", "why", "what", "is", "the", "a", "an", "do", "you", "define", "to", "me", "describe", "tells", "tell", "about", "show", "give", "of", "in", "on", "with", "can", "please"}
            
            # Pattern: Catch common question formats and extract the core subject
            patterns = [
                r"(?:what is|who is|show me|define|explain|tell me about|how you describe|how do you describe)\s+([\b\w\s]+)",
                r"(?:tell me something about|what can you say about)\s+([\b\w\s]+)"
            ]
            
            subject = None
            for p in patterns:
                match = re.search(p, user_input, re.IGNORECASE)
                if match:
                    subject = match.group(1).strip()
                    # Clean trailing noise like 'to me', 'now', etc.
                    subject = re.sub(r"\s+(?:to|for|me|us|please|now|today)$", "", subject, flags=re.IGNORECASE)
                    break

            # Pattern 2: How are [A] and [B] related / link [A] [B]
            link_match = re.search(r"(?:how are|relation between|link)\s+(\w+)\s+(?:and|to|with)\s+(\w+)", user_input, re.IGNORECASE)
            
            args = []
            if link_match:
                a, b = link_match.groups()
                args = [f"LINK:{a.title()}:{b.title()}"]
            elif subject:
                args = [f"QUERY:{subject.title()}"]
            else:
                # Advanced Keyword Selection: Filter out stop words and pick the technical noun
                words = re.findall(r'\b\w{4,}\b', user_input) 
                meaningful_words = [w for w in words if w.lower() not in STOP_WORDS]
                
                if meaningful_words:
                    # Prefer capitalized words (likely proper nouns), otherwise the shortest one that isn't a stop word
                    # (LLMs often define nouns, which are usually shorter than descriptive verbs)
                    capitalized = [w for w in meaningful_words if w[0].isupper()]
                    if capitalized:
                        args = [f"QUERY:{capitalized[0].title()}"]
                    else:
                        # Sort by length, but prioritize later words (often the object of the sentence)
                        args = [f"QUERY:{meaningful_words[-1].title()}"]
                else:
                    simple_words = [w for w in user_input.split() if w.lower() not in STOP_WORDS]
                    if simple_words:
                        args = [f"QUERY:{simple_words[0].title()}"]

            if args:
                output = bridge.run_query(args)
                
                # Enhanced Parsing for the "Greater Than LLM" experience
                if "Reasoning about:" in output:
                    print("-" * 50)
                    # Use Regex to beautify the raw C++ output
                    definition = re.search(r"Definition: (.*?)\.", output)
                    context = re.search(r"- Relational Context: (.*?)\.", output)
                    cross_ref = re.search(r"\(Cross-Referencing (.*?)\)", output)
                    model = re.search(r"- Computational Model: (.*?)\n(.*?)\n", output)
                    evidence = re.findall(r"\&> \"(.*?)\"", output)
                    trace = re.findall(r"\? \(Inferred from (.*?)\): \"(.*?)\"", output)

                    if definition:
                        print(f"AmI > Concept Insight: {definition.group(1)}.")
                    
                    if context:
                        print(f"\n[Structural Graph]\nConnected to: {context.group(1)}")
                    
                    if cross_ref:
                        print(f"Deep Context: {cross_ref.group(1)}")

                    if model:
                        print(f"\n[Neural Execution]\n{model.group(1).strip()}")
                        print(f"Prediction: {model.group(2).strip()}")

                    if evidence:
                        print("\n[Verifiable Evidence (RAG)]")
                        for e in evidence[:3]:
                            print(f"  \u2022 \"{e}\"")
                    
                    if trace:
                        print("\n[Global Memory Trace]")
                        for source, line in trace[:2]:
                            print(f"  \u2022 From {source}: \"{line}\"")
                    
                    if not any([definition, context, model, evidence, trace]):
                        print(f"AmI > {output.strip()}")
                    
                    print("-" * 50)
                else:
                    print(f"AmI > {output.strip()}")
            else:
                print("AmI > I'm not sure what you're asking. Try 'What is Gravity?'")
        
        except KeyboardInterrupt:
            break

    print("\nGoodbye.")

if __name__ == "__main__":
    main()
