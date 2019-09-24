#ifndef OPENLWM2M_OBJECTINSTANCE_HPP_
#define OPENLWM2M_OBJECTINSTANCE_HPP_

#include <stdint.h>

#include "itembase.hpp"
#include "resource.hpp"
#include "storage.hpp"

namespace openlwm2m {

class Object;

class ObjectInstance : public ItemBase {
public:
    typedef Lwm2mDynamicStorage<ObjectInstance> Storage;

    ObjectInstance(Object* parent);
    ~ObjectInstance();

    void init();
    void release();

    Status addResource(ResourceInfo& info);

    Resource* getResourceById(uint16_t id) {return mResourceStorage.getItemById(id);}
    Resource* getFirstResource() {return mResourceStorage.getFirstItem();}
    Resource* getNextResource(){return mResourceStorage.getNextItem();}

    ResourceInstance* getResourceInstance(uint16_t resId, uint16_t resInstanceId = 0);

private:
    Resource::Storage mResourceStorage;
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_OBJECTINSTANCE_HPP_ */