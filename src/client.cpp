#include "client.hpp"

#include <cstring>

#include "interface.hpp"
#include "log.hpp"
#include "timer.hpp"
#include "utils.hpp"

#define LOG_MODULE "Client"

namespace openlwm2m {

/*******************************************************************************
 * Client
 ******************************************************************************/

Client::Client(const char* name, bool queueMode)
    : mName(name),
      mQueueMode(queueMode),
      mBootstrapHandler(name, mObjectManager),
      mBootstrapCallback(NULL),
      mBootstrapContext(NULL),
      mCurrentHandler(NULL),
      mState(STATE_INIT)
{
    LOG_DEBUG("Create client");

    for (int i = 0; i < CONFIG_NUM_SERVERS; i++) {
        mServerHandlerStorage.pushItem(new ServerHandler(name, queueMode, mObjectManager));
    }
}

Client::~Client()
{
    LOG_DEBUG("Delete client");

    mServerHandlerStorage.clear();
}

/*******************************************************************************
 * Public
 ******************************************************************************/

Status Client::run()
{
    Status status = STS_OK;

    if ((status = Timer::run()) != STS_OK) {
        return status;
    }

    if (Object::isInstanceChanged()) {
        for (ServerHandler* handler = mServerHandlerStorage.getFirstItem(); handler;
             handler = mServerHandlerStorage.getNextItem()) {
            if (handler->getState() == ServerHandler::STATE_REGISTERED) {
                handler->updateRegistration();
            }
        }
    }

    return STS_OK;
}

Object* Client::createObject(uint16_t id, bool single, bool mandatory, size_t maxInstances, Status* status)
{
    if (mState != STATE_INIT) {
        if (status) *status = STS_ERR_NOT_ALLOWED;
        return NULL;
    }

    return mObjectManager.createObject(id, single, mandatory, maxInstances, status);
}

Object* Client::getObjectById(uint16_t id)
{
    return mObjectManager.getObjectById(id);
}

Object* Client::getFirstObject()
{
    return mObjectManager.getFirstObject();
}

Object* Client::getNextObject()
{
    return mObjectManager.getNextObject();
}

ResourceInstance* Client::getResourceInstance(uint16_t objId, uint16_t objInstanceId, uint16_t resId,
                                              uint16_t resInstanceId)
{
    Object* object = getObjectById(objId);

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
        return STS_ERR_NOT_ALLOWED;
    }

    mTransport = transport;

    mObjectManager.init();

    status = mObjectManager.getObjectById(OBJ_LWM2M_SERVER)
                 ->setResourceCallback(RES_LIFETIME, &Client::updateRegistration, this);
    ASSERT(status == STS_OK);

    if ((mBootstrapHandler.bind(transport)) != STS_OK) {
        return status;
    }

    mState = STATE_INITIALIZED;

    return STS_OK;
}

Status Client::start(bool bootstrap, BootstrapCallback bootstrapCallback, void* context)
{
    Status status = STS_OK;

    if (mState != STATE_INITIALIZED) {
        return STS_ERR_NOT_ALLOWED;
    }

    mBootstrapCallback = bootstrapCallback;
    mBootstrapContext = context;

    if (bootstrap) {
        if ((status = startBootstrap()) != STS_OK) {
            return status;
        }
    }
    else {
        if ((status = registration()) != STS_OK) {
            return status;
        }
    }

    return STS_OK;
}

Status Client::registration()
{
    LOG_DEBUG("Register client");

    if (mState != STATE_INITIALIZED || mServerHandlerStorage.size()) {
        return STS_ERR_NOT_ALLOWED;
    }

    Status status = STS_OK;

    if ((status = createRegHandlers()) != STS_OK) {
        return status;
    }

    if ((status = startNextRegistration()) != STS_OK) {
        mServerHandlerStorage.clear();
        return status;
    }

    mState = STATE_REGISTRATION;

    return STS_OK;
}

