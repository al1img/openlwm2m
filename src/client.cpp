#include "client.hpp"

#include <cstring>

#include "interface.hpp"
#include "log.hpp"
#include "timer.hpp"

#define LOG_MODULE "Client"

namespace openlwm2m {

/*******************************************************************************
 * Client
 ******************************************************************************/

Client::Client(const char* name, bool queueMode, PollRequest pollRequest)
    : mName(name),
      mQueueMode(queueMode),
      mPollRequest(pollRequest),
      mRegHandlerStorage(NULL, (RegHandler::Params){mObjectManager, name, queueMode, pollRequest}, CONFIG_NUM_SERVERS),
      mState(STATE_INIT)
{
    LOG_DEBUG("Create client");
}

Client::~Client()
{
    LOG_DEBUG("Delete client");

    mRegHandlerStorage.clear();
}

/*******************************************************************************
 * Public
 ******************************************************************************/

Status Client::poll(uint64_t currentTimeMs, uint64_t* pollTimeMs)
{
    if (pollTimeMs) {
        *pollTimeMs = ULONG_MAX;
    }

    Status status = STS_OK;

    if ((status = Timer::poll(currentTimeMs, pollTimeMs)) != STS_OK) {
        return status;
    }

    return STS_OK;
}

Object* Client::createObject(uint16_t id, Object::Instance instance, size_t maxInstances, Object::Mandatory mandatory,
                             uint16_t interfaces, Status* status)
{
    if (mState != STATE_INIT) {
        if (status) *status = STS_ERR_INVALID_STATE;
        return NULL;
    }

    return mObjectManager.createObject(id, instance, maxInstances, mandatory, interfaces, status);
}

Object* Client::getObject(Interface interface, uint16_t id)
{
    return mObjectManager.getObject(interface, id);
}

Object* Client::getFirstObject(Interface interface)
{
    return mObjectManager.getFirstObject(interface);
}

Object* Client::getNextObject(Interface interface)
{
    return mObjectManager.getNextObject(interface);
}

ResourceInstance* Client::getResourceInstance(Interface interface, uint16_t objId, uint16_t objInstanceId,
                                              uint16_t resId, uint16_t resInstanceId)
{
    Object* object = getObject(interface, objId);

    if (!object) {
        return NULL;
    }

    return object->getResourceInstance(objInstanceId, resId, resInstanceId);
}

Status Client::init(TransportItf* transport)
{
    LOG_DEBUG("Init client");

    if (mState != STATE_INIT) {
        return STS_ERR_INVALID_STATE;
    }

    mTransport = transport;

    mObjectManager.init();

    mState = STATE_INITIALIZED;

    return STS_OK;
}

Status Client::registration()
{
    LOG_DEBUG("Register client");

    if (mState != STATE_INITIALIZED && mState != STATE_BOOTSTRAP) {
        return STS_ERR_INVALID_STATE;
    }

    Status status = STS_OK;

    if ((status = createRegHandlers()) != STS_OK) {
        return status;
    }

    if ((status = startNextPriorityReg()) != STS_OK) {
        mRegHandlerStorage.clear();
        return status;
    }

    // Starts all non priority handlers

    for (RegHandler* regHandler = mRegHandlerStorage.getFirstItem(); regHandler;
         regHandler = mRegHandlerStorage.getNextItem()) {
        // Skip already started handlers
        if (regHandler->getState() != RegHandler::STATE_INIT) {
            continue;
        }

        // Skip handlers with priority order
        ObjectInstance* serverInstance = mObjectManager.getServerInstance(regHandler->getId());
        ASSERT(serverInstance);

        if (serverInstance->getResourceInstance(RES_REGISTRATION_PRIORITY_ORDER)) {
            continue;
        }

        if ((status = regHandler->registration(true)) != STS_OK) {
            mRegHandlerStorage.clear();
            return status;
        }
    }

    return status;
}

void Client::bootstrapDiscover()
{
}

void Client::bootstrapRead()
{
}

Status Client::bootstrapWriteJSON(const char* path, const char* dataJSON)
{
    return mObjectManager.write(ITF_BOOTSTRAP, DATA_FMT_SENML_JSON, path,
                                reinterpret_cast<void*>(const_cast<char*>(dataJSON)), strlen(dataJSON));
}

Status Client::bootstrapWrite(DataFormat dataFormat, const char* path, void* data, size_t size)
{
    return mObjectManager.write(ITF_BOOTSTRAP, dataFormat, path, data, size);
}

void Client::bootstrapDelete()
{
}

void Client::deviceRead()
{
}
void Client::deviceDiscover()
{
}
void Client::deviceWrite()
{
}
void Client::deviceWriteAttributes()
{
}
void Client::deviceExecute()
{
}
void Client::deviceCreate()
{
}
void Client::deviceDelete()
{
}
void Client::readComposite()
{
}
void Client::writeComposite()
{
}

void Client::reportingObserve()
{
}
void Client::reportingCancelObservation()
{
}
void Client::reportingObserveComposite()
{
}
void Client::reportingCancelObserveComposite()
{
}

/*******************************************************************************
 * Private
 ******************************************************************************/

Status Client::createRegHandlers()
{
    Object* object = getObject(ITF_REGISTER, OBJ_LWM2M_SERVER);
    ASSERT(object);

    ObjectInstance* serverInstance = object->getFirstInstance();

    while (serverInstance) {
        RegHandler* handler =
            mRegHandlerStorage.newItem(serverInstance->getResourceInstance(RES_SHORT_SERVER_ID)->getInt());
        ASSERT(handler);

        Status status = STS_OK;

        if ((status = handler->bind(mTransport)) != STS_OK) {
            LOG_ERROR("Can't bind to lwm2m server %d", status);

            mRegHandlerStorage.deleteItem(handler);
        }

        serverInstance = object->getNextInstance();
    }

    if (mRegHandlerStorage.size() == 0) {
        LOG_ERROR("No valid lwm2m servers found");

        return STS_ERR_NOT_EXIST;
    }

    return STS_OK;
}

Status Client::startNextPriorityReg()
{
    RegHandler* minPriorityHandler = NULL;
    uint64_t minPriority = ULONG_MAX;

    for (RegHandler* regHandler = mRegHandlerStorage.getFirstItem(); regHandler;
         regHandler = mRegHandlerStorage.getNextItem()) {
        // Skip already started handlers
        if (regHandler->getState() != RegHandler::STATE_INIT) {
            continue;
        }

        ObjectInstance* serverInstance = mObjectManager.getServerInstance(regHandler->getId());
        ASSERT(serverInstance);

        ResourceInstance* regPriority = serverInstance->getResourceInstance(RES_REGISTRATION_PRIORITY_ORDER);

        // Skip handlers without priority order
        if (!regPriority) {
            continue;
        }

        if (regPriority->getUint() <= minPriority) {
            minPriorityHandler = regHandler;
            minPriority = regPriority->getUint();
        }
    }

    if (minPriorityHandler) {
        return minPriorityHandler->registration();
    }

    return STS_OK;
}

void Client::registrationStatus(RegHandler* handler, Status status)
{
    LOG_DEBUG("Handler /%d reg status: %d", handler->getId(), status);

    // if priority handler finished, start next priority handler
    ObjectInstance* serverInstance = mObjectManager.getServerInstance(handler->getId());
    ASSERT(serverInstance);

    if (serverInstance->getResourceInstance(RES_REGISTRATION_PRIORITY_ORDER)) {
        startNextPriorityReg();
    }
}

}  // namespace openlwm2m
