#ifndef OPENLWM2M_REGHANDLER_HPP_
#define OPENLWM2M_REGHANDLER_HPP_

#include "interface.hpp"
#include "itembase.hpp"
#include "storage.hpp"

namespace openlwm2m {

class RegHandler : public ItemBase {
private:
    friend class Client;
    friend class StorageBase<RegHandler>;
    friend class StorageItem<RegHandler, TransportItf &>;

    typedef StorageItem<RegHandler, TransportItf &> Storage;

    TransportItf &mTransport;

    RegHandler(ItemBase *parent, uint16_t id, TransportItf &transport);
    virtual ~RegHandler();

    void init();
    void release();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_REGHANDLER_HPP_ */