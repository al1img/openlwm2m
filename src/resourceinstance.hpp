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
        ASSERT_MESSAGE(true, "Method not supported");
        return "";
    }

    virtual Status setString(const char* value)
    {
        ASSERT_MESSAGE(true, "Method not supported");
        return STS_ERR_NOT_EXIST;
    }

    virtual int64_t getInt() const
    {
        ASSERT_MESSAGE(true, "Method not supported");
        return 0;
    }

    virtual Status setInt(int64_t value)
    {
        ASSERT_MESSAGE(true, "Method not supported");
        return STS_ERR_NOT_EXIST;
    }

    virtual uint64_t getUint() const
    {
        ASSERT_MESSAGE(true, "Method not supported");
        return 0;
    }

    virtual Status setUint(uint64_t value)
    {
        ASSERT_MESSAGE(true, "Method not supported");
        return STS_ERR_NOT_EXIST;
    }

    virtual uint8_t getBool() const
    {
        ASSERT_MESSAGE(true, "Method not supported");
        return 0;
    };

    virtual Status setBool(uint8_t value)
    {
        ASSERT_MESSAGE(true, "Method not supported");
        return STS_ERR_NOT_EXIST;
    }

protected:
    ResourceDesc& mDesc;

    ResourceInstance(ItemBase* parent, ResourceDesc& desc);
    virtual ~ResourceInstance();
    void valueChanged();

private:
    friend class Resource;
    friend class Lwm2mStorage<ResourceInstance, ResourceDesc&>;
    friend class Lwm2mDynamicStorage<ResourceInstance, ResourceDesc&>;

    typedef Lwm2mDynamicStorage<ResourceInstance, ResourceDesc&> Storage;

    void init();
    void release();
};

class ResourceString : public ResourceInstance {
public:
    const char* getString() const { return mValue; }
    Status setString(const char* value);

private:
    friend class Resource;

    char* mValue;

    ResourceString(ItemBase* parent, ResourceDesc& desc);
    virtual ~ResourceString();
};

class ResourceInt : public ResourceInstance {
public:
    int64_t getInt() const { return mValue; }
    Status setInt(int64_t value);

private:
    friend class Resource;

    int64_t mValue;

    ResourceInt(ItemBase* parent, ResourceDesc& desc);
    virtual ~ResourceInt();
};

class ResourceUint : public ResourceInstance {
public:
    uint64_t getUint() const { return mValue; }
    Status setUint(uint64_t value);

private:
    friend class Resource;

    uint64_t mValue;

    ResourceUint(ItemBase* parent, ResourceDesc& desc);
    virtual ~ResourceUint();
};

class ResourceBool : public ResourceInstance {
public:
    uint8_t getBool() const { return mValue; }
    Status setBool(uint8_t value);

private:
    friend class Resource;

    uint8_t mValue;

    ResourceBool(ItemBase* parent, ResourceDesc& desc);
    virtual ~ResourceBool();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_RESOURCEINSTANCE_HPP_ */