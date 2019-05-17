#ifndef OPENLWM2M_RESOURCE_HPP_
#define OPENLWM2M_RESOURCE_HPP_

#include "interface.hpp"
#include "lwm2mbase.hpp"
#include "resourcedesc.hpp"
#include "resourceinstance.hpp"
#include "status.hpp"

namespace openlwm2m {

class Resource : public Lwm2mBase {
    ResourceInstance* createInstance(Interface interface, Status* status = NULL);
    bool hasFreeInstance();

private:
    friend class ObjectInstance;

    ResourceDesc& mDesc;

    Resource(Lwm2mBase* parent, ResourceDesc& desc);
    ~Resource();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_RESOURCE_HPP_ */