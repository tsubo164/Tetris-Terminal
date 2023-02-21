#ifndef SCORER_H
#define SCORER_H

#include "point.h"
#include "field.h"

enum TspinKind {
    TSPIN_NONE = 0,
    TSPIN_NORMAL,
    TSPIN_MINI
};

class Scorer {
public:
    Scorer();
    ~Scorer();

    void Reset();
    void Start();
    void Commit();

    void AddLineClear(int count);
    void AddSoftDrop();
    void AddHardDrop(int distance);
    void AddTspin(Point pos, int rotation, const Field &field);

    int GetScore() const;
    int GetLines() const;
    int GetLevel() const;
    int GetComboCounter() const;
    int GetTspinKind() const;

private:
    int score_ = 0;
    int lines_ = 0;
    int level_ = 1;

    // For each locking
    int cleared_lines_ = 0;
    int combo_counter_ = -1;
    int tspin_kind_ = 0;

    int get_combo_count() const;
};

#endif
