#ifndef OPENLWM2M_RESOURCE_HPP_
#define OPENLWM2M_RESOURCE_HPP_

#include "resourcedesc.hpp"

namespace openlwm2m {

class Resource {
private:
    friend class ObjectInstance;

    uint16_t mObjectInstanceId;
    ResourceDesc& mDesc;

    Resource(uint16_t objectInstanceId, ResourceDesc& desc);
    ~Resource();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_RESOURCE_HPP_ */