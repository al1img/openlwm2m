#include "bootstraphandler.hpp"

#include <cstdio>

#include "log.hpp"
#include "utils.hpp"

#define LOG_MODULE "BootstrapHandler"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

BootstrapHandler::BootstrapHandler(const char* clientName, ObjectManager& objectManager)
    : mClientName(clientName),
      mObjectManager(objectManager),
      mTransport(NULL),
      mSession(NULL),
      mState(STATE_INIT),
      mTimer(0)
{
}

BootstrapHandler::~BootstrapHandler()
{
    if (mState == STATE_BOOTSTRAPING) {
        mTimer.stop();
        mTransport->deleteSession(mSession);
    }
}

Status BootstrapHandler::bind(TransportItf* transport)
{
    mTransport = transport;

    return STS_OK;
}

Status BootstrapHandler::bootstrapRequest(RequestHandler handler, void* context)
{
    Status status = STS_OK;

    if (mState != STATE_INIT && mState != STATE_BOOTSTAPPED) {
        return STS_ERR_NOT_ALLOWED;
    }

    Object* object = mObjectManager.getObjectById(OBJ_LWM2M_SECURITY);
    ASSERT(object);

    ObjectInstance* securityInstance = NULL;

    for (ObjectInstance* instance = object->getFirstInstance(); instance; instance = object->getNextInstance()) {
        if (static_cast<ResourceBool*>(instance->getResourceInstance(RES_BOOTSTRAP_SERVER))->getValue()) {
            securityInstance = instance;
            break;
        }
    }

    if (!securityInstance) {
        return STS_ERR_NOT_FOUND;
    }

    const char* serverUri =
        static_cast<ResourceString*>(securityInstance->getResourceInstance(RES_LWM2M_SERVER_URI))->getValue();

    LOG_INFO("Bootstrap request to: %s", serverUri);

    mSession = mTransport->createSession(serverUri, &status);

    if (!mSession) {
        return status;
    }

    DataFormat preferredFormat = CONFIG_DEFAULT_DATA_FORMAT;

    if ((status = mTransport->bootstrapRequest(mSession, mClientName, &preferredFormat,
                                               &BootstrapHandler::bootstrapRequstCallback, this)) != STS_OK) {
        return status;
    }

    mRequestHandler = handler;
    mRequestContext = context;

    mState = STATE_BOOTSTRAPING;

    return STS_OK;
}

Status BootstrapHandler::bootstrapFinish()
{
    Status status = STS_OK;

    if (mState != STATE_BOOTSTRAPING) {
        return STS_ERR_NOT_ALLOWED;
    }

    LOG_DEBUG("Bootstrap finish");

    bootstrapFinish(status);

    return status;
}

Status BootstrapHandler::discover(char* data, size_t* size, uint16_t objectId)
{
    size_t curSize = 0;

    if (mState != STATE_BOOTSTRAPING) {
        return STS_ERR_NOT_ALLOWED;
    }

    mTimer.restart();

    LOG_DEBUG("Bootstrap discover /%d", objectId);

    int ret = snprintf(data, *size, "lwm2m=\"%s\"", LWM2M_VERSION);
    if (ret < 0 || static_cast<size_t>(ret) >= *size) {
        return STS_ERR_NO_MEM;
    }

    curSize += ret;

    if (objectId == INVALID_ID) {
        for (Object* object = mObjectManager.getFirstObject(); object != NULL;
             object = mObjectManager.getNextObject()) {
            ret = discoverObject(&data[curSize], *size - curSize, object);

            if (ret < 0) {
                return STS_ERR_NO_MEM;
            }

            curSize += ret;
        }
    }
    else {
        Object* object = mObjectManager.getObjectById(objectId);

        if (!object) {
            return STS_ERR_NOT_FOUND;
        }

        ret = discoverObject(&data[curSize], *size - curSize, object);

        if (ret < 0) {
            return STS_ERR_NO_MEM;
        }

        curSize += ret;
    }

    *size = curSize;

    return STS_OK;
}

