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

void Learner::analyzeRelationships() {
    std::cout << "[Learner] Analyzing relationships between " << activeConcepts.size() << " concepts." << std::endl;
    // Basic logic: if concepts are in the same active set, they might be related
    for (size_t i = 0; i < activeConcepts.size(); ++i) {
        for (size_t j = i + 1; j < activeConcepts.size(); ++j) {
            activeConcepts[i].addRelationship(activeConcepts[j].name);
            activeConcepts[j].addRelationship(activeConcepts[i].name);
        }
    }
}

void Learner::process() {
    std::cout << "[Learner] --- State: " << getStateName() << " ---" << std::endl;
    
    switch (currentState) {
        case LearningState::IDENTIFY:
            // In a real system, this would parse input resources
            std::cout << "[Learner] Scanning Knowledge Store for raw facts..." << std::endl;
            break;
        case LearningState::ANALYZE:
            analyzeRelationships();
            break;
        case LearningState::SUMMARIZE: {
            std::cout << "[Learner] Summarizing findings into algorithmic patterns." << std::endl;
            Algorithm algo = Algorithm::synthesizeFromConcepts(activeConcepts);
            if (!algo.logicDescription.empty()) {
                AmiValue algoVal = { (void*)strdup(algo.logicDescription.c_str()), AMI_TYPE_STRING };
                ami_add_fact(ks, "last_synthesized_algo", algoVal);
            }
            break;
        }
        case LearningState::APPLY: {
            std::cout << "[Learner] Applying newly synthesized logic in simulation." << std::endl;
            Algorithm dummy; dummy.logicDescription = "Test";
            if (Simulator::runTrial(dummy, activeConcepts)) {
                std::cout << "[Learner] Simulation SUCCESS. Ready for core application." << std::endl;
            }
            break;
        }
        case LearningState::REVIEW:
            std::cout << "[Learner] Validating results and refining rules." << std::endl;
            break;
        default:
            std::cout << "[Learner] Performing " << getStateName() << " routine." << std::endl;
            break;
    }
}

} // namespace Ami
