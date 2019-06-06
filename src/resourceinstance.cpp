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
    mValue = new char[(desc.mParams.max ? desc.mParams.max : CONFIG_DEFAULT_STRING_LEN) + 1];
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

void ResourceInstanceString::setString(const char* value)
{
    LOG_DEBUG("Set string instance /%d/%d/%d/%d, value: %s", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId(), value);

#if CONFIG_RESERVE_MEMORY
    strncpy(mValue, value, (mDesc.mParams.max ? mDesc.mParams.max : CONFIG_DEFAULT_STRING_LEN) + 1);
#else
    delete[] mValue;
    mValue = new char[strlen(value) + 1];
    strcpy(mValue, value);
#endif
}

}  // namespace openlwm2m