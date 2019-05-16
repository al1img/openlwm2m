#include "resource.hpp"
#include "log.hpp"

#define LOG_MODULE "Resource"

namespace openlwm2m {

Resource::Resource(uint16_t objectInstanceId, ResourceDesc& desc) : mObjectInstanceId(objectInstanceId), mDesc(desc)
{
    LOG_DEBUG("Create resource /%d/%d/%d", mDesc.mObjectId, mObjectInstanceId, mDesc.mId);
}

Resource::~Resource() { LOG_DEBUG("Delete resource /%d/%d/%d", mDesc.mObjectId, mObjectInstanceId, mDesc.mId); }

}  // namespace openlwm2m
