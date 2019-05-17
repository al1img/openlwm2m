#include "resource.hpp"
#include "log.hpp"

#define LOG_MODULE "Resource"

namespace openlwm2m {

Resource::Resource(Lwm2mBase* parent, ResourceDesc& desc)
    : Lwm2mBase(parent, desc.getId()),
      mDesc(desc)
#ifdef RESERVE_MEMORY
      ,
      mInstanceStorage(mDesc.mMaxInstances)
#endif
{
    LOG_DEBUG("Create resource /%d/%d/%d", getParent()->getParent()->getId(), getParent()->getId(), getId());

#ifdef RESERVE_MEMORY
    for (size_t i = 0; i < mDesc.mMaxInstances; i++) {
        ResourceInstance* instance = new ResourceInstance(this, i, mDesc);
        mInstanceStorage.pushItem(instance);
    }
#endif

    // Appendix D.1
    // If the Resource field “Mandatory” is “Mandatory” and the field “Instances” of theResource is “Single” then, the
    // number of Resource Instance MUST be 1
    if (mDesc.mMandatory == ResourceDesc::MANDATORY && mDesc.mInstance == ResourceDesc::SINGLE) {
        Status status;

        createInstance(&status);
        if (status != STS_OK) {
            LOG_ERROR("Can't create resource instance, status: %d", status);
        }
    }
}

Resource::~Resource()
{
    LOG_DEBUG("Delete resource /%d/%d/%d", getParent()->getParent()->getId(), getParent()->getId(), getId());

    deleteInstances();
}

bool Resource::hasFreeInstance()
{
#ifdef RESERVE_MEMORY
    return mInstanceStorage.size() < mInstanceStorage.maxSize();
#else
    return mDesc.mMaxInstances == 0 || mInstanceList.size() < mDesc.mMaxInstances;
#endif
}

ResourceInstance* Resource::createInstance(Status* status)
{
    if (status) {
        *status = STS_OK;
    }

    // Check size
    if (!hasFreeInstance()) {
        if (status) {
            *status = STS_ERR_MEM;
        }

        return NULL;
    }

    ResourceInstance* instance;

#ifdef RESERVE_MEMORY

    instance = static_cast<ResourceInstance*>(mInstanceStorage.newItem(status));

    if (instance) {
        LOG_DEBUG("Create resource instance /%d/%d/%d/%d", getParent()->getParent()->getId(), getParent()->getId(),
                  getId(), instance->getId());
    }

#else
    Node* node;
    uint16_t id = 0;

    // Get free id
    node = mInstanceList.begin();

    while (node) {
        instance = static_cast<ResourceInstance*>(node->get());

        if (id < instance->getId()) {
            break;
        }

        id++;
        node = node->next();
    }

    // Create instance
    instance = new ResourceInstance(this, id, mDesc);

    if (node) {
        mInstanceList.insert(instance, node);
    }
    else {
        mInstanceList.append(instance);
    }

#endif

    return instance;
}

void Resource::deleteInstances()
{
#ifdef RESERVE_MEMORY
    ResourceInstance* instance = static_cast<ResourceInstance*>(mInstanceStorage.begin(true));

    while (instance) {
        delete instance;
        instance = static_cast<ResourceInstance*>(mInstanceStorage.next(true));
    }
#else
    Node* node = mInstanceList.begin();

    while (node) {
        delete static_cast<ResourceInstance*>(node->get());
        node = node->next();
    }
#endif
}

}  // namespace openlwm2m
