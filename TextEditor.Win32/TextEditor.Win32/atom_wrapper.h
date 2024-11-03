#ifndef ATOM_WRAPPER_H
#define ATOM_WRAPPER_H

#include <Windows.h>
#include <string_utilities.h>

typedef struct _ATOM_WRAPPER {
    ATOM atom;
} ATOM_WRAPPER;

void ATOM_WRAPPER_initialize(ATOM_WRAPPER *pAtom_wrapper, WIDE_STRING *pName, const UINT style, const WNDPROC callback);
#endif