from fastapi import FastAPI, BackgroundTasks, HTTPException
from fastapi.responses import HTMLResponse, JSONResponse
from fastapi.staticfiles import StaticFiles
from pydantic import BaseModel
import subprocess
import os
import re
import json
import asyncio

app = FastAPI(title="AmI Cognitive Interface")

# Static configuration
BRAIN_EXE = os.path.join(".", "build", "ami_brain_v1.exe")
KNOWLEDGE_FILE = "brain_stream.txt"
LEARNED_FILE = "learned_data.txt"

# In-memory state for UI updates
app_state = {
    "is_learning": False,
    "last_output": "",
    "concepts_count": 0,
    "current_mood": {"curiosity": 0, "anxiety": 0, "focus": 0},
    "logs": []
}

class ChatRequest(BaseModel):
    message: str

def parse_brain_status(output: str):
    """Extracts stats and mood from brain output."""
    stats = {}
    
    # Extract concepts count if available in output
    concept_match = re.search(r"Saving (\d+) concepts", output)
    if concept_match:
        app_state["concepts_count"] = int(concept_match.group(1))
        
    # Extract mood
    mood_match = re.search(r"Curiosity: ([\d\.]+), Anxiety: ([\d\.]+), Focus: ([\d\.]+)", output)
    if mood_match:
        app_state["current_mood"]["curiosity"] = float(mood_match.group(1))
        app_state["current_mood"]["anxiety"] = float(mood_match.group(2))
        app_state["current_mood"]["focus"] = float(mood_match.group(3))
        
    return stats

async def run_autonomous_loop():
    """Background task to run the research bridge logic."""
    app_state["is_learning"] = True
    app_state["logs"].append("Starting Autonomous Learning Cycle...")
    
    try:
        # We invoke the research_bridge.py script we built earlier
        process = await asyncio.create_subprocess_exec(
            "python", "interface_py/research_bridge.py",
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        
        while True:
            line = await process.stdout.readline()
            if not line:
                break
            
            clean_line = line.decode('utf-8', errors='replace').strip()
            if clean_line:
                app_state["logs"].append(clean_line)
                if len(app_state["logs"]) > 100:
                    app_state["logs"].pop(0)
                
                # Update status in real-time if we see recognizable patterns
                parse_brain_status(clean_line)
                
        await process.wait()
        app_state["logs"].append("Learning Cycle Complete.")
    except Exception as e:
        app_state["logs"].append(f"Error in learning loop: {str(e)}")
    finally:
        app_state["is_learning"] = False

@app.get("/", response_class=HTMLResponse)
async def get_index():
    with open("interface_py/index.html", "r", encoding="utf-8") as f:
        return f.read()

@app.post("/chat")
async def chat(request: ChatRequest):
    try:
        # 1. Initial Query
        cmd = [BRAIN_EXE, "--chat", request.message]
        result = subprocess.run(cmd, capture_output=True, text=True, encoding='utf-8', errors='replace')
        output = result.stdout
        
        # 2. Check for Reactive Research Trigger
        research_match = re.search(r"AMI_ACTION:RESEARCH:\[(.*?)\]", output)
        if research_match:
            raw_topic = research_match.group(1)
            
            # CLEAN TOPIC: "what is biology" -> "biology"
            search_topic = raw_topic.lower()
            for prefix in ["what is ", "who is ", "tell me about ", "define ", "explain "]:
                if search_topic.startswith(prefix):
                    search_topic = search_topic[len(prefix):]
            search_topic = search_topic.strip("? ").title()
            
            app_state["logs"].append(f"[Reactive] Gap: '{raw_topic}' -> Search: '{search_topic}'")
            
            # Immediately perform web search
            search_cmd = ["python", "interface_py/web_search.py", search_topic]
            search_res = subprocess.run(search_cmd, capture_output=True, text=True, encoding='utf-8', errors='replace')
            search_data = search_res.stdout
            
            if "EVI:" in search_data:
                # Ingest data into knowledge stream
                with open(KNOWLEDGE_FILE, "a", encoding="utf-8") as f:
                    f.write(f"\nCON:{search_topic}\n")
                    f.write(f"{search_data}\n")
                
                # Fast re-train
                app_state["logs"].append(f"[Reactive] Learning about {search_topic}...")
                train_cmd = [BRAIN_EXE, "--train", KNOWLEDGE_FILE]
                subprocess.run(train_cmd, capture_output=True)
                
                # RE-QUERY for the better answer (target the specific concept now)
                app_state["logs"].append(f"[Reactive] Brain updated. Synthesizing answer...")
                
                # IMPORTANT: We query the cleaned TOPIC now, not the original "what is..."
                cmd = [BRAIN_EXE, "--chat", search_topic]
                result = subprocess.run(cmd, capture_output=True, text=True, encoding='utf-8', errors='replace')
                output = result.stdout
        
        # 3. Final Synthesis & Formatting
        clean_output = []
        for line in output.split('\n'):
            line = line.strip()
            if not line: continue
            
            # Suppression of debug noise
            if any(x in line for x in ["Initializing Latent Search", "scrutinized my latent", "missing from the structural"]):
                continue
                
            if "[AmI]" in line or "Synthesis:" in line or "Context:" in line:
                # Format as clean bullet points
                clean_line = line.replace("[AmI]", "").replace("Synthesis:", "").strip()
                if clean_line: clean_output.append("• " + clean_line)
            elif line.startswith("•") or line.startswith(">"):
                clean_output.append(line)
            elif not line.startswith("[") and not line.startswith("-") and "AMI_ACTION" not in line:
                clean_output.append(line)
        
        response_text = "\n".join(clean_output).strip()
        if not response_text:
            response_text = "I have integrated the new data, but my structural synthesis for this specific topic is still forming. Please try asking 'Tell me about [Topic]' directly."

        return {"response": response_text, "raw": output}
    except Exception as e:
        app_state["logs"].append(f"Chat Error: {str(e)}")
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/train")
async def trigger_training(background_tasks: BackgroundTasks):
    if app_state["is_learning"]:
        return {"message": "Learning is already in progress."}
    
    background_tasks.add_task(run_autonomous_loop)
    return {"message": "Autonomous learning loop started in background."}

@app.get("/status")
async def get_status():
    return app_state

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
