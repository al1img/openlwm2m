#ifndef OPENLWM2M_UTILS_HPP_
#define OPENLWM2M_UTILS_HPP_

#include <stdint.h>
#include <cstddef>

namespace openlwm2m {

class Utils {
public:
    static int strCopy(char* dst, const char* src, size_t len);
    static int strCat(char* dst, const char* src, size_t len);
    static int convertPath(char* path, uint16_t* objectId, uint16_t* objectInstanceId, uint16_t* resourceId,
                           uint16_t* resourceInstanceId);
    static int makePath(uint16_t objectId, uint16_t objectInstanceId, uint16_t resourceId, uint16_t resourceInstanceId,
                        char* path, size_t len);
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_UTILS_HPP_ */