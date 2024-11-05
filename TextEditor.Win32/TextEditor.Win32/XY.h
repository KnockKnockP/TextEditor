#ifndef XY_H
#define XY_H

#include <Windows.h>

typedef struct _XY {
    int x, y;
} XY;

BOOL XY_equals(const XY *p1, const XY *p2);
#endif