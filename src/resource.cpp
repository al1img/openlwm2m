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

Resource::Resource(Lwm2mBase* parent, uint16_t id, ResourceDesc& desc)
    : Lwm2mBase(parent, id), mDesc(desc), mInstanceStorage(this, mDesc, mDesc.mParams.mMaxInstances)
{
    LOG_DEBUG("Create resource /%d/%d/%d", getParent()->getParent()->getId(), getParent()->getId(), getId());

    // Appendix D.1
    // If the Resource field “Mandatory” is “Mandatory” and the field “Instances” of theResource is “Single” then, the
    // number of Resource Instance MUST be 1
    if (desc.mParams.mMandatory == ResourceDesc::MANDATORY && desc.mParams.mInstance == ResourceDesc::SINGLE &&
        mInstanceStorage.size() == 0) {
        ResourceInstance* instance = createInstance(LWM2M_INVALID_ID);
        LWM2M_ASSERT(instance);
    }
}

Resource::~Resource()
{
    LOG_DEBUG("Delete resource /%d/%d/%d", getParent()->getParent()->getId(), getParent()->getId(), getId());
}

}  // namespace openlwm2m
