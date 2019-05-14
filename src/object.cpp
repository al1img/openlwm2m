#include "object.hpp"
#include "log.hpp"

#define LOG_MODULE "Object"

namespace openlwm2m {

Object::Object(uint16_t id, int maxInstances, bool mandatory, uint16_t interfaces)
    : mId(id), mMaxInstances(maxInstances), mMandatory(mandatory), mInterfaces(interfaces)
{
    LOG_DEBUG("Create object %d", id);
}

Object::~Object() { LOG_DEBUG("Delete object %d", mId); }

}  // namespace openlwm2m
