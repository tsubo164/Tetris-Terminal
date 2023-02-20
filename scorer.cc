#include "scorer.h"
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
}

void Scorer::Start()
{
    cleared_lines_ = 0;
}

void Scorer::Commit()
{
    // Line clear
    static const int pts_by_lines[] = {0, 100, 300, 500, 800};
    score_ += pts_by_lines[cleared_lines_] * level_;

    // Total lines
    lines_ += cleared_lines_;

    // Level
    if (lines_ >= 5 * level_)
        level_++;
}

void Scorer::AddLineClear(int count)
{
    assert(count > 0 && count < 5);
    cleared_lines_ = count;
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
