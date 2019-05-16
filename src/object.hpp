#ifndef OPENLWM2M_OBJECT_HPP_
#define OPENLWM2M_OBJECT_HPP_

#include <stdint.h>

#include "interface.hpp"
#include "list.hpp"
#include "objectinstance.hpp"
#include "resource.hpp"
#include "status.hpp"

namespace openlwm2m {

class Object {
public:
    enum Mandatory { MANDATORY, OPTIONAL };

    enum Instance { SINGLE, MULTIPLE };

    Status createResource(uint16_t id, uint16_t operations, Resource::Instance instance, size_t maxInstances,
                          Resource::Mandatory mandatory, Resource::Type type, int min = 0, int max = 0);

private:
    friend class Client;

    uint16_t mId;
    Instance mInstance;
    size_t mMaxInstances;
    Mandatory mMandatory;
    uint16_t mInterfaces;

    bool mStarted;

    List mResourceDescList;
    List mInstanceList;

    Object(uint16_t id, Instance instance, size_t maxInstances, Mandatory mandatory, uint16_t interfaces);
    ~Object();

    Status start();

    int getInstanceCount();
    ObjectInstance *createInstance(Interface interface, Status *status = NULL);

    void deleteInstances();
    void deleteResources();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_OBJECT_HPP_ */