#include "resource.hpp"
#include "log.hpp"

#define LOG_MODULE "Resource"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

ResourceInstance* Resource::createInstance(uint16_t id, Status* status)
{
    // Check size
    if (!mInstanceStorage.hasFreeItem()) {
        if (status) *status = STS_ERR_MEM;
        return NULL;
    }

    // Create instance
    return mInstanceStorage.newItem(id, mDesc, status);
}

/*******************************************************************************
 * Private
 ******************************************************************************/

Resource::Resource(ItemBase* parent, uint16_t id, ResourceDesc& desc)
    : ItemBase(parent, id), mDesc(desc), mInstanceStorage(this, mDesc, mDesc.mParams.mMaxInstances)
{
}

Resource::~Resource()
{
}

void Resource::init()
{
    LOG_DEBUG("Create resource /%d/%d/%d", getParent()->getParent()->getId(), getParent()->getId(), getId());

    // Appendix D.1
    // If the Resource field “Mandatory” is “Mandatory” and the field “Instances” of theResource is “Single” then, the
    // number of Resource Instance MUST be 1
    if (mDesc.mParams.mMandatory == ResourceDesc::MANDATORY && mDesc.mParams.mInstance == ResourceDesc::SINGLE &&
        mInstanceStorage.size() == 0) {
        ResourceInstance* instance = createInstance();
        ASSERT(instance);
    }
}

void Resource::release()
{
    LOG_DEBUG("Delete resource /%d/%d/%d", getParent()->getParent()->getId(), getParent()->getId(), getId());

    mInstanceStorage.clear();
}

}  // namespace openlwm2m
