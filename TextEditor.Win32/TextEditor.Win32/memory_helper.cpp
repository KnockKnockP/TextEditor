#include <leak_checker.h>

#include <memory_helper.h>

#include <signal.h>
#include <stdlib.h>

namespace TextEditor {
namespace memory {

namespace {

void *NormalizeAndAllocate(size_t bytes) {
    if (!bytes) {
        bytes = 1;
    }

    void *memory = malloc(bytes);
    if (!memory) {
        raise(SIGABRT);
    }

    return memory;
}

}  // namespace

void *AllocateBytes(size_t bytes) {
    return NormalizeAndAllocate(bytes);
}

void *ReallocateBytes(void *memory, size_t bytes) {
    if (!bytes) {
        bytes = 1;
    }

    void *reallocated = realloc(memory, bytes);
    if (!reallocated) {
        raise(SIGABRT);
    }

    return reallocated;
}

void Free(void *memory) {
    free(memory);
}

}  // namespace memory
}  // namespace TextEditor

void *operator new(size_t size) {
    return TextEditor::memory::AllocateBytes(size);
}

void *operator new[](size_t size) {
    return TextEditor::memory::AllocateBytes(size);
}

void operator delete(void *memory) {
    TextEditor::memory::Free(memory);
}

void operator delete[](void *memory) {
    TextEditor::memory::Free(memory);
}
