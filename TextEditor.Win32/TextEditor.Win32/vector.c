#include <vector.h>
#include <memory_helper.h>

#ifndef VECTOR_CREATE_IMPLEMENTATION
#define VECTOR_CREATE_IMPLEMENTATION(type) \
VECTOR_NAME(type) VECTOR_CREATE_##type(void) { \
    VECTOR_NAME(type) vector = { 0 }; \
    VECTOR_INITIALIZE_##type(&vector); \
    return vector; \
}
#endif

#ifndef VECTOR_INITIALIZE_IMPLEMENTATION
#define VECTOR_INITIALIZE_IMPLEMENTATION(type) \
void VECTOR_INITIALIZE_##type(VECTOR_NAME(type) *pVector) { \
    pVector->size = 0; \
    pVector->pArray = NULL; \
}
#endif

#ifndef VECTOR_PUSH_IMPLEMENTATION
#define VECTOR_PUSH_IMPLEMENTATION(type) \
void VECTOR_PUSH_##type(VECTOR_NAME(type) *pVector, const type element) { \
    const size_t new_bytes = sizeof(type) * (pVector->size + 1); \
    type *pTemporary = NULL; \
    if (!pVector->pArray) { \
        pTemporary = malloc(new_bytes); \
        if (!pTemporary) { \
            return; \
        } \
    } else { \
        pTemporary = realloc(pVector->pArray, new_bytes); \
        if (!pTemporary) { \
            return; \
        } \
    } \
    pVector->pArray = pTemporary; \
    pVector->pArray[pVector->size++] = element; \
}
#endif

#ifndef VECTOR_POP_IMPLEMENTATION
#define VECTOR_POP_IMPLEMENTATION(type) \
void VECTOR_POP_##type(VECTOR_NAME(type) *pVector) { \
    if (!pVector->size || !pVector->pArray) { \
        return; \
    } \
    if (pVector->size == 1) { \
        VECTOR_DESTROY_##type(pVector); \
        return; \
    } \
    const size_t new_bytes = sizeof(type) * (pVector->size - 1); \
    type *pTemporary = realloc(pVector->pArray, new_bytes); \
    if (!pTemporary) { \
        return; \
    } \
    --pVector->size; \
    pVector->pArray = pTemporary; \
}
#endif

#ifndef VECTOR_CLEAR_IMPLEMENTATION
#define VECTOR_CLEAR_IMPLEMENTATION(type) \
void VECTOR_CLEAR_##type(VECTOR_NAME(type) *pVector) { \
    VECTOR_DESTROY_##type(pVector); \
}
#endif

#ifndef VECTOR_DESTROY_IMPLEMENTATION
#define VECTOR_DESTROY_IMPLEMENTATION(type) \
void VECTOR_DESTROY_##type(VECTOR_NAME(type) *pVector) { \
    pVector->size = 0; \
    MEMORY_HELPER_free((void **)&pVector->pArray); \
}
#endif

#ifndef VECTOR_IMPLEMENTATION
#define VECTOR_IMPLEMENTATION(type) \
VECTOR_CREATE_IMPLEMENTATION(type) \
VECTOR_INITIALIZE_IMPLEMENTATION(type) \
VECTOR_PUSH_IMPLEMENTATION(type) \
VECTOR_POP_IMPLEMENTATION(type) \
VECTOR_CLEAR_IMPLEMENTATION(type) \
VECTOR_DESTROY_IMPLEMENTATION(type)
#endif

VECTOR_IMPLEMENTATION(LPWSTR)