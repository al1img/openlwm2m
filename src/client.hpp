/** \file client.hpp
 * LWM2M Client
 */

#ifndef OPENLWM2M_CLIENT_HPP_
#define OPENLWM2M_CLIENT_HPP_

#include <stdint.h>

#include "list.hpp"
#include "object.hpp"
#include "status.hpp"

namespace openlwm2m {

/**
 * lwm2m client.
 */
class Client {
public:
    /**
     * Constructor.
     */
    Client();
    ~Client();

    /**
     * Creates lwm2m object.
     *
     * See: D.1 Object Template.
     *
     * @param[in] id            Object id.
     * @param[in] instance      Indicates whether this Object supports multiple ObjectInstance's or not.
     * @param[in] maxInstances  Defines maximum number of instances. This parameter relevant for multiple
     *                          ObjectInstance's. It can be set to 0 to have unlimited instances in case memory
     *                          reservation is disabled.
     * @param[in] mandatory     Indicates whether this Object is mandatory or optional.
     * @param[in] interfaces    Defines interfaces which can access this object.
     *
     * @param[out] status       Returns status of operation openlwm2m::Status.
     *
     * @retval pointer to Object.
     */
    Object* createObject(uint16_t id, Object::Instance instance, size_t maxInstances, Object::Mandatory mandatory,
                         uint16_t interfaces, Status* status = NULL);

    /**
     * Returns lwm2m object.
     *
     * @param[in] id            Object id.
     * @param[in] interface     Interface which requires this object.
     * @param[out] status       Returns status of operation openlwm2m::Status.
     *
     * @retval pointer to Object.
     */
    Object* getObject(uint16_t id, Interface interface, Status* status = NULL);

    /**
     * Starts bootstrap procedure.
     *
     * See: 6.1. Bootstrap Interface
     *
     * @param[in] id            Object id.
     * @param[in] interface     Interface which requires this object.
     * @param[out] status       Returns status of operation openlwm2m::Status.
     *
     * @retval pointer to Object.
     */
    Status startBootstrap();

private:
    enum State { STATE_INIT, STATE_BOOTSTRAP };

    List mObjectList;
    State mState;
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_CLIENT_HPP_ */
