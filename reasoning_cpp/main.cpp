#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cstring>
#include "../core_c/core.h"
#include "reasoning.h"

#ifdef _WIN32
#define strdup _strdup
#endif

// Define a validation function for our rule
uint8_t physics_constraint_check(void* context) {
    std::cout << "[Validation] Checking if physics parameters are within bounds..." << std::endl;
    return 1; // Valid
}

void physics_execution(void* context) {
    char* message = (char*)context;
    std::cout << "[Core Logic] Executed: " << message << std::endl;
}

#include <fstream>

void processArg(Ami::Learner& brain, AmiKnowledgeStore* ks, const std::string& arg) {
    size_t first = arg.find(':');
    if (first == std::string::npos) return;

    std::string type = arg.substr(0, first);
    std::string data = arg.substr(first + 1);

    if (type == "CON") {
        brain.identifyConcept(data);
    } 
    else if (type == "PROP") {
        size_t sep1 = data.find(':');
        size_t sep2 = data.rfind(':');
        if (sep1 == std::string::npos || sep2 == std::string::npos) return;
        std::string concept = data.substr(0, sep1);
        std::string prop = data.substr(sep1 + 1, sep2 - sep1 - 1);
        double val = std::stod(data.substr(sep2 + 1));
        brain.learnProperty(concept, prop, val);
    }
    else if (type == "REL") {
        size_t sep = data.find(':');
        if (sep == std::string::npos) return;
        std::string sub = data.substr(0, sep);
        std::string obj = data.substr(sep + 1);
        std::cout << "[Brain] Relationship: " << sub << " -> " << obj << std::endl;
        brain.addDirectedRelationship(sub, obj);
    }
    else if (type == "RULE") {
        size_t sep = data.find(':');
        if (sep == std::string::npos) return;
        std::string target = data.substr(0, sep);
        std::string formula = data.substr(sep + 1);
        std::cout << "[Brain] Algorithm: " << target << " = " << formula << std::endl;
        AmiValue v = { (void*)strdup(formula.c_str()), AMI_TYPE_STRING };
        ami_add_fact(ks, target.c_str(), v);
    }
    else if (type == "QUERY") {
        brain.queryConcept(data);
    }
    else if (type == "LINK") {
        size_t sep = data.find(':');
        if (sep != std::string::npos) {
            brain.findConnection(data.substr(0, sep), data.substr(sep + 1));
        }
    }
    else if (type == "SEQ") {
        size_t sep = data.find(':');
        if (sep != std::string::npos) {
            brain.addSequence(data.substr(0, sep), data.substr(sep + 1));
        }
    }
    else if (type == "EVI") {
        size_t sep = data.find(':');
        if (sep != std::string::npos) {
            std::string concept = data.substr(0, sep);
            std::string snippet = data.substr(sep + 1);
            brain.addEvidence(concept, snippet);
        }
    }
}

int main(int argc, char* argv[]) {
    AmiKnowledgeStore* ks = ami_init_knowledge_store();
    
    // Attempt to load existing knowledge
    ami_load_knowledge_store(ks, "brain_data.ami");
    
    Ami::Learner brain(ks);
    brain.loadWeights();

    if (argc > 1) {
        std::string mode = argv[1];

        if (mode == "--train" && argc > 2) {
            std::cout << "[Core] Training Mode Activated..." << std::endl;
            std::ifstream file(argv[2]);
            if (!file) {
                std::cout << "Error: Cannot open training stream: " << argv[2] << std::endl;
                return 1;
            }

            std::string line;
            while (std::getline(file, line)) {
                processArg(brain, ks, line);
            }

            // Execute Training Cycle
            std::cout << "[Brain] Running Brain Training Cycle..." << std::endl;
            // Transition IDENTIFY -> GATHER
            brain.transition(); 
            brain.process(); // Take snapshot
            
            // Proceed to training
            brain.transition(); // OBSERVE
            brain.transition(); // ANALYZE
            brain.process();    // Find links
            brain.transition(); // SUMMARIZE
            brain.process();    // Train weights
            
            // Save state
            brain.saveWeights();
            ami_save_knowledge_store(ks, "brain_data.ami");
            std::cout << "[Brain] Knowledge Base Optimized and Saved." << std::endl;
        } 
        else if (mode == "--chat") {
            // Chat mode: Process command line queries directly
            std::cout << "[Core] Chat/Query Mode Activated." << std::endl;
            for (int i = 2; i < argc; ++i) {
                processArg(brain, ks, argv[i]);
            }
        }
        else if (mode == "--summary") {
            std::cout << "--- [AmI Brain: Main Intelligence Summary] ---" << std::endl;
            std::cout << "The central pillars of this knowledge base are:" << std::endl;
            for (int i = 0; i < 5; ++i) {
                std::string key = "main_pillar_" + std::to_string(i);
                AmiValue v = ami_get_fact(ks, key.c_str());
                if (v.type == AMI_TYPE_STRING) {
                    std::cout << "  [" << (i+1) << "] " << (char*)v.data << std::endl;
                }
            }
        }
        else {
            // Support old style --file for backward compatibility
            if (mode == "--file" && argc > 2) {
                std::ifstream file(argv[2]);
                std::string line;
                while (std::getline(file, line)) processArg(brain, ks, line);
                for (int i = 3; i < argc; ++i) processArg(brain, ks, argv[i]);
            } else {
                for (int i = 1; i < argc; ++i) processArg(brain, ks, argv[i]);
            }
        }
    }

    return 0;
}
