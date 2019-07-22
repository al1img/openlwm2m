#include "utils.hpp"

#include <cstring>

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

int Utils::strCopy(char* dst, const char* src, size_t len)
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

int Utils::strCat(char* dst, const char* src, size_t len)
{
    size_t dstLen = strlen(dst);

    if (dstLen + 1 >= len) {
        return -1;
    }

    return strCopy(&dst[dstLen], src, len - dstLen - 1);
}

}  // namespace openlwm2m