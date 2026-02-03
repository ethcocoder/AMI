#include "reasoning.h"
#include <algorithm>
#include <cstring>
#include <set>

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
    // Quietly add to concept map
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

void Learner::addSequence(std::string first, std::string second) {
    sequenceMap[first][second] += 1.0; // Increment sequence weight
}

void Learner::addEvidence(std::string concept, std::string snippet) {
    // RAG: Store the original text context for a concept
    if (evidenceMap[concept].size() < 3) { // Store top 3 citations to save space
        evidenceMap[concept].push_back(snippet);
    }
}

void Learner::analyzeRelationships() {
    std::cout << "[Learner] Searching for logical connections between " << activeConcepts.size() << " objects..." << std::endl;
    
    // Core Centrality Analysis: Find the "Main Ideas"
    std::map<std::string, int> centrality;
    for (const auto& entry : relationshipMap) {
        centrality[entry.first] += entry.second.size();
        for (const auto& target : entry.second) {
            centrality[target]++;
        }
    }

    // Identify top pillars (Main Ideas) - FILTER OUT COMMON WORDS
    std::set<std::string> stopWords = {"The", "And", "Was", "For", "With", "That", "This", "From", "Into", "Been", "They", "Were", "Their", "Will", "Would", "Which", "Could", "There", "These", "Those", "Section", "Chapter", "Very", "More", "Often", "Always", "About", "Also", "Then", "Thus", "Both", "Some", "Each", "Every", "Another", "Same", "Able", "Been"};
    
    std::vector<std::pair<std::string, int>> pillars;
    for (auto const& [name, count] : centrality) {
        // Allow shorter technical terms (2+ chars) or all-caps short terms
        bool isShortTechnical = (name.length() >= 2 && name.length() < 4 && isupper(name[0]));
        if (stopWords.count(name) || (name.length() < 4 && !isShortTechnical)) continue;
        pillars.push_back({name, count});
    }
    std::sort(pillars.begin(), pillars.end(), [](auto &left, auto &right) {
        return left.second > right.second;
    });

    std::cout << "[Brain] Central Pillars identified: ";
    for (int i = 0; i < std::min((int)pillars.size(), 5); ++i) {
        std::cout << pillars[i].first << " ";
        // Persist as Main Idea
        std::string key = "main_pillar_" + std::to_string(i);
        AmiValue v = { (void*)strdup(pillars[i].first.c_str()), AMI_TYPE_STRING };
        ami_add_fact(ks, key.c_str(), v);
    }
    std::cout << std::endl;
}

