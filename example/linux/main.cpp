#include "client.hpp"
#include "log.hpp"

#define LOG_MODULE "Main"

using namespace openlwm2m;

int main() {
    LOG_INFO("Start lwm2m client");

    Client client;

    LOG_INFO("Stop lwm2m client");

    return 0;
}