#include "core.h"
#include <string.h>
#include <stdlib.h>

#define MAX_FACTS 1024

typedef struct Fact {
    char* key;
    void* value;
} Fact;

struct AmiKnowledgeStore {
    Fact facts[MAX_FACTS];
    size_t count;
};

AmiKnowledgeStore* ami_init_knowledge_store() {
    AmiKnowledgeStore* ks = (AmiKnowledgeStore*)ami_malloc(sizeof(AmiKnowledgeStore));
    ks->count = 0;
    return ks;
}

void ami_add_fact(AmiKnowledgeStore* ks, const char* key, void* value) {
    if (ks->count < MAX_FACTS) {
        ks->facts[ks->count].key = _strdup(key);
        ks->facts[ks->count].value = value;
        ks->count++;
    }
}

void* ami_get_fact(AmiKnowledgeStore* ks, const char* key) {
    for (size_t i = 0; i < ks->count; i++) {
        if (strcmp(ks->facts[i].key, key) == 0) {
            return ks->facts[i].value;
        }
    }
    return NULL;
}
