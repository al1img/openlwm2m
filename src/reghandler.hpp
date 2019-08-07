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
    Status registration(RegistrationHandler handler = NULL, void* context = NULL);
    Status deregistration(RegistrationHandler handler = NULL, void* context = NULL);

    State getState() const { return mState; }

private:
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

    ContextHandler mRegistrationContext;
    ContextHandler mDeregistrationContext;

    char mObjectsStr[CONFIG_DEFAULT_STRING_LEN + 1];
    char mBindingStr[CONFIG_BINDING_STR_MAX_LEN + 1];
    int64_t mLifetime;

    static Status timerCallback(void* context);
    Status onTimerCallback();

    static void registrationCallback(void* context, Status status);
    void onRegistrationCallback(Status status);

    static void deregistrationCallback(void* context, Status status);
    void onDeregistrationCallback(Status status);

    Status getObjectsStr(char* str, int maxSize);

    void registrationStatus(Status status);

    Status sendRegistration();
    Status sendUpdate();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_REGHANDLER_HPP_ */
