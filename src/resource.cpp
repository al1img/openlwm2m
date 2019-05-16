#include "resource.hpp"
#include "log.hpp"

#define LOG_MODULE "Resource"

namespace openlwm2m {

ResourceDesc::ResourceDesc(uint16_t id, uint16_t objectId, uint16_t operations, Resource::Instance instance,
                           size_t maxInstances, Resource::Mandatory mandatory, Resource::Type type, int min, int max)
    : mId(id),
      mObjectId(objectId),
      mOperations(operations),
      mInstance(instance),
      mMaxInstances(maxInstances),
      mMandatory(mandatory),
      mType(type),
      mMin(min),
      mMax(max)
{
    LOG_DEBUG("Create resource desc /%d/%d", mObjectId, mId);
}

ResourceDesc::~ResourceDesc() { LOG_DEBUG("Delete resource desc /%d/%d", mObjectId, mId); }

Resource::Resource(uint16_t objectInstanceId, ResourceDesc& desc) : mObjectInstanceId(objectInstanceId), mDesc(desc)
{
    LOG_DEBUG("Create resource /%d/%d/%d", mDesc.mObjectId, mObjectInstanceId, mDesc.mId);
}

Resource::~Resource() { LOG_DEBUG("Delete resource /%d/%d/%d", mDesc.mObjectId, mObjectInstanceId, mDesc.mId); }

}  // namespace openlwm2m
