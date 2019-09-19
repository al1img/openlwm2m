#include "resourceinstance.hpp"
#include "resource.hpp"

#include <cstring>

#include "log.hpp"

#define LOG_MODULE "ResourceInstance"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

/*******************************************************************************
 * ResourceInstance
 ******************************************************************************/

ResourceInstance::ResourceInstance(Resource* parent) : ItemBase(parent)
{
}

ResourceInstance::~ResourceInstance()
{
}

void ResourceInstance::init()
{
    LOG_DEBUG("Create /%d/%d/%d/%d", getParent()->getParent()->getParent()->getId(), getParent()->getParent()->getId(),
              getParent()->getId(), getId());

    valueChanged();
}

void ResourceInstance::release()
{
    LOG_DEBUG("Delete /%d/%d/%d/%d", getParent()->getParent()->getParent()->getId(), getParent()->getParent()->getId(),
              getParent()->getId(), getId());
}

Resource* ResourceInstance::getResource() const
{
    return static_cast<Resource*>(getParent());
}

void ResourceInstance::valueChanged()
{
    getResource()->getInfo().valueChanged(this);
}

/*******************************************************************************
 * ResourceString
 ******************************************************************************/

ResourceString::ResourceString(Resource* parent) : ResourceInstance(parent)
{
    mSize = getResource()->getInfo().max().maxUint;
    mValue = new char[mSize + 1];
    mValue[0] = '\0';
}

ResourceString::~ResourceString()
{
    delete[] mValue;
}

Status ResourceString::checkString(const char* value)
{
    if (strlen(value) > mSize) {
        return STS_ERR_INVALID_VALUE;
    }

    return STS_OK;
}

Status ResourceString::setString(const char* value)
{
    Status status = STS_OK;

    LOG_INFO("Set string /%d/%d/%d/%d, value: %s", getParent()->getParent()->getParent()->getId(),
             getParent()->getParent()->getId(), getParent()->getId(), getId(), value);

    if ((status = checkString(value)) != STS_OK) {
        return status;
    }

    if (strcmp(value, mValue) == 0) {
        return STS_OK;
    }

    strncpy(mValue, value, mSize);
    mValue[mSize] = '\0';

    valueChanged();

    return STS_OK;
}

/*******************************************************************************
 * ResourceInt
 ******************************************************************************/

ResourceInt::ResourceInt(Resource* parent) : ResourceInstance(parent), mValue(0)
{
}

ResourceInt::~ResourceInt()
{
}

Status ResourceInt::setInt(int64_t value)
{
    LOG_INFO("Set int /%d/%d/%d/%d, value: %ld", getParent()->getParent()->getParent()->getId(),
             getParent()->getParent()->getId(), getParent()->getId(), getId(), value);

    if (value == mValue) {
        return STS_OK;
    }

    if (value < getResource()->getInfo().min().minInt || value > getResource()->getInfo().max().maxInt) {
        return STS_ERR_INVALID_VALUE;
    }

    mValue = value;

    valueChanged();

    return STS_OK;
}

/*******************************************************************************
 * ResourceUint
 ******************************************************************************/

ResourceUint::ResourceUint(Resource* parent) : ResourceInstance(parent), mValue(0)
{
}

ResourceUint::~ResourceUint()
{
}

Status ResourceUint::setUint(uint64_t value)
{
    LOG_INFO("Set uint /%d/%d/%d/%d, value: %ld", getParent()->getParent()->getParent()->getId(),
             getParent()->getParent()->getId(), getParent()->getId(), getId(), value);

    if (value == mValue) {
        return STS_OK;
    }

    if (value < getResource()->getInfo().min().minUint || value > getResource()->getInfo().max().maxUint) {
        return STS_ERR_INVALID_VALUE;
    }

    mValue = value;

    valueChanged();

    return STS_OK;
}

/*******************************************************************************
 * ResourceBool
 ******************************************************************************/

ResourceBool::ResourceBool(Resource* parent) : ResourceInstance(parent), mValue(0)
{
}

ResourceBool::~ResourceBool()
{
}

Status ResourceBool::setBool(uint8_t value)
{
    LOG_INFO("Set bool /%d/%d/%d/%d, value: %u", getParent()->getParent()->getParent()->getId(),
             getParent()->getParent()->getId(), getParent()->getId(), getId(), value);

    if (value == mValue) {
        return STS_OK;
    }

    mValue = value;

    valueChanged();

    return STS_OK;
}

}  // namespace openlwm2m
