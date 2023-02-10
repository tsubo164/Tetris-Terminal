#ifndef FIELD_H
#define FIELD_H

#include "point.h"

constexpr int FIELD_WIDTH = 10;
constexpr int FIELD_HEIGHT = 20;

int GetFieldCellKind(Point field);
void SetFieldCellKind(Point field, int kind);

int GetClearedLineCount();
void GetClearedLines(int *cleared_line_y);

void ClearLines();

#endif
