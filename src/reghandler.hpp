#ifndef OPENLWM2M_REGHANDLER_HPP_
#define OPENLWM2M_REGHANDLER_HPP_

#include "interface.hpp"
#include "itembase.hpp"
#include "objectinstance.hpp"
#include "storage.hpp"

namespace openlwm2m {

class Client;

class RegHandler : public ItemBase {
private:
    friend class Client;
    friend class StorageBase<RegHandler>;
    friend class StorageItem<RegHandler, Client&>;

    typedef StorageItem<RegHandler, Client&> Storage;

    Client& mClient;
    void* mConnection;
    ObjectInstance* mServerInstance;

    RegHandler(ItemBase* parent, uint16_t id, Client& client);
    virtual ~RegHandler();

    Status connect();

    void init();
    void release();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_REGHANDLER_HPP_ */