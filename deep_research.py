import os
import time
import subprocess
import re
from interface_py.ami_utils import AmiBridge

def deep_research_session(seed_topic, iterations=5):
    print(f"=== [AmI Deep Research Session] ===")
    print(f"Target: {seed_topic}")
    
    # Initialize files
    stream_file = "deep_research_stream.txt"
    with open("research_seed.txt", "w", encoding="utf-8") as f:
        f.write(f"Objective: Research and synthesize knowledge about: {seed_topic}.\n")
        f.write(f"Context: Specifically investigate the Atmosphere of the red planet and the strategic role of Elon Musk and SpaceX.\n")
        f.write(f"Goal: Provide a comprehensive technical and historical summary of {seed_topic}.\n")

    bridge = AmiBridge(os.path.join("build", "ami_brain_v1.exe"))
    
    # Ingest seed
    print("[1/3] Ingesting Seed Topic...")
    os.system(f"python interface_py/ingest.py research_seed.txt --out {stream_file}")

    for i in range(iterations):
        print(f"\n--- [Iteration {i+1}/{iterations}] ---")
        
        # Run Brain
        output = bridge.run_training(stream_file)
        
        # Check for research requests
        requests = re.findall(r"AMI_EXTERNAL_RESEARCH_REQUEST:\[(.*?)\]", output)
        
        if not requests:
            print("[Status] No knowledge gaps identified. Internal synthesis complete.")
            # Boost curiosity manually to force exploration if needed
            # For this demo, we'll just continue or break
            continue

        print(f"[Brain] Identified {len(requests)} Knowledge Gaps.")
        
        # Requests are handled by perform_real_research inside bridge.run_training
        # We just need to make sure the brain reads the NEWly appended data in the next iteration.
        
        # Small delay between brain cycles to simulate "processing"
        time.sleep(1)

    print("\n=== [Research Session Complete] ===")
    print(f"[Results] Knowledge Graph expanded in {stream_file}")
    
    # Final Summary
    print("\n--- [Final Intelligence Summary] ---")
    summary = subprocess.run([os.path.join("build", "ami_brain_v1.exe"), "--summary"], capture_output=True, text=True)
    print(summary.stdout)

if __name__ == "__main__":
    import sys
    topic = sys.argv[1] if len(sys.argv) > 1 else "Mars Colonization"
    deep_research_session(topic)