Status BootstrapHandler::read(DataFormat* dataFormat, void* data, size_t* size, uint16_t objectId,
                              uint16_t objectInstanceId)
{
    Status status = STS_OK;

    if (mState != STATE_BOOTSTRAPING) {
        return STS_ERR_NOT_ALLOWED;
    }

    mTimer.restart();

    LOG_DEBUG("Bootstrap read /%d/%d", objectId, objectInstanceId);

    if (objectId != OBJ_LWM2M_SECURITY && objectId != OBJ_LWM2M_SERVER) {
        return STS_ERR_NOT_ALLOWED;
    }

    if (*dataFormat == DATA_FMT_ANY) {
        *dataFormat = CONFIG_DEFAULT_DATA_FORMAT;
    }

    DataConverter* outConverter = mObjectManager.getConverterById(*dataFormat);

    if (outConverter == NULL) {
        return STS_ERR_NOT_FOUND;
    }

    if ((status = outConverter->startEncoding(data, *size)) != STS_OK) {
        return status;
    }

    Object* object = mObjectManager.getObjectById(objectId);

    if (!object) {
        return STS_ERR_NOT_FOUND;
    }

    if (objectInstanceId == INVALID_ID) {
        if ((status = object->read(outConverter)) != STS_OK) {
            return status;
        }
    }
    else {
        ObjectInstance* objectInstance = object->getInstanceById(objectInstanceId);

        if ((status = objectInstance->read(outConverter)) != STS_OK) {
            return status;
        }
    }

    return outConverter->finishEncoding(size);
}

Status BootstrapHandler::write(DataFormat dataFormat, void* data, size_t size, uint16_t objectId,
                               uint16_t objectInstanceId, uint16_t resourceId)
{
    Status status = STS_OK;

    if (mState != STATE_BOOTSTRAPING) {
        return STS_ERR_NOT_ALLOWED;
    }

    mTimer.restart();

    LOG_DEBUG("Bootstrap write /%d/%d/%d", objectId, objectInstanceId, resourceId);

    if ((status = mObjectManager.bootstrapWrite(dataFormat, data, size, objectId, objectInstanceId, resourceId)) !=
        STS_OK) {
        return status;
    }

    return STS_OK;
}

/*******************************************************************************
 * Private
 ******************************************************************************/

Status BootstrapHandler::timerCallback(void* context)
{
    return static_cast<BootstrapHandler*>(context)->onTimerCallback();
}

Status BootstrapHandler::onTimerCallback()
{
    bootstrapFinish(STS_ERR_TIMEOUT);

    return STS_OK;
}

void BootstrapHandler::bootstrapRequstCallback(void* context, void* data, Status status)
{
    static_cast<BootstrapHandler*>(context)->onBootstrapRequestCallback(status);
}

void BootstrapHandler::onBootstrapRequestCallback(Status status)
{
    if (status == STS_OK) {
        LOG_INFO("Bootstrap request success");

        mTimer.start(sBootstrapTimeoutMs, &BootstrapHandler::timerCallback, this, true);
    }
    else {
        bootstrapFinish(status);
    }
}

void BootstrapHandler::bootstrapFinish(Status status)
{
    if (status == STS_OK) {
        LOG_INFO("Bootstrap finished");
    }
    else {
        LOG_ERROR("Bootstrap failed, status: %d", status);
    }

    mState = STATE_BOOTSTAPPED;
    mTimer.stop();
    mTransport->deleteSession(mSession);

    if (mRequestHandler) {
        mRequestHandler(mRequestContext, this, status);
    }
}

int BootstrapHandler::discoverObject(char* data, size_t maxSize, Object* object)
{
    char buf[16];
    int curSize = 0;

    ObjectInstance* instance = object->getFirstInstance();

    if (!instance) {
        int ret = Utils::makePath(buf, sizeof(buf), object->getId());
        if (ret < 0) {
            return -1;
        }

        ret = snprintf(&data[curSize], maxSize - curSize, ",<%s>", buf);

        if (ret < 0 || static_cast<size_t>(ret) >= maxSize - curSize) {
            return -1;
        }

        curSize += ret;

        return curSize;
    }

    for (; instance != NULL; instance = object->getNextInstance()) {
        int ret = Utils::makePath(buf, sizeof(buf), object->getId(), instance->getId());
        if (ret < 0) {
            return -1;
        }

        if (object->getId() == OBJ_LWM2M_SECURITY &&
            !static_cast<ResourceBool*>(instance->getResourceInstance(RES_BOOTSTRAP_SERVER))->getValue()) {
            ret = snprintf(
                &data[curSize], maxSize - curSize, ",<%s>;ssid=%ld;uri=%s", buf,
                static_cast<ResourceInt*>(instance->getResourceInstance(RES_SECURITY_SHORT_SERVER_ID))->getValue(),
                static_cast<ResourceString*>(instance->getResourceInstance(RES_LWM2M_SERVER_URI))->getValue());
        }
        else {
            ret = snprintf(&data[curSize], maxSize - curSize, ",<%s>", buf);
        }

        if (ret < 0 || static_cast<size_t>(ret) >= maxSize - curSize) {
            return -1;
        }

        curSize += ret;
    }

    return curSize;
}

}  // namespace openlwm2m
