#include "field.h"
#include "cell.h"
#include <cassert>

static char field_grid[FIELD_HEIGHT][FIELD_WIDTH] =
{
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0},
};

static bool is_line_cleared[FIELD_HEIGHT] = {0};
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
    if (is_line_cleared[y])
        return;

    is_line_cleared[y] = true;
    cleared_line_count++;
}

int GetFieldCellKind(Point field)
{
    if (field.x < 0 || field.x >= FIELD_WIDTH)
        return B;

    if (field.y < 0 || field.y >= FIELD_HEIGHT)
        return B;

    return field_grid[FIELD_HEIGHT - field.y - 1][field.x];
}

void SetFieldCellKind(Point field, int kind)
{
    assert(IsValidCell(kind));

    if (field.x < 0 || field.x >= FIELD_WIDTH)
        return;

    if (field.y < 0 || field.y >= FIELD_HEIGHT)
        return;

    field_grid[FIELD_HEIGHT - field.y - 1][field.x] = kind;

    // XXX TEMP
    if (is_line_full(field.y))
        mark_line_cleared(field.y);
}

int GetClearedLineCount()
{
    return cleared_line_count;
}

void GetClearedLines(int *cleared_line_y)
{
    int index = 0;

    for (int y = 0; y < FIELD_HEIGHT; y++) {
        if (is_line_cleared[y]) {
            cleared_line_y[index++] = y;

            if (index == 4)
                return;
        }
    }
}

static void copy_line(int src_line, int dst_line)
{
    for (int x = 0; x < FIELD_WIDTH; x++) {
        int cell;

        if (src_line < FIELD_HEIGHT)
            cell = GetFieldCellKind({x, src_line});
        else
            cell = E;

        SetFieldCellKind({x, dst_line}, cell);
    }
}

void ClearLines()
{
    int src_line = -1;
    int dst_line = -1;

    while (dst_line < FIELD_HEIGHT) {
        // Copy a line downwards
        if (src_line != dst_line)
            copy_line(src_line, dst_line);

        // Move up destination line by one
        dst_line++;

        // Move up source line to the next uncleared line
        do {
            src_line++;
        }
        while (is_line_cleared[src_line] && src_line < FIELD_HEIGHT);
    }

    // Zero clear
    for (int i = 0; i < FIELD_HEIGHT; i++) {
        is_line_cleared[i] = false;
    }
    cleared_line_count = 0;
}
