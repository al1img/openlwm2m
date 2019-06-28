#ifndef OPENLWM2M_REGHANDLER_HPP_
#define OPENLWM2M_REGHANDLER_HPP_

#include "interface.hpp"
#include "itembase.hpp"
#include "objectinstance.hpp"
#include "storage.hpp"
#include "timer.hpp"

namespace openlwm2m {

class Client;

class RegHandler : public ItemBase {
private:
    friend class Client;
    friend class Lwm2mStorage<RegHandler, Client&>;
    friend class Lwm2mDynamicStorage<RegHandler, Client&>;

    typedef Lwm2mDynamicStorage<RegHandler, Client&> Storage;
    typedef Node<RegHandler> StorageNode;

    enum State { STATE_INIT, STATE_INIT_DELAY, STATE_REGISTRATION };

    Client& mClient;
    void* mSession;
    ObjectInstance* mServerInstance;
    ObjectInstance* mSecurityInstance;
    State mState;
    Timer mTimer;

    RegHandler(ItemBase* parent, uint16_t id, Client& client);
    ~RegHandler();

    void init();
    void release();

    Status bind(ObjectInstance* serverInstance);
    Status startRegistration();

    static Status timerCallback(void* context);
    Status onTimerCallback();

    static void registrationCallback(void* context, Status status);
    void onRegistrationCallback(Status status);

    Status getObjectsString(char* str, int maxSize);

    void registrationStatus(Status status);
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_REGHANDLER_HPP_ */
