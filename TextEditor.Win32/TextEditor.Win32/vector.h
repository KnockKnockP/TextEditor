#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>
#include <Windows.h>
#include <memory_helper.h>

#ifndef VECTOR_NAME
#define VECTOR_NAME(type) VECTOR_##type
#endif

#ifndef VECTOR_DECLARATION
#define VECTOR_DECLARATION(type) \
typedef struct VECTOR_NAME(type) { \
    int size; \
    type *pArray; \
} VECTOR_NAME(type)
#endif

#ifndef VECTOR_CREATE_DECLARATION
#define VECTOR_CREATE_DECLARATION(type) VECTOR_NAME(type) VECTOR_CREATE_##type(void)
#endif

#ifndef VECTOR_INITIALIZE_DECLARATION
#define VECTOR_INITIALIZE_DECLARATION(type) void VECTOR_INITIALIZE_##type(VECTOR_NAME(type) *pVector)
#endif

#ifndef VECTOR_PUSH_DECLARATION
#define VECTOR_PUSH_DECLARATION(type) void VECTOR_PUSH_##type(VECTOR_NAME(type) *pVector, const type element)
#endif

#ifndef VECTOR_FIND_DECLARATION
#define VECTOR_FIND_DECLARATION(type) int VECTOR_FIND_##type(VECTOR_NAME(type) *pVector, const type element)
#endif

#ifndef VECTOR_FIND_AND_REPLACE_DECLARATION
#define VECTOR_FIND_AND_REPLACE_DECLARATION(type) void VECTOR_FIND_AND_REPLACE_##type(VECTOR_NAME(type) *pVector, const type element, const type new_value)
#endif

#ifndef VECTOR_REMOVE_DECLARATION
#define VECTOR_REMOVE_DECLARATION(type) void VECTOR_REMOVE_##type(VECTOR_NAME(type) *pVector, const int index)
#endif

#ifndef VECTOR_POP_DECLARATION
#define VECTOR_POP_DECLARATION(type) void VECTOR_POP_##type(VECTOR_NAME(type) *pVector)
#endif

#ifndef VECTOR_CLEAR_DECLARATION
#define VECTOR_CLEAR_DECLARATION(type) void VECTOR_CLEAR_##type(VECTOR_NAME(type) *pVector)
#endif

#ifndef VECTOR_DESTROY_DECLARATION
#define VECTOR_DESTROY_DECLARATION(type) void VECTOR_DESTROY_##type(VECTOR_NAME(type) *pVector)
#endif

#ifndef VECTOR_DECLARE_ALL
#define VECTOR_DECLARE_ALL(type) \
VECTOR_DECLARATION(type); \
VECTOR_CREATE_DECLARATION(type); \
VECTOR_INITIALIZE_DECLARATION(type); \
VECTOR_PUSH_DECLARATION(type); \
VECTOR_FIND_DECLARATION(type); \
VECTOR_FIND_AND_REPLACE_DECLARATION(type); \
VECTOR_POP_DECLARATION(type); \
VECTOR_REMOVE_DECLARATION(type); \
VECTOR_CLEAR_DECLARATION(type); \
VECTOR_DESTROY_DECLARATION(type)
#endif

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

#ifndef VECTOR_FIND_IMPLEMENTATION
#define VECTOR_FIND_IMPLEMENTATION(type) int VECTOR_FIND_##type(VECTOR_NAME(type) *pVector, const type element) { \
    if (!pVector->pArray) { \
        return -1; \
    } \
    for (int i = 0; i < pVector->size; ++i) { \
        if (pVector->pArray[i] == element) { \
            return i; \
        } \
    } \
    return -1; \
}
#endif

#ifndef VECTOR_FIND_AND_REPLACE_IMPLEMENTATION
#define VECTOR_FIND_AND_REPLACE_IMPLEMENTATION(type) \
void VECTOR_FIND_AND_REPLACE_##type(VECTOR_NAME(type) *pVector, const type element, const type new_value) { \
    if (!pVector->pArray) { \
        return; \
    } \
    for (int i = 0; i < pVector->size; ++i) { \
        if (pVector->pArray[i] == element) { \
            pVector->pArray[i] = new_value; \
            break; \
        } \
    } \
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

#ifndef VECTOR_REMOVE_IMPLEMENTATION
#define VECTOR_REMOVE_IMPLEMENTATION(type) \
void VECTOR_REMOVE_##type(VECTOR_NAME(type) *pVector, const int index) { \
    if (!pVector->pArray) { \
        return; \
    } \
    type *pTemporary = malloc(sizeof(type) * (pVector->size - 1)); \
    if (!pTemporary) { \
        return; \
    } \
    for (int i = 0; i < index; ++i) { \
        pTemporary[i] = pVector->pArray[i]; \
    } \
    for (int i = index + 1; i < pVector->size; ++i) { \
        pTemporary[i] = pVector->pArray[i]; \
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
VECTOR_FIND_IMPLEMENTATION(type) \
VECTOR_FIND_AND_REPLACE_IMPLEMENTATION(type) \
VECTOR_POP_IMPLEMENTATION(type) \
VECTOR_REMOVE_IMPLEMENTATION(type) \
VECTOR_CLEAR_IMPLEMENTATION(type) \
VECTOR_DESTROY_IMPLEMENTATION(type)
#endif

VECTOR_DECLARE_ALL(LPWSTR);
#endif