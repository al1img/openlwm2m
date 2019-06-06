#include "coaptransport.hpp"

#include "client.hpp"
#include "log.hpp"

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

    ObjectInstance* serverObjectInstance = client.getObject(ITF_BOOTSTRAP, OBJ_LWM2M_SERVER)->createInstance();
    ObjectInstance* securityObjectInstance = client.getObject(ITF_BOOTSTRAP, OBJ_LWM2M_SECURITY)->createInstance();

    securityObjectInstance->getResourceInstance(RES_LWM2M_SERVER_URI)->setString("coap://::1:5683");

    status = client.registration();
    ASSERT_MESSAGE(status == STS_OK, "Registration failed");

    LOG_INFO("Stop lwm2m client");

    return 0;
}