#ifndef OPENLWM2M_OBJECT_HPP_
#define OPENLWM2M_OBJECT_HPP_

#include <stdint.h>

#include "list.hpp"
#include "resource.hpp"

namespace openlwm2m {

class Object {
public:
    Resource* createResource(uint16_t id, uint16_t operations, int maxInstances, bool mandatory, ResourceType type,
                             int min = 0, int max = 0);

private:
    friend class Client;

    uint16_t mId;
    int mMaxInstances;
    bool mMandatory;
    uint16_t mInterfaces;

    List mResourceList;

    Object(uint16_t id, int maxInstances, bool mandatory, uint16_t interfaces);
    ~Object();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_OBJECT_HPP_ */