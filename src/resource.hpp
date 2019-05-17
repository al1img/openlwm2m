#ifndef OPENLWM2M_RESOURCE_HPP_
#define OPENLWM2M_RESOURCE_HPP_

#include "interface.hpp"
#include "lwm2mbase.hpp"
#include "resourcedesc.hpp"
#include "resourceinstance.hpp"
#include "status.hpp"
#ifdef RESERVE_MEMORY
#include "storage.hpp"
#else
#include "list.hpp"
#endif

namespace openlwm2m {

class Resource : public Lwm2mBase {
public:
    ResourceInstance* createInstance(Status* status = NULL);
    bool hasFreeInstance();

private:
    friend class ObjectInstance;

    ResourceDesc& mDesc;
#ifdef RESERVE_MEMORY
    Storage mInstanceStorage;
#else
    List mInstanceList;
#endif

    Resource(Lwm2mBase* parent, ResourceDesc& desc);
    ~Resource();

    void deleteInstances();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_RESOURCE_HPP_ */