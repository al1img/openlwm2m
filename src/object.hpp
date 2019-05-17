#ifndef OPENLWM2M_OBJECT_HPP_
#define OPENLWM2M_OBJECT_HPP_

#include <stdint.h>

#include "interface.hpp"
#include "list.hpp"
#include "lwm2mbase.hpp"
#include "objectinstance.hpp"
#include "resourcedesc.hpp"
#include "status.hpp"
#ifdef RESERVE_MEMORY
#include "storage.hpp"
#endif

namespace openlwm2m {

/**
 * lwm2m object.
 */
class Object : public Lwm2mBase {
public:
    enum Mandatory { MANDATORY, OPTIONAL };

    enum Instance { SINGLE, MULTIPLE };

    Status createResource(uint16_t id, uint16_t operations, ResourceDesc::Instance instance, size_t maxInstances,
                          ResourceDesc::Mandatory mandatory, ResourceDesc::Type type, int min = 0, int max = 0);

    ObjectInstance *createInstance(Status *status = NULL);

    bool hasFreeInstance();

private:
    friend class Client;

    Instance mInstance;
    size_t mMaxInstances;
    Mandatory mMandatory;
    uint16_t mInterfaces;

    bool mStarted;

    List mResourceDescList;
#ifdef RESERVE_MEMORY
    Storage mInstanceStorage;
#else
    List mInstanceList;
#endif

    Object(uint16_t id, Instance instance, size_t maxInstances, Mandatory mandatory, uint16_t interfaces);
    ~Object();

    Status start();

    size_t getInstanceCount();

    void deleteInstances();
    void deleteResources();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_OBJECT_HPP_ */