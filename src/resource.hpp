#ifndef OPENLWM2M_RESOURCE_HPP_
#define OPENLWM2M_RESOURCE_HPP_

#include "interface.hpp"
#include "itembase.hpp"
#include "resourcedesc.hpp"
#include "resourceinstance.hpp"
#include "status.hpp"
#include "storage.hpp"

namespace openlwm2m {

#define RES_LWM2M_SERVER_URI 0

#define RES_SHORT_SERVER_ID 0

class Resource : public ItemBase {
public:
    ResourceInstance* createInstance(uint16_t id = INVALID_ID, Status* status = NULL);

    ResourceInstance* getInstanceById(uint16_t id);
    ResourceInstance* getFirstInstance();
    ResourceInstance* getNextInstance();

private:
    friend class ObjectInstance;
    friend class StorageBase<Resource>;
    friend class StorageArray<Resource, ResourceDesc&>;

    typedef StorageArray<Resource, ResourceDesc&> Storage;

    ResourceDesc& mDesc;
    ResourceInstance::Storage mInstanceStorage;
    ResourceInstance::StorageNode* mInstanceNode;

    Resource(ItemBase* parent, uint16_t id, ResourceDesc& desc);
    virtual ~Resource();

    void init();
    void release();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_RESOURCE_HPP_ */