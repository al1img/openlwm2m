#include "client.hpp"

#include <signal.h>
#include <time.h>
#include <unistd.h>

#include "coaptransport.hpp"
#include "log.hpp"
#include "memory.hpp"

#define LOG_MODULE "Main"

using namespace openlwm2m;

bool sTerminate = false;
bool sPollRequest = false;

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

void pollRequest()
{
    LOG_DEBUG("Poll request");

    sPollRequest = true;
}

int main()
{
    LOG_INFO("Start lwm2m client");

    registerSignals();

    CoapTransport transport;
    Client client("Test client", false, pollRequest);

    Status status = STS_OK;

    status = client.init(&transport);
    ASSERT_MESSAGE(status == STS_OK, "Can't initialize client");

#if CONFIG_RESERVE_MEMORY
    memInitDone();
#endif

    ObjectInstance* securityObjectInstance = client.getObject(ITF_BOOTSTRAP, OBJ_LWM2M_SECURITY)->createInstance();
    ObjectInstance* serverObjectInstance = client.getObject(ITF_BOOTSTRAP, OBJ_LWM2M_SERVER)->createInstance();

    securityObjectInstance->getResourceInstance(RES_LWM2M_SERVER_URI)->setString("coap://::1:5683");
    securityObjectInstance->getResourceInstance(RES_SECURITY_SHORT_SERVER_ID)->setInt(101);
    serverObjectInstance->getResourceInstance(RES_SHORT_SERVER_ID)->setInt(101);
    serverObjectInstance->getResourceInstance(RES_BINDING)->setString("U");
    serverObjectInstance->getResourceInstance(RES_LIFETIME)->setInt(300);

    securityObjectInstance = client.getObject(ITF_BOOTSTRAP, OBJ_LWM2M_SECURITY)->createInstance();
    serverObjectInstance = client.getObject(ITF_BOOTSTRAP, OBJ_LWM2M_SERVER)->createInstance();

    securityObjectInstance->getResourceInstance(RES_LWM2M_SERVER_URI)->setString("coap://::1:7777");
    securityObjectInstance->getResourceInstance(RES_SECURITY_SHORT_SERVER_ID)->setInt(102);
    serverObjectInstance->getResourceInstance(RES_SHORT_SERVER_ID)->setInt(102);

    status = client.registration();
    ASSERT_MESSAGE(status == STS_OK, "Registration failed");

    uint64_t currentTimeMs = 0;
    uint64_t pollTimeMs = 0;

    while (!sTerminate) {
        timespec tp;
        int ret = clock_gettime(CLOCK_MONOTONIC, &tp);
        ASSERT(ret == 0);

        currentTimeMs = tp.tv_sec * 1000 + tp.tv_nsec / 1000000;

        if (currentTimeMs >= pollTimeMs || sPollRequest) {
            status = client.poll(currentTimeMs, &pollTimeMs);
            ASSERT_MESSAGE(status == STS_OK, "Client failed");

            LOG_DEBUG("Poll client: current time: %lu, next time: %lu", currentTimeMs, pollTimeMs);

            sPollRequest = false;
        }

        transport.run();
    }

    LOG_INFO("Stop lwm2m client");

    return 0;
}
