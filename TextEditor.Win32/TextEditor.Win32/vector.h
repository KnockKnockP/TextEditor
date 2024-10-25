#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>
#include <Windows.h>

#ifndef VECTOR_NAME
#define VECTOR_NAME(type) VECTOR_##type
#endif

#ifndef VECTOR_DECLARATION
#define VECTOR_DECLARATION(type) \
typedef struct VECTOR_NAME(type) { \
    size_t size; \
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
VECTOR_POP_DECLARATION(type); \
VECTOR_CLEAR_DECLARATION(type); \
VECTOR_DESTROY_DECLARATION(type);
#endif

VECTOR_DECLARE_ALL(LPWSTR)
#endif