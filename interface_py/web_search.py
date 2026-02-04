import urllib.request
import urllib.parse
import json
import sys
import re

def clean_text(text):
    """Remove references like [1], HTML tags, and other artifacts."""
    # Remove HTML tags
    text = re.sub('<[^<]+?>', '', text)
    # Remove citation brackets [1], [2], etc.
    text = re.sub(r'\[\d+\]', '', text)
    # Normalize whitespace
    text = " ".join(text.split())
    return text

def fetch_wikipedia_summary(topic):
    """Fetches the first paragraph of a Wikipedia article."""
    try:
        # Wikipedia API endpoint for summaries
        base_url = "https://en.wikipedia.org/api/rest_v1/page/summary/"
        
        # Encode the topic (snake_case usually works best for Wiki URLs)
        formatted_topic = topic.replace(" ", "_")
        url = base_url + urllib.parse.quote(formatted_topic)
        
        req = urllib.request.Request(
            url, 
            headers={'User-Agent': 'AmI_Cognitive_Architecture/1.0 (Integration Test)'}
        )
        
        with urllib.request.urlopen(req) as response:
            data = json.loads(response.read().decode())
            
            if 'extract' in data:
                summary = clean_text(data['extract'])
                return summary
            else:
                return None
                
    except urllib.error.HTTPError as e:
        # 404 means page not found
        return None
    except Exception as e:
        return None

if __name__ == "__main__":
    sys.stdout.reconfigure(encoding='utf-8')
    if len(sys.argv) < 2:
        print("Usage: python web_search.py <topic>")
        sys.exit(1)
        
    topic = " ".join(sys.argv[1:])
    summary = fetch_wikipedia_summary(topic)
    
    if summary:
        # Output in AmI-compatible format
        # EVI:Concept:Evidence Sentence
        print(f"EVI:{topic}:{summary}")
    else:
        print(f"NOT_FOUND:{topic}")
