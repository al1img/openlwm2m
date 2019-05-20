#ifndef OPENLWM2M_OBJECTINSTANCE_HPP_
#define OPENLWM2M_OBJECTINSTANCE_HPP_

#include <stdint.h>

#include "lwm2mbase.hpp"
#include "lwm2mstorage.hpp"
#include "resource.hpp"
#include "resourcedesc.hpp"

namespace openlwm2m {

class ObjectInstance : public Lwm2mBase {
private:
    friend class Object;
    friend class Lwm2mStorage<ObjectInstance, ResourceDesc::Storage&>;

    typedef Lwm2mStorage<ObjectInstance, ResourceDesc::Storage&> Storage;

    Resource::Storage mResourceStorage;

    ObjectInstance(Lwm2mBase* parent, uint16_t id, ResourceDesc::Storage& resourceDescStorage);
    virtual ~ObjectInstance();

    void create();
    void release();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_OBJECTINSTANCE_HPP_ */