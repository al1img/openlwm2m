#include "resource.hpp"
#include "log.hpp"

#define LOG_MODULE "Resource"

namespace openlwm2m {

ResourceDesc::ResourceDesc(uint16_t id, uint16_t operations, ResourceInstance instance, int maxInstances,
                           ResourceMandatory mandatory, ResourceType type, int min, int max)
    : mId(id),
      mOperations(operations),
      mInstance(instance),
      mMaxInstances(maxInstances),
      mMandatory(mandatory),
      mType(type),
      mMin(min),
      mMax(max)
{
    LOG_DEBUG("Create resource %d", mId);
}

ResourceDesc::~ResourceDesc() { LOG_DEBUG("Delete resource %d", mId); }

}  // namespace openlwm2m
