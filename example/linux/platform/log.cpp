#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <cstdlib>

#include "log.hpp"

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_BRIGHT_RED "\x1b[91m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

static size_t sMaxModuleLen = 0;

void platformLog(int logLevel, const char* module, const char* format, ...)
{
    time_t curTime;

    time(&curTime);
    tm* tmInfo = localtime(&curTime);

    char timeStr[32];

    strftime(timeStr, 32, "%d.%m.%Y %H:%M:%S", tmInfo);

    static const char* logLevelStr[] = {"", "CRI", "ERR", "WRN", "INF", "DBG", "UNK"};

    static const char* logLevelColor[] = {"",
                                          ANSI_COLOR_BRIGHT_RED,
                                          ANSI_COLOR_RED,
                                          ANSI_COLOR_YELLOW,
                                          ANSI_COLOR_GREEN,
                                          ANSI_COLOR_RESET,
                                          ANSI_COLOR_MAGENTA};

    if (logLevel > LOG_LEVEL_DEBUG) {
        logLevel = LOG_LEVEL_DEBUG + 1;
    }

    size_t moduleLen = strlen(module);

    if (moduleLen > sMaxModuleLen) {
        sMaxModuleLen = moduleLen;
    }

    printf("%s", logLevelColor[logLevel]);

    printf("%s | %s | %*s | ", timeStr, logLevelStr[logLevel], static_cast<int>(-sMaxModuleLen), module);

    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    printf("\n" ANSI_COLOR_RESET);
}

void platformAssert(const char* function, int line, const char* message)
{
    if (!message) {
        printf(ANSI_COLOR_RED "Assert failed in function: %s, line: %d\n", function, line);
    }
    else {
        printf(ANSI_COLOR_RED "%s at function: %s, line: %d\n", message, function, line);
    }

    exit(1);
}