#include "utils.hpp"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

int Utils::stringCopy(char* dst, const char* src, size_t len)
{
    size_t i = 0;

    for (; i < len && src[i] != 0; i++) {
        dst[i] = src[i];
    }

    if (i == len) {
        return -1;
    }

    dst[i] = 0;

    return static_cast<int>(i);
}

}  // namespace openlwm2m