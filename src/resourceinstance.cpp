#include "resourceinstance.hpp"

#include <cstring>

#include "log.hpp"

#define LOG_MODULE "ResourceInstance"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

Status ResourceInstance::setString(const char* value)
{
    LOG_DEBUG("Set string /%d/%d/%d/%d, value: %s", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId(), value);

    ASSERT_MESSAGE(mDesc.mParams.type == ResourceDesc::TYPE_STRING, "Method not supported");

    if (strcmp(value, mValueString) == 0) {
        return STS_OK;
    }

#if !CONFIG_RESERVE_MEMORY
    delete[] mValueString;
    mValueString = new char[(mDesc.mParams.maxUint ? mDesc.mParams.maxUint : strlen(value)) + 1];
#endif

    strncpy(mValueString, value, (mDesc.mParams.maxUint ? mDesc.mParams.maxUint : strlen(value)) + 1);

    valueChanged();

    return STS_OK;
}

Status ResourceInstance::setInt(int64_t value)
{
    LOG_DEBUG("Set int /%d/%d/%d/%d, value: %ld", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId(), value);

    ASSERT_MESSAGE(mDesc.mParams.type == ResourceDesc::TYPE_INT, "Method not supported");

    if (value == mValueInt) {
        return STS_OK;
    }

    if (value < mDesc.mParams.minInt || value > mDesc.mParams.maxInt) {
        return STS_ERR_INVALID_VALUE;
    }

    mValueInt = value;

    valueChanged();

    return STS_OK;
}

Status ResourceInstance::setUint(uint64_t value)
{
    LOG_DEBUG("Set uint /%d/%d/%d/%d, value: %ld", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId(), value);

    ASSERT_MESSAGE(mDesc.mParams.type == ResourceDesc::TYPE_UINT, "Method not supported");

    if (value == mValueUint) {
        return STS_OK;
    }

    if (value < mDesc.mParams.minUint || value > mDesc.mParams.maxUint) {
        return STS_ERR_INVALID_VALUE;
    }

    mValueUint = value;

    valueChanged();

    return STS_OK;
}

Status ResourceInstance::setBool(uint8_t value)
{
    LOG_DEBUG("Set instance /%d/%d/%d/%d, value: %u", getParent()->getParent()->getParent()->getId(),
              getParent()->getParent()->getId(), getParent()->getId(), getId(), value);

    ASSERT_MESSAGE(mDesc.mParams.type == ResourceDesc::TYPE_BOOL, "Method not supported");

    if (value == mValueBool) {
        return STS_OK;
    }

    mValueBool = value;

    valueChanged();

    return STS_OK;
}

/*******************************************************************************
 * Private
 ******************************************************************************/

ResourceInstance::ResourceInstance(ItemBase* parent, uint16_t id, ResourceDesc& desc)
    : ItemBase(parent, id), mDesc(desc)
{
#if CONFIG_RESERVE_MEMORY
    if (mDesc.mParams.type == ResourceDesc::TYPE_STRING) {
        mValueString = new char[(mDesc.mParams.maxUint ? mDesc.mParams.maxUint : CONFIG_DEFAULT_STRING_LEN) + 1];
    }
#endif
}

ResourceInstance::~ResourceInstance()
{
    if (mDesc.mParams.type == ResourceDesc::TYPE_STRING) {
        delete[] mValueString;
    }
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

/*******************************************************************************
 * ResourceInstanceString
 ******************************************************************************/
#if 0


Status ResourceInstanceInt::setInt(int64_t value)
{
}

/*******************************************************************************
 * ResourceInstanceInt
 ******************************************************************************/

Status ResourceInstanceBool::setBool(uint8_t value)
{
}
#endif
}  // namespace openlwm2m