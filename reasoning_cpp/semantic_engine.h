#ifndef SEMANTIC_ENGINE_H
#define SEMANTIC_ENGINE_H

#include "../core_c/vector_ops.h"
#include <string>
#include <vector>
#include <map>
#include <algorithm>

class SemanticEngine {
private:
    std::map<std::string, int> vocabulary;
    std::vector<double> idf;
    std::map<std::string, SparseVector*> conceptVectors;
    int docCount;

    // Helper to clean and split text
    std::vector<std::string> tokenize(const std::string& text);

public:
    SemanticEngine();
    ~SemanticEngine();

    // Learn vocabulary and IDF from a dataset
    // Format: Pair<ConceptName, Description>
    void learn(const std::vector<std::pair<std::string, std::string>>& data);

    // Convert text to immediate vector (caller controls memory!)
    SparseVector* textToVector(const std::string& text);

    // Find top-K similar concepts
    std::vector<std::pair<std::string, double>> findSimilar(const std::string& query, int topK);
    
    // Check if engine is ready
    bool isReady() const { return !vocabulary.empty(); }
};

#endif // SEMANTIC_ENGINE_H
