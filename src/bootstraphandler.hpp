#ifndef OPENLWM2M_BOOTSTRAPHANDLER_HPP_
#define OPENLWM2M_BOOTSTRAPHANDLER_HPP_

#include "lwm2m.hpp"
#include "objectmanager.hpp"
#include "timer.hpp"

namespace openlwm2m {

class BootstrapHandler {
public:
    typedef void (*RequestHandler)(void* context, BootstrapHandler* handler, Status status);

    enum State { STATE_INIT, STATE_BOOTSTRAPING, STATE_BOOTSTAPPED };

    BootstrapHandler(const char* clientName, ObjectManager& objectManager);
    ~BootstrapHandler();

    Status bind(TransportItf* transport);
    Status bootstrapRequest(RequestHandler handler = NULL, void* context = NULL);
    Status bootstrapFinish();
    Status discover(void* data, size_t* size, uint16_t objectId = INVALID_ID);
    Status read(DataFormat* format, void* data, size_t* size, uint16_t objectId,
                uint16_t objectInstanceId = INVALID_ID);
    Status write(DataFormat format, void* data, size_t size, uint16_t objectId, uint16_t objectInstanceId = INVALID_ID,
                 uint16_t resourceId = INVALID_ID);
    Status deleteInstance(uint16_t objectId = INVALID_ID, uint16_t objectInstanceId = INVALID_ID);

    State getState() const { return mState; }
    void* getSession() const { return mSession; }

private:
    static const int sBootstrapTimeoutMs = 60000;

    const char* mClientName;
    ObjectManager& mObjectManager;

    TransportItf* mTransport;
    void* mSession;

    State mState;
    Timer mTimer;

    RequestHandler mRequestHandler;
    void* mRequestContext;

    static Status timerCallback(void* context);
    Status onTimerCallback();

    static void bootstrapRequstCallback(void* context, void* data, Status status);
    void onBootstrapRequestCallback(Status status);

    void bootstrapFinish(Status status);
    int discoverObject(char* data, size_t maxSize, Object* object);
    Status deleteAllObjectInstances();
    Status deleteObjectAllInstances(Object* object);
    Status deleteObjectInstance(ObjectInstance* instance);
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_BOOTSTRAPHANDLER_HPP_ */
