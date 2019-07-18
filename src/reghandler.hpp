#ifndef OPENLWM2M_REGHANDLER_HPP_
#define OPENLWM2M_REGHANDLER_HPP_

#include "interface.hpp"
#include "itembase.hpp"
#include "object.hpp"
#include "objectinstance.hpp"
#include "storage.hpp"
#include "timer.hpp"

namespace openlwm2m {

class Client;

class RegHandler : public ItemBase {
private:
    struct Param {
        ObjectManager& objectManager;
        const char* clientName;
        bool queueMode;
        void (*pollRequest)();
    };

    friend class Client;
    friend class Lwm2mStorage<RegHandler, Client&>;
    friend class Lwm2mDynamicStorage<RegHandler, Client&>;

    typedef Lwm2mDynamicStorage<RegHandler, Param> Storage;

    enum State { STATE_INIT, STATE_INIT_DELAY, STATE_REGISTRATION };

    Param mParam;
    TransportItf* mTransport;
    void* mSession;
    ObjectInstance* mServerInstance;
    ObjectInstance* mSecurityInstance;
    State mState;
    Timer mTimer;

    RegHandler(ItemBase* parent, Param param);
    ~RegHandler();

    void init();
    void release();

    void setTransport(TransportItf* transport);

    Status bind(ObjectInstance* serverInstance);
    Status startRegistration();

    static Status timerCallback(void* context);
    Status onTimerCallback();

    static void registrationCallback(void* context, Status status);
    void onRegistrationCallback(Status status);

    Status getObjectsStr(char* str, int maxSize);

    void registrationStatus(Status status);
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_REGHANDLER_HPP_ */
