#ifndef SCORER_H
#define SCORER_H

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

    int GetScore() const;
    int GetLines() const;
    int GetLevel() const;
    int GetComboCounter() const;

private:
    int score_ = 0;
    int lines_ = 0;
    int level_ = 1;

    // For each locking
    int cleared_lines_ = 0;
    int combo_counter_ = -1;

    int get_combo_count() const;
};

#endif
