#include "utils.hpp"

#include <cstdio>
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

int Utils::convertPath(char* path, uint16_t* objectId, uint16_t* objectInstanceId, uint16_t* resourceId,
                       uint16_t* resourceInstanceId)
{
    uint16_t* setValues[4] = {objectId, objectInstanceId, resourceId, resourceInstanceId};

    for (int i = 0; i < 4; i++) {
        *setValues[i] = UINT16_MAX;
    }

    for (int i = 0; i < 4; i++) {
        if (path == NULL || *path == '\0') {
            return 0;
        }

        if (*path != '/') {
            return -1;
        }

        path++;

        uint64_t value = strtol(path, &path, 0);

        if (value > UINT16_MAX) {
            return -1;
        }

        *setValues[i] = value;
    }

    return 0;
}

int Utils::makePath(uint16_t objectId, uint16_t objectInstanceId, uint16_t resourceId, uint16_t resourceInstanceId,
                    char* path, size_t len)
{
    uint16_t values[4] = {objectId, objectInstanceId, resourceId, resourceInstanceId};
    size_t size = 0;
    int ret = 0;

    if (len == 0 || path == NULL) {
        return -1;
    }

    for (int i = 0; i < 4; i++) {
        if (values[i] == UINT16_MAX) {
            break;
        }

        ret = snprintf(&path[size], len - size, "/%d", values[i]);
        if (ret < 0) {
            return -1;
        }

        size += ret;
    }

    path[size] = '\0';

    return size;
}

}  // namespace openlwm2m