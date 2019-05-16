#ifndef OPENLWM2M_OBJECT_HPP_
#define OPENLWM2M_OBJECT_HPP_

#include <stdint.h>

#include "list.hpp"
#include "resource.hpp"

namespace openlwm2m {

enum ObjectMandatory { OBJ_MANDATORY, OBJ_OPTIONAL };

enum ObjectInstance { OBJ_SINGLE, OBJ_MULTIPLE };

class Object {
public:
    void createResource(uint16_t id, uint16_t operations, ResourceInstance instance, int maxInstances,
                        ResourceMandatory mandatory, ResourceType type, int min = 0, int max = 0);

private:
    friend class Client;

    uint16_t mId;
    ObjectInstance mInstance;
    int mMaxInstances;
    ObjectMandatory mMandatory;
    uint16_t mInterfaces;

    List mResourceDescList;

    Object(uint16_t id, ObjectInstance instance, int maxInstances, ObjectMandatory mandatory, uint16_t interfaces);
    ~Object();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_OBJECT_HPP_ */