Status Client::discover(void* session, const char* path, void* data, size_t* size)
{
    Status status = STS_OK;
    uint16_t objectId, objectInstanceId, resourceId, resourceInstanceId;

    LOG_INFO("Discover, path: %s", path);

    if (Utils::convertPath(path, &objectId, &objectInstanceId, &resourceId, &resourceInstanceId) < 0) {
        LOG_ERROR("Path not found");
        return STS_ERR_NOT_FOUND;
    }

    if (session == mBootstrapHandler.getSession()) {
        if (resourceInstanceId != INVALID_ID || resourceId != INVALID_ID || objectInstanceId != INVALID_ID) {
            LOG_ERROR("Path not found");
            return STS_ERR_NOT_FOUND;
        }

        if ((status = mBootstrapHandler.discover(data, size, objectId)) != STS_OK) {
            LOG_ERROR("Bootstrap discover error: %d", status);
            return status;
        }

        return STS_OK;
    }

    for (ServerHandler* handler = mServerHandlerStorage.getFirstItem(); handler;
         handler = mServerHandlerStorage.getNextItem()) {
        if (session == handler->getSession()) {
            if (resourceInstanceId != INVALID_ID) {
                LOG_ERROR("Path not found");
                return STS_ERR_NOT_FOUND;
            }

            if ((status = handler->discover(data, size, objectId, objectInstanceId, resourceId)) != STS_OK) {
                LOG_ERROR("Device discover error: %d", status);
                return status;
            }

            return STS_OK;
        }
    }

    LOG_ERROR("Session not found");

    return STS_ERR_NOT_FOUND;
}

Status Client::read(void* session, const char* path, DataFormat* format, void* data, size_t* size)
{
    Status status = STS_OK;
    uint16_t objectId, objectInstanceId, resourceId, resourceInstanceId;

    LOG_INFO("Read, path: %s", path);

    if (Utils::convertPath(path, &objectId, &objectInstanceId, &resourceId, &resourceInstanceId) < 0) {
        LOG_ERROR("Path not found");
        return STS_ERR_NOT_FOUND;
    }

    if (session == mBootstrapHandler.getSession()) {
        if (resourceInstanceId != INVALID_ID || resourceId != INVALID_ID) {
            LOG_ERROR("Path not found");
            return STS_ERR_NOT_FOUND;
        }

        if ((status = mBootstrapHandler.read(format, data, size, objectId, objectInstanceId)) != STS_OK) {
            LOG_ERROR("Bootstrap read error: %d", status);
            return status;
        }

        return STS_OK;
    }

    for (ServerHandler* handler = mServerHandlerStorage.getFirstItem(); handler;
         handler = mServerHandlerStorage.getNextItem()) {
        if (session == handler->getSession()) {
            if ((status = handler->read(format, data, size, objectId, objectInstanceId, resourceId,
                                        resourceInstanceId)) != STS_OK) {
                LOG_ERROR("Device discover error: %d", status);
                return status;
            }

            return STS_OK;
        }
    }

    LOG_ERROR("Session not found");

    return STS_ERR_NOT_FOUND;
}

Status Client::write(void* session, const char* path, DataFormat format, void* data, size_t size)
{
    uint16_t objectId, objectInstanceId, resourceId, resourceInstanceId;

    LOG_INFO("Write, path: %s", path);

    if (Utils::convertPath(path, &objectId, &objectInstanceId, &resourceId, &resourceInstanceId) < 0) {
        LOG_ERROR("Path not found");
        return STS_ERR_NOT_FOUND;
    }

    if (session == mBootstrapHandler.getSession()) {
        if (resourceInstanceId != INVALID_ID) {
            LOG_ERROR("Path not found");
            return STS_ERR_NOT_FOUND;
        }

        Status status = mBootstrapHandler.write(format, data, size, objectId, objectInstanceId, resourceId);

        if (status != STS_OK) {
            LOG_ERROR("Bootstrap write error: %d", status);
        }

        return status;
    }

    LOG_ERROR("Session not found");

    return STS_ERR_NOT_FOUND;
}

