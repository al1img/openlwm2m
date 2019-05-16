#include "resourcedesc.hpp"
#include "log.hpp"

#define LOG_MODULE "ResourceDesc"

namespace openlwm2m {

ResourceDesc::ResourceDesc(uint16_t id, uint16_t objectId, uint16_t operations, ResourceDesc::Instance instance,
                           size_t maxInstances, ResourceDesc::Mandatory mandatory, ResourceDesc::Type type, int min,
                           int max)
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

}  // namespace openlwm2m