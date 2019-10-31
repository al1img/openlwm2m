#ifndef OPENLWM2M_SERVERHANDLER_HPP_
#define OPENLWM2M_SERVERHANDLER_HPP_

#include "interface.hpp"
#include "itembase.hpp"
#include "object.hpp"
#include "objectinstance.hpp"
#include "objectmanager.hpp"
#include "storage.hpp"
#include "timer.hpp"

namespace openlwm2m {

class ServerHandler : public ItemBase {
public:
    typedef Lwm2mDynamicStorage<ServerHandler> Storage;
    typedef void (*RegistrationHandler)(void* context, ServerHandler* handler, Status status);

    enum State {
        STATE_INIT,
        STATE_INIT_DELAY,
        STATE_REGISTRATION,
        STATE_REGISTERED,
        STATE_DEREGISTRATION,
        STATE_DEREGISTERED
    };

    ServerHandler(const char* clientName, bool queueMode, ObjectManager& objectManager, void (*pollRequest)());
    ~ServerHandler();

    void init();
    void release();

    Status bind(TransportItf* transport);
    Status registration(bool ordered = false, RegistrationHandler handler = NULL, void* context = NULL);
    Status deregistration(RegistrationHandler handler = NULL, void* context = NULL);
    Status updateRegistration();

    State getState() const { return mState; }

private:
    static const int sUpdateRegistrationTimeout = 1000;

    struct ContextHandler {
        RegistrationHandler handler;
        void* context;
    };

    const char* mClientName;
    bool mQueueMode;
    ObjectManager& mObjectManager;
    void (*mPollRequest)();

    TransportItf* mTransport;
    void* mSession;
    ObjectInstance* mServerInstance;
    ObjectInstance* mSecurityInstance;

    State mState;
    Timer mTimer;

    bool mOrdered;
    uint64_t mCurrentSequence;

    ContextHandler mRegistrationContext;
    ContextHandler mDeregistrationContext;

    char mLocation[CONFIG_DEFAULT_STRING_LEN + 1];
    char mObjectsStr[CONFIG_DEFAULT_STRING_LEN + 1];
    char mBindingStr[CONFIG_BINDING_STR_MAX_LEN + 1];
    int64_t mLifetime;

    static Status timerCallback(void* context);
    Status onTimerCallback();

    static void registrationCallback(void* context, void* data, Status status);
    void onRegistrationCallback(char* location, Status status);

    static void updateCallback(void* context, void* data, Status status);
    void onUpdateCallback(Status status);

    static void deregistrationCallback(void* context, void* data, Status status);
    void onDeregistrationCallback(Status status);

    Status getObjectsStr(char* str, int maxSize);

    Status sendRegistration();
    Status sendUpdate();

    bool setupRetry();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_SERVERHANDLER_HPP_ */
