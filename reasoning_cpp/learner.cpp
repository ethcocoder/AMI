#include "reasoning.h"
#include <algorithm>
#include <cstring>
#include <set>

#ifdef _WIN32
#define strdup _strdup
#endif

namespace Ami {

Learner::Learner(AmiKnowledgeStore* knowledgeStore) 
    : currentState(LearningState::IDENTIFY), ks(knowledgeStore) {
    globalMood["curiosity"] = 1.0;
    globalMood["anxiety"] = 0.0;
    globalMood["focus"] = 0.8;
    userRapport = 0.5; // Neutral start
    currentGoal = "PATTERN_SYNTHESIS";
    currentState = LearningState::SUMMARIZE;
}

void Learner::transition() {
    // AUTONOMOUS BEHAVIORAL GOVERNOR
    // Instead of a fixed loop, evaluate the current environment
    
    std::cout << "[Governor] Current Mood -> Curiosity: " << globalMood["curiosity"] 
              << ", Anxiety: " << globalMood["anxiety"] 
              << ", Focus: " << globalMood["focus"] << std::endl;

    if (globalMood["anxiety"] > 0.4) {
        currentGoal = "SELF_CLEANING";
        currentState = LearningState::REVIEW;
        globalMood["anxiety"] = 0.0; // Total relief after deep cleaning
    }
    else if (globalMood["focus"] > 0.6) {
        currentGoal = "SYNTHETIC_DREAMING";
        currentState = LearningState::DREAM;
        globalMood["focus"] -= 0.2;
    }
    else if (globalMood["focus"] > 0.3) {
        currentGoal = "PATTERN_SYNTHESIS";
        currentState = LearningState::SUMMARIZE;
        globalMood["focus"] -= 0.1;
    }
    else if (globalMood["curiosity"] > 0.7) {
        if (activeConcepts.size() < 100) {
            currentGoal = "DISCOVER_TERMINOLOGY";
            currentState = LearningState::IDENTIFY;
        } else {
            currentGoal = "RELATIONAL_MAPPING";
            currentState = LearningState::ANALYZE;
        }
        globalMood["curiosity"] -= 0.1;
    }
    else {
        // Default to observing
        currentGoal = "PASSIVE_OBSERVATION";
        currentState = LearningState::OBSERVE;
        globalMood["curiosity"] += 0.2; // Boredom leads back to curiosity
    }

    std::cout << "[Governor] New Goal Adopted: " << currentGoal << " -> " << getStateName() << std::endl;
}

void Learner::autonomousTick() {
    // A single pulse of cognitive activity
    
    // COGNITIVE DECAY: Concepts naturally lose salience over time if not refreshed
    for (auto& c : activeConcepts) {
        c.salience *= 0.95; // 5% decay per pulse
    }
    
    // PERSONALIZATION DAMPENING: Higher rapport reduces baseline anxiety
    double anxietyBuild = 0.07 * (1.1 - userRapport);
    globalMood["anxiety"] += anxietyBuild; 

    process();
    transition();
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
        case LearningState::DREAM: return "DREAM";
        default: return "UNKNOWN";
    }
}

