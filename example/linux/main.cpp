#include "client.hpp"
#include "log.hpp"

#define LOG_MODULE "Main"

using namespace openlwm2m;

int main()
{
    LOG_INFO("Start lwm2m client");

    Client client;

    Status status = client.start();

    if (status != STS_OK) {
        LOG_ERROR("Can't start client, status %d", status);
    }

    LOG_INFO("Stop lwm2m client");

    return 0;
}