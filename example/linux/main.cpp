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

    Status status = client.bootstrapStart();
    if (status != STS_OK) {
        LOG_ERROR("Can't start client, status %d", status);
    }

    client.bootstrapFinish();

    LOG_INFO("Stop lwm2m client");

    return 0;
}