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

    if (Object::isInstanceChanged()) {
        for (RegHandler* regHandler = mRegHandlerStorage.getFirstItem(); regHandler;
             regHandler = mRegHandlerStorage.getNextItem()) {
            if (regHandler->getState() == RegHandler::STATE_REGISTERED) {
                regHandler->updateRegistration();
            }
        }
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
    Status status = STS_OK;

    LOG_DEBUG("Init client");

    if (mState != STATE_INIT) {
        return STS_ERR_INVALID_STATE;
    }

    mTransport = transport;

    mObjectManager.init();

    status = mObjectManager.getObject(ITF_BOOTSTRAP, OBJ_LWM2M_SERVER)
                 ->setResourceChangedCbk(RES_LIFETIME, &Client::updateRegistration, this);
    ASSERT(status == STS_OK);

    mState = STATE_INITIALIZED;

    return STS_OK;
}

Status Client::registration()
{
    LOG_DEBUG("Register client");

    if (mState != STATE_INITIALIZED || mRegHandlerStorage.size()) {
        return STS_ERR_INVALID_STATE;
    }

    Status status = STS_OK;

    if ((status = createRegHandlers()) != STS_OK) {
        return status;
    }

    if ((status = startNextRegistration()) != STS_OK) {
        mRegHandlerStorage.clear();
        return status;
    }

    mState = STATE_REGISTRATION;

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
    return mObjectManager.write(ITF_BOOTSTRAP, path, DATA_FMT_SENML_JSON,
                                reinterpret_cast<void*>(const_cast<char*>(dataJSON)), strlen(dataJSON));
}

Status Client::bootstrapWrite(const char* path, DataFormat dataFormat, void* data, size_t size)
{
    return mObjectManager.write(ITF_BOOTSTRAP, path, dataFormat, data, size);
}

void Client::bootstrapDelete()
{
}

Status Client::deviceRead(const char* path, DataFormat reqFormat, void* data, size_t size, DataFormat* format)
{
    LOG_DEBUG("Read, path: %s", path);

    return mObjectManager.read(ITF_DEVICE, path, DATA_FMT_TEXT, NULL, 0, reqFormat, data, size, format);
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

void Client::updateRegistration(void* context, ResourceInstance* resInstance)
{
    static_cast<Client*>(context)->onUpdateRegistration(resInstance);
}

void Client::onUpdateRegistration(ResourceInstance* resInstance)
{
    uint16_t shortServerId = static_cast<ObjectInstance*>(resInstance->getParent()->getParent())
                                 ->getResourceInstance(RES_SHORT_SERVER_ID)
                                 ->getInt();

    RegHandler* regHandler = mRegHandlerStorage.getItemById(shortServerId);

    if (regHandler != NULL && regHandler->getState() == RegHandler::STATE_REGISTERED) {
    }
}

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

        return STS_ERR_NOT_FOUND;
    }

    return STS_OK;
}

void Client::registrationStatus(void* context, RegHandler* handler, Status status)
{
    static_cast<Client*>(context)->onRegistrationStatus(handler, status);
}

void Client::onRegistrationStatus(RegHandler* handler, Status status)
{
    if (status != STS_OK) {
        ResourceInstance* bootstrapOnFailure =
            mObjectManager.getServerInstance(handler->getId())->getResourceInstance(RES_BOOTSTRAP_ON_REG_FAILURE);

        if (bootstrapOnFailure && bootstrapOnFailure->getBool()) {
            // TODO: start bootstrap and return
        }
    }

    if (handler == mCurrentHandler) {
        Status status = startNextRegistration();

        if (status != STS_OK && mCurrentHandler) {
            LOG_ERROR("Can't start registration, server: %d, status %d", mCurrentHandler->getId(), status);
        }
    }
}

Status Client::startNextRegistration()
{
    uint64_t minPriority = ULONG_MAX;

    mCurrentHandler = NULL;

    // Start priority handlers

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
            mCurrentHandler = regHandler;
            minPriority = regPriority->getUint();
        }
    }

    if (mCurrentHandler) {
        return mCurrentHandler->registration(true, registrationStatus, this);
    }

    // Non blocking priority order

    for (RegHandler* regHandler = mRegHandlerStorage.getFirstItem(); regHandler;
         regHandler = mRegHandlerStorage.getNextItem()) {
        ObjectInstance* serverInstance = mObjectManager.getServerInstance(regHandler->getId());
        ASSERT(serverInstance);

        ResourceInstance* regPriority = serverInstance->getResourceInstance(RES_REGISTRATION_PRIORITY_ORDER);
        ResourceInstance* failureBlock = serverInstance->getResourceInstance(RES_REG_FAILURE_BLOCK);

        if (regPriority && regHandler->getState() == RegHandler::STATE_DEREGISTERED &&
            (!failureBlock || !failureBlock->getBool())) {
            mCurrentHandler = regHandler;
            return mCurrentHandler->registration(false, registrationStatus, this);
        }
    }

    // All other

    for (RegHandler* regHandler = mRegHandlerStorage.getFirstItem(); regHandler;
         regHandler = mRegHandlerStorage.getNextItem()) {
        // Skip already started handlers
        if (regHandler->getState() != RegHandler::STATE_INIT) {
            continue;
        }

        mCurrentHandler = regHandler;
        return mCurrentHandler->registration(false, registrationStatus, this);
    }

    mState = STATE_READY;

    return STS_OK;
}

}  // namespace openlwm2m
