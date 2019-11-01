#include "platform.hpp"

#include <time.h>

#include "log.hpp"

/*******************************************************************************
 * getCurrentTime
 ******************************************************************************/

uint64_t Platform::getCurrentTime()
{
    timespec tp;

    int ret = clock_gettime(CLOCK_MONOTONIC, &tp);
    ASSERT(ret == 0);

    return tp.tv_sec * 1000 + tp.tv_nsec / 1000000;
}