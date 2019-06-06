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
    virtual Status setString(const char* value)
    {
        ASSERT_MESSAGE(false, "Method not supported");
        return STS_ERR_INVALID_VALUE;
    };

    virtual int64_t getInt()
    {
        ASSERT_MESSAGE(false, "Method not supported");
        return 0;
    };
    virtual Status setInt(int64_t value)
    {
        ASSERT_MESSAGE(false, "Method not supported");
        return STS_ERR_INVALID_VALUE;
    };

    virtual uint64_t getUint()
    {
        ASSERT_MESSAGE(false, "Method not supported");
        return 0;
    };
    virtual Status setUint(uint64_t value)
    {
        ASSERT_MESSAGE(false, "Method not supported");
        return STS_ERR_INVALID_VALUE;
    };

    virtual uint8_t getBool()
    {
        ASSERT_MESSAGE(false, "Method not supported");
        return 0;
    };
    virtual Status setBool(uint8_t value)
    {
        ASSERT_MESSAGE(false, "Method not supported");
        return STS_ERR_INVALID_VALUE;
    };

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
    Status setString(const char* value);
};

class ResourceInstanceInt : public ResourceInstance {
private:
    int64_t mValue;

    ResourceInstanceInt(ItemBase* parent, uint16_t id, ResourceDesc& desc);
    virtual ~ResourceInstanceInt();

    void init();
    void release();

    int64_t getInt();
    Status setInt(int64_t value);
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_RESOURCEINSTANCE_HPP_ */