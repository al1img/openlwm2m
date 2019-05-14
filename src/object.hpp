#ifndef OPENLWM2M_OBJECT_HPP_
#define OPENLWM2M_OBJECT_HPP_

#include <stdint.h>

namespace openlwm2m {

class Object {
public:
    Object(uint16_t id, int maxInstances, bool mandatory, uint16_t interfaces);
    ~Object();

private:
    uint16_t mId;
    int mMaxInstances;
    bool mMandatory;
    uint16_t mInterfaces;
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_OBJECT_HPP_ */