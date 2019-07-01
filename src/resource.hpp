#ifndef OPENLWM2M_RESOURCE_HPP_
#define OPENLWM2M_RESOURCE_HPP_

#include "interface.hpp"
#include "itembase.hpp"
#include "resourcedesc.hpp"
#include "resourceinstance.hpp"
#include "status.hpp"
#include "storage.hpp"

namespace openlwm2m {

class Resource : public ItemBase {
public:
    ResourceInstance* createInstance(uint16_t id = INVALID_ID, Status* status = NULL);
    Status deleteInstance(ResourceInstance* instance);

    ResourceInstance* getInstanceById(uint16_t id);
    ResourceInstance* getFirstInstance();
    ResourceInstance* getNextInstance();

private:
    friend class ObjectInstance;
    friend class Lwm2mStorage<Resource, ResourceDesc&>;

    typedef Lwm2mStorage<Resource, ResourceDesc&> Storage;

    ResourceDesc& mDesc;
    ResourceInstance::Storage mInstanceStorage;

    static ResourceInstance* newInstance(ItemBase* parent, ResourceDesc& desc);

    Resource(ItemBase* parent, ResourceDesc& desc);
    ~Resource();

    void init();
    void release();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_RESOURCE_HPP_ */