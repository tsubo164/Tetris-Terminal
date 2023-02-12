#ifndef FIELD_H
#define FIELD_H

#include "point.h"
#include <cstdint>
#include <array>

constexpr int FIELD_WIDTH = 10;
constexpr int FIELD_HEIGHT = 20;

class Field {
public:
    Field();
    ~Field();

    int GetFieldCellKind(Point pos);
    void SetFieldCellKind(Point pos, int kind);

    int GetClearedLineCount();
    void GetClearedLines(int *cleared_line_y);

    void ClearLines();

private:
    struct Line {
        std::array<int8_t, FIELD_WIDTH> elem {0};
        bool is_cleared = false;
        int8_t count = 0;

        Line () {}
        const int8_t operator[](int i) const { return elem[i]; }
        int8_t &operator[](int i) { return elem[i]; }

        bool IsFilled() const { return count == FIELD_WIDTH; }
        void MarkCleared() { is_cleared = true; }
    };

    std::array<Line, FIELD_HEIGHT> lines;
    int cleared_line_count = 0;
};

#endif
