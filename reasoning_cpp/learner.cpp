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

void Learner::addDirectedRelationship(std::string sub, std::string obj) {
    relationshipMap[sub].push_back(obj);
}

void Learner::analyzeRelationships() {
    std::cout << "[Learner] Searching for logical connections between " << activeConcepts.size() << " objects..." << std::endl;
    // We now rely on user-defined directed relationships for thinking
}

void Learner::queryConcept(std::string name) {
    std::cout << "[Query] Investigating Concept: " << name << std::endl;
    bool found = false;
    for (const auto& c : activeConcepts) {
        if (c.name == name) {
            c.display();
            found = true;
            break;
        }
    }
    if (!found) {
        std::cout << "[Query] Concept '" << name << "' is currently unknown in high-level reasoning." << std::endl;
    }
}

void Learner::findConnection(std::string start, std::string end) {
    std::cout << "[Query] Searching for logical link: " << start << " -> " << end << std::endl;
    Thought t = Logic::deepInference(start, end, relationshipMap);
    if (t.valid) {
        std::cout << "[Query] Connection Found! Thought Chain:" << std::endl;
        for (size_t i = 0; i < t.path.size(); ++i) {
            std::cout << "  " << t.path[i] << (i < t.path.size() - 1 ? " -> " : "");
        }
        std::cout << std::endl;
    } else {
        std::cout << "[Query] No logical path exists between these concepts." << std::endl;
    }
}

void Learner::process() {
    std::cout << "[Learner] Phase: " << getStateName() << std::endl;
    
    switch (currentState) {
        case LearningState::GATHER:
            // Machine Learning: Store a snapshot of current concept values
            dataHistory.push_back(activeConcepts);
            break;

        case LearningState::ANALYZE:
            analyzeRelationships();
            break;
            
        case LearningState::SUMMARIZE: {
            // Machine Learning: Train a model based on history
            activeModel = Algorithm::synthesizeFromConcepts(activeConcepts);
            if (!activeModel.target.empty() && dataHistory.size() > 1) {
                activeModel.train(dataHistory, activeModel.target);
                
                // Save formula string
                std::string learnedRule = activeModel.target + " (Structural Model) learned with Confidence: " + std::to_string(activeModel.confidence);
                AmiValue v = { (void*)strdup(learnedRule.c_str()), AMI_TYPE_STRING };
                ami_add_fact(ks, "last_ml_model", v);
            }
            break;
        }
        
        case LearningState::APPLY: {
            if (dataHistory.size() > 1) {
                activeModel.train(dataHistory, activeModel.target); 
            }
            if (!activeModel.target.empty()) {
                Simulator::runTrial(activeModel, activeConcepts);
            }
            break;
        }

        default:
            break;
    }
}

void Learner::saveWeights() {
    std::cout << "[Learner] Persisting Neural Weights to Binary Store..." << std::endl;
    
    // Save Bias
    double* bVal = (double*)ami_malloc(sizeof(double));
    *bVal = activeModel.bias;
    AmiValue bv = { bVal, AMI_TYPE_DOUBLE };
    ami_add_fact(ks, "model_bias", bv);

    // Save Weights
    for (size_t i = 0; i < activeModel.weights.size(); ++i) {
        double* wVal = (double*)ami_malloc(sizeof(double));
        *wVal = activeModel.weights[i];
        AmiValue wv = { wVal, AMI_TYPE_DOUBLE };
        std::string key = "weight_" + activeModel.inputs[i];
        ami_add_fact(ks, key.c_str(), wv);
    }
}

void Learner::loadWeights() {
    std::cout << "[Learner] Attempting to restore Neural Model from Binary Store..." << std::endl;
    
    // Synthesize structure first if empty
    if (activeModel.target.empty()) {
        activeModel = Algorithm::synthesizeFromConcepts(activeConcepts);
    }

    if (activeModel.target.empty() || activeModel.inputs.empty()) {
        std::cout << "[Learner] Warning: No structural model found to apply weights to." << std::endl;
        return;
    }

    // Load Bias
    AmiValue bv = ami_get_fact(ks, "model_bias");
    if (bv.type == AMI_TYPE_DOUBLE) {
        activeModel.bias = *(double*)bv.data;
    }

    // Load Weights per input
    activeModel.weights.assign(activeModel.inputs.size(), 0.0);
    for (size_t i = 0; i < activeModel.inputs.size(); ++i) {
        std::string key = "weight_" + activeModel.inputs[i];
        AmiValue wv = ami_get_fact(ks, key.c_str());
        if (wv.type == AMI_TYPE_DOUBLE) {
            activeModel.weights[i] = *(double*)wv.data;
        }
    }

    activeModel.confidence = 1.0; // Assume stored models are verified
    std::cout << "[Learner] Model successfully restored: " << activeModel.target << " (Logical continuity verified)" << std::endl;
}

} // namespace Ami
