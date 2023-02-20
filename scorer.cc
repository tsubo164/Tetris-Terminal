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
    cleared_lines_ = 0;
    combo_counter_ = -1;
}

void Scorer::Start()
{
    if (cleared_lines_ == 0)
        // Combo break
        combo_counter_ = -1;

    cleared_lines_ = 0;
}

void Scorer::Commit()
{
    // Line clear
    static const int pts_by_lines[] = {0, 100, 300, 500, 800};
    score_ += pts_by_lines[cleared_lines_] * level_;

    // Total lines
    lines_ += cleared_lines_;
    combo_counter_ = std::min(13, combo_counter_ + 1);
    score_ += 50 * get_combo_count() * level_;

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
    cleared_lines_ = count;
}

void Scorer::AddSoftDrop()
{
    score_++;
}

void Scorer::AddHardDrop(int distance)
{
    score_ += 2 * distance;
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
