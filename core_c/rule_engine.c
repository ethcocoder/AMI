#include "core.h"
#include <stdio.h>

void ami_execute_rule(AmiRule* rule, void* context) {
    if (rule && rule->execute) {
        printf("Executing rule: %s (ID: %u)\n", rule->name, rule->id);
        rule->execute(context);
    } else {
        fprintf(stderr, "Error: Invalid rule or missing execution logic.\n");
    }
}
