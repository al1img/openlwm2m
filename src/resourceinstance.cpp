#include "resourceinstance.hpp"

#include <cstring>

#include "log.hpp"

#define LOG_MODULE "ResourceInstance"

namespace openlwm2m {

/*******************************************************************************
 * ResourceInstance
 ******************************************************************************/

ResourceInstance::ResourceInstance(ItemBase* parent, uint16_t id, ResourceDesc& desc)
    : ItemBase(parent, id), mDesc(desc)
{
}

ResourceInstance::~ResourceInstance()
{
}

void ResourceInstance::init()
{
    LOG_DEBUG("Create resource instance /%d/%d/%d/%d", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId());
}
void ResourceInstance::release()
{
    LOG_DEBUG("Delete resource instance /%d/%d/%d/%d", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId());
}

ResourceInstance* ResourceInstance::newInstance(ItemBase* parent, uint16_t id, ResourceDesc& desc)
{
    switch (desc.mParams.type) {
        case ResourceDesc::TYPE_STRING:
            return new ResourceInstanceString(parent, id, desc);
    }

    return new ResourceInstance(parent, id, desc);
}

/*******************************************************************************
 * ResourceInstanceString
 ******************************************************************************/

ResourceInstanceString::ResourceInstanceString(ItemBase* parent, uint16_t id, ResourceDesc& desc)
    : ResourceInstance(parent, id, desc), mValue(NULL)
{
#if CONFIG_RESERVE_MEMORY
    mValue = new char[(desc.mParams.maxUint ? desc.mParams.maxUint : CONFIG_DEFAULT_STRING_LEN) + 1];
#endif
}

ResourceInstanceString::~ResourceInstanceString()
{
    delete[] mValue;
}

void ResourceInstanceString::init()
{
    LOG_DEBUG("Create string instance /%d/%d/%d/%d", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId());
}
void ResourceInstanceString::release()
{
    LOG_DEBUG("Delete string instance /%d/%d/%d/%d", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId());
}

const char* ResourceInstanceString::getString()
{
    LOG_DEBUG("Get string instance /%d/%d/%d/%d, value: %s", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId(), mValue);

    return mValue;
}

Status ResourceInstanceString::setString(const char* value)
{
    LOG_DEBUG("Set string instance /%d/%d/%d/%d, value: %s", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId(), value);

#if !CONFIG_RESERVE_MEMORY
    delete[] mValue;
    mValue = new char[(mDesc.mParams.maxUint ? mDesc.mParams.maxUint : strlen(value)) + 1];
#endif

    strncpy(mValue, value, (mDesc.mParams.maxUint ? mDesc.mParams.maxUint : strlen(value)) + 1);

    return STS_OK;
}

/*******************************************************************************
 * ResourceInstanceInt
 ******************************************************************************/

ResourceInstanceInt::ResourceInstanceInt(ItemBase* parent, uint16_t id, ResourceDesc& desc)
    : ResourceInstance(parent, id, desc), mValue(0)
{
}

ResourceInstanceInt::~ResourceInstanceInt()
{
}

void ResourceInstanceInt::init()
{
    LOG_DEBUG("Create integer instance /%d/%d/%d/%d", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId());
}

void ResourceInstanceInt::release()
{
    LOG_DEBUG("Delete integer instance /%d/%d/%d/%d", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId());
}

int64_t ResourceInstanceInt::getInt()
{
    LOG_DEBUG("Get integer instance /%d/%d/%d/%d, value: %l", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId(), mValue);

    return mValue;
}

Status ResourceInstanceInt::setInt(int64_t value)
{
    LOG_DEBUG("Set string instance /%d/%d/%d/%d, value: %l", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId(), value);

    if (value < mDesc.mParams.minInt || value > mDesc.mParams.maxInt) {
        return STS_ERR_INVALID_VALUE;
    }

    mValue = value;

    return STS_OK;
}

}  // namespace openlwm2m