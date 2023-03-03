#ifndef FIELD_H
#define FIELD_H

#include "point.h"
#include "piece.h"
#include <cstdint>
#include <array>

constexpr int FIELD_WIDTH = 10;
constexpr int FIELD_HEIGHT = 20;

class Field {
public:
    Field();
    ~Field();

    void Clear();
    bool IsEmpty() const;

    // Cell
    int GetCellKind(Point pos) const;
    void SetCellKind(Point pos, int kind);
    void SetPiece(const Piece &piece);
    void SetTopHole(int start_x, int end_x);

    // Cleared lines
    int GetClearedLineCount() const;
    void GetClearedLines(int *cleared_line_y) const;
    void ClearLines();

private:
    struct Line {
        std::array<int8_t, FIELD_WIDTH> elem {0};
        bool is_cleared = false;
        int8_t tile_count = 0;

        Line () {}
        const int8_t operator[](int i) const { return elem[i]; }
        int8_t &operator[](int i) { return elem[i]; }

        bool IsFilled() const { return tile_count == FIELD_WIDTH; }
        void MarkCleared() { is_cleared = true; }

        void SetCell(int x, int kind)
        {
            assert(IsValidCell(kind));
            assert(IsEmptyCell((*this)[x]));

            (*this)[x] = kind;
            tile_count++;

            if (IsFilled())
                MarkCleared();
        }
    };

    std::array<Line, FIELD_HEIGHT> lines_;
    int cleared_line_count_ = 0;
    int hole_start_ = -1, hole_end_ = -1;

    bool is_inside_hole(Point pos) const;
};

#endif
