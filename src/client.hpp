/** \file client.hpp
 * LWM2M Client
 */

#ifndef OPENLWM2M_CLIENT_HPP_
#define OPENLWM2M_CLIENT_HPP_

#include <stdint.h>

#include "bootstraphandler.hpp"
#include "interface.hpp"
#include "lwm2m.hpp"
#include "object.hpp"
#include "objectmanager.hpp"
#include "serverhandler.hpp"

namespace openlwm2m {

/**
 * lwm2m client.
 */
class Client : public ClientItf {
public:
    typedef void (*BootstrapCallback)(void* context, Status status);

    /**
     * Constructor.
     */
    Client(const char* name, bool queueMode);
    ~Client();

    Status run();

    /**
     * Creates lwm2m object.
     *
     * See: D.1 Object Template.
     *
     * @param[in] id            Object id.
     * @param[in] single        Indicates whether this Object supports multiple ObjectInstance's or not.
     * @param[in] mandatory     Indicates whether this Object is mandatory or optional.
     * @param[in] maxInstances  Defines maximum number of instances. This parameter relevant for multiple
     *                          ObjectInstance's.
     *
     * @param[out] status       Returns status of operation openlwm2m::Status.
     *
     * @retval pointer to Object.
     */
    Object* createObject(uint16_t id, bool single, bool mandatory, size_t maxInstances = 1, Status* status = NULL);

    /**
     * Returns lwm2m object.
     *
     * @param[in] interface     Interface which requires this object.
     * @param[in] id            Object id.
     *
     * @retval pointer to Object.
     */
    Object* getObjectById(uint16_t id);

    Object* getFirstObject();
    Object* getNextObject();

    ResourceInstance* getResourceInstance(uint16_t objId, uint16_t objInstanceId, uint16_t resId,
                                          uint16_t resInstanceId = 0);

    TransportItf* getTransport() const { return mTransport; }

    Status init(TransportItf* transport);

    Status start(bool bootstrap = false, BootstrapCallback bootstrapCallback = NULL, void* context = NULL);

    // Rework
    Status discover(void* session, const char* path, void* data, size_t* size);
    Status read(void* session, const char* path, DataFormat* format, void* data, size_t* size);
    Status write(void* session, const char* path, DataFormat format, void* data, size_t size);
    Status deleteInstance(void* session, const char* path);
    Status bootstrapFinish(void* session);

    // Bootstrap

    void bootstrapDiscover();
    void bootstrapRead();
    Status bootstrapWriteJSON(const char* path, const char* dataJSON);
    Status bootstrapWrite(DataFormat dataFormat, void* data, size_t size, uint16_t objectId,
                          uint16_t objectInstanceId = INVALID_ID, uint16_t resourceId = INVALID_ID);
    void bootstrapDelete();

    // Device

    Status deviceRead(const char* path, DataFormat reqFormat, void* data, size_t* size, DataFormat* format);
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

    TransportItf* mTransport;

    ObjectManager mObjectManager;
    ServerHandler::Storage mServerHandlerStorage;
    BootstrapHandler mBootstrapHandler;

    BootstrapCallback mBootstrapCallback;
    void* mBootstrapContext;

    ServerHandler* mCurrentHandler;
    State mState;

    static void bootstrapFinished(void* context, BootstrapHandler* handler, Status status);
    void onBootstrapFinished(Status status);

    static void updateRegistration(void* context, ResourceInstance* resInstance);
    void onUpdateRegistration(ResourceInstance* resInstance);

    static void registrationStatus(void* context, ServerHandler* handler, Status status);
    void onRegistrationStatus(ServerHandler* handler, Status status);

    Status startBootstrap();
    Status createRegHandlers();
    Status registration();
    Status startNextRegistration();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_CLIENT_HPP_ */
