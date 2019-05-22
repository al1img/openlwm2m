#ifndef OPENLWM2M_RESOURCE_HPP_
#define OPENLWM2M_RESOURCE_HPP_

#include "interface.hpp"
#include "itembase.hpp"
#include "lwm2mstorage.hpp"
#include "resourcedesc.hpp"
#include "resourceinstance.hpp"
#include "status.hpp"

namespace openlwm2m {

class Resource : public ItemBase {
public:
    ResourceInstance* createInstance(uint16_t id, Status* status = NULL);

private:
    friend class ObjectInstance;
    friend class Lwm2mStorage<Resource, ResourceDesc&>;

    typedef Lwm2mStorage<Resource, ResourceDesc&> Storage;

    ResourceDesc& mDesc;
    ResourceInstance::Storage mInstanceStorage;

    Resource(ItemBase* parent, uint16_t id, ResourceDesc& desc);
    virtual ~Resource();

    void init();
    void destroy();

    void create() {}
    void release() {}
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_RESOURCE_HPP_ */