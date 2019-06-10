#include "client.hpp"
#include "coaptransport.hpp"
#include "log.hpp"
#include "memory.hpp"

#define LOG_MODULE "Main"

using namespace openlwm2m;

int main()
{
    LOG_INFO("Start lwm2m client");

    CoapTransport transport;
    Client client(transport);

    Status status = STS_OK;

    status = client.init();
    ASSERT_MESSAGE(status == STS_OK, "Can't initialize client");
#if CONFIG_RESERVE_MEMORY
    memInitDone();
#endif

    ObjectInstance* securityObjectInstance = client.getObject(ITF_BOOTSTRAP, OBJ_LWM2M_SECURITY)->createInstance();
    ObjectInstance* serverObjectInstance = client.getObject(ITF_BOOTSTRAP, OBJ_LWM2M_SERVER)->createInstance();

    securityObjectInstance->getResourceInstance(RES_LWM2M_SERVER_URI)->setString("coap://::1:5683");
    securityObjectInstance->getResourceInstance(RES_SECURITY_SHORT_SERVER_ID)->setInt(101);
    serverObjectInstance->getResourceInstance(RES_SHORT_SERVER_ID)->setInt(101);

    securityObjectInstance = client.getObject(ITF_BOOTSTRAP, OBJ_LWM2M_SECURITY)->createInstance();
    serverObjectInstance = client.getObject(ITF_BOOTSTRAP, OBJ_LWM2M_SERVER)->createInstance();

    securityObjectInstance->getResourceInstance(RES_LWM2M_SERVER_URI)->setString("coap://::1:7777");
    securityObjectInstance->getResourceInstance(RES_SECURITY_SHORT_SERVER_ID)->setInt(102);
    serverObjectInstance->getResourceInstance(RES_SHORT_SERVER_ID)->setInt(102);

    status = client.registration();
    ASSERT_MESSAGE(status == STS_OK, "Registration failed");

    LOG_INFO("Stop lwm2m client");

    return 0;
}