#include "scorer.h"
#include <algorithm>
#include <cassert>
#include <cstdlib>

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
    clear_points_ = 0;
    combo_counter_ = -1;
    combo_points_ = 0;
}

void Scorer::Start()
{
    if (clear_count_ == 0)
        // Combo break
        combo_counter_ = -1;

    clear_count_ = 0;
    clear_points_ = 0;
    combo_points_ = 0;
}

void Scorer::Commit()
{
    // Lines
    lines_ += clear_count_;

    // Score
    score_ += clear_points_;
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

void Scorer::AddLineClear(int count, int tspin_kind)
{
    assert(count >= 0 && count <= 4);
    assert(tspin_kind >= TSPIN_NONE && tspin_kind <= TSPIN_MINI);

    // Clear count
    clear_count_ = count;

    // Clear points
    static const int point_table[][5] = {
        //  0     1      2     3    4
        {   0,  100,   300,  500, 800}, // normal
        { 400,  800,  1200, 1600,   0}, // T-spin
        { 100,  200,   400,    0,   0}  // T-Spin Mini
    };
    clear_points_ = point_table[tspin_kind][clear_count_] * level_;

    // Combo counter
    combo_counter_ = std::min(13, combo_counter_ + 1);
    combo_points_ = 50 * get_combo_count() * level_;
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

int Scorer::GetClearPoints() const
{
    return clear_points_;
}

int Scorer::GetComboCounter() const
{
    return combo_counter_;
}

int Scorer::GetComboPoints() const
{
    return combo_points_;
}
