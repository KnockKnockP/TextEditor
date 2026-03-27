#include <leak_checker.h>

#include <XY.h>

namespace TextEditor {
    bool Point::operator==(const Point &other) const {
        return x == other.x && y == other.y;
    }
}