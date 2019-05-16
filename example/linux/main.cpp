#include "client.hpp"
#include "log.hpp"

#define LOG_MODULE "Main"

using namespace openlwm2m;

int main()
{
    LOG_INFO("Start lwm2m client");

    Client client;

    Status status = client.startBootstrap();
    if (status != STS_OK) {
        LOG_ERROR("Can't start client, status %d", status);
    }

    Object *object = client.getObject(1, &status);
    if (!object) {
        LOG_ERROR("Can't get object, status %d", status);
    }

    object->createInstance(ITF_BOOTSTRAP, &status);
    if (status != STS_OK) {
        LOG_ERROR("Can't create instance, status %d", status);
    }

    LOG_INFO("Stop lwm2m client");

    return 0;
}