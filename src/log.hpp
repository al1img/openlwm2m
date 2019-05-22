#ifndef OPENLWM2M_LOG_HPP_
#define OPENLWM2M_LOG_HPP_

#include "config.hpp"

#define LOG_LEVEL_NONE 0
#define LOG_LEVEL_CRITICAL 1
#define LOG_LEVEL_ERROR 2
#define LOG_LEVEL_WARNING 3
#define LOG_LEVEL_INFO 4
#define LOG_LEVEL_DEBUG 5

#if CONFIG_LOG_LEVEL >= LOG_LEVEL_CRITICAL
#define LOG_CRITICAL(format, ...) platformLog(LOG_LEVEL_CRITICAL, LOG_MODULE, format, ##__VA_ARGS__)
#else
#define LOG_CRITICAL()
#endif

#if CONFIG_LOG_LEVEL >= LOG_LEVEL_ERROR
#define LOG_ERROR(format, ...) platformLog(LOG_LEVEL_ERROR, LOG_MODULE, format, ##__VA_ARGS__)
#else
#define LOG_ERROR()
#endif

#if CONFIG_LOG_LEVEL >= LOG_LEVEL_WARNING
#define LOG_WARNING(format, ...) platformLog(LOG_LEVEL_WARNING, LOG_MODULE, format, ##__VA_ARGS__)
#else
#define LOG_LOG_WARNING()
#endif

#if CONFIG_LOG_LEVEL >= LOG_LEVEL_INFO
#define LOG_INFO(format, ...) platformLog(LOG_LEVEL_INFO, LOG_MODULE, format, ##__VA_ARGS__)
#else
#define LOG_INFO()
#endif

#if CONFIG_LOG_LEVEL >= LOG_LEVEL_DEBUG
#define LOG_DEBUG(format, ...) platformLog(LOG_LEVEL_DEBUG, LOG_MODULE, format, ##__VA_ARGS__)
#else
#define LOG_LOG_DEBUG()
#endif

#if CONFIG_LOG_LEVEL >= LOG_LEVEL_CRITICAL
extern void platformLog(int logLevel, const char* module, const char* format, ...);
#endif

#define ASSERT(condition)                              \
    if (!(condition)) {                                \
        platformAssert(__PRETTY_FUNCTION__, __LINE__); \
    }

#define ASSERT_MESSAGE(condition, message)                      \
    if (!(condition)) {                                         \
        platformAssert(__PRETTY_FUNCTION__, __LINE__, message); \
    }

extern void platformAssert(const char* function, int line, const char* message = NULL);

#endif /* OPENLWM2M_LOG_HPP_ */
