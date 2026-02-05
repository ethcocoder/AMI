#include "semantic_engine.h"
#include <iostream>
#include <sstream>
#include <cmath>
#include <set>
#include <map>

SemanticEngine::SemanticEngine() : docCount(0) {}

SemanticEngine::~SemanticEngine() {
    for (auto& pair : conceptVectors) {
        free_vector(pair.second);
    }
}

std::vector<std::string> SemanticEngine::tokenize(const std::string& text) {
    std::vector<std::string> tokens;
    std::string current;
    for (char c : text) {
        if (isalnum(c)) {
            current += tolower(c);
        } else {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        }
    }
    if (!current.empty()) tokens.push_back(current);
    return tokens;
}

void SemanticEngine::learn(const std::vector<std::pair<std::string, std::string>>& data) {
    // Clear old data
    vocabulary.clear();
    idf.clear();
    for (auto& pair : conceptVectors) free_vector(pair.second);
    conceptVectors.clear();

    docCount = data.size();
    if (docCount == 0) return;

    // 1. Build Vocab and DF
    std::map<std::string, int> docFreq;
    std::set<std::string> allWords;

    for (const auto& pair : data) {
        auto tokens = tokenize(pair.second);
        std::set<std::string> uniqueTokens(tokens.begin(), tokens.end());
        for (const auto& t : uniqueTokens) {
            docFreq[t]++;
            allWords.insert(t);
        }
    }

    // Assign IDs
    int idx = 0;
    for (const auto& w : allWords) {
        vocabulary[w] = idx++;
    }

    // 2. Compute IDF
    idf.resize(vocabulary.size());
    for (const auto& w : allWords) {
        double val = std::log((double)docCount / (double)(docFreq[w] + 1));
        idf[vocabulary[w]] = val;
    }

    std::cout << "[SemanticEngine] Learned " << vocabulary.size() << " words from " << docCount << " concepts." << std::endl;

    // 3. Vectorize Concepts
    for (const auto& pair : data) {
        conceptVectors[pair.first] = textToVector(pair.second);
    }
}

SparseVector* SemanticEngine::textToVector(const std::string& text) {
    auto tokens = tokenize(text);
    if (tokens.empty()) return create_vector(1);

    std::map<int, int> tfCounts;
    for (const auto& t : tokens) {
        if (vocabulary.count(t)) {
            tfCounts[vocabulary[t]]++;
        }
    }

    SparseVector* vec = create_vector(tfCounts.size());
    
    // Iterate map (which is sorted by key/index) to populate vector
    double totalTerms = (double)tokens.size();
    
    for (const auto& pair : tfCounts) {
        int idx = pair.first;
        double count = (double)pair.second;
        double tf = count / totalTerms;
        double val = tf * idf[idx];
        
        vector_add(vec, idx, (float)val);
    }

    return vec;
}

std::vector<std::pair<std::string, double>> SemanticEngine::findSimilar(const std::string& query, int topK) {
    std::vector<std::pair<std::string, double>> results;
    
    SparseVector* qVec = textToVector(query);
    if (qVec->size == 0) {
        free_vector(qVec);
        return results;
    }

    for (const auto& pair : conceptVectors) {
        float score = vector_cosine_similarity(qVec, pair.second);
        if (score > 0.001f) {
            results.push_back({pair.first, (double)score});
        }
    }

    free_vector(qVec);

    // Sort descending
    std::sort(results.begin(), results.end(), 
              [](const std::pair<std::string, double>& a, const std::pair<std::string, double>& b) {
                  return a.second > b.second;
              });

    if (results.size() > topK) {
        results.resize(topK);
    }

    return results;
}
