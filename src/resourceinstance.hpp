#ifndef OPENLWM2M_RESOURCEINSTANCE_HPP_
#define OPENLWM2M_RESOURCEINSTANCE_HPP_

#include <stdint.h>

#include "itembase.hpp"
#include "resourcedesc.hpp"

namespace openlwm2m {

class Resource;

class ResourceInstance : public ItemBase {
public:
    virtual const char* getString()
    {
        ASSERT_MESSAGE(false, "Method not supported");
        return NULL;
    };
    virtual void setString(const char* value) { ASSERT_MESSAGE(false, "Method not supported"); };

protected:
    ResourceInstance(ItemBase* parent, uint16_t id, ResourceDesc& desc);
    virtual ~ResourceInstance();

    ResourceDesc& mDesc;

private:
    friend class Resource;
    friend class StorageBase<ResourceInstance>;
    friend class StorageItem<ResourceInstance, ResourceDesc&>;

    typedef StorageItem<ResourceInstance, ResourceDesc&> Storage;
    typedef Node<ResourceInstance> StorageNode;

    void init();
    void release();

    static ResourceInstance* newInstance(ItemBase* parent, uint16_t id, ResourceDesc& desc);
};

class ResourceInstanceString : public ResourceInstance {
private:
    friend class ResourceInstance;

    char* mValue;

    ResourceInstanceString(ItemBase* parent, uint16_t id, ResourceDesc& desc);
    virtual ~ResourceInstanceString();

    void init();
    void release();

    const char* getString();
    void setString(const char* value);
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_RESOURCEINSTANCE_HPP_ */