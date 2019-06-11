#ifndef OPENLWM2M_RESOURCEINSTANCE_HPP_
#define OPENLWM2M_RESOURCEINSTANCE_HPP_

#include <stdint.h>

#include "itembase.hpp"
#include "resourcedesc.hpp"

namespace openlwm2m {

class Object;

class ResourceInstance : public ItemBase {
public:
    const char* getString() const
    {
        ASSERT_MESSAGE(mDesc.mParams.type == ResourceDesc::TYPE_STRING, "Method not supported");
        return mValueString;
    };

    Status setString(const char* value);

    int64_t getInt() const
    {
        ASSERT_MESSAGE(mDesc.mParams.type == ResourceDesc::TYPE_INT, "Method not supported");
        return mValueInt;
    };

    Status setInt(int64_t value);

    uint64_t getUint() const
    {
        ASSERT_MESSAGE(mDesc.mParams.type == ResourceDesc::TYPE_UINT, "Method not supported");
        return mValueUint;
    };

    Status setUint(uint64_t value);

    uint8_t getBool() const
    {
        ASSERT_MESSAGE(mDesc.mParams.type == ResourceDesc::TYPE_BOOL, "Method not supported");
        return mValueBool;
    };

    Status setBool(uint8_t value);

private:
    friend class Resource;
    friend class StorageBase<ResourceInstance>;
    friend class StorageItem<ResourceInstance, ResourceDesc&>;

    typedef StorageItem<ResourceInstance, ResourceDesc&> Storage;
    typedef Node<ResourceInstance> StorageNode;

    ResourceDesc& mDesc;

    union {
        char* mValueString;
        int64_t mValueInt;
        uint64_t mValueUint;
        double mValueFloat;
        uint8_t mValueBool;
    };

    ResourceInstance(ItemBase* parent, uint16_t id, ResourceDesc& desc);
    ~ResourceInstance();

    void init();
    void release();

    void valueChanged();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_RESOURCEINSTANCE_HPP_ */