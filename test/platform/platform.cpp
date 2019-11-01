#include "platform.hpp"

#include "log.hpp"

uint64_t sCurrentTimeMs;

/*******************************************************************************
 * getCurrentTime
 ******************************************************************************/

void setCurrentTime(uint64_t currentTimeMs)
{
    sCurrentTimeMs = currentTimeMs;
}

uint64_t Platform::getCurrentTime()
{
    return sCurrentTimeMs;
}