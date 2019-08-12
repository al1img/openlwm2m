#ifndef OPENLWM2M_REGHANDLER_HPP_
#define OPENLWM2M_REGHANDLER_HPP_

#include "interface.hpp"
#include "itembase.hpp"
#include "object.hpp"
#include "objectinstance.hpp"
#include "objectmanager.hpp"
#include "storage.hpp"
#include "timer.hpp"

namespace openlwm2m {

class Client;

class RegHandler : public ItemBase {
public:
    struct Params {
        ObjectManager& objectManager;
        const char* clientName;
        bool queueMode;
        void (*pollRequest)();
    };

    typedef Lwm2mDynamicStorage<RegHandler, Params> Storage;
    typedef void (*RegistrationHandler)(void* context, Status status);

    enum State {
        STATE_INIT,
        STATE_INIT_DELAY,
        STATE_REGISTRATION,
        STATE_REGISTERED,
        STATE_DEREGISTRATION,
        STATE_DEREGISTERED
    };

    RegHandler(ItemBase* parent, Params params);
    ~RegHandler();

    void init();
    void release();

    Status bind(TransportItf* transport);
    Status registration(bool withRetry = false, RegistrationHandler handler = NULL, void* context = NULL);
    Status deregistration(RegistrationHandler handler = NULL, void* context = NULL);
    Status updateRegistration();

    State getState() const { return mState; }

private:
    static const int sUpdateRegistrationTimeout = 1000;

    struct ContextHandler {
        RegistrationHandler handler;
        void* context;
    };

    Params mParams;
    TransportItf* mTransport;
    void* mSession;
    ObjectInstance* mServerInstance;
    ObjectInstance* mSecurityInstance;

    State mState;
    Timer mTimer;

    bool mWithRetry;
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

#endif /* OPENLWM2M_REGHANDLER_HPP_ */
