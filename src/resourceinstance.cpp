#include "resourceinstance.hpp"

#include <cstring>

#include "log.hpp"

#define LOG_MODULE "ResourceInstance"

namespace openlwm2m {

/*******************************************************************************
 * ResourceInstance
 ******************************************************************************/

/*******************************************************************************
 * Public
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
    LOG_DEBUG("Create /%d/%d/%d/%d", getParent()->getParent()->getParent()->getId(), getParent()->getParent()->getId(),
              getParent()->getId(), getId());
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

ResourceInstance* ResourceInstance::newInstance(ItemBase* parent, uint16_t id, ResourceDesc& desc)
{
    switch (desc.mParams.type) {
        case ResourceDesc::TYPE_STRING:
            return new ResourceInstanceString(parent, id, desc);

        case ResourceDesc::TYPE_INT:
            return new ResourceInstanceInt(parent, id, desc);

        case ResourceDesc::TYPE_BOOL:
            return new ResourceInstanceBool(parent, id, desc);
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
    LOG_DEBUG("Create string /%d/%d/%d/%d", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId());

    valueChanged();
}

void ResourceInstanceString::release()
{
    LOG_DEBUG("Delete string /%d/%d/%d/%d", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId());
}

Status ResourceInstanceString::setString(const char* value)
{
    LOG_DEBUG("Set string /%d/%d/%d/%d, value: %s", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId(), value);

    if (strcmp(value, mValue) == 0) {
        return STS_OK;
    }

#if !CONFIG_RESERVE_MEMORY
    delete[] mValue;
    mValue = new char[(mDesc.mParams.maxUint ? mDesc.mParams.maxUint : strlen(value)) + 1];
#endif

    strncpy(mValue, value, (mDesc.mParams.maxUint ? mDesc.mParams.maxUint : strlen(value)) + 1);

    valueChanged();

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
    LOG_DEBUG("Create int /%d/%d/%d/%d", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId());

    valueChanged();
}

void ResourceInstanceInt::release()
{
    LOG_DEBUG("Delete int /%d/%d/%d/%d", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId());
}

Status ResourceInstanceInt::setInt(int64_t value)
{
    LOG_DEBUG("Set int /%d/%d/%d/%d, value: %ld", getParent()->getParent()->getParent()->getId(),
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
 * ResourceInstanceInt
 ******************************************************************************/

ResourceInstanceBool::ResourceInstanceBool(ItemBase* parent, uint16_t id, ResourceDesc& desc)
    : ResourceInstance(parent, id, desc), mValue(0)
{
}

ResourceInstanceBool::~ResourceInstanceBool()
{
}

void ResourceInstanceBool::init()
{
    LOG_DEBUG("Create bool /%d/%d/%d/%d", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId());

    valueChanged();
}

void ResourceInstanceBool::release()
{
    LOG_DEBUG("Delete bool /%d/%d/%d/%d", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId());
}

Status ResourceInstanceBool::setBool(uint8_t value)
{
    LOG_DEBUG("Set instance /%d/%d/%d/%d, value: %u", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId(), value);

    if (value == mValue) {
        return STS_OK;
    }

    mValue = value;

    valueChanged();

    return STS_OK;
}

}  // namespace openlwm2m