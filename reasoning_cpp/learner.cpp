#include "reasoning.h"
#include <algorithm>
#include <cstring>

#ifdef _WIN32
#define strdup _strdup
#endif

namespace Ami {

Learner::Learner(AmiKnowledgeStore* knowledgeStore) 
    : currentState(LearningState::IDENTIFY), ks(knowledgeStore) {}

void Learner::transition() {
    switch (currentState) {
        case LearningState::IDENTIFY: currentState = LearningState::GATHER; break;
        case LearningState::GATHER: currentState = LearningState::OBSERVE; break;
        case LearningState::OBSERVE: currentState = LearningState::ANALYZE; break;
        case LearningState::ANALYZE: currentState = LearningState::SUMMARIZE; break;
        case LearningState::SUMMARIZE: currentState = LearningState::APPLY; break;
        case LearningState::APPLY: currentState = LearningState::REVIEW; break;
        case LearningState::REVIEW: currentState = LearningState::IDENTIFY; break;
    }
}

std::string Learner::getStateName() const {
    switch (currentState) {
        case LearningState::IDENTIFY: return "IDENTIFY";
        case LearningState::GATHER: return "GATHER";
        case LearningState::OBSERVE: return "OBSERVE";
        case LearningState::ANALYZE: return "ANALYZE";
        case LearningState::SUMMARIZE: return "SUMMARIZE";
        case LearningState::APPLY: return "APPLY";
        case LearningState::REVIEW: return "REVIEW";
        default: return "UNKNOWN";
    }
}

void Learner::identifyConcept(std::string name) {
    std::cout << "[Learner] Identifying new concept: " << name << std::endl;
    activeConcepts.emplace_back(name);
}

void Learner::learnProperty(std::string conceptName, std::string propName, double value) {
    std::cout << "[Learner] Learning Property: " << conceptName << "." << propName << " = " << value << std::endl;
    for (auto& c : activeConcepts) {
        if (c.name == conceptName) {
            c.setProperty(propName, value);
            return;
        }
    }
    // If concept doesn't exist, create it and add property
    Concept c(conceptName);
    c.setProperty(propName, value);
    activeConcepts.push_back(c);
}

void Learner::analyzeRelationships() {
    std::cout << "[Learner] Deep Relational Analysis..." << std::endl;
    for (size_t i = 0; i < activeConcepts.size(); ++i) {
        for (size_t j = i + 1; j < activeConcepts.size(); ++j) {
            std::string sub = activeConcepts[i].name;
            std::string obj = activeConcepts[j].name;
            relationshipMap[sub].push_back(obj);
            activeConcepts[i].addRelationship(obj);
        }
    }
}

void Learner::process() {
    std::cout << "[Learner] --- Process Mode: " << getStateName() << " ---" << std::endl;
    
    switch (currentState) {
        case LearningState::ANALYZE:
            analyzeRelationships();
            // Perform Inference Check
            if (Logic::inferRelationship("Gravity", "Force", relationshipMap)) {
                std::cout << "[Brain] INFERRED: Gravity is linked to the Force domain." << std::endl;
            }
            break;
            
        case LearningState::SUMMARIZE: {
            std::cout << "[Learner] Patterns found in " << activeConcepts.size() << " data points." << std::endl;
            Algorithm algo = Algorithm::synthesizeFromConcepts(activeConcepts);
            if (!algo.target.empty()) {
                std::string logic = algo.target + " synthesized via " + algo.operation;
                AmiValue algoVal = { (void*)strdup(logic.c_str()), AMI_TYPE_STRING };
                ami_add_fact(ks, "last_brain_discovery", algoVal);
            }
            break;
        }
        
        case LearningState::APPLY: {
            Algorithm algo = Algorithm::synthesizeFromConcepts(activeConcepts);
            if (Simulator::runTrial(algo, activeConcepts)) {
                std::cout << "[Learner] Result matches expected deterministic outcome." << std::endl;
            }
            break;
        }

        default:
            break;
    }
}

} // namespace Ami
