#include "log.h"

#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <deque>

static const int MAX_LOG_COUNT = 1024;
static std::deque<std::string> logs;
static bool is_log_enabled = true;

void EnableLog(bool enable)
{
    is_log_enabled = enable;
}

void AddLog(const char *str, ...)
{
    if (!is_log_enabled)
        return;

    // Format
    static char buf[1024] = {'\0'};
    va_list va;

    va_start(va, str);
    vsprintf(buf, str, va);
    va_end(va);

    // Push
    logs.push_back(buf);

    if (logs.size() > MAX_LOG_COUNT)
        logs.pop_front();
}

void SaveLog()
{
    if (!is_log_enabled)
        return;

    // Random file name
    std::random_device rd;
    std::mt19937 rng(rd());
    const std::string filename = "log_tetris_" + std::to_string(rng()) + ".txt";

    // Save
    std::ofstream ofs(filename);

    if (!ofs) {
        std::cerr << "can't open file: " << filename << std::endl;
        return;
    }

    for (auto log: logs)
        ofs << log << std::endl;
}

void Assert(int expr, const char *str, const char *file, int line)
{
    if (expr)
        return;

    fprintf(stderr, "Assertion failed: '%s'", str);
    fprintf(stderr, "File: %s, Line: %d", file, line);

    AddLog("Assertion failed: '%s'", str);
    AddLog("File: %s, Line: %d", file, line);

    SaveLog();
    std::abort();
}
