#include "resource.hpp"
#include "log.hpp"

#define LOG_MODULE "Resource"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

ResourceInstance* Resource::createInstance(uint16_t id, Status* status)
{
    return mInstanceStorage.newItem(id, status);
}

Status Resource::deleteInstance(ResourceInstance* instance)
{
    if (instance) {
        return mInstanceStorage.deleteItem(instance);
    }

    return STS_OK;
}

ResourceInstance* Resource::getInstanceById(uint16_t id)
{
    return mInstanceStorage.getItemById(id);
}

ResourceInstance* Resource::getFirstInstance()
{
    return mInstanceStorage.getFirstItem();
}

ResourceInstance* Resource::getNextInstance()
{
    return mInstanceStorage.getNextItem();
}

/*******************************************************************************
 * Private
 ******************************************************************************/

ResourceInstance* Resource::newInstance(ItemBase* parent, ResourceDesc& desc)
{
    switch (desc.mParams.type) {
        case DATA_TYPE_STRING:
            return new ResourceString(parent, desc);

        case DATA_TYPE_INT:
            return new ResourceInt(parent, desc);

        case DATA_TYPE_UINT:
            return new ResourceUint(parent, desc);

        case DATA_TYPE_BOOL:
            return new ResourceBool(parent, desc);

        default:
            return new ResourceInstance(parent, desc);
    }
}

Resource::Resource(ItemBase* parent, ResourceDesc& desc)
    : ItemBase(parent), mDesc(desc), mInstanceStorage(this, mDesc, mDesc.mParams.maxInstances, newInstance)
{
}

Resource::~Resource()
{
}

void Resource::init()
{
    LOG_DEBUG("Create /%d/%d/%d", getParent()->getParent()->getId(), getParent()->getId(), getId());

    // Appendix D.1
    // If the Resource field “Mandatory” is “Mandatory” and the field “Instances” of theResource is “Single” then, the
    // number of Resource Instance MUST be 1
    if (mDesc.mParams.mandatory == ResourceDesc::MANDATORY && mDesc.mParams.instance == ResourceDesc::SINGLE &&
        mInstanceStorage.size() == 0) {
        ResourceInstance* instance = createInstance(0);
        ASSERT(instance);
    }
}

void Resource::release()
{
    LOG_DEBUG("Delete /%d/%d/%d", getParent()->getParent()->getId(), getParent()->getId(), getId());

    mInstanceStorage.clear();
}

}  // namespace openlwm2m
