#ifndef OPENLWM2M_CLIENT_HPP_
#define OPENLWM2M_CLIENT_HPP_

#include <stdint.h>

#include "list.hpp"
#include "object.hpp"
#include "status.hpp"

namespace openlwm2m {

class Client {
public:
    Client();
    ~Client();

    Object* createObject(uint16_t id, Object::Instance instance, size_t maxInstances, Object::Mandatory mandatory,
                         uint16_t interfaces);

    Status start();

private:
    List mObjectList;
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_CLIENT_HPP_ */
