#include "resourceinstance.hpp"

#include <cstring>

#include "log.hpp"

#define LOG_MODULE "ResourceInstance"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

/*******************************************************************************
 * ResourceString
 ******************************************************************************/

ResourceString::ResourceString(ItemBase* parent, ResourceDesc& desc) : ResourceInstance(parent, desc)
{
    mSize = mDesc.mParams.maxUint ? mDesc.mParams.maxUint : CONFIG_DEFAULT_STRING_LEN;
    mValue = new char[mSize + 1];
    mValue[0] = '\0';
}

ResourceString::~ResourceString()
{
    delete[] mValue;
}

Status ResourceString::setString(const char* value)
{
    LOG_INFO("Set string /%d/%d/%d/%d, value: %s", getParent()->getParent()->getParent()->getId(),
             getParent()->getParent()->getId(), getParent()->getId(), getId(), value);

    if (strcmp(value, mValue) == 0) {
        return STS_OK;
    }

#if !CONFIG_RESERVE_MEMORY
    if (!mDesc.mParams.maxUint && strlen(value) > mSize) {
        delete[] mValue;
        mSize = strlen(value);
        mValue = new char[mSize + 1];
    }
#endif

    strncpy(mValue, value, mSize);
    mValue[mSize] = '\0';

    valueChanged();

    return STS_OK;
}

/*******************************************************************************
 * ResourceInt
 ******************************************************************************/

ResourceInt::ResourceInt(ItemBase* parent, ResourceDesc& desc) : ResourceInstance(parent, desc), mValue(0)
{
}

ResourceInt::~ResourceInt()
{
}

Status ResourceInt::setFloat(double value)
{
    int64_t nativeValue = value;

    if (value != nativeValue) {
        return STS_ERR_INVALID_VALUE;
    }

    return setInt(nativeValue);
}

Status ResourceInt::setInt(int64_t value)
{
    LOG_INFO("Set int /%d/%d/%d/%d, value: %ld", getParent()->getParent()->getParent()->getId(),
             getParent()->getParent()->getId(), getParent()->getId(), getId(), value);

    if (value == mValue) {
        return STS_OK;
    }

    if (value < mDesc.mParams.minInt || value > mDesc.mParams.maxInt) {
        return STS_ERR_INVALID_VALUE;
    }

    mValue = value;

    valueChanged();

    return STS_OK;
}

/*******************************************************************************
 * ResourceUint
 ******************************************************************************/

ResourceUint::ResourceUint(ItemBase* parent, ResourceDesc& desc) : ResourceInstance(parent, desc), mValue(0)
{
}

ResourceUint::~ResourceUint()
{
}

Status ResourceUint::setFloat(double value)
{
    uint64_t nativeValue = value;

    if (value != nativeValue) {
        return STS_ERR_INVALID_VALUE;
    }

    return setUint(nativeValue);
}

Status ResourceUint::setUint(uint64_t value)
{
    LOG_INFO("Set uint /%d/%d/%d/%d, value: %ld", getParent()->getParent()->getParent()->getId(),
             getParent()->getParent()->getId(), getParent()->getId(), getId(), value);

    if (value == mValue) {
        return STS_OK;
    }

    if (value < mDesc.mParams.minUint || value > mDesc.mParams.maxUint) {
        return STS_ERR_INVALID_VALUE;
    }

    mValue = value;

    valueChanged();

    return STS_OK;
}

/*******************************************************************************
 * ResourceBool
 ******************************************************************************/

ResourceBool::ResourceBool(ItemBase* parent, ResourceDesc& desc) : ResourceInstance(parent, desc), mValue(0)
{
}

ResourceBool::~ResourceBool()
{
}

Status ResourceBool::setBool(uint8_t value)
{
    LOG_INFO("Set instance /%d/%d/%d/%d, value: %u", getParent()->getParent()->getParent()->getId(),
             getParent()->getParent()->getId(), getParent()->getId(), getId(), value);

    if (value == mValue) {
        return STS_OK;
    }

    mValue = value;

    valueChanged();

    return STS_OK;
}

/*******************************************************************************
 * ResourceInstance
 ******************************************************************************/

ResourceInstance::ResourceInstance(ItemBase* parent, ResourceDesc& desc) : ItemBase(parent), mDesc(desc)
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

void ResourceInstance::valueChanged()
{
    if (mDesc.mParams.cbk) {
        mDesc.mParams.cbk(mDesc.mParams.context, this);
    }
}

}  // namespace openlwm2m
