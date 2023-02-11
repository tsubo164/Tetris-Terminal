#include "field.h"
#include "cell.h"

#include <algorithm>
#include <cassert>
#include <vector>
#include <array>

struct Line {
    Line () {}
    std::array<char,FIELD_WIDTH> elem {0};
    bool is_cleared = false;

    const char operator[](int i) const { return elem[i]; }
    char &operator[](int i) { return elem[i]; }
};

static std::vector<Line> lines(FIELD_HEIGHT);
static int cleared_line_count = 0;

static bool is_line_full(int y)
{
    for (int x = 0; x < FIELD_WIDTH; x++)
        if (IsEmptyCell(GetFieldCellKind({x, y})))
            return false;
    return true;
}

static void mark_line_cleared(int y)
{
    if (lines[y].is_cleared)
        return;

    cleared_line_count++;

    lines[y].is_cleared = true;
}

bool is_inside_field(Point pos)
{
    if (pos.x < 0 || pos.x >= FIELD_WIDTH)
        return false;

    if (pos.y < 0 || pos.y >= FIELD_HEIGHT)
        return false;

    return true;
}

int GetFieldCellKind(Point pos)
{
    if (!is_inside_field(pos))
        return B;

    return lines[pos.y][pos.x];
}

void SetFieldCellKind(Point pos, int kind)
{
    assert(IsValidCell(kind));

    if (!is_inside_field(pos))
        return;

    lines[pos.y][pos.x] = kind;

    // XXX TEMP
    if (is_line_full(pos.y))
        mark_line_cleared(pos.y);
}

int GetClearedLineCount()
{
    return cleared_line_count;
}

void GetClearedLines(int *cleared_line_y)
{
    int index = 0;
    int y = 0;

    for (auto line = lines.begin(); line != lines.end(); ++line, ++y) {
        if (line->is_cleared) {
            cleared_line_y[index++] = y;

            if (index == 4)
                return;
        }
    }
}

void ClearLines()
{
    auto it = std::remove_if(
            lines.begin(),
            lines.end(),
            [](const Line &line) { return line.is_cleared; });

    std::fill(it, lines.end(), Line());

    cleared_line_count = 0;
}
