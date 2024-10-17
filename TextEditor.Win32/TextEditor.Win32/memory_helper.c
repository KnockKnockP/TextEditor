#include <memory_helper.h>
#include <stdlib.h>
#include <stddef.h>

void MEMORY_HELPER_free(void **ppMemory) {
    if (*ppMemory) {
        free(*ppMemory);
        *ppMemory = NULL;
    }
}