void Learner::queryConcept(std::string name) {
    std::cout << "[AmI] Reasoning about: " << name << "..." << std::endl;
    bool found = false;
    const Concept* targetConcept = nullptr;
    
    for (const auto& c : activeConcepts) {
        if (c.name == name) {
            targetConcept = &c;
            found = true;
            break;
        }
    }

    if (found) {
        // 1. Structural Synthesis (Sentence generation)
        std::cout << "Definition: " << name;
        std::string current = name;
        std::set<std::string> usedWords;
        int wordCount = 0;
        
        // Strategy: First try the exact concept casing, then try lower case
        std::vector<std::string> searchStart = {name, (char)tolower(name[0]) + name.substr(1)};

        for (int i = 0; i < 20; ++i) { 
            usedWords.insert(current);
            std::string bestNext = "";
            double maxW = -1.0;
            
            // Try current as is, or lowercase if it was a mid-sentence word
            std::string lookup = current;
            if (!sequenceMap.count(lookup) && isupper(lookup[0])) {
                lookup[0] = tolower(lookup[0]);
            }

            if (sequenceMap.count(lookup) && !sequenceMap[lookup].empty()) {
                for (const auto& next : sequenceMap[lookup]) {
                    if (next.second > maxW && !usedWords.count(next.first)) {
                        maxW = next.second;
                        bestNext = next.first;
                    }
                }
            }
            
            if (bestNext != "") {
                std::cout << " " << bestNext;
                current = bestNext;
                wordCount++;
                if (bestNext.back() == '.' || bestNext.back() == ';' || bestNext.back() == '!') break;
            } else break;
        }
        
        if (wordCount < 5) {
             // BRAIN SYNTHESIS: If Markov chain is poor, synthesize from RAG evidence
             if (evidenceMap.count(name) && !evidenceMap[name].empty()) {
                 std::string bestEvidence = "";
                 for (const auto& snippet : evidenceMap[name]) {
                     // Look for defining verbs
                     if (snippet.find(" is ") != std::string::npos || 
                         snippet.find(" represents ") != std::string::npos ||
                         snippet.find(" means ") != std::string::npos ||
                         snippet.find(" defined ") != std::string::npos) {
                         bestEvidence = snippet;
                         break;
                     }
                 }
                 if (bestEvidence.empty()) bestEvidence = evidenceMap[name][0];
                 
                 size_t dot = bestEvidence.find('.');
                 std::string fragment = (dot != std::string::npos ? bestEvidence.substr(0, dot) : bestEvidence);
                 
                 // If the fragment contains the name, try to extract the predicate
                 size_t namePos = fragment.find(name);
                 if (namePos != std::string::npos) {
                     std::string predicate = fragment.substr(namePos + name.length());
                     // Clean up leading punctuation/spaces
                     size_t firstChar = predicate.find_first_not_of(" \"',;:—");
                     if (firstChar != std::string::npos) {
                         std::cout << " " << (predicate[firstChar-1] == ' ' ? "" : " ") << predicate.substr(firstChar);
                     } else {
                         std::cout << " " << predicate;
                     }
                 } else {
                     std::cout << " (context: " << fragment << ")";
                 }
             }
        }
        std::cout << "." << std::endl;

        // 2. Structural Interaction (Multi-Hop Context)
        if (relationshipMap.count(name)) {
            std::cout << "  - Relational Context: " << name << " is fundamentally linked to ";
            const auto& rels = relationshipMap.at(name);
            for (size_t i = 0; i < std::min((size_t)5, rels.size()); ++i) {
                std::cout << rels[i] << (i < std::min((size_t)5, rels.size()) - 1 ? ", " : "");
            }
            std::cout << "." << std::endl;
            
            // Secondary Hop: Explain WHY it's linked
            if (!rels.empty()) {
                std::string neighbor = rels[0];
                if (evidenceMap.count(neighbor) && !evidenceMap[neighbor].empty()) {
                    std::cout << "    (Cross-Referencing " << neighbor << ": " << evidenceMap[neighbor][0] << ")" << std::endl;
                }
            }
        }

        // 3. Computed Intelligence (Neural Algorithm Integration)
        if (activeModel.target == name) {
            std::cout << "  - Computational Model: Active Simulation running..." << std::endl;
            double result = activeModel.predict(activeConcepts);
            std::cout << "    > Estimated Current State of " << name << ": " << result << " (Confidence: " << activeModel.confidence << ")" << std::endl;
        }

        // 4. Advanced RAG Retrieval
        if (evidenceMap.count(name) && !evidenceMap[name].empty()) {
            std::cout << "\n[Retrieval] Source Evidence (Direct):" << std::endl;
            for (const auto& cite : evidenceMap[name]) {
                std::cout << "  &> \"" << cite << "\"" << std::endl;
            }
        } else {
            // Global Fallback: Search all evidence for the string
            std::cout << "\n[Retrieval] No direct evidence. Performing Global Trace..." << std::endl;
            int foundCount = 0;
            for (const auto& entry : evidenceMap) {
                for (const auto& line : entry.second) {
                    if (line.find(name) != std::string::npos || line.find(name.substr(0, name.length()-1)) != std::string::npos) {
                        std::cout << "  ? (Inferred from " << entry.first << "): \"" << line << "\"" << std::endl;
                        if (++foundCount > 2) break;
                    }
                }
                if (foundCount > 2) break;
            }
        }
    } else {
        // Semantic Search Fallback for Unknown Concepts
        std::cout << "[AmI] Direct concept '" << name << "' is missing from the structural map." << std::endl;
        std::cout << "[AmI] Initializing Latent Search Across All Ingested Knowledge..." << std::endl;
        int searchCount = 0;
        for (const auto& entry : evidenceMap) {
            for (const auto& line : entry.second) {
                if (line.find(name) != std::string::npos) {
                    std::cout << "  > Found in context of " << entry.first << ": \"" << line << "\"" << std::endl;
                    searchCount++;
                }
                if (searchCount > 3) break;
            }
            if (searchCount > 3) break;
        }
        if (searchCount == 0) {
            std::cout << "[AmI] No latent traces found. I require further training on this subject." << std::endl;
        }
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
    std::cout << "[Learner] Persisting Neural Weights and Concept Map..." << std::endl;
    
    // 1. Save Concepts names
    for (const auto& c : activeConcepts) {
        std::string key = "concept_exists_" + c.name;
        AmiValue cv = { (void*)strdup("TRUE"), AMI_TYPE_STRING };
        ami_add_fact(ks, key.c_str(), cv);
    }

    // 2. Save Relationship Map (Structural Logic)
    for (const auto& entry : relationshipMap) {
        for (const auto& target : entry.second) {
            std::string key = "rel_link_" + entry.first + ":" + target;
            AmiValue rv = { (void*)strdup("TRUE"), AMI_TYPE_STRING };
            ami_add_fact(ks, key.c_str(), rv);
        }
    }

    // 3. Save Sequence Map (Conversational Flow)
    for (const auto& entry : sequenceMap) {
        for (const auto& next : entry.second) {
            std::string key = "seq_weight_" + entry.first + ":" + next.first;
            double* w = (double*)ami_malloc(sizeof(double));
            *w = next.second;
            ami_add_fact(ks, key.c_str(), { w, AMI_TYPE_DOUBLE });
        }
    }

    // 4. Save Evidence (RAG Chunks)
    for (const auto& entry : evidenceMap) {
        for (size_t i = 0; i < entry.second.size(); ++i) {
            std::string key = "evidence_" + entry.first + "_" + std::to_string(i);
            AmiValue ev = { (void*)strdup(entry.second[i].c_str()), AMI_TYPE_STRING };
            ami_add_fact(ks, key.c_str(), ev);
        }
    }

    // 5. Save Bias
    double* bVal = (double*)ami_malloc(sizeof(double));
    *bVal = activeModel.bias;
    AmiValue bv = { bVal, AMI_TYPE_DOUBLE };
    ami_add_fact(ks, "model_bias", bv);

    // 3. Save Weights
    for (size_t i = 0; i < activeModel.weights.size(); ++i) {
        double* wVal = (double*)ami_malloc(sizeof(double));
        *wVal = activeModel.weights[i];
        AmiValue wv = { wVal, AMI_TYPE_DOUBLE };
        std::string key = "weight_" + activeModel.inputs[i];
        ami_add_fact(ks, key.c_str(), wv);
    }
}

void Learner::loadWeights() {
    // 1. Restore Concept Map and Relationships quietly
    size_t factCount = ami_get_fact_count(ks);
    for (size_t i = 0; i < factCount; ++i) {
        std::string key = ami_get_fact_key(ks, i);
        if (key.find("concept_exists_") == 0) {
            identifyConcept(key.substr(15));
        }
        else if (key.find("rel_link_") == 0) {
            std::string link = key.substr(9);
            size_t sep = link.find(':');
            if (sep != std::string::npos) {
                addDirectedRelationship(link.substr(0, sep), link.substr(sep + 1));
            }
        }
        else if (key.find("seq_weight_") == 0) {
            std::string link = key.substr(11);
            size_t sep = link.find(':');
            if (sep != std::string::npos) {
                std::string first = link.substr(0, sep);
                std::string second = link.substr(sep + 1);
                AmiValue v = ami_get_fact(ks, key.c_str());
                if (v.type == AMI_TYPE_DOUBLE) {
                    sequenceMap[first][second] = *(double*)v.data;
                }
            }
        }
        else if (key.find("evidence_") == 0) {
            AmiValue v = ami_get_fact(ks, key.c_str());
            if (v.type == AMI_TYPE_STRING) {
                // Find concept name from "evidence_Name_X"
                std::string sub = key.substr(9);
                size_t lastUnderscore = sub.find_last_of('_');
                if (lastUnderscore != std::string::npos) {
                    std::string concept = sub.substr(0, lastUnderscore);
                    addEvidence(concept, (char*)v.data);
                }
            }
        }
    }

    // 2. Synthesize structure first if empty
    if (activeModel.target.empty()) {
        activeModel = Algorithm::synthesizeFromConcepts(activeConcepts);
    }

    if (activeModel.target.empty() || activeModel.inputs.empty()) {
        std::cout << "[Learner] Warning: No structural model found to apply weights to." << std::endl;
        return;
    }

    // 3. Load Bias
    AmiValue bv = ami_get_fact(ks, "model_bias");
    if (bv.type == AMI_TYPE_DOUBLE) {
        activeModel.bias = *(double*)bv.data;
    }

    // 4. Load Weights per input
    activeModel.weights.assign(activeModel.inputs.size(), 0.0);
    for (size_t i = 0; i < activeModel.inputs.size(); ++i) {
        std::string key = "weight_" + activeModel.inputs[i];
        AmiValue wv = ami_get_fact(ks, key.c_str());
        if (wv.type == AMI_TYPE_DOUBLE) {
            activeModel.weights[i] = *(double*)wv.data;
        }
    }

    activeModel.confidence = 1.0; // Assume stored models are verified
    // std::cout << "[Learner] Restoration Complete: " << activeConcepts.size() << " concepts restored." << std::endl;
}

} // namespace Ami
