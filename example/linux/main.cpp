#include <signal.h>
#include <time.h>
#include <unistd.h>

#include "client.hpp"
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

    Client client("TestClient", false, pollRequest);

    transport.setClient(&client);

    Status status = STS_OK;

    status = client.init(&transport);
    ASSERT_MESSAGE(status == STS_OK, "Can't initialize client");

    memInitDone();

    status = client.bootstrapWriteJSON("/0/0",
                                       "\
[\
{\"bn\":\"/0/0/\",\"n\":\"0\",\"vs\":\"coap://::1:5683\"},\
{\"n\":\"10\",\"v\":1}\
]\
");
    ASSERT_MESSAGE(status == STS_OK, "Can't write bootstrap");

    status = client.bootstrapWriteJSON("/1/0",
                                       "\
[\
{\"bn\":\"/1/0/\",\"n\":\"0\",\"v\":1},\
{\"n\":\"1\",\"v\":300},\
{\"n\":\"7\",\"vs\":\"U\"},\
{\"n\":\"19\",\"v\":30},\
{\"n\":\"20\",\"v\":1}\
]\
");
    ASSERT_MESSAGE(status == STS_OK, "Can't write bootstrap");

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
