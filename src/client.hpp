#ifndef OPENLWM2M_CLIENT_HPP_
#define OPENLWM2M_CLIENT_HPP_

#include <stdint.h>

#include "list.hpp"
#include "object.hpp"

namespace openlwm2m {

class Client {
public:
    Client();
    ~Client();

    Object* createObject(uint16_t id, int maxInstances, bool mandatory, uint16_t interfaces);

private:
    List<Object*> mObjectList;
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_CLIENT_HPP_ */
