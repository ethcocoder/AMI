#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "../core_c/core.h"
#include "reasoning.h"

void sample_rule_action(void* context) {
    char* message = (char*)context;
    std::cout << "[Core Action] " << message << std::endl;
}

int main() {
    std::cout << "========== AmI Deep Intelligence System v0.2 ==========" << std::endl;

    // 1. Initialize C Core Knowledge Store
    AmiKnowledgeStore* ks = ami_init_knowledge_store();
    ami_add_fact(ks, "mode", (void*)"Deep Reasoning");
    
    // 2. Initialize C++ Learner
    Ami::Learner brain(ks);

    // 3. IDENTIFY: Discovering Concepts
    brain.process(); // State: IDENTIFY
    brain.identifyConcept("Mass");
    brain.identifyConcept("Acceleration");
    brain.transition();

    // 4. GATHER & OBSERVE
    brain.process(); // GATHER
    brain.transition();
    brain.process(); // OBSERVE
    brain.transition();

    // 5. ANALYZE: Relationship Extraction
    brain.process(); // ANALYZE (Analyzes relationships between Mass and Acceleration)
    brain.transition();

    // 6. SUMMARIZE: Algorithm Synthesis
    brain.process(); // SUMMARIZE (Synthesizes F=ma)
    brain.transition();

    // 7. APPLY: Rule Execution
    brain.process(); // APPLY
    AmiRule force_rule = {
        .id = 201,
        .name = "Synthesized Force Calculation",
        .execute = sample_rule_action,
        .priority = 10
    };
    
    if (Ami::Validator::validateRule(force_rule)) {
        ami_execute_rule(&force_rule, (void*)"Force calculated as 98.1N for Mass 10kg.");
    }
    brain.transition();

    // 8. REVIEW
    brain.process(); // REVIEW
    
    std::cout << "=======================================================" << std::endl;
    return 0;
}
