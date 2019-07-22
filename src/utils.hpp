#ifndef OPENLWM2M_UTILS_HPP_
#define OPENLWM2M_UTILS_HPP_

#include <cstddef>

namespace openlwm2m {

class Utils {
public:
    static int strCopy(char* dst, const char* src, size_t len);
    static int strCat(char* dst, const char* src, size_t len);
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_UTILS_HPP_ */