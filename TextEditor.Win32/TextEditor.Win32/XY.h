#ifndef TEXTEDITOR_POINT_H
#define TEXTEDITOR_POINT_H

namespace TextEditor {
    struct Point {
        int x;
        int y;

        Point()
            : x(0),
              y(0) {
        }

        Point(int x_, int y_)
            : x(x_),
              y(y_) {
        }

        bool operator==(const Point &other) const;
    };
}
#endif
