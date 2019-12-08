#ifndef OPENLWM2M_RESOURCEINSTANCE_HPP_
#define OPENLWM2M_RESOURCEINSTANCE_HPP_

#include <stdint.h>

#include "dataformat.hpp"
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

    Status write(DataConverter::ResourceData* resourceData);
    Status read(DataConverter::ResourceData* resourceData);
    Status write(DataConverter* converter, bool checkOperation);
    Status read(DataConverter* converter, bool checkOperation);

    Status setString(const char* value);
    Status setInt(int64_t value);
    Status setUint(uint64_t value);
    Status setFloat(double value);
    Status setBool(uint8_t value);

    const char* getString();
    int64_t getInt();
    uint64_t getUint();
    double getFloat();
    uint8_t getBool();

    // TODO: rest of types

protected:
    void valueChanged();
};

class ResourceString : public ResourceInstance {
public:
    ResourceString(Resource* parent);
    ~ResourceString();

    const char* getValue() const { return mValue; }
    Status checkValue(const char* value);
    Status setValue(const char* value);

private:
    char* mValue;
    size_t mSize;
};

class ResourceInt : public ResourceInstance {
public:
    ResourceInt(Resource* parent);
    ~ResourceInt();

    int64_t getValue() const { return mValue; }
    Status checkValue(int64_t value);
    Status setValue(int64_t value);

private:
    int64_t mValue;
};

class ResourceUint : public ResourceInstance {
public:
    ResourceUint(Resource* parent);
    ~ResourceUint();

    uint64_t getValue() const { return mValue; }
    Status checkValue(uint64_t value);
    Status setValue(uint64_t value);

private:
    uint64_t mValue;
};

class ResourceFloat : public ResourceInstance {
public:
    ResourceFloat(Resource* parent);
    ~ResourceFloat();

    double getValue() const { return mValue; }
    Status checkValue(double value);
    Status setValue(double value);

private:
    double mValue;
};

class ResourceBool : public ResourceInstance {
public:
    ResourceBool(Resource* parent);
    ~ResourceBool();

    uint8_t getValue() const { return mValue; }
    Status setValue(uint8_t value);

private:
    uint8_t mValue;
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_RESOURCEINSTANCE_HPP_ */