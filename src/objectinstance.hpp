#ifndef OPENLWM2M_OBJECTINSTANCE_HPP_
#define OPENLWM2M_OBJECTINSTANCE_HPP_

#include <stdint.h>

#include "itembase.hpp"
#include "resource.hpp"
#include "resourcedesc.hpp"
#include "storage.hpp"

namespace openlwm2m {

class ObjectInstance : public ItemBase {
public:
    Resource* getResourceById(uint16_t id);
    ResourceInstance* getResourceInstance(uint16_t resId, uint16_t resInstanceId = 0);

private:
    friend class Object;
    friend class Lwm2mStorage<ObjectInstance, ResourceDesc::Storage&>;
    friend class Lwm2mDynamicStorage<ObjectInstance, ResourceDesc::Storage&>;

    typedef Lwm2mDynamicStorage<ObjectInstance, ResourceDesc::Storage&> Storage;
    typedef Node<ObjectInstance> StorageNode;

    Resource::Storage mResourceStorage;

    ObjectInstance(ItemBase* parent, uint16_t id, ResourceDesc::Storage& resourceDescStorage);
    ~ObjectInstance();

    void init();
    void release();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_OBJECTINSTANCE_HPP_ */