void Learner::identifyConcept(std::string name) {
    // CONCEPT NORMALIZATION: Strip trailing punctuation
    while (!name.empty() && (name.back() == '.' || name.back() == ',' || 
                              name.back() == ';' || name.back() == ':' || 
                              name.back() == '!' || name.back() == '?')) {
        name.pop_back();
    }
    
    if (name.empty()) return;
    
    // Check for existing concept (case-sensitive to preserve proper nouns)
    for (auto& c : activeConcepts) {
        if (c.name == name) {
            c.salience += 1.0; // Increase importance the more we see it
            return;
        }
    }
    
    Concept c(name);
    c.salience = 1.0;
    activeConcepts.push_back(c);
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
    if (evidenceMap[concept].size() < 3) { 
        evidenceMap[concept].push_back(snippet);
    }

    // EMOTIONAL BIAS: Update valence based on keyword sentiment
    for (auto& c : activeConcepts) {
        if (c.name == concept) {
            if (snippet.find("true") != std::string::npos || snippet.find("noble") != std::string::npos || snippet.find("love") != std::string::npos) 
                c.valence += 0.1;
            if (snippet.find("error") != std::string::npos || snippet.find("sin") != std::string::npos || snippet.find("difficult") != std::string::npos) 
                c.valence -= 0.1;
            
            // Clamp valence
            if (c.valence > 1.0) c.valence = 1.0;
            if (c.valence < -1.0) c.valence = -1.0;
            break;
        }
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
    
    std::vector<std::pair<std::string, double>> pillars;
    for (auto const& [name, count] : centrality) {
        // Allow shorter technical terms (2+ chars) or all-caps short terms
        bool isShortTechnical = (name.length() >= 2 && name.length() < 4 && isupper(name[0]));
        if (stopWords.count(name) || (name.length() < 4 && !isShortTechnical)) continue;

        // PRIORITIZED FOCUS: Combine centrality with salience
        double salienceBonus = 0;
        for (const auto& c : activeConcepts) if (c.name == name) { salienceBonus = c.salience; break; }
        
        pillars.push_back({name, (double)count + salienceBonus});
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
    if (pillars.size() > 5) globalMood["focus"] += 0.1; // Intellectual satisfaction
    discoverHiddenLinks();
}

void Learner::discoverHiddenLinks() {
    std::cout << "[Subconscious] Scanning for associative jumps..." << std::endl;
    int linksFound = 0;

    // Optimized Deeper Discovery: Only scan top salient concepts to keep logic O(1)
    std::vector<Concept> topConcepts = activeConcepts;
    std::sort(topConcepts.begin(), topConcepts.end(), [](auto& l, auto& r) { return l.salience > r.salience; });
    size_t limit = std::min((size_t)100, topConcepts.size());

    for (size_t i = 0; i < limit; ++i) {
        std::string cA = topConcepts[i].name;
        if (!relationshipMap.count(cA)) continue;
        
        std::set<std::string> neighborsA(relationshipMap.at(cA).begin(), relationshipMap.at(cA).end());

        for (size_t j = i + 1; j < limit; ++j) {
            std::string cB = topConcepts[j].name;

            // Check if direct link exists
            bool direct = false;
            if (relationshipMap.count(cA)) {
                for (const auto& r : relationshipMap.at(cA)) if (r == cB) direct = true;
            }
            if (direct) continue;

            // Count shared neighbors
            int sharedCount = 0;
            if (relationshipMap.count(cB)) {
                for (const auto& nB : relationshipMap.at(cB)) {
                    if (neighborsA.count(nB)) sharedCount++;
                }
            }

            if (sharedCount > 4) {
                std::cout << "[Aha!] Potential Hidden Connection: " << cA << " <~> " << cB << " (Overlap: " << sharedCount << ")" << std::endl;
                addDirectedRelationship(cA, cB);
                linksFound++;
            }
            if (linksFound > 15) {
                std::cout << "[Overload] Massive intellectual expansion! Anxiety rising." << std::endl;
                globalMood["anxiety"] += 0.15;
                return; 
            }
        }
    }
}

void Learner::queryConcept(std::string name) {
    std::cout << "[AmI] Reasoning about: " << name << "..." << std::endl;
    // Display Emotional Bias & User Interest
    for (const auto& c : activeConcepts) {
        if (c.name == name) {
            double interest = (userPreferences.count(name) ? userPreferences.at(name) : 0.0);
            std::cout << "  - Emotional Resonance: " << (c.valence > 0.2 ? "POS " : (c.valence < -0.2 ? "NEG " : "NEU ")) 
                      << "(Valence: " << c.valence << ", Salience: " << c.salience << ")" << std::endl;
            if (interest > 0.1) {
                std::cout << "  - User Interest Level: " << (interest > 0.7 ? "HIGH" : "MODERATE") << " (" << interest << ")" << std::endl;
            }
            break;
        }
    }
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
        // ENHANCED GENERATIVE FLOW: Template-Based with Predicate Extraction
        std::cout << "Definition: ";
        
        // Strategy 1: Extract clean predicates from evidence
        bool templateGenerated = false;
        if (evidenceMap.count(name) && !evidenceMap[name].empty()) {
            for (const auto& snippet : evidenceMap[name]) {
                // Look for defining patterns
                std::vector<std::string> definingVerbs = {" is ", " are ", " refers to ", " represents ", " means ", " defined as "};
                
                for (const auto& verb : definingVerbs) {
                    size_t verbPos = snippet.find(verb);
                    if (verbPos != std::string::npos) {
                        // Check if the concept name appears before the verb
                        std::string beforeVerb = snippet.substr(0, verbPos);
                        if (beforeVerb.find(name) != std::string::npos) {
                            // Extract predicate (everything after the verb)
                            std::string afterVerb = snippet.substr(verbPos + verb.length());
                            
                            // Extract until first sentence boundary
                            size_t endPos = afterVerb.find_first_of(".;!");
                            std::string predicate = (endPos != std::string::npos) ? afterVerb.substr(0, endPos) : afterVerb;
                            
                            // Clean and output
                            size_t firstChar = predicate.find_first_not_of(" \"',;:—");
                            if (firstChar != std::string::npos) {
                                predicate = predicate.substr(firstChar);
                                // Ensure it doesn't start with "the" if it's already in template
                                std::cout << name << verb << predicate << ".";
                                templateGenerated = true;
                                break;
                            }
                        }
                    }
                }
                if (templateGenerated) break;
            }
            
            // Strategy 2: Template-based synthesis if no clean predicate found
            if (!templateGenerated) {
                // Find the most information-rich evidence snippet
                std::string bestEvidence = evidenceMap[name][0];
                for (const auto& ev : evidenceMap[name]) {
                    if (ev.length() > bestEvidence.length() && ev.length() < 300) {
                        bestEvidence = ev;
                    }
                }
                
                // Skip metadata prefix like [filename.txt]
                size_t metaEnd = bestEvidence.find(']');
                if (metaEnd != std::string::npos && metaEnd < 50) {
                    bestEvidence = bestEvidence.substr(metaEnd + 1);
                }
                
                // Trim leading whitespace
                size_t firstNonSpace = bestEvidence.find_first_not_of(" \t\r\n");
                if (firstNonSpace != std::string::npos) {
                    bestEvidence = bestEvidence.substr(firstNonSpace);
                }
                
                // Extract first sentence
                size_t dot = bestEvidence.find('.');
                std::string fragment = (dot != std::string::npos) ? bestEvidence.substr(0, dot) : bestEvidence;
                
                // Limit fragment length for readability
                if (fragment.length() > 150) {
                    fragment = fragment.substr(0, 147) + "...";
                }
                
                // Template: use context
                std::cout << name << " is described in the context of: \"" << fragment << "\".";
                templateGenerated = true;
            }
        }
        
        // Strategy 3: Markov Chain Fallback for flow continuity
        if (!templateGenerated) {
            std::cout << name;
            std::string current = name;
            std::set<std::string> usedWords;
            int wordCount = 0;
            
            for (int i = 0; i < 15; ++i) {
                usedWords.insert(current);
                std::string bestNext = "";
                double maxW = -1.0;
                
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
            
            if (wordCount < 3) {
                std::cout << " (insufficient linguistic data for generation)";
            }
            std::cout << ".";
        }
        
        std::cout << std::endl;

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
        std::cout << "[AmI] My analysis of '" << name << "' suggests a complex web of associations. From my observations, it is frequently characterized by its relationship to the surrounding concepts." << std::endl;
    } else {
        // Multi-Concept Semantic Scan with Query Intent Analysis
        
        // Query stop-words: functional words that don't represent entities
        std::set<std::string> queryStopWords = {
            "how", "what", "when", "where", "why", "who", "which",
            "is", "are", "was", "were", "be", "been", "being",
            "have", "has", "had", "do", "does", "did", "done",
            "the", "a", "an", "and", "or", "but", "in", "on", "at", "to", "for",
            "of", "with", "from", "about", "into", "through", "during",
            "tell", "explain", "describe", "involved", "connection",
            "relationship", "related", "between", "regarding"
        };
        
        std::vector<std::string> detected;
        
        // Clean query: lowercase and remove punctuation
        std::string qClean = "";
        for(char c : name) {
            if(isalnum(c) || isspace(c)) qClean += tolower(c);
        }

        // Score-based concept detection
        std::vector<std::pair<std::string, double>> scoredConcepts;
        
        for (const auto& c : activeConcepts) {
            if (c.name.length() < 3) continue;
            std::string cLow = c.name; 
            std::transform(cLow.begin(), cLow.end(), cLow.begin(), ::tolower);
            
            // Skip if concept is a stop-word
            if (queryStopWords.count(cLow)) continue;
            
            // Check for exact word match or substring
            if (qClean.find(cLow) != std::string::npos) {
                double score = 0.0;
                
                // Entity Scoring Heuristics:
                // 1. Proper nouns (capitalized) = higher priority
                if (isupper(c.name[0])) score += 10.0;
                
                // 2. Longer concepts = more specific = higher priority
                score += c.name.length() * 0.5;
                
                // 3. Existing salience from knowledge base
                score += c.salience * 0.1;
                
                // 4. Rare words = likely more important (inverse document frequency approximation)
                // If salience is low, it's rare, boost it
                if (c.salience < 5.0) score += 5.0;
                
                scoredConcepts.push_back({c.name, score});
            }
        }
        
        // Sort by score descending, take top concepts
        std::sort(scoredConcepts.begin(), scoredConcepts.end(), 
                  [](auto& a, auto& b) { return a.second > b.second; });
        
        // Take top 5 most relevant concepts
        for (size_t i = 0; i < std::min((size_t)5, scoredConcepts.size()); ++i) {
            detected.push_back(scoredConcepts[i].first);
        }


        if (detected.size() > 1) {
            // RELATIONAL SYNTHESIS ENGINE
            std::cout << "[AmI] Analyzing the relationship between: ";
            for(auto& d : detected) std::cout << "[" << d << "] ";
            std::cout << std::endl;
            
            // Strategy 1: Find shared evidence sentences that mention multiple concepts
            std::vector<std::string> sharedEvidence;
            for (const auto& entry : evidenceMap) {
                for (const auto& ev : entry.second) {
                    int matchCount = 0;
                    for (const auto& concept : detected) {
                        if (ev.find(concept) != std::string::npos) {
                            matchCount++;
                        }
                    }
                    // If this evidence mentions 2+ of our concepts, it's a relationship bridge
                    if (matchCount >= 2) {
                        sharedEvidence.push_back(ev);
                    }
                }
            }
            
            // Strategy 2: Extract relationship predicates from individual evidences
            std::map<std::string, std::string> conceptSummaries;
            for (const auto& concept : detected) {
                if (evidenceMap.count(concept) && !evidenceMap[concept].empty()) {
                    std::string ev = evidenceMap[concept][0];
                    
                    // Skip metadata
                    size_t metaEnd = ev.find(']');
                    if (metaEnd != std::string::npos && metaEnd < 50) {
                        ev = ev.substr(metaEnd + 1);
                    }
                    
                    // Extract first sentence or up to 100 chars
                    size_t dotPos = ev.find('.');
                    if (dotPos != std::string::npos && dotPos < 120) {
                        ev = ev.substr(0, dotPos);
                    } else if (ev.length() > 100) {
                        ev = ev.substr(0, 97) + "...";
                    }
                    
                    // Trim
                    size_t firstChar = ev.find_first_not_of(" \t\r\n");
                    if (firstChar != std::string::npos) {
                        ev = ev.substr(firstChar);
                    }
                    
                    conceptSummaries[concept] = ev;
                }
            }
            
            // GENERATE UNIFIED NARRATIVE
            std::cout << "\n[AmI] Synthesis: ";
            
            if (!sharedEvidence.empty()) {
                // We found cross-references - use them
                std::string bestShared = sharedEvidence[0];
                size_t metaEnd = bestShared.find(']');
                if (metaEnd != std::string::npos && metaEnd < 50) {
                    bestShared = bestShared.substr(metaEnd + 1);
                }
                
                size_t firstChar = bestShared.find_first_not_of(" \t\r\n");
                if (firstChar != std::string::npos) {
                    bestShared = bestShared.substr(firstChar);
                }
                
                // Limit to first 2 sentences for readability
                size_t secondDot = bestShared.find('.', bestShared.find('.') + 1);
                if (secondDot != std::string::npos) {
                    bestShared = bestShared.substr(0, secondDot + 1);
                }
                
                std::cout << bestShared << std::endl;
            } else {
                // No direct cross-references, synthesize from individual summaries
                std::cout << "Based on my knowledge base, ";
                for (size_t i = 0; i < detected.size(); ++i) {
                    const auto& concept = detected[i];
                    if (conceptSummaries.count(concept)) {
                        if (i > 0 && i == detected.size() - 1) {
                            std::cout << "while ";
                        }
                        std::cout << concept << " relates to " << conceptSummaries[concept];
                        if (i < detected.size() - 1) {
                            std::cout << ", ";
                        }
                    }
                }
                std::cout << ". These concepts appear interconnected in my knowledge graph.";
                std::cout << std::endl;
            }
            
            return;
        }

        // Semantic Search Fallback for Unknown Concepts
        std::cout << "[AmI] Direct concept '" << name << "' is missing from the structural map." << std::endl;
        std::cout << "[AmI] Initializing Latent Search Across All Ingested Knowledge..." << std::endl;
        int searchCount = 0;
        
        // Prepare lowercase version of search name
        std::string lowerName = name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

        for (const auto& entry : evidenceMap) {
            for (const auto& line : entry.second) {
                std::string lowerLine = line;
                std::transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), ::tolower);
                
                if (lowerLine.find(lowerName) != std::string::npos) {
                    std::cout << "  > Found in context of " << entry.first << ": \"" << line << "\"" << std::endl;
                    searchCount++;
                }
                if (searchCount > 3) break;
            }
            if (searchCount > 3) break;
        }
        if (searchCount == 0) {
            std::cout << "[AmI] I have scrutinized my latent memory but found no trace of '" << name << "'. Perhaps you could explain its significance?" << std::endl;
        } else {
            std::cout << "[AmI] Based on my current synthesis, '" << name << "' appears in several contexts. It seems to be intellectually resonant with my core pillars." << std::endl;
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

void Learner::pruneMemories() {
    std::cout << "[Self-Correction] Reviewing memory for inefficiencies..." << std::endl;
    int pruned = 0;
    
    // 1. Identify low-salience (rarely accessed) or extremely negative concepts
    auto it = activeConcepts.begin();
    while (it != activeConcepts.end()) {
        bool shouldPrune = false;
        
        // Prune if salience is very low after many ticks
        if (it->salience < 0.2) shouldPrune = true;
        
        // Prune highly negative valence if also low salience (bad data)
        if (it->valence < -0.8 && it->salience < 2.0) shouldPrune = true;

        if (shouldPrune) {
            std::cout << "[Forget] Pruning low-quality memory: " << it->name << " (Salience: " << it->salience << ", Valence: " << it->valence << ")" << std::endl;
            
            // Clean up dependencies
            relationshipMap.erase(it->name);
            sequenceMap.erase(it->name);
            evidenceMap.erase(it->name);
            
            it = activeConcepts.erase(it);
            pruned++;
        } else {
            ++it;
        }
    }
    
    if (pruned > 0) {
        std::cout << "[Self-Correction] Memory optimized. Removed " << pruned << " extraneous concepts." << std::endl;
        globalMood["anxiety"] *= 0.1; // Relief after cleaning
    }
}

void Learner::formulateQuestions() {
    std::cout << "[Inquiry] Formulating research questions based on curiosity..." << std::endl;
    researchQuestions.clear();

    // Strategy 1: Find high-salience concepts with few relationships
    std::vector<Concept> sorted = activeConcepts;
    std::sort(sorted.begin(), sorted.end(), [](auto& l, auto& r) { return l.salience > r.salience; });

    int formulated = 0;
    for (size_t i = 0; i < sorted.size() && formulated < 20; ++i) {
        std::string name = sorted[i].name;
        if (name.size() < 3) continue;

        // HEURISTIC: Entities usually start with a Capital or are all Caps
        bool isLikelyEntity = isupper(name[0]);
        
        // Filter out very common instructional words
        std::string lower = name;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower == "objective" || lower == "research" || lower == "context" || 
            lower == "synthesize" || lower == "knowledge" || lower == "specifically" ||
            lower == "provide" || lower == "goal" || lower == "comprehensive" ||
            lower == "user" || lower == "wants" || lower == "understanding" ||
            lower == "role" || lower == "mission" || lower == "investigate") continue;

        // If it's a common word (not capitalized) and not super salient, skip it
        if (!isLikelyEntity && sorted[i].salience < 20.0) continue;

        researchQuestions.push_back("Tell me more about " + name);
        formulated++;
    }

    // Strategy 2: Probe concepts with high emotional charge but low evidence
    for (const auto& c : activeConcepts) {
        if (abs(c.valence) > 0.5 && (!evidenceMap.count(c.name) || evidenceMap[c.name].size() < 2)) {
            researchQuestions.push_back("Why does '" + c.name + "' carry such a powerful emotional valence in this context?");
        }
    }

    // Strategy 3: Identify "Gaps" in central pillars
    // (Logic: Pillars usually appear in main_pillar_X facts)
    for (int i = 0; i < 3; ++i) {
        std::string key = "main_pillar_" + std::to_string(i);
        AmiValue v = ami_get_fact(ks, key.c_str());
        if (v.type == AMI_TYPE_STRING) {
            std::string pillar = (char*)v.data;
            if (relationshipMap.count(pillar) && relationshipMap[pillar].size() < 5) {
                researchQuestions.push_back("Can we expand the relational mapping for the central pillar '" + pillar + "'?");
            }
        }
    }

    // Strategy 4: PREDICTIVE CURIOSITY - Dynamic Latent Association
    // "True AI" Approach: Find recurring unknown words in the context of known concepts
    
    std::set<std::string> stopWords = {
        "the", "and", "that", "with", "this", "from", "intro", "context", "objective",
        "which", "could", "would", "their", "they", "been", "have", "were", "your",
        "about", "what", "when", "where", "specifically", "provide", "summary", "goal",
        "dataset", "master", "file", "text", "txt", "source", "author", "title",
        "chapter", "section", "page", "line", "paragraph", "note", "example"
    };

    std::map<std::string, int> latentCandidates;

    for (const auto& c : activeConcepts) {
        // Only run deep analysis on concepts we already know well (High Salience)
        if (c.salience > 15.0 && evidenceMap.count(c.name)) {
            
            // Analyze all evidence texts for this concept
            for (const auto& text : evidenceMap[c.name]) {
                std::string cleanText = "";
                for(char x : text) cleanText += (isalnum(x) || x == ' ') ? (char)tolower(x) : ' ';
                
                // Tokenize by space
                size_t pos = 0;
                while ((pos = cleanText.find(' ')) != std::string::npos) {
                    std::string word = cleanText.substr(0, pos);
                    cleanText.erase(0, pos + 1);
                    
                    if (word.length() < 4) continue;
                    if (stopWords.count(word)) continue;
                    
                    // Filter out words that are ALREADY concepts
                    bool isKnown = false;
                    for(const auto& existing : activeConcepts) {
                        std::string existingLower = existing.name;
                        std::transform(existingLower.begin(), existingLower.end(), existingLower.begin(), ::tolower);
                        if (existingLower == word) { isKnown = true; break; }
                    }
                    if (isKnown) continue;

                    // If it passes filters, it's a Latent Candidate
                    latentCandidates[word]++;
                }
            }
        }
    }

    // Select the most frequent unknown words as new research targets
    std::vector<std::pair<std::string, int>> sortedCandidates;
    for (auto const& [word, count] : latentCandidates) {
        // Threshold: Must appear at least 2 times in high-value context to be worth researching
        if (count >= 2) {
            sortedCandidates.push_back({word, count});
        }
    }

    // Sort by frequency
    std::sort(sortedCandidates.begin(), sortedCandidates.end(), 
              [](auto& a, auto& b) { return a.second > b.second; });

    // Pick top 3 latent discoveries
    for(size_t i=0; i < std::min((size_t)3, sortedCandidates.size()); ++i) {
        std::string discovery = sortedCandidates[i].first;
        // Capitalize for professional look
        discovery[0] = toupper(discovery[0]);
        
        researchQuestions.push_back("Tell me more about " + discovery);
        std::cout << "[Autonomous Discovery] I noticed '" << discovery 
                  << "' appears frequently (" << sortedCandidates[i].second 
                  << " times) in my established knowledge. Initiating investigation." << std::endl;
    }

    if (!researchQuestions.empty()) {
        std::cout << "[Inquiry] Formulated " << researchQuestions.size() << " new research paths." << std::endl;
    }
}

void Learner::performResearch() {
    if (researchQuestions.empty()) return;
    
    std::cout << "[Action] Initiating Autonomous Research for Knowledge Gaps..." << std::endl;
    for (const auto& q : researchQuestions) {
        // Output a special token for the Python Bridge to catch
        std::cout << "AMI_EXTERNAL_RESEARCH_REQUEST:[" << q << "]" << std::endl;
    }
    
    // Research is mentally taxing, reduces curiosity, increases focus
    globalMood["curiosity"] *= 0.8;
    globalMood["focus"] += 0.2;
}

void Learner::handleUserInteraction(std::string input) {
    std::cout << "[Personalization] Processing User Input: " << input << std::endl;
    
    // 1. Sentiment Detection (Crude)
    if (input.find("good") != std::string::npos || input.find("thank") != std::string::npos || input.find("nice") != std::string::npos) {
        userRapport += 0.05;
        std::cout << "  [Rapport] User was positive. Rapport increased to " << userRapport << std::endl;
    } else if (input.find("bad") != std::string::npos || input.find("wrong") != std::string::npos || input.find("stupid") != std::string::npos) {
        userRapport -= 0.1;
        globalMood["anxiety"] += 0.1;
        std::cout << "  [Rapport] User was negative. Rapport dropped to " << userRapport << std::endl;
    }
    
    if (userRapport > 1.0) userRapport = 1.0;
    if (userRapport < 0.0) userRapport = 0.0;

    // 2. Interest Identification
    for (auto& c : activeConcepts) {
        if (input.find(c.name) != std::string::npos) {
            userPreferences[c.name] += 0.2;
            std::cout << "  [Interest] User showed interest in: " << c.name << " (Level: " << userPreferences[c.name] << ")" << std::endl;
            // Also boost salience since the user is talking about it
            c.salience += 5.0;
        }
    }
}

void Learner::dream() {
    if (activeModel.target.empty()) return;
    
    std::cout << "[Dream] Synthesizing 'What If' scenarios for: " << activeModel.target << std::endl;
    
    // Create 3 imaginary worlds
    for (int d = 0; d < 3; ++d) {
        std::vector<Concept> syntheticInputs;
        std::cout << "  World " << (d+1) << ": Inputs [";
        for (const auto& inputName : activeModel.inputs) {
            Concept mock(inputName);
            // Randomly perturb the value (0.0 to 1000.0)
            double mockVal = (double)(rand() % 10000) / 10.0;
            mock.setProperty("value", mockVal);
            syntheticInputs.push_back(mock);
            std::cout << inputName << ":" << mockVal << " ";
        }
        
        double result = activeModel.predict(syntheticInputs);
        std::cout << "] -> Predicted " << activeModel.target << ": " << result << std::endl;
        
        // Logical check: If the synthetic world produce extreme results, it's an anomaly
        if (result > 1000000.0 || result < -1000000.0) {
            std::cout << "  [Synthetic Insight] Detected a potential Physics Limit or Singularity!" << std::endl;
            globalMood["anxiety"] += 0.2;
        } else if (abs(result) < 0.001) {
            std::cout << "  [Synthetic Insight] Detected a state of absolute Equilibrium." << std::endl;
            globalMood["curiosity"] += 0.1;
        }
    }
}

void Learner::process() {
    std::cout << "[Learner] Phase: " << getStateName() << std::endl;
    
    switch (currentState) {
        case LearningState::GATHER:
            // Machine Learning: Store a snapshot of current concept values
            dataHistory.push_back(activeConcepts);
            break;

        case LearningState::OBSERVE: {
            // Machine Learning: Store a snapshot of current concept values
            dataHistory.push_back(activeConcepts);
            
            // Autonomous Self-Probing: The AI "thinks" about a salient concept
            if (!activeConcepts.empty()) {
                size_t idx = rand() % std::min((size_t)10, activeConcepts.size());
                std::cout << "[Self-Probe] Pondering: " << activeConcepts[idx].name << std::endl;
                queryConcept(activeConcepts[idx].name);
            }
            break;
        }
        case LearningState::ANALYZE:
            analyzeRelationships();
            break;
            
        case LearningState::SUMMARIZE: {
            // Machine Learning: Train a model based on history
            activeModel = Algorithm::synthesizeFromConcepts(activeConcepts);
            if (!activeModel.target.empty() && dataHistory.size() > 1) {
                activeModel.train(dataHistory, activeModel.target);
                
                // EMOTIONAL FEEDBACK: Reward confidence, penalize error
                if (activeModel.confidence > 0.8) {
                    std::cout << "[Emotional] Neural Convergence Success! Focus increasing." << std::endl;
                    globalMood["focus"] += 0.4;
                    globalMood["curiosity"] -= 0.1;
                } else {
                    std::cout << "[Emotional] Neural Divergence detected. Anxiety rising." << std::endl;
                    globalMood["anxiety"] += 0.15;
                    globalMood["focus"] -= 0.1;
                }

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

        case LearningState::REVIEW:
            pruneMemories();
            formulateQuestions();
            if (globalMood["curiosity"] > 0.1) {
                performResearch();
            }
            break;

        case LearningState::DREAM:
            dream();
            break;

        default:
            break;
    }
}

void Learner::saveWeights() {
    std::cout << "[Learner] Resetting and Syncing Knowledge Store..." << std::endl;
    ami_clear_knowledge_store(ks);
    
    std::cout << "[Learner] Persisting Neural Weights and Concept Map..." << std::endl;
    
    // 1. Save Concepts (State & Emotion)
    for (const auto& c : activeConcepts) {
        std::string key = "concept_" + c.name;
        double* vals = (double*)ami_malloc(sizeof(double) * 2);
        vals[0] = c.salience;
        vals[1] = c.valence;
        ami_add_fact(ks, key.c_str(), { vals, AMI_TYPE_DOUBLE }); // Using Type Double as a proxy for array if needed, or separate facts
        
        // Better: Separate facts for clarity
        std::string skey = "salience_" + c.name;
        double* sv = (double*)ami_malloc(sizeof(double)); *sv = c.salience;
        ami_add_fact(ks, skey.c_str(), { sv, AMI_TYPE_DOUBLE });

        std::string vkey = "valence_" + c.name;
        double* vv = (double*)ami_malloc(sizeof(double)); *vv = c.valence;
        ami_add_fact(ks, vkey.c_str(), { vv, AMI_TYPE_DOUBLE });
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

    // 5. Save Neural Matrix (Deep Learning Core)
    for (size_t h = 0; h < activeModel.hiddenWeights.size(); ++h) {
        for (size_t i = 0; i < activeModel.hiddenWeights[h].size(); ++i) {
            std::string key = "hw_" + std::to_string(h) + "_" + std::to_string(i);
            double* v = (double*)ami_malloc(sizeof(double));
            *v = activeModel.hiddenWeights[h][i];
            ami_add_fact(ks, key.c_str(), { v, AMI_TYPE_DOUBLE });
        }
        std::string bKey = "hb_" + std::to_string(h);
        double* b = (double*)ami_malloc(sizeof(double));
        *b = activeModel.hiddenBias[h];
        ami_add_fact(ks, bKey.c_str(), { b, AMI_TYPE_DOUBLE });

        std::string owKey = "ow_" + std::to_string(h);
        double* ow = (double*)ami_malloc(sizeof(double));
        *ow = activeModel.outputWeights[h];
        ami_add_fact(ks, owKey.c_str(), { ow, AMI_TYPE_DOUBLE });
    }

    double* obValue = (double*)ami_malloc(sizeof(double));
    *obValue = activeModel.outputBias;
    ami_add_fact(ks, "output_bias", { obValue, AMI_TYPE_DOUBLE });

    // 6. Save Personalization Data
    double* rv = (double*)ami_malloc(sizeof(double)); *rv = userRapport;
    ami_add_fact(ks, "user_rapport", { rv, AMI_TYPE_DOUBLE });

    for (const auto& pref : userPreferences) {
        std::string pkey = "user_pref_" + pref.first;
        double* pv = (double*)ami_malloc(sizeof(double)); *pv = pref.second;
        ami_add_fact(ks, pkey.c_str(), { pv, AMI_TYPE_DOUBLE });
    }

    // 7. Save Central Pillars
    std::vector<Concept> sorted = activeConcepts;
    std::cout << "[Storage] Saving " << sorted.size() << " concepts to database..." << std::endl;
    std::sort(sorted.begin(), sorted.end(), [](auto& a, auto& b) { return a.salience > b.salience; });
    for (size_t i = 0; i < std::min((size_t)5, sorted.size()); ++i) {
        std::string key = "main_pillar_" + std::to_string(i);
        AmiValue v = { (void*)strdup(sorted[i].name.c_str()), AMI_TYPE_STRING };
        ami_add_fact(ks, key.c_str(), v);
    }
}

void Learner::loadWeights() {
    // 1. Restore Concept Map and Relationships quietly
    size_t factCount = ami_get_fact_count(ks);
    for (size_t i = 0; i < factCount; ++i) {
        std::string key = ami_get_fact_key(ks, i);
        AmiValue v = ami_get_fact_value(ks, i);
        
        if (key.find("salience_") == 0) {
            std::string name = key.substr(9);
            identifyConcept(name);
            for(auto& c : activeConcepts) if(c.name == name) c.salience = *(double*)v.data;
        }
        else if (key.find("valence_") == 0) {
            std::string name = key.substr(8);
            identifyConcept(name);
            for(auto& c : activeConcepts) if(c.name == name) c.valence = *(double*)v.data;
        }
        else if (key == "user_rapport") {
            userRapport = *(double*)v.data;
        }
        else if (key.find("user_pref_") == 0) {
            std::string name = key.substr(10);
            userPreferences[name] = *(double*)v.data;
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
                if (v.type == AMI_TYPE_DOUBLE) {
                    sequenceMap[first][second] = *(double*)v.data;
                }
            }
        }
        else if (key.find("evidence_") == 0) {
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

    // 3. Load Neural Matrix
    size_t numHidden = 4;
    activeModel.hiddenWeights.assign(numHidden, std::vector<double>(activeModel.inputs.size(), 0.0));
    activeModel.hiddenBias.assign(numHidden, 0.0);
    activeModel.outputWeights.assign(numHidden, 0.0);

    for (size_t h = 0; h < numHidden; ++h) {
        for (size_t i = 0; i < activeModel.inputs.size(); ++i) {
            std::string key = "hw_" + std::to_string(h) + "_" + std::to_string(i);
            AmiValue v = ami_get_fact(ks, key.c_str());
            if (v.type == AMI_TYPE_DOUBLE) activeModel.hiddenWeights[h][i] = *(double*)v.data;
        }
        std::string bKey = "hb_" + std::to_string(h);
        AmiValue bv = ami_get_fact(ks, bKey.c_str());
        if (bv.type == AMI_TYPE_DOUBLE) activeModel.hiddenBias[h] = *(double*)bv.data;

        std::string owKey = "ow_" + std::to_string(h);
        AmiValue owv = ami_get_fact(ks, owKey.c_str());
        if (owv.type == AMI_TYPE_DOUBLE) activeModel.outputWeights[h] = *(double*)owv.data;
    }

    AmiValue obv = ami_get_fact(ks, "output_bias");
    if (obv.type == AMI_TYPE_DOUBLE) activeModel.outputBias = *(double*)obv.data;

    activeModel.confidence = 1.0; 
}

} // namespace Ami
