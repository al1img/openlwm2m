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
                         uint16_t interfaces, Status* status = NULL);

    Object* getObject(uint16_t id, Status* status = NULL);

    Status startBootstrap();

private:
    enum State { STATE_INIT, STATE_BOOTSTRAP };

    List mObjectList;
    State mState;
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_CLIENT_HPP_ */
