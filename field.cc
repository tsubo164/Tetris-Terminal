#include "field.h"
#include "cell.h"
#include <cassert>

static char field_grid[FIELD_HEIGHT][FIELD_WIDTH] =
{
    {B,B,B,0,0,0,0,0,0,B,B,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,0,0,0,0,0,0,0,0,0,0,B},
    {B,B,B,B,B,B,B,B,B,B,B,B},
};

static bool is_line_cleared[FIELD_HEIGHT] = {0};
static int cleared_line_count = 0;
//static int cleared_lines[4] = {0};

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
}

bool IsLineCleard(int y)
{
    return is_line_cleared[y];
}

bool IsLineFull(int y)
{
    for (int x = 1; x < FIELD_WIDTH - 1; x++)
        if (IsEmptyCell(GetFieldCellKind({x, y})))
            return false;

    return true;
}

void MarkLineCleared(int y)
{
    is_line_cleared[y] = true;
    cleared_line_count++;
}

static void copy_line(int src_line, int dst_line)
{
    for (int x = 1; x < FIELD_WIDTH - 1; x++) {
        int cell;

        if (src_line < FIELD_HEIGHT - 1)
            cell = GetFieldCellKind({x, src_line});
        else
            cell = E;

        SetFieldCellKind({x, dst_line}, cell);
    }
}

void ClearLines()
{
    int src_line = 0;
    int dst_line = 0;

    while (dst_line < FIELD_HEIGHT - 1) {
        // Copy a line downwards
        if (src_line != dst_line)
            copy_line(src_line, dst_line);

        // Move up destination line by one
        dst_line++;

        // Move up source line to the next uncleared line
        do {
            src_line++;
        }
        while (is_line_cleared[src_line] && src_line < FIELD_HEIGHT - 1);
    }

    // Zero clear
    for (int i = 0; i < FIELD_HEIGHT; i++) {
        is_line_cleared[i] = false;
    }
    cleared_line_count = 0;
}
