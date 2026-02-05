#ifndef VECTOR_OPS_H
#define VECTOR_OPS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Sparse Vector Structure
// optimized for high-dimensional but sparse data (TF-IDF)
typedef struct {
    int* indices;       // Array of vocabulary indices
    float* values;      // Array of TF-IDF values
    size_t size;        // Number of non-zero elements
    size_t capacity;    // Allocated capacity
} SparseVector;

// Management
SparseVector* create_vector(size_t capacity);
void free_vector(SparseVector* vec);
void vector_add(SparseVector* vec, int index, float value);

// Math Operations
float vector_dot(const SparseVector* a, const SparseVector* b);
float vector_norm(const SparseVector* v);
float vector_cosine_similarity(const SparseVector* a, const SparseVector* b);

#ifdef __cplusplus
}
#endif

#endif // VECTOR_OPS_H
