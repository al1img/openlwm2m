#ifndef OPENLWM2M_RESOURCE_HPP_
#define OPENLWM2M_RESOURCE_HPP_

#include "interface.hpp"
#include "lwm2mbase.hpp"
#include "lwm2mstorage.hpp"
#include "resourcedesc.hpp"
#include "resourceinstance.hpp"
#include "status.hpp"

namespace openlwm2m {

class Resource : public Lwm2mBase {
public:
    ResourceInstance* createInstance(uint16_t id, Status* status = NULL);

private:
    friend class ObjectInstance;
    friend class Lwm2mStorage<Resource, ResourceDesc&>;

    typedef Lwm2mStorage<Resource, ResourceDesc&> Storage;

    ResourceDesc& mDesc;
    ResourceInstance::Storage mInstanceStorage;

    Resource(Lwm2mBase* parent, uint16_t id, ResourceDesc& desc);
    virtual ~Resource();

    void init();
    void destroy();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_RESOURCE_HPP_ */