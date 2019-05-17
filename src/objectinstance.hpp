#ifndef OPENLWM2M_OBJECTINSTANCE_HPP_
#define OPENLWM2M_OBJECTINSTANCE_HPP_

#include <stdint.h>

#include "list.hpp"
#include "lwm2mbase.hpp"

namespace openlwm2m {

class ObjectInstance : public Lwm2mBase {
private:
    friend class Object;

    List mResourceList;

    ObjectInstance(Lwm2mBase* parent, uint16_t id, List& resourceDescList);
    ~ObjectInstance();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_OBJECTINSTANCE_HPP_ */