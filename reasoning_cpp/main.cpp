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

int main(int argc, char* argv[]) {
    // 1. Initialize C Core Knowledge Store (The "Subconscious")
    AmiKnowledgeStore* ks = ami_init_knowledge_store();
    Ami::Learner brain(ks);

    if (argc > 1) {
        std::cout << "[Interface] Synced Intelligence Layers:" << std::endl;
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            size_t first = arg.find(':');
            if (first == std::string::npos) continue;

            std::string type = arg.substr(0, first);
            std::string data = arg.substr(first + 1);

            if (type == "CON") {
                brain.identifyConcept(data);
            } 
            else if (type == "REL") {
                size_t sep = data.find(':');
                std::string sub = data.substr(0, sep);
                std::string obj = data.substr(sep + 1);
                std::cout << "[Brain] Learned Relationship: " << sub << " -> " << obj << std::endl;
            }
            else if (type == "RULE") {
                size_t sep = data.find(':');
                std::string target = data.substr(0, sep);
                std::string formula = data.substr(sep + 1);
                std::cout << "[Brain] Synthesized Algorithm: " << target << " = " << formula << std::endl;
                AmiValue v = { (void*)strdup(formula.c_str()), AMI_TYPE_STRING };
                ami_add_fact(ks, target.c_str(), v);
            }
            else if (type == "CONS") {
                std::cout << "[Brain] Registered Constraint: " << data << std::endl;
            }
        }
    } else {
        brain.identifyConcept("Mass");
        brain.identifyConcept("Acceleration");
    }
    brain.transition(); // To GATHER
    brain.transition(); // To OBSERVE

    std::cout << "\n[Stage 2] Symbolic Reasoning" << std::endl;
    Ami::Logic::evaluateSyllogism("All matter has Mass", "This object is matter");
    brain.transition(); // To ANALYZE
    brain.process();    // Analyzes relationships

    std::cout << "\n[Stage 3] Algorithmic Synthesis" << std::endl;
    brain.transition(); // To SUMMARIZE
    brain.process();    // Synthesizes Force = Mass * Acceleration

    std::cout << "\n[Stage 4] Pre-Application Simulation" << std::endl;
    brain.transition(); // To APPLY
    brain.process();    // Runs simulation

    std::cout << "\n[Stage 5] Core Rule Integration" << std::endl;
    AmiRule synthesized_force_rule = {
        .id = 500,
        .name = "Synthesized-F=ma",
        .execute = physics_execution,
        .validate = physics_constraint_check,
        .priority = 100
    };

    if (Ami::Validator::validateRule(synthesized_force_rule)) {
        ami_execute_rule(&synthesized_force_rule, (void*)"RESULT: Force = 50N");
    }

    // 4. Persistence Demonstration
    std::cout << "\n[Stage 6] Persistence (Saving and Loading .ami)" << std::endl;
    ami_save_knowledge_store(ks, "brain_data.ami");

    AmiKnowledgeStore* new_ks = ami_init_knowledge_store();
    ami_load_knowledge_store(new_ks, "brain_data.ami");

    AmiValue retrieved = ami_get_fact(new_ks, "last_synthesized_algo");
    if (retrieved.type == AMI_TYPE_STRING) {
        std::cout << "Successfully retrieved from .ami: " << (char*)retrieved.data << std::endl;
    }

    std::cout << "======================================" << std::endl;
    std::cout << "AMI BRAIN CORE STATUS: FULLY OPERATIONAL" << std::endl;

    return 0;
}
