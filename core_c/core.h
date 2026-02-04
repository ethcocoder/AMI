#ifndef AMI_CORE_H
#define AMI_CORE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    AMI_TYPE_INT,
    AMI_TYPE_FLOAT,
    AMI_TYPE_DOUBLE,
    AMI_TYPE_STRING,
    AMI_TYPE_POINTER,
    AMI_TYPE_NONE
} AmiDataType;

typedef struct {
    void* data;
    AmiDataType type;
} AmiValue;

/**
 * @brief Represents a rule in the AmI system.
 */
typedef struct {
    uint32_t id;
    const char* name;
    void (*execute)(void* context);
    uint8_t (*validate)(void* context); // Constraint checking
    uint8_t priority;
} AmiRule;

/**
 * @brief Memory management wrappers
 */
void* ami_malloc(size_t size);
void ami_free(void* ptr);

/**
 * @brief Knowledge Store interface
 */
typedef struct AmiKnowledgeStore AmiKnowledgeStore;

AmiKnowledgeStore* ami_init_knowledge_store();
void ami_add_fact(AmiKnowledgeStore* ks, const char* key, AmiValue value);
AmiValue ami_get_fact(AmiKnowledgeStore* ks, const char* key);
void ami_save_knowledge_store(AmiKnowledgeStore* ks, const char* filename);
void ami_load_knowledge_store(AmiKnowledgeStore* ks, const char* filename);
void ami_clear_knowledge_store(AmiKnowledgeStore* ks);
// Iteration
size_t ami_get_fact_count(AmiKnowledgeStore* ks);
const char* ami_get_fact_key(AmiKnowledgeStore* ks, size_t index);
AmiValue ami_get_fact_value(AmiKnowledgeStore* ks, size_t index);

/**
 * @brief Rule Engine interface
 */
void ami_execute_rule(AmiRule* rule, void* context);
uint8_t ami_check_constraints(AmiRule* rule, void* context);

#ifdef __cplusplus
}
#endif

#endif // AMI_CORE_H
