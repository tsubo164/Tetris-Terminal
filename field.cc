#include "field.h"
#include "cell.h"

#include <algorithm>
#include <cassert>

Field::Field()
{
}

Field::~Field()
{
}

bool is_inside_field(Point pos)
{
    if (pos.x < 0 || pos.x >= FIELD_WIDTH)
        return false;

    if (pos.y < 0 || pos.y >= FIELD_HEIGHT)
        return false;

    return true;
}

int Field::GetFieldCellKind(Point pos) const
{
    if (!is_inside_field(pos))
        return B;

    return lines[pos.y][pos.x];
}

void Field::SetFieldCellKind(Point pos, int kind)
{
    assert(IsValidCell(kind));

    if (!is_inside_field(pos))
        return;

    const int x = pos.x, y = pos.y;

    assert(IsEmptyCell(lines[y][x]));
    lines[y][x] = kind;
    lines[y].count++;

    if (lines[y].IsFilled()) {
        lines[y].MarkCleared();
        cleared_line_count++;
    }
}

int Field::GetClearedLineCount() const
{
    return cleared_line_count;
}

void Field::GetClearedLines(int *cleared_line_y) const
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

void Field::ClearLines()
{
    auto it = std::remove_if(
            lines.begin(),
            lines.end(),
            [](const Line &line) { return line.is_cleared; });

    std::fill(it, lines.end(), Line());

    cleared_line_count = 0;
}
