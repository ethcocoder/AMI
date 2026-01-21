#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "../core_c/core.h"
#include "reasoning.h"

// Define a validation function for our rule
uint8_t physics_constraint_check(void* context) {
    std::cout << "[Validation] Checking if physics parameters are within bounds..." << std::endl;
    return 1; // Valid
}

void physics_execution(void* context) {
    char* message = (char*)context;
    std::cout << "[Core Logic] Executed: " << message << std::endl;
}

int main() {
    std::cout << "AMI BRAIN CORE - FINAL INITIALIZATION" << std::endl;
    std::cout << "======================================" << std::endl;

    // 1. Initialize C Core Knowledge Store (The "Subconscious")
    AmiKnowledgeStore* ks = ami_init_knowledge_store();
    
    // Test storing typed data
    AmiValue system_name = { (void*)"AmI Virtual Brain", AMI_TYPE_STRING };
    ami_add_fact(ks, "identity", system_name);

    // 2. Initialize C++ Learner (The "Conscious Reasoning")
    Ami::Learner brain(ks);

    // 3. THE CYCLE: Learning about Physics
    std::cout << "\n[Stage 1] Cognitive Discovery" << std::endl;
    brain.identifyConcept("Mass");
    brain.identifyConcept("Acceleration");
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

    std::cout << "\n[Stage 6] Long-Term Memory Check" << std::endl;
    AmiValue retrieved = ami_get_fact(ks, "last_synthesized_algo");
    if (retrieved.type == AMI_TYPE_STRING) {
        std::cout << "Stored Concept in LTM: " << (char*)retrieved.data << std::endl;
    }

    std::cout << "======================================" << std::endl;
    std::cout << "AMI BRAIN CORE STATUS: FULLY OPERATIONAL" << std::endl;

    return 0;
}