Status Client::deleteInstance(void* session, const char* path)
{
    uint16_t objectId, objectInstanceId, resourceId, resourceInstanceId;

    LOG_INFO("Delete, path: %s", path);

    if (Utils::convertPath(path, &objectId, &objectInstanceId, &resourceId, &resourceInstanceId) < 0) {
        LOG_ERROR("Path not found");
        return STS_ERR_NOT_FOUND;
    }

    if (session == mBootstrapHandler.getSession()) {
        if (resourceId != INVALID_ID || resourceInstanceId != INVALID_ID) {
            LOG_ERROR("Path not found");
            return STS_ERR_NOT_FOUND;
        }

        Status status = mBootstrapHandler.deleteInstance(objectId, objectInstanceId);

        if (status != STS_OK) {
            LOG_ERROR("Bootstrap delete error: %d", status);
        }

        return status;
    }

    LOG_ERROR("Session not found");

    return STS_ERR_NOT_FOUND;
}

Status Client::bootstrapFinish(void* session)
{
    LOG_INFO("Boostrap finish");

    if (session == mBootstrapHandler.getSession()) {
        Status status = mBootstrapHandler.bootstrapFinish();

        if (status != STS_OK) {
            LOG_ERROR("Bootstrap finish error: %d", status);
        }

        return status;
    }

    LOG_ERROR("Session not found");

    return STS_ERR_NOT_FOUND;
}

void Client::bootstrapDiscover()
{
}

void Client::bootstrapRead()
{
}

Status Client::bootstrapWriteJSON(const char* path, const char* dataJSON)
{
    uint16_t objectId = INVALID_ID, objectInstanceId = INVALID_ID, resourceId = INVALID_ID,
             resourceInstanceId = INVALID_ID;

    if (Utils::convertPath(path, &objectId, &objectInstanceId, &resourceId, &resourceInstanceId) < 0 ||
        resourceInstanceId != INVALID_ID) {
        return STS_ERR_NOT_ALLOWED;
    }

    return mObjectManager.bootstrapWrite(DATA_FMT_SENML_JSON, reinterpret_cast<void*>(const_cast<char*>(dataJSON)),
                                         strlen(dataJSON), objectId, objectInstanceId, resourceId);
}

Status Client::bootstrapWrite(DataFormat dataFormat, void* data, size_t size, uint16_t objectId,
                              uint16_t objectInstanceId, uint16_t resourceId)
{
    return mBootstrapHandler.write(dataFormat, data, size, objectId, objectInstanceId, resourceId);
}

void Client::bootstrapDelete()
{
}

