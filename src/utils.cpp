#include "utils.hpp"

#include <cstdlib>
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

int Utils::convertPath(char* name, uint16_t* objectId, uint16_t* objectInstanceId, uint16_t* resourceId,
                       uint16_t* resourceInstanceId)
{
    uint16_t* setValues[4] = {objectId, objectInstanceId, resourceId, resourceInstanceId};

    for (int i = 0; i < 4; i++) {
        *setValues[i] = -1;
    }

    for (int i = 0; i < 4; i++) {
        if (name == NULL || *name == '\0') {
            return 0;
        }

        if (*name != '/') {
            return -1;
        }

        name++;

        uint64_t value = strtol(name, &name, 0);

        if (value > UINT16_MAX) {
            return -1;
        }

        *setValues[i] = value;
    }

    return 0;
}

}  // namespace openlwm2m