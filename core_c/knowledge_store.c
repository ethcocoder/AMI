#include "core.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_FACTS 1000000

typedef struct Fact {
    char* key;
    AmiValue value;
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

void ami_add_fact(AmiKnowledgeStore* ks, const char* key, AmiValue value) {
    if (ks->count < MAX_FACTS) {
        ks->facts[ks->count].key = strdup(key);
        ks->facts[ks->count].value = value;
        ks->count++;
    }
}

AmiValue ami_get_fact(AmiKnowledgeStore* ks, const char* key) {
    for (size_t i = 0; i < ks->count; i++) {
        if (strcmp(ks->facts[i].key, key) == 0) {
            return ks->facts[i].value;
        }
    }
    AmiValue none = { .data = NULL, .type = AMI_TYPE_NONE };
    return none;
}

void ami_save_knowledge_store(AmiKnowledgeStore* ks, const char* filename) {
    FILE* f = fopen(filename, "wb");
    if (!f) return;

    fwrite(&ks->count, sizeof(size_t), 1, f);
    for (size_t i = 0; i < ks->count; i++) {
        size_t key_len = strlen(ks->facts[i].key);
        fwrite(&key_len, sizeof(size_t), 1, f);
        fwrite(ks->facts[i].key, 1, key_len, f);
        
        fwrite(&ks->facts[i].value.type, sizeof(AmiDataType), 1, f);
        if (ks->facts[i].value.type == AMI_TYPE_STRING) {
            size_t val_len = strlen((char*)ks->facts[i].value.data);
            fwrite(&val_len, sizeof(size_t), 1, f);
            fwrite(ks->facts[i].value.data, 1, val_len, f);
        } else if (ks->facts[i].value.type == AMI_TYPE_DOUBLE) {
            fwrite(ks->facts[i].value.data, sizeof(double), 1, f);
        } else {
            uint64_t dummy = 0; 
            fwrite(&dummy, sizeof(uint64_t), 1, f);
        }
    }
    fclose(f);
    printf("[Core] Knowledge Store saved to %s\n", filename);
}

void ami_load_knowledge_store(AmiKnowledgeStore* ks, const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return;

    size_t new_count;
    if (fread(&new_count, sizeof(size_t), 1, f) != 1) {
        fclose(f);
        return;
    }

    for (size_t i = 0; i < new_count; i++) {
        size_t key_len;
        fread(&key_len, sizeof(size_t), 1, f);
        char* key = (char*)ami_malloc(key_len + 1);
        fread(key, 1, key_len, f);
        key[key_len] = '\0';

        AmiDataType type;
        fread(&type, sizeof(AmiDataType), 1, f);

        AmiValue val;
        val.type = type;
        if (type == AMI_TYPE_STRING) {
            size_t val_len;
            fread(&val_len, sizeof(size_t), 1, f);
            val.data = ami_malloc(val_len + 1);
            fread(val.data, 1, val_len, f);
            ((char*)val.data)[val_len] = '\0';
        } else if (type == AMI_TYPE_DOUBLE) {
            val.data = ami_malloc(sizeof(double));
            fread(val.data, sizeof(double), 1, f);
        } else {
            uint64_t dummy;
            fread(&dummy, sizeof(uint64_t), 1, f);
            val.data = NULL;
        }
        ami_add_fact(ks, key, val);
        free(key); // ami_add_fact uses strdup
    }
    fclose(f);
    printf("[Core] Knowledge Store loaded from %s\n", filename);
}

size_t ami_get_fact_count(AmiKnowledgeStore* ks) {
    return ks->count;
}

const char* ami_get_fact_key(AmiKnowledgeStore* ks, size_t index) {
    if (index < ks->count) return ks->facts[index].key;
    return NULL;
}
