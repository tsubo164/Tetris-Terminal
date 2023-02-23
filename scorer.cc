#include "scorer.h"
#include <algorithm>
#include <cassert>

Scorer::Scorer()
{
}

Scorer::~Scorer()
{
}

void Scorer::Reset()
{
    score_ = 0;
    lines_ = 0;
    level_ = 1;

    // For each locking
    clear_count_ = 0;
    combo_counter_ = -1;
    combo_points_ = 0;
    tspin_kind_ = 0;
    tspin_points_ = 0;
}

void Scorer::Start()
{
    if (clear_count_ == 0)
        // Combo break
        combo_counter_ = -1;

    clear_count_ = 0;
    combo_points_ = 0;
    tspin_kind_ = 0;
    tspin_points_ = 0;
}

void Scorer::Commit()
{
    // Line clear
    score_ += clear_points_;

    // Total lines
    lines_ += clear_count_;
    combo_counter_ = std::min(13, combo_counter_ + 1);
    combo_points_ = 50 * get_combo_count() * level_;
    score_ += combo_points_;

    // Level
    if (lines_ >= 5 * level_)
        level_++;
}

int Scorer::get_combo_count() const
{
    const int count = combo_counter_;
    static const int combo_counts[14] = {
     // 0  1  2  3  4  5  6  7  8  9 10 11 12 13+
        0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 5, 5, 5
    };

    assert(count >= 0 && count < 14);
    return combo_counts[count];
}

void Scorer::AddLineClear(int count)
{
    assert(count > 0 && count < 5);
    clear_count_ = count;

    static const int pts_by_lines[] = {0, 100, 300, 500, 800};
    clear_points_ = pts_by_lines[clear_count_] * level_;
}

void Scorer::AddSoftDrop()
{
    score_++;
}

void Scorer::AddHardDrop(int distance)
{
    score_ += 2 * distance;
}

void Scorer::AddTspin(Point pos, int rotation, const Field &field)
{
    // Tetris will never make t-spin
    const int cleared_lines = field.GetClearedLineCount();
    if (cleared_lines == 4)
        return;

    // Detection
    const Piece tcorners = GetTcorners(rotation);
    int front_occluded = 0;
    int back_occluded = 0;

    for (int i = 0; i < 4; i++) {
        const Point world = pos + tcorners.cells[i];
        const int kind = field.GetCellKind(world);

        if (!IsEmptyCell(kind)) {
            if (i == 0 || i == 1)
                front_occluded++;
            if (i == 2 || i == 3)
                back_occluded++;
        }
    }

    // Kind
    if (front_occluded == 2 && back_occluded == 1)
        tspin_kind_ = TSPIN_NORMAL;
    else if (front_occluded == 1 && back_occluded == 2)
        tspin_kind_ = TSPIN_MINI;
    else
        tspin_kind_ = TSPIN_NONE;

    // Points
    static const int tspin_points[][4] = {
    /* lines         0     1      2     3 */
    /* T-spin */ { 400,  800,  1200, 1600},
    /* Mini   */ { 100,  200,   400,    0}
    };

    if (tspin_kind_ == TSPIN_NORMAL) {
        tspin_points_ = tspin_points[0][cleared_lines];
    }
    else if (tspin_kind_ == TSPIN_MINI) {
        tspin_points_ = tspin_points[1][cleared_lines];
    }

    score_ += tspin_points_;
}

int Scorer::GetScore() const
{
    return score_;
}

int Scorer::GetLines() const
{
    return lines_;
}

int Scorer::GetLevel() const
{
    return level_;
}

int Scorer::GetComboCounter() const
{
    return combo_counter_;
}

int Scorer::GetComboPoints() const
{
    return combo_points_;
}

int Scorer::GetTspinKind() const
{
    return tspin_kind_;
}

int Scorer::GetTspinPoints() const
{
    return tspin_points_;
}

int Scorer::GetClearPoints() const
{
    return clear_points_;
}
