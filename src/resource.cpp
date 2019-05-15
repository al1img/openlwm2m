#include "resource.hpp"
#include "log.hpp"

#define LOG_MODULE "Resource"

namespace openlwm2m {

Resource::Resource(uint16_t id, uint16_t operations, int maxInstances, bool mandatory, ResourceType type, int min,
                   int max)
    : mId(id),
      mOperations(operations),
      mMaxInstances(maxInstances),
      mMandatory(mandatory),
      mType(type),
      mMin(min),
      mMax(max)
{
    LOG_DEBUG("Create resource %d", id);
}

Resource::~Resource() { LOG_DEBUG("Delete resource %d", mId); }

}  // namespace openlwm2m
