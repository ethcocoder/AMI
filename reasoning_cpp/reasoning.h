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
    std::vector<std::string> relatedConcepts; // Links to other concepts

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

    // Specific reasoning actions
    void identifyConcept(std::string name);
    void analyzeRelationships();
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
    static void synthesizeFromConcepts(const std::vector<Concept>& concepts);
};

} // namespace Ami

#endif // AMI_REASONING_H
