#include "vector_ops.h"
#include <stdlib.h>
#include <math.h>

SparseVector* create_vector(size_t capacity) {
    SparseVector* vec = (SparseVector*)malloc(sizeof(SparseVector));
    vec->indices = (int*)malloc(capacity * sizeof(int));
    vec->values = (float*)malloc(capacity * sizeof(float));
    vec->size = 0;
    vec->capacity = capacity;
    return vec;
}

void free_vector(SparseVector* vec) {
    if (vec) {
        free(vec->indices);
        free(vec->values);
        free(vec);
    }
}

void vector_add(SparseVector* vec, int index, float value) {
    if (vec->size >= vec->capacity) {
        vec->capacity *= 2;
        vec->indices = (int*)realloc(vec->indices, vec->capacity * sizeof(int));
        vec->values = (float*)realloc(vec->values, vec->capacity * sizeof(float));
    }
    vec->indices[vec->size] = index;
    vec->values[vec->size] = value;
    vec->size++;
}

float vector_dot(const SparseVector* a, const SparseVector* b) {
    float dot = 0.0f;
    size_t i = 0, j = 0;
    
    // Assumes indices are sorted ascending for O(N+M) complexity
    while (i < a->size && j < b->size) {
        if (a->indices[i] < b->indices[j]) {
            i++;
        } else if (a->indices[i] > b->indices[j]) {
            j++;
        } else {
            // Match found
            dot += a->values[i] * b->values[j];
            i++;
            j++;
        }
    }
    return dot;
}

float vector_norm(const SparseVector* v) {
    float sum_sq = 0.0f;
    for (size_t i = 0; i < v->size; i++) {
        sum_sq += v->values[i] * v->values[i];
    }
    return sqrtf(sum_sq);
}

float vector_cosine_similarity(const SparseVector* a, const SparseVector* b) {
    float dot = vector_dot(a, b);
    float norm_a = vector_norm(a);
    float norm_b = vector_norm(b);
    
    if (norm_a == 0.0f || norm_b == 0.0f) return 0.0f;
    return dot / (norm_a * norm_b);
}
