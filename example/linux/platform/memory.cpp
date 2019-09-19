#include <cstdlib>

#include "config.hpp"
#include "log.hpp"

#define LOG_MODULE "memory"

/*******************************************************************************
 * New
 ******************************************************************************/

size_t allocatedMemSize = 0;
bool initDone = false;

void* operator new(size_t size)
{
    ASSERT_MESSAGE(!initDone, "Can't allocate memory after init");

    allocatedMemSize += size;

    return malloc(size);
}

void memInitDone()
{
    initDone = true;

    LOG_INFO("Total allocated memory: %lu", allocatedMemSize);
}
