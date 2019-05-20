#ifndef OPENLWM2M_RESOURCEINSTANCE_HPP_
#define OPENLWM2M_RESOURCEINSTANCE_HPP_

#include <stdint.h>

#include "lwm2mbase.hpp"
#include "resourcedesc.hpp"

namespace openlwm2m {

class Resource;

class ResourceInstance : public Lwm2mBase {
private:
    friend class Resource;
    friend class Lwm2mStorage<ResourceInstance, ResourceDesc&>;

    ResourceDesc& mDesc;

    typedef Lwm2mStorage<ResourceInstance, ResourceDesc&> Storage;

    ResourceInstance(Lwm2mBase* parent, uint16_t id, ResourceDesc& desc);
    virtual ~ResourceInstance();

    void create();
    void release();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_RESOURCEINSTANCE_HPP_ */