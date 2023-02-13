#include "field.h"

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

    return lines_[pos.y][pos.x];
}

void Field::SetPiece(const Piece &piece)
{
    for (auto pos: piece.cells) {
        const int x = pos.x, y = pos.y;

        assert(IsEmptyCell(lines_[y][x]));
        assert(is_inside_field(pos));

        lines_[y].SetCell(x, piece.kind);

        if (lines_[y].IsFilled())
            cleared_line_count_++;
    }
}

int Field::GetClearedLineCount() const
{
    return cleared_line_count_;
}

void Field::GetClearedLines(int *cleared_line_y) const
{
    int index = 0;
    int y = 0;

    for (auto line = lines_.begin(); line != lines_.end(); ++line, ++y) {
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
            lines_.begin(),
            lines_.end(),
            [](const Line &line) { return line.is_cleared; });

    std::fill(it, lines_.end(), Line());

    cleared_line_count_ = 0;
}
