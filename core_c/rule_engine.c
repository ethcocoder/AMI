#include "core.h"
#include <stdio.h>

void ami_execute_rule(AmiRule* rule, void* context) {
    if (rule && rule->execute) {
        // First check constraints
        if (ami_check_constraints(rule, context)) {
            printf("[Core] Executing rule: %s\n", rule->name);
            rule->execute(context);
        } else {
            fprintf(stderr, "[Core] Constraint check FAILED for rule: %s\n", rule->name);
        }
    } else {
        fprintf(stderr, "[Core] Error: Invalid rule or missing execution logic.\n");
    }
}

uint8_t ami_check_constraints(AmiRule* rule, void* context) {
    if (rule && rule->validate) {
        return rule->validate(context);
    }
    // If no validation function, assume it's valid (deterministic trust)
    return 1;
}
