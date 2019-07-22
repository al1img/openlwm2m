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

    enum State { STATE_INIT, STATE_INIT_DELAY, STATE_REGISTRATION };

    RegHandler(ItemBase* parent, Params params);
    ~RegHandler();

    void init();
    void release();

    void setTransport(TransportItf* transport);

    Status bind(ObjectInstance* serverInstance);
    Status startRegistration();

    State getState() const { return mState; }

    ObjectInstance* getServerInstance() const { return mServerInstance; }

private:
    Params mParams;
    TransportItf* mTransport;
    void* mSession;
    ObjectInstance* mServerInstance;
    ObjectInstance* mSecurityInstance;
    State mState;
    Timer mTimer;

    static Status timerCallback(void* context);
    Status onTimerCallback();

    static void registrationCallback(void* context, Status status);
    void onRegistrationCallback(Status status);

    Status getObjectsStr(char* str, int maxSize);

    void registrationStatus(Status status);
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_REGHANDLER_HPP_ */
