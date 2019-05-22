#ifndef OPENLWM2M_OBJECTINSTANCE_HPP_
#define OPENLWM2M_OBJECTINSTANCE_HPP_

#include <stdint.h>

#include "itembase.hpp"
#include "resource.hpp"
#include "resourcedesc.hpp"
#include "storage.hpp"

namespace openlwm2m {

class ObjectInstance : public ItemBase {
private:
    friend class Object;
    friend class StorageBase<ObjectInstance>;
    friend class StorageItem<ObjectInstance, ResourceDesc::Storage&>;

    typedef StorageItem<ObjectInstance, ResourceDesc::Storage&> Storage;

    Resource::Storage mResourceStorage;

    ObjectInstance(ItemBase* parent, uint16_t id, ResourceDesc::Storage& resourceDescStorage);
    virtual ~ObjectInstance();

    void init();
    void release();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_OBJECTINSTANCE_HPP_ */