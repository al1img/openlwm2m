/** \file client.hpp
 * LWM2M Client
 */

#ifndef OPENLWM2M_CLIENT_HPP_
#define OPENLWM2M_CLIENT_HPP_

#include <stdint.h>

#include "interface.hpp"
#include "object.hpp"
#include "objectmanager.hpp"
#include "reghandler.hpp"
#include "status.hpp"

#define LWM2M_VERSION "1.1"

namespace openlwm2m {

/**
 * lwm2m client.
 */
class Client : public ClientItf {
public:
    typedef void (*PollRequest)();

    /**
     * Constructor.
     */
    Client(const char* name, bool queueMode, PollRequest pollRequest);
    ~Client();

    Status poll(uint64_t currentTimeMs, uint64_t* pollTimeMs);

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
     * @param[in] interface     Interface which requires this object.
     * @param[in] id            Object id.
     *
     * @retval pointer to Object.
     */
    Object* getObject(Interface interface, uint16_t id);

    Object* getFirstObject(Interface interface);
    Object* getNextObject(Interface interface);

    ResourceInstance* getResourceInstance(Interface interface, uint16_t objId, uint16_t objInstanceId, uint16_t resId,
                                          uint16_t resInstanceId = 0);

    TransportItf* getTransport() const { return mTransport; }

    Status init(TransportItf* transport);

    Status registration();

    // Bootstrap

    void bootstrapDiscover();
    void bootstrapRead();
    Status bootstrapWriteJSON(const char* path, const char* dataJSON);
    Status bootstrapWrite(DataFormat dataFormat, const char* path, void* data, size_t size);
    void bootstrapDelete();

    // Device

    void deviceRead();
    void deviceDiscover();
    void deviceWrite();
    void deviceWriteAttributes();
    void deviceExecute();
    void deviceCreate();
    void deviceDelete();
    void readComposite();
    void writeComposite();

    // Reporting
    void reportingObserve();
    void reportingCancelObservation();
    void reportingObserveComposite();
    void reportingCancelObserveComposite();

private:
    friend class RegHandler;

    enum State { STATE_INIT, STATE_INITIALIZED, STATE_BOOTSTRAP, STATE_REGISTRATION, STATE_READY };

    const char* mName;
    bool mQueueMode;
    PollRequest mPollRequest;

    TransportItf* mTransport;

    ObjectManager mObjectManager;
    RegHandler::Storage mRegHandlerStorage;

    RegHandler* mCurrentHandler = NULL;
    State mState;

    static void updateRegistration(void* context, ResourceInstance* resInstance);
    void onUpdateRegistration(ResourceInstance* resInstance);

    static void registrationStatus(void* context, RegHandler* handler, Status status);
    void onRegistrationStatus(RegHandler* handler, Status status);

    Status createRegHandlers();

    Status startNextRegistration();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_CLIENT_HPP_ */
