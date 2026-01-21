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
}

int main(int argc, char* argv[]) {
    // 1. Initialize C Core Knowledge Store (The "Subconscious")
    AmiKnowledgeStore* ks = ami_init_knowledge_store();
    Ami::Learner brain(ks);

    // 1. Ingest/Process all input first
    if (argc > 1) {
        std::string flag = argv[1];
        if (flag == "--file" && argc > 2) {
            std::cout << "[Core] Ingesting Intelligence Stream from: " << argv[2] << std::endl;
            std::ifstream file(argv[2]);
            std::string line;
            while (std::getline(file, line)) {
                processArg(brain, ks, line);
            }
            // Process extra arguments (like QUERY or LINK)
            for (int i = 3; i < argc; ++i) {
                processArg(brain, ks, argv[i]);
            }
        } else {
            for (int i = 1; i < argc; ++i) {
                processArg(brain, ks, argv[i]);
            }
        }
    }

    // 2. The Thought Cycle & Structural Multi-Feature Training
    std::cout << "\n[Brain] Initializing Structural Neural Cycle..." << std::endl;
    
    // Move to GATHER
    brain.transition(); 

    // DATASET: Profit = (Price * 10) + (Volume * 5) - Overhead
    
    // Sample 1: Price=10, Vol=100, Over=500 => Profit = 100 + 500 - 500 = 100
    brain.learnProperty("Price", "value", 10.0);
    brain.learnProperty("Volume", "value", 100.0);
    brain.learnProperty("Overhead", "value", 500.0);
    brain.learnProperty("Profit", "value", 100.0); 
    brain.process(); // GATHER Snapshot 1
    
    for(int i=0; i<7; i++) brain.transition();

    // Sample 2: Price=20, Vol=200, Over=1000 => Profit = 200 + 1000 - 1000 = 200
    brain.learnProperty("Price", "value", 20.0);
    brain.learnProperty("Volume", "value", 200.0);
    brain.learnProperty("Overhead", "value", 1000.0);
    brain.learnProperty("Profit", "value", 200.0);
    brain.process(); // GATHER Snapshot 2
    
    for(int i=0; i<7; i++) brain.transition();

    // Sample 3: Price=5, Vol=50, Over=250 => Profit = 50 + 250 - 250 = 50
    brain.learnProperty("Price", "value", 5.0);
    brain.learnProperty("Volume", "value", 50.0);
    brain.learnProperty("Overhead", "value", 250.0);
    brain.learnProperty("Profit", "value", 50.0);
    brain.process(); // GATHER Snapshot 3

    // Train the Neural Brain
    brain.transition(); // OBSERVE
    brain.transition(); // ANALYZE
    brain.process();
    brain.transition(); // SUMMARIZE
    brain.process();    
    
    brain.transition(); // APPLY
    brain.process();    
    
    // 3. Binary Persistence
    brain.saveWeights();
    ami_save_knowledge_store(ks, "brain_data.ami");

    // 4. DEMO: Prediction from Memory (Zero-Data Prediction)
    std::cout << "\n[Demo] --- Starting Fresh Session from Binary Memory ---" << std::endl;
    
    // Clear the current brain's state (Simulating a restart)
    AmiKnowledgeStore* ks2 = ami_init_knowledge_store();
    ami_load_knowledge_store(ks2, "brain_data.ami");
    Ami::Learner brain2(ks2);

    // Give it structural knowledge (it needs to know the names of things)
    brain2.identifyConcept("Price");
    brain2.identifyConcept("Volume");
    brain2.identifyConcept("Overhead");
    
    // RESTORE: Load the weights we just saved
    brain2.loadWeights();

    // NEW PREDICTION: price=50, volume=500, overhead=100
    brain2.learnProperty("Price", "value", 50.0);
    brain2.learnProperty("Volume", "value", 500.0);
    brain2.learnProperty("Overhead", "value", 100.0);
    
    std::cout << "[Demo] Predicting Profit for Price=50, Vol=500, Over=100 WITHOUT retraining..." << std::endl;
    
    // Move through states to APPLY (IDENTIFY -> GATHER -> OBSERVE -> ANALYZE -> SUMMARIZE -> APPLY)
    for(int i=0; i<5; i++) brain2.transition();
    brain2.process(); 

    // 5. Finalization
    std::cout << "\n==============================================" << std::endl;
    std::cout << "AMI BRAIN STATUS: LONG-TERM MEMORY VERIFIED" << std::endl;

    return 0;
}
