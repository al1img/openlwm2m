#ifndef OPENLWM2M_RESOURCEINSTANCE_HPP_
#define OPENLWM2M_RESOURCEINSTANCE_HPP_

#include <stdint.h>

#include "itembase.hpp"
#include "resourcedesc.hpp"

namespace openlwm2m {

class Object;

class ResourceInstance : public ItemBase {
public:
    virtual const char* getString() const
    {
        ASSERT_MESSAGE(false, "Method not supported");
        return NULL;
    };
    virtual Status setString(const char* value)
    {
        ASSERT_MESSAGE(false, "Method not supported");
        return STS_ERR_INVALID_VALUE;
    };

    virtual int64_t getInt() const
    {
        ASSERT_MESSAGE(false, "Method not supported");
        return 0;
    };
    virtual Status setInt(int64_t value)
    {
        ASSERT_MESSAGE(false, "Method not supported");
        return STS_ERR_INVALID_VALUE;
    };

    virtual uint64_t getUint() const
    {
        ASSERT_MESSAGE(false, "Method not supported");
        return 0;
    };
    virtual Status setUint(uint64_t value)
    {
        ASSERT_MESSAGE(false, "Method not supported");
        return STS_ERR_INVALID_VALUE;
    };

    virtual uint8_t getBool() const
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
    ResourceDesc& mDesc;

    ResourceInstance(ItemBase* parent, uint16_t id, ResourceDesc& desc);
    virtual ~ResourceInstance();

    void valueChanged();

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

    const char* getString() const { return mValue; }
    Status setString(const char* value);
};

class ResourceInstanceInt : public ResourceInstance {
private:
    friend class ResourceInstance;

    int64_t mValue;

    ResourceInstanceInt(ItemBase* parent, uint16_t id, ResourceDesc& desc);
    virtual ~ResourceInstanceInt();

    void init();
    void release();

    int64_t getInt() const { return mValue; }
    Status setInt(int64_t value);
};

class ResourceInstanceBool : public ResourceInstance {
private:
    friend class ResourceInstance;

    uint8_t mValue;

    ResourceInstanceBool(ItemBase* parent, uint16_t id, ResourceDesc& desc);
    virtual ~ResourceInstanceBool();

    void init();
    void release();

    uint8_t getBool() const { return mValue ? 1 : 0; }
    Status setBool(uint8_t value);
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_RESOURCEINSTANCE_HPP_ */