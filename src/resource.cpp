#include "resource.hpp"
#include "log.hpp"

#define LOG_MODULE "Resource"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

ResourceInstance* Resource::createInstance(uint16_t id, Status* status)
{
    return mInstanceStorage.newItem(id, mDesc, status);
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
    mInstanceNode = mInstanceStorage.begin();

    if (mInstanceNode) {
        return mInstanceNode->get();
    }

    return NULL;
}

ResourceInstance* Resource::getNextInstance()
{
    if (mInstanceNode) {
        mInstanceNode = mInstanceNode->next();

        if (mInstanceNode) {
            return mInstanceNode->get();
        }
    }

    return NULL;
}

/*******************************************************************************
 * Private
 ******************************************************************************/

Resource::Resource(ItemBase* parent, uint16_t id, ResourceDesc& desc)
    : ItemBase(parent, id),
      mDesc(desc),
      mInstanceStorage(this, mDesc, mDesc.mParams.maxInstances, &ResourceInstance::newInstance)
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
