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

    State getState() const { return mState; }

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
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_BOOTSTRAPHANDLER_HPP_ */
