#ifndef OPENLWM2M_RESOURCEINSTANCE_HPP_
#define OPENLWM2M_RESOURCEINSTANCE_HPP_

#include <stdint.h>

#include "itembase.hpp"
#include "resourcedesc.hpp"

namespace openlwm2m {

class Resource;

class ResourceInstance : public ItemBase {
private:
    friend class Resource;
    friend class StorageBase<ResourceInstance>;
    friend class StorageItem<ResourceInstance, ResourceDesc&>;

    typedef StorageItem<ResourceInstance, ResourceDesc&> Storage;

    ResourceDesc& mDesc;

    ResourceInstance(ItemBase* parent, uint16_t id, ResourceDesc& desc);
    virtual ~ResourceInstance();

    void init();
    void release();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_RESOURCEINSTANCE_HPP_ */