#include <memory_helper.h>
#include <stdlib.h>
#include <stddef.h>

void MEMORY_HELPER_free(void *pMemory) {
    if (pMemory) {
        free(pMemory);
        pMemory = NULL;
    }
}