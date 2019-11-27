#ifndef OPENLWM2M_UTILS_HPP_
#define OPENLWM2M_UTILS_HPP_

#include <stdint.h>
#include <cstddef>

#include "lwm2m.hpp"

namespace openlwm2m {

class Utils {
public:
    static int strCopy(char* dst, const char* src, size_t len);
    static int strCat(char* dst, const char* src, size_t len);
    static int convertPath(const char* path, uint16_t* objectId, uint16_t* objectInstanceId, uint16_t* resourceId,
                           uint16_t* resourceInstanceId);
    static int makePath(char* path, size_t len, uint16_t objectId, uint16_t objectInstanceId = INVALID_ID,
                        uint16_t resourceId = INVALID_ID, uint16_t resourceInstanceId = INVALID_ID);
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_UTILS_HPP_ */