#include "resource.hpp"
#include "log.hpp"
#include "objectinstance.hpp"

#define LOG_MODULE "Resource"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

ResourceInfo::ResourceInfo(uint16_t id, uint16_t operations, ItemInstance instance, size_t maxInstances,
                           ItemMandatory mandatory, DataType type, Min min, Max max)
    : ItemBase(NULL, id),
      mOperations(operations),
      mInstance(instance),
      mMaxInstances(maxInstances),
      mMandatory(mandatory),
      mType(type),
      mMin(min),
      mMax(max),
      mCallback(NULL),
      mContext(NULL)
{
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

void ResourceInfo::setValueChangedCbk(ValueChangeCbk callback, void* context)
{
    mCallback = callback;
    mContext = context;
}

void ResourceInfo::valueChanged(ResourceInstance* instance)
{
    if (mCallback) {
        mCallback(mContext, instance);
    }
}

/*******************************************************************************
 * Public
 ******************************************************************************/

Resource::Resource(ObjectInstance* parent, ResourceInfo& info) : ItemBase(parent), mInfo(info)
{
    for (size_t i = 0; i < info.getMaxInstances(); i++) {
        mInstanceStorage.addItem(newInstance(this));
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
        createInstance(0);
    }
}

void Resource::release()
{
    LOG_DEBUG("Delete /%d/%d/%d", getParent()->getParent()->getId(), getParent()->getId(), getId());

    mInstanceStorage.clear();
}

ObjectInstance* Resource::getObjectInstance() const
{
    return static_cast<ObjectInstance*>(getParent());
}

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

ResourceInstance* Resource::newInstance(Resource* parent)
{
    switch (parent->getInfo().getType()) {
        case ResourceInfo::TYPE_STRING:
            return new ResourceString(parent);

        case ResourceInfo::TYPE_INT:
            return new ResourceInt(parent);

        case ResourceInfo::TYPE_UINT:
            return new ResourceUint(parent);

        case ResourceInfo::TYPE_BOOL:
            return new ResourceBool(parent);

        default:
            return new ResourceInstance(parent);
    }
}

}  // namespace openlwm2m
