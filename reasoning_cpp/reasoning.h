#ifndef AMI_REASONING_H
#define AMI_REASONING_H

#include <string>
#include <vector>
#include <map>
#include <iostream>

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
    std::vector<std::string> relatedConcepts;

    Concept(std::string name);
    void addAttribute(std::string key, std::string value);
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
    SUMMARIZE,
    APPLY,
    REVIEW
};

/**
 * @brief Validation system for deterministic reasoning.
 */
class Validator {
public:
    static bool validateRule(const AmiRule& rule);
};

/**
 * @brief Algorithm synthesis logic.
 */
class Algorithm {
public:
    std::string logicDescription;
    static Algorithm synthesizeFromConcepts(const std::vector<Concept>& concepts);
};

/**
 * @brief Symbolic Logic processing.
 */
class Logic {
public:
    static bool evaluateSyllogism(const std::string& p1, const std::string& p2);
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

public:
    Learner(AmiKnowledgeStore* knowledgeStore);
    void transition();
    std::string getStateName() const;
    void process();
    void identifyConcept(std::string name);
    void analyzeRelationships();
};

} // namespace Ami

#endif // AMI_REASONING_H
