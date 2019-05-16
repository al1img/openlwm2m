#ifndef OPENLWM2M_OBJECTINSTANCE_HPP_
#define OPENLWM2M_OBJECTINSTANCE_HPP_

#include <stdint.h>

#include "list.hpp"

namespace openlwm2m {

class ObjectInstance {
public:
    uint16_t getId() const { return mId; }

private:
    friend class Object;

    uint16_t mId;
    uint16_t mObjectId;
    List mResourceList;

    ObjectInstance(uint16_t id, uint16_t objectId, List& resourceDescList);
    ~ObjectInstance();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_OBJECTINSTANCE_HPP_ */