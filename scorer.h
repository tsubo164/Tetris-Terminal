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

    void AddLineClear(int count, int tspin_kind);
    void AddSoftDrop();
    void AddHardDrop(int distance);
    void AddTspin(Point kick_offset, Point pos, int rotation, const Field &field);

    int GetScore() const;
    int GetLines() const;
    int GetLevel() const;
    int GetClearPoints() const;
    int GetComboCounter() const;
    int GetComboPoints() const;

private:
    int score_ = 0;
    int lines_ = 0;
    int level_ = 1;

    // For each locking
    int clear_count_ = 0;
    int clear_points_ = 0;
    int combo_counter_ = -1;
    int combo_points_ = 0;

    int get_combo_count() const;
};

#endif
