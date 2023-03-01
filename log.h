#ifndef LOG_H
#define LOG_H

void AddLog(const char *str, ...);
void SaveLog();

void Assert(int expr, const char *str, const char *file, int line);

#define TET_ASSERT(expr) Assert((expr), #expr, __FILE__, __LINE__)

#endif
