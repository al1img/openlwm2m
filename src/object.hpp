#ifndef OPENLWM2M_OBJECT_HPP_
#define OPENLWM2M_OBJECT_HPP_

#include <stdint.h>

#include "interface.hpp"
#include "itembase.hpp"
#include "objectinstance.hpp"
#include "resourcedesc.hpp"
#include "status.hpp"
#include "storage.hpp"

namespace openlwm2m {

/**
 * lwm2m object.
 */
class Object : public ItemBase {
public:
    enum Mandatory { MANDATORY, OPTIONAL };

    enum Instance { SINGLE, MULTIPLE };

    Status createResource(uint16_t id, uint16_t operations, ResourceDesc::Instance instance, size_t maxInstances,
                          ResourceDesc::Mandatory mandatory, ResourceDesc::Type type, int min = 0, int max = 0);

    ObjectInstance *createInstance(uint16_t id = INVALID_ID, Status *status = NULL);

private:
    struct Params {
        Instance mInstance;
        Mandatory mMandatory;
        uint16_t mInterfaces;
        size_t mMaxInstances;
    };

    friend class Client;
    friend class StorageBase<Object>;
    friend class StorageArray<Object, Params>;

    typedef StorageArray<Object, Params> Storage;

    Params mParams;

    bool mInitialized;

    ResourceDesc::Storage mResourceDescStorage;
    ObjectInstance::Storage *mInstanceStorage;

    Object(ItemBase *parent, uint16_t id, Params params);
    virtual ~Object();

    void init();
    void release();
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_OBJECT_HPP_ */