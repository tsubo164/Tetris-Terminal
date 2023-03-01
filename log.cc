#include "log.h"

#include <iostream>
#include <fstream>
#include <string>
#include <deque>

static std::deque<std::string> logs;

void AddLog(const char *str, ...)
{
    static char buf[1024] = {'\0'};
    va_list va;

    va_start(va, str);
    vsprintf(buf, str, va);
    va_end(va);

    logs.push_back(buf);

    if (logs.size() > 1024)
        logs.pop_front();
}

void SaveLog()
{
    std::ofstream ofs("log.txt");

    if (!ofs) {
        std::cerr << "can't open log.txt" << std::endl;
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
