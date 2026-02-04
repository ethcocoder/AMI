#include "core.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_FACTS 2000000
#define HASH_SIZE 1000003 // Large prime for hash table

typedef struct Fact {
    char* key;
    AmiValue value;
    struct Fact* next; // For collision chaining
} Fact;

struct AmiKnowledgeStore {
    Fact* table[HASH_SIZE]; // Hash index
    Fact facts[MAX_FACTS];  // Linear storage for iteration
    size_t count;
};

// Simple DJB2 Hash Algorithm
static unsigned long hash_key(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash % HASH_SIZE;
}

AmiKnowledgeStore* ami_init_knowledge_store() {
    AmiKnowledgeStore* ks = (AmiKnowledgeStore*)ami_malloc(sizeof(AmiKnowledgeStore));
    ks->count = 0;
    for (int i = 0; i < HASH_SIZE; i++) ks->table[i] = NULL;
    return ks;
}

void ami_add_fact(AmiKnowledgeStore* ks, const char* key, AmiValue value) {
    if (ks->count < MAX_FACTS) {
        // Linear Store
        ks->facts[ks->count].key = strdup(key);
        ks->facts[ks->count].value = value;
        
        // Hash Table Store (O(1))
        unsigned long idx = hash_key(key);
        Fact* new_f = &ks->facts[ks->count];
        new_f->next = ks->table[idx];
        ks->table[idx] = new_f;
        
        ks->count++;
    }
}

AmiValue ami_get_fact(AmiKnowledgeStore* ks, const char* key) {
    unsigned long idx = hash_key(key);
    Fact* curr = ks->table[idx];
    while (curr) {
        if (strcmp(curr->key, key) == 0) {
            return curr->value;
        }
        curr = curr->next;
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
    // printf("[Core] Knowledge Store saved to %s\n", filename);
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
    // printf("[Core] Knowledge Store loaded from %s\n", filename);
}

size_t ami_get_fact_count(AmiKnowledgeStore* ks) {
    return ks->count;
}

const char* ami_get_fact_key(AmiKnowledgeStore* ks, size_t index) {
    if (index < ks->count) return ks->facts[index].key;
    return NULL;
}

AmiValue ami_get_fact_value(AmiKnowledgeStore* ks, size_t index) {
    if (index < ks->count) return ks->facts[index].value;
    AmiValue none = { .data = NULL, .type = AMI_TYPE_NONE };
    return none;
}


void ami_clear_knowledge_store(AmiKnowledgeStore* ks) {
    if (!ks) return;
    for (size_t i = 0; i < ks->count; i++) {
        free(ks->facts[i].key);
        if (ks->facts[i].value.data) {
            free(ks->facts[i].value.data);
        }
    }
    ks->count = 0;
    for (int i = 0; i < HASH_SIZE; i++) ks->table[i] = NULL;
}

