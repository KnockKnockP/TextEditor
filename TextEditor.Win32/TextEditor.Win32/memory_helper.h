#ifndef TEXTEDITOR_MEMORY_HELPER_H
#define TEXTEDITOR_MEMORY_HELPER_H

#include <stddef.h>

namespace TextEditor {
namespace memory {

void *AllocateBytes(size_t bytes);
void *ReallocateBytes(void *memory, size_t bytes);
void Free(void *memory);

template <typename T>
T *Allocate(size_t count) {
    return static_cast<T *>(AllocateBytes(sizeof(T) * count));
}

template <typename T>
T *Reallocate(T *memory, size_t count) {
    return static_cast<T *>(ReallocateBytes(memory, sizeof(T) * count));
}

}  // namespace memory
}  // namespace TextEditor

void *operator new(size_t size);
void *operator new[](size_t size);
void operator delete(void *memory);
void operator delete[](void *memory);

#endif
