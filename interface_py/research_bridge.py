import subprocess
import sys
import os
import time

# Configuration
BRAIN_EXE = os.path.join(".", "build", "ami_brain_v1.exe")
WEB_SEARCH_SCRIPT = os.path.join("interface_py", "web_search.py")
LEARNING_FILE = "learned_data.txt"
MASTER_DATASET = "brain_stream.txt"

def run_brain_cycle(round_num, input_files):
    """Runs one cycle of the AmI Brain and captures research requests."""
    print(f"\n=== [Cycle {round_num}] Running AmI Brain ===")
    
    # Construct command: ./brain --train file1 file2 ...
    cmd = [BRAIN_EXE, "--train"] + input_files
    
    process = subprocess.Popen(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        encoding='utf-8',
        bufsize=1
    )
    
    research_requests = set()
    
    # Real-time output monitoring
    while True:
        try:
            line = process.stdout.readline()
        except UnicodeDecodeError:
            continue # Skip bad chars
            
        if not line and process.poll() is not None:
            break
        
        if line:
            # DEBUG: Print everything to see what's happening
            print(f"[RAW] {line.strip()}")
            
            # Echo normal output for user visibility
            if "[Autonomous Discovery]" in line:
                print(line.strip())
                
            # Detect Research Requests
            if "AMI_EXTERNAL_RESEARCH_REQUEST:" in line:
                print(f"  [!] Caught Curiosity Signal: {line.strip()}")
                start = line.find('[') + 1
                end = line.find(']')
                if start != -1 and end != -1:
                    request = line[start:end]
                    # Format: "Tell me more about X" -> extract "X"
                    if "Tell me more about " in request:
                        topic = request.replace("Tell me more about ", "")
                        research_requests.add(topic)
                
    return research_requests

def fulfill_curiosity(requests):
    """Fetches knowledge for requested topics."""
    new_knowledge = []
    
    print("\n--- [Bridge] Fulfilling Curiosity ---")
    for topic in requests:
        print(f"  > Researching: '{topic}'...")
        
        # Call the Sense Organ
        try:
            result = subprocess.check_output(
                ["python", WEB_SEARCH_SCRIPT, topic], 
                encoding='utf-8',
                errors='replace'
            ).strip()
            
            if result.startswith("EVI:"):
                print(f"     -> FOUND! Acquired {len(result)} bytes of knowledge.")
                new_knowledge.append(result)
            else:
                print("     -> Not found.")
                
        except Exception as e:
            print(f"     -> Error: {e}")
            
    return new_knowledge

def main():
    # Initialize learning file if not exists
    if not os.path.exists(LEARNING_FILE):
        with open(LEARNING_FILE, "w", encoding='utf-8') as f:
            f.write("\n")
            
    current_knowledge_files = [MASTER_DATASET]
    
    # The Loop
    MAX_CYCLES = 3
    for i in range(1, MAX_CYCLES + 1):
        # 1. Run Brain
        # Add learned_data to inputs if it has content
        run_inputs = list(current_knowledge_files)
        if os.path.getsize(LEARNING_FILE) > 10:
            run_inputs.append(LEARNING_FILE)
            
        requests = run_brain_cycle(i, run_inputs)
        
        if not requests:
            print(f"\n[Cycle {i}] AmI is satisfied. No new questions asked. Evolution Complete.")
            break
            
        # 2. Fetch Knowledge
        new_facts = fulfill_curiosity(requests)
        
        if not new_facts:
            print(f"\n[Cycle {i}] Could not find answers to any questions. Stopping.")
            break
            
        # 3. Integrate Knowledge (Synaptic Growth)
        print(f"\n[Bridge] Integrating {len(new_facts)} new synaptic connections into '{LEARNING_FILE}'...")
        with open(LEARNING_FILE, "a", encoding='utf-8') as f:
            for fact in new_facts:
                f.write(fact + "\n")
                
        print(f"[Bridge] Integration Complete. Brain mass increased.")
        time.sleep(2) # Short rest before next cycle

if __name__ == "__main__":
    main()
