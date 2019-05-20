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
    LWM2M_ASSERT_MESSAGE(status == STS_OK, "Can't initialize client");

    status = client.bootstrapStart();
    LWM2M_ASSERT_MESSAGE(status == STS_OK, "Start bootstrap failed");

    client.bootstrapFinish();

    LOG_INFO("Stop lwm2m client");

    return 0;
}