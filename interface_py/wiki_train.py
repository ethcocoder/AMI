import os
import sys
import json
import urllib.request
import subprocess
import argparse

def fetch_dumpstatus(project):
    urls = [
        f"https://dumps.wikimedia.org/{project}/latest/dumpstatus.json",
        f"https://dumps.wikimedia.org/{project}/dumpstatus.json",
    ]
    last_err = None
    for url in urls:
        try:
            req = urllib.request.Request(url, headers={"User-Agent": "AmI-WikiTrainer/1.0"})
            with urllib.request.urlopen(req) as resp:
                return json.loads(resp.read().decode("utf-8"))
        except Exception as exc:
            last_err = exc
    if last_err:
        raise last_err
    return None

def download_file(url, target):
    print(f"Downloading {url} to {target}...")
    try:
        urllib.request.urlretrieve(url, target)
        print("Download complete.")
        return True
    except Exception as e:
        print(f"Error downloading: {e}")
        return False

def main():
    parser = argparse.ArgumentParser(description="AmI Wikipedia Training Utility")
    parser.add_argument("--project", default="amwiki", help="Wikimedia project (e.g., amwiki, enwiki)")
    parser.add_argument("--limit", type=int, default=100, help="Max articles to process")
    parser.add_argument("--binary", default=os.path.join("build", "ami_brain_v1.exe"), help="Path to AmI binary")
    args = parser.parse_args()

    # 1. Get latest dump info
    try:
        status = fetch_dumpstatus(args.project)
        if not status:
            print("Could not fetch dump status.")
            return

        # Find the articles-multistream.xml.bz2 or similar
        jobs = status.get("jobs", {})
        multistream = jobs.get("articlesmultistreamdump", {})
        files = multistream.get("files", {})
        
        target_file = None
        for name in files:
            if name.endswith("pages-articles-multistream.xml.bz2"):
                target_file = name
                break
        
        if not target_file:
            # Fallback
            for name in files:
                if "pages-articles" in name and name.endswith(".bz2"):
                    target_file = name
                    break
        
        if not target_file:
            print("Could not find a suitable .bz2 dump file.")
            return

        date = status.get("version")
        url = f"https://dumps.wikimedia.org/{args.project}/{date}/{target_file}"
        local_path = os.path.join("data", target_file)
        os.makedirs("data", exist_ok=True)

        if not os.path.exists(local_path):
            if not download_file(url, local_path):
                return
        else:
            print(f"Using existing dump: {local_path}")

        # 2. Ingest
        print(f"Ingesting {local_path}...")
        stream_file = "wiki_stream.txt"
        ingest_cmd = [
            sys.executable, "interface_py/ingest.py", 
            local_path, 
            "--out", stream_file, 
            "--wiki-max-articles", str(args.limit),
            "--dedupe"
        ]
        subprocess.run(ingest_cmd, check=True)

        # 3. Train
        print("Training AmI on Wikipedia stream...")
        train_cmd = [args.binary, "--train", stream_file]
        subprocess.run(train_cmd, check=True)
        
        print("\nWikipedia training complete.")

    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    main()