Status Client::deviceRead(const char* path, DataFormat reqFormat, void* data, size_t* size, DataFormat* format)
{
    LOG_DEBUG("Read, path: %s", path);

    //    return mObjectManager.read(ITF_DEVICE, path, DATA_FMT_TEXT, NULL, 0, reqFormat, data, size, format);

    return STS_OK;
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

void Client::bootstrapFinished(void* context, BootstrapHandler* handler, Status status)
{
    static_cast<Client*>(context)->onBootstrapFinished(status);
}

void Client::onBootstrapFinished(Status status)
{
    if (status == STS_OK) {
        LOG_INFO("Bootstrap finished");
    }
    else {
        LOG_ERROR("Bootstrap failed, status: %d", status);
    }

    if (mBootstrapCallback) {
        mBootstrapCallback(mBootstrapContext, status);
    }

    mState = STATE_INITIALIZED;

    if ((status = registration()) != STS_OK) {
        LOG_ERROR("Can't start registration: %d", status);

        if ((status = startBootstrap()) != STS_OK) {
            LOG_ERROR("Can't start bootstrap: %d", status);
        }
    }
}

void Client::updateRegistration(void* context, ResourceInstance* resInstance)
{
    static_cast<Client*>(context)->onUpdateRegistration(resInstance);
}

void Client::onUpdateRegistration(ResourceInstance* resInstance)
{
    uint16_t shortServerId =
        resInstance->getResource()->getObjectInstance()->getResourceInstance(RES_SHORT_SERVER_ID)->getInt();

    ServerHandler* handler = mServerHandlerStorage.getItemById(shortServerId);

    if (handler != NULL && handler->getState() == ServerHandler::STATE_REGISTERED) {
        Status status = handler->updateRegistration();

        if (status != STS_OK) {
            LOG_ERROR("Can't update registration, status %d", status);
        }
    }
}

Status Client::startBootstrap()
{
    Status status = STS_OK;

    LOG_INFO("Start bootstrap");

    if ((status = mBootstrapHandler.bootstrapRequest(&Client::bootstrapFinished, this)) != STS_OK) {
        return status;
    }

    mState = STATE_BOOTSTRAP;

    return STS_OK;
}

Status Client::createRegHandlers()
{
    Object* object = getObjectById(OBJ_LWM2M_SERVER);
    ASSERT(object);

    ObjectInstance* serverInstance = object->getFirstInstance();

    while (serverInstance) {
        ServerHandler* handler =
            mServerHandlerStorage.allocateItem(serverInstance->getResourceInstance(RES_SHORT_SERVER_ID)->getInt());
        ASSERT(handler);

        Status status = STS_OK;

        if ((status = handler->bind(mTransport)) != STS_OK) {
            LOG_ERROR("Can't bind to lwm2m server %d", status);

            mServerHandlerStorage.deallocateItem(handler);
        }

        serverInstance = object->getNextInstance();
    }

    if (mServerHandlerStorage.size() == 0) {
        LOG_ERROR("No valid lwm2m servers found");

        return STS_ERR_NOT_FOUND;
    }

    return STS_OK;
}

void Client::registrationStatus(void* context, ServerHandler* handler, Status status)
{
    static_cast<Client*>(context)->onRegistrationStatus(handler, status);
}

void Client::onRegistrationStatus(ServerHandler* handler, Status status)
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

    for (ServerHandler* handler = mServerHandlerStorage.getFirstItem(); handler;
         handler = mServerHandlerStorage.getNextItem()) {
        // Skip already started handlers
        if (handler->getState() != ServerHandler::STATE_INIT) {
            continue;
        }

        ObjectInstance* serverInstance = mObjectManager.getServerInstance(handler->getId());
        ASSERT(serverInstance);

        ResourceInstance* regPriority = serverInstance->getResourceInstance(RES_REGISTRATION_PRIORITY_ORDER);

        // Skip handlers without priority order
        if (!regPriority) {
            continue;
        }

        if (regPriority->getUint() <= minPriority) {
            mCurrentHandler = handler;
            minPriority = regPriority->getUint();
        }
    }

    if (mCurrentHandler) {
        return mCurrentHandler->registration(true, registrationStatus, this);
    }

    // Non blocking priority order

    for (ServerHandler* handler = mServerHandlerStorage.getFirstItem(); handler;
         handler = mServerHandlerStorage.getNextItem()) {
        ObjectInstance* serverInstance = mObjectManager.getServerInstance(handler->getId());
        ASSERT(serverInstance);

        ResourceInstance* regPriority = serverInstance->getResourceInstance(RES_REGISTRATION_PRIORITY_ORDER);
        ResourceInstance* failureBlock = serverInstance->getResourceInstance(RES_REG_FAILURE_BLOCK);

        if (regPriority && handler->getState() == ServerHandler::STATE_DEREGISTERED &&
            (!failureBlock || !failureBlock->getBool())) {
            mCurrentHandler = handler;
            return mCurrentHandler->registration(false, registrationStatus, this);
        }
    }

    // All other

    for (ServerHandler* handler = mServerHandlerStorage.getFirstItem(); handler;
         handler = mServerHandlerStorage.getNextItem()) {
        // Skip already started handlers
        if (handler->getState() != ServerHandler::STATE_INIT) {
            continue;
        }

        mCurrentHandler = handler;

        return mCurrentHandler->registration(false, registrationStatus, this);
    }

    mState = STATE_READY;

    return STS_OK;
}

}  // namespace openlwm2m
