#include "resource.hpp"
#include "log.hpp"
#include "objectinstance.hpp"

#define LOG_MODULE "Resource"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

ResourceInfo::ResourceInfo(uint16_t id, uint16_t operations, DataType type, bool single, bool mandatory,
                           size_t maxInstances, Min min, Max max)
    : ItemBase(NULL, id),
      mOperations(operations),
      mType(type),
      mSingle(single),
      mMandatory(mandatory),
      mMaxInstances(maxInstances),
      mMin(min),
      mMax(max),
      mCallback(NULL),
      mContext(NULL)
{
    if (mSingle) {
        mMaxInstances = 1;
    }
}

ResourceInfo::~ResourceInfo()
{
}

void ResourceInfo::init()
{
}

void ResourceInfo::release()
{
}

void ResourceInfo::setCallback(Callback callback, void* context)
{
    mCallback = callback;
    mContext = context;
}

void ResourceInfo::valueChanged(ResourceInstance* instance)
{
    if (mCallback) {
        LOG_DEBUG("Value of resource instance /%d/%d/%d/%d changed",
                  instance->getParent()->getParent()->getParent()->getId(), instance->getParent()->getParent()->getId(),
                  instance->getParent()->getId(), instance->getId());
        mCallback(mContext, instance);
    }
}

/*******************************************************************************
 * Public
 ******************************************************************************/

Resource::Resource(ObjectInstance* parent, ResourceInfo& info) : ItemBase(parent, info.getId()), mInfo(info)
{
    for (size_t i = 0; i < info.getMaxInstances(); i++) {
        mInstanceStorage.pushItem(newInstance(this));
    }
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
    if (mInfo.isMandatory() && mInfo.isSingle()) {
        ResourceInstance* instance = createInstance(0);
        ASSERT(instance);
    }
}

void Resource::release()
{
    mInstanceStorage.clear();

    LOG_DEBUG("Delete /%d/%d/%d", getParent()->getParent()->getId(), getParent()->getId(), getId());
}

ObjectInstance* Resource::getObjectInstance() const
{
    return static_cast<ObjectInstance*>(getParent());
}

ResourceInstance* Resource::createInstance(uint16_t id, Status* status)
{
    return mInstanceStorage.allocateItem(id, status);
}

Status Resource::deleteInstance(uint16_t id)
{
    ResourceInstance* instance = mInstanceStorage.getItemById(id);

    if (!instance) {
        return STS_ERR_NOT_FOUND;
    }

    return mInstanceStorage.deallocateItem(instance);
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

Status Resource::write(DataConverter* converter, bool checkOperation, bool replace)
{
    return STS_OK;
}

Status Resource::read(DataConverter* converter, bool checkOperation)
{
    Status status = STS_OK;

    if (getInfo().checkOperation(OP_EXECUTE)) {
        return STS_OK;
    }

    if (checkOperation && !getInfo().checkOperation(OP_READ)) {
        return STS_ERR_NO_ACCESS;
    }

    for (ResourceInstance* instance = getFirstInstance(); instance != NULL; instance = getNextInstance()) {
        DataConverter::ResourceData resourceData;

        if ((status = instance->read(&resourceData)) != STS_OK) {
            return status;
        }

        if ((status = converter->nextEncoding(&resourceData)) != STS_OK) {
            return status;
        }
    }

    return STS_OK;
}

/*******************************************************************************
 * Private
 ******************************************************************************/

ResourceInstance* Resource::newInstance(Resource* parent)
{
    switch (parent->getInfo().getType()) {
        case DATA_TYPE_STRING:
            return new ResourceString(parent);

        case DATA_TYPE_INT:
            return new ResourceInt(parent);

        case DATA_TYPE_UINT:
            return new ResourceUint(parent);

        case DATA_TYPE_BOOL:
            return new ResourceBool(parent);

        case DATA_TYPE_FLOAT:
            return new ResourceFloat(parent);

        default:
            return new ResourceInstance(parent);
    }
}

}  // namespace openlwm2m
