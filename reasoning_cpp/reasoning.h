#ifndef AMI_REASONING_H
#define AMI_REASONING_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include "semantic_engine.h"

extern "C" {
    #include "../core_c/core.h"
}

namespace Ami {

/**
 * @brief Represents an abstracted concept or data point.
 */
class Concept {
public:
    std::string name;
    std::map<std::string, std::string> attributes;
    std::map<std::string, double> properties; // Numerical state
    std::vector<std::string> relatedConcepts;
    
    // Emotional Intelligence Layer
    double salience; // Importance / Frequency of attention
    double valence;  // Emotional Charge (-1.0 to 1.0)

    Concept(std::string name);
    void addAttribute(std::string key, std::string value);
    void setProperty(std::string key, double value);
    double getPropertyValue(std::string key) const;
    void addRelationship(std::string targetConceptName);
    void display() const;
};

/**
 * @brief State Machine for the Learning Process.
 */
enum class LearningState {
    IDENTIFY,
    GATHER,
    OBSERVE,
    ANALYZE,
    SUMMARIZE, // Algorithmic Synthesis
    APPLY,     // Simulation
    REVIEW,    // Validation & Persistence
    DREAM      // Synthetic Simulation / Generative Thought
};

/**
 * @brief Validation system for deterministic reasoning.
 */
class Validator {
public:
    static bool validateRule(const AmiRule& rule);
};

/**
 * @brief Algorithm synthesis and execution.
 */
class Algorithm {
public:
    std::string target;
    std::vector<std::string> inputs;
    
    // Deep Learning Core: 2-Layer Neural Network (Input -> Hidden -> Output)
    std::vector<std::vector<double>> hiddenWeights; // [HiddenNode][InputNode]
    std::vector<double> hiddenBias;
    std::vector<double> outputWeights; // One weight per hidden node
    double outputBias;

    double trainingError;
    double confidence;

    Algorithm() : outputBias(0), trainingError(1.0), confidence(0.0) {}
    double predict(const std::vector<Concept>& currentConcepts) const;
    void train(const std::vector<std::vector<Concept>>& history, const std::string& targetName);
    
    static Algorithm synthesizeFromConcepts(const std::vector<Concept>& concepts);
};

/**
 * @brief Represents a logical thought process or inference path.
 */
struct Thought {
    std::string conclusion;
    std::vector<std::string> path;
    bool valid;
};

/**
 * @brief Symbolic Logic processing.
 */
class Logic {
public:
    static Thought deepInference(const std::string& start, const std::string& end, const std::map<std::string, std::vector<std::string>>& relations, std::vector<std::string> currentPath = {});
};

/**
 * @brief Simulation environment to test algorithms before applying.
 */
class Simulator {
public:
    static bool runTrial(const Algorithm& algo, const std::vector<Concept>& inputs);
};

class Learner {
private:
    LearningState currentState;
    AmiKnowledgeStore* ks;
    std::vector<Concept> activeConcepts;
    std::vector<std::vector<Concept>> dataHistory; // DataSet for Machine Learning
    std::map<std::string, std::vector<std::string>> relationshipMap;
    std::map<std::string, std::map<std::string, double>> sequenceMap; 
    std::map<std::string, std::vector<std::string>> evidenceMap; // Concept -> [Actual Source Snippets]
    
    // System-wide Emotional State (Moods)
    std::map<std::string, double> globalMood; // "curiosity", "anxiety", "focus"
    std::string currentGoal;
    
    SemanticEngine semanticEngine;
    Algorithm activeModel; 
    std::vector<std::string> researchQuestions;
    std::vector<std::string> cachedPillars;
    
    // User Personalization Layer
    double userRapport; // 0.0 to 1.0
    std::map<std::string, double> userPreferences; // Topic -> Interest level

public:
    Learner(AmiKnowledgeStore* knowledgeStore);
    void transition();
    void autonomousTick(); // Main Cognitive Loop
    std::string getStateName() const;
    void process();

    void identifyConcept(std::string name);
    void learnProperty(std::string conceptName, std::string propName, double value);
    void addDirectedRelationship(std::string sub, std::string obj);
    void addSequence(std::string first, std::string second);
    void addEvidence(std::string concept, std::string snippet);
    void analyzeRelationships();
    void discoverHiddenLinks(); 
    void pruneMemories(); // Self-Correction / Forgetting Layer
    void dream();         // Synthetic Generative Thought
    void formulateQuestions(); // Active Learning / Inquiry
    void performResearch();    // Autonomous External Research
    void handleUserInteraction(std::string input); // Personalization logic
    std::vector<std::string> getResearchQuestions() const { return researchQuestions; }

    // Query Engine
    void queryConcept(std::string name);
    void findConnection(std::string start, std::string end);
    void saveWeights();
    void loadWeights();
};

} // namespace Ami

#endif // AMI_REASONING_H
