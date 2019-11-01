#ifndef OPENLWM2M_PLATFORM_HPP_
#define OPENLWM2M_PLATFORM_HPP_

#include <stdint.h>

class Platform {
public:
    static uint64_t getCurrentTime();
};

#endif /* OPENLWM2M_PLATFORM_HPP_ */
