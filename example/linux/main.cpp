#include "client.hpp"

#include <signal.h>
#include <unistd.h>

#include "coaptransport.hpp"
#include "log.hpp"
#include "memory.hpp"

#define LOG_MODULE "Main"

using namespace openlwm2m;

bool sTerminate = false;

void sigIntHandler(int sig)
{
    sTerminate = true;
}

void registerSignals()
{
    struct sigaction act = {};

    act.sa_handler = sigIntHandler;
    act.sa_flags = SA_RESETHAND;

    sigaction(SIGINT, &act, NULL);
}

int main()
{
    LOG_INFO("Start lwm2m client");

    registerSignals();

    CoapTransport transport;
    Client client("Test client", false, transport);

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

    uint64_t currentTimeMs = 0;
    uint64_t pollInMs = 0;

    while (!sTerminate) {
        if (pollInMs == 0) {
            status = client.poll(currentTimeMs, &pollInMs);
            ASSERT_MESSAGE(status == STS_OK, "Client failed");
        }

        usleep(1000);

        currentTimeMs++;
        if (pollInMs) {
            pollInMs--;
        }
    }

    LOG_INFO("Stop lwm2m client");

    return 0;
}
