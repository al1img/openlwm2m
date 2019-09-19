#ifndef OPENLWM2M_RESOURCEINSTANCE_HPP_
#define OPENLWM2M_RESOURCEINSTANCE_HPP_

#include <stdint.h>

#include "itembase.hpp"
#include "storage.hpp"

namespace openlwm2m {

class Resource;

class ResourceInstance : public ItemBase {
public:
    typedef Lwm2mDynamicStorage<ResourceInstance> Storage;

    ResourceInstance(Resource* parent);
    ~ResourceInstance();

    void init();
    void release();

    Resource* getResource() const;

protected:
    void valueChanged();
};

class ResourceString : public ResourceInstance {
public:
    ResourceString(Resource* parent);
    ~ResourceString();

    const char* getString() const { return mValue; }
    Status checkString(const char* value);
    Status setString(const char* value);

private:
    char* mValue;
    size_t mSize;
};

class ResourceInt : public ResourceInstance {
public:
    ResourceInt(Resource* parent);
    ~ResourceInt();

    int64_t getInt() const { return mValue; }
    Status checkInt(int64_t value);
    Status setInt(int64_t value);

private:
    int64_t mValue;
};

class ResourceUint : public ResourceInstance {
public:
    ResourceUint(Resource* parent);
    ~ResourceUint();

    uint64_t getUint() const { return mValue; }
    Status checkUint(uint64_t value);
    Status setUint(uint64_t value);

private:
    uint64_t mValue;
};

class ResourceBool : public ResourceInstance {
public:
    ResourceBool(Resource* parent);
    ~ResourceBool();

    uint8_t getBool() const { return mValue; }
    Status setBool(uint8_t value);

private:
    uint8_t mValue;
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_RESOURCEINSTANCE_HPP_ */