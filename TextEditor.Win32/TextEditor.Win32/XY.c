#include <leak_checker.h>

#include <XY.h>

BOOL XY_equals(const XY *p1, const XY *p2) {
	return (p1->x == p2->x && p1->y == p2->y);
}