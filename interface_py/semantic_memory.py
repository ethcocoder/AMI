import math
import re
import collections

class SemanticEngine:
    def __init__(self):
        self.vocabulary = {}  # word -> index
        self.idf = {}         # word -> idf_score
        self.documents = []   # List of (concept_name, text)
        self.vectors = []     # List of (concept_name, vector)
        self.doc_count = 0

    def tokenize(self, text):
        """Simple tokenizer: lowercase, remove punctuation, split."""
        text = text.lower()
        # Keep only alphanumeric and spaces
        text = re.sub(r'[^a-z0-9\s]', '', text)
        return text.split()

    def fit(self, concept_text_pairs):
        """Learn vocab and IDF from a list of (concept, text) tuples."""
        self.documents = concept_text_pairs
        self.doc_count = len(concept_text_pairs)
        
        # 1. Build Vocabulary and Document Frequencies
        doc_freq = collections.defaultdict(int)
        all_words = set()
        
        for _, text in concept_text_pairs:
            words = set(self.tokenize(text)) # Unique words per doc for DF
            for w in words:
                doc_freq[w] += 1
                all_words.add(w)
        
        # Assign indices to words
        self.vocabulary = {word: i for i, word in enumerate(sorted(list(all_words)))}
        
        # 2. Calculate IDF
        # IDF(w) = log(TotalDocs / (DocFreq(w) + 1))
        for w in all_words:
            self.idf[w] = math.log(self.doc_count / (doc_freq[w] + 1))
            
        print(f"[SemanticEngine] Learned {len(self.vocabulary)} unique words from {self.doc_count} concepts.")
        
        # 3. Pre-compute vectors for all known concepts
        self.vectors = []
        for concept, text in concept_text_pairs:
            vec = self.text_to_vector(text)
            self.vectors.append((concept, vec))

    def text_to_vector(self, text):
        """Convert text to TF-IDF vector."""
        words = self.tokenize(text)
        word_counts = collections.Counter(words)
        total_words = len(words) if words else 1
        
        # Sparse vector representation: {index: value}
        # Using dict for sparsity (better memory for large vocab)
        vector = {}
        
        for w, count in word_counts.items():
            if w in self.vocabulary:
                idx = self.vocabulary[w]
                tf = count / total_words
                tfidf = tf * self.idf[w]
                vector[idx] = tfidf
                
        return vector

    def cosine_similarity(self, vec_a, vec_b):
        """Calculate cosine similarity between two sparse vectors."""
        # Dot product
        dot = 0
        norm_a = 0
        norm_b = 0
        
        # Iterate over keys of A
        for idx, val_a in vec_a.items():
            norm_a += val_a * val_a
            if idx in vec_b:
                dot += val_a * vec_b[idx]
                
        # Calculate norm B separately
        for val_b in vec_b.values():
            norm_b += val_b * val_b
            
        if norm_a == 0 or norm_b == 0:
            return 0.0
            
        return dot / (math.sqrt(norm_a) * math.sqrt(norm_b))

    def find_similar(self, query_text, top_k=3):
        """Find concepts similar to the query text."""
        query_vec = self.text_to_vector(query_text)
        scores = []
        
        for concept, vec in self.vectors:
            score = self.cosine_similarity(query_vec, vec)
            if score > 0:
                scores.append((concept, score))
                
        # Sort desc
        scores.sort(key=lambda x: x[1], reverse=True)
        return scores[:top_k]

# Unit Test
if __name__ == "__main__":
    engine = SemanticEngine()
    
    knowledge = [
        ("King", "A king is a male monarch and ruler of a kingdom."),
        ("Queen", "A queen is a female monarch and ruler of a kingdom."),
        ("Apple", "An apple is a sweet, edible fruit produced by an apple tree."),
        ("Mars", "Mars is the fourth planet from the Sun and the second-smallest planet in the Solar System."),
        ("SpaceX", "SpaceX is an American aerospace manufacturer and space transportation services company.")
    ]
    
    engine.fit(knowledge)
    
    test_q = "Monarch ruler"
    print(f"\nQuery: '{test_q}'")
    results = engine.find_similar(test_q)
    for c, s in results:
        print(f"  > {c}: {s:.4f}")
        
    test_q2 = "Space planet"
    print(f"\nQuery: '{test_q2}'")
    results = engine.find_similar(test_q2)
    for c, s in results:
        print(f"  > {c}: {s:.4f}")
