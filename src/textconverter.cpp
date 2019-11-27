#include "textconverter.hpp"

#include <cstdio>
#include <cstring>

#include "log.hpp"
#include "utils.hpp"

#define LOG_MODULE "TextConverter"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

TextConverter::TextConverter() : DataConverter(DATA_FMT_TEXT)
{
}

TextConverter::~TextConverter()
{
}

Status TextConverter::startDecoding(void* data, size_t size)
{
    LOG_DEBUG("Start decoding, size: %zu", size);

    mData = data;
    mSize = size;
    mFinish = false;
    mObjectId = mObjectInstanceId = mResourceId = mResourceInstanceId = INVALID_ID;

    if (size > sBufferSize) {
        return STS_ERR_NO_MEM;
    }

    if (data) {
        memcpy(mBuffer, data, size);

        mBuffer[size] = '\0';
    }

    return STS_OK;
}

Status TextConverter::nextDecoding(ResourceData* resourceData)
{
    LOG_DEBUG("Next decoding");

    *resourceData = (ResourceData){mObjectId, mObjectInstanceId, mResourceId, mResourceInstanceId, DATA_TYPE_NONE};

    if (mFinish) {
        return STS_ERR_NOT_FOUND;
    }

    mFinish = true;

    if (mData) {
        resourceData->strValue = reinterpret_cast<char*>(mBuffer);
        resourceData->dataType = DATA_TYPE_STRING;
    }

    return STS_OK;
}

Status TextConverter::startEncoding(void* data, size_t size)
{
    LOG_DEBUG("Start encoding, size: %zu", size);

    mData = data;
    mSize = size;
    mFinish = false;

    return STS_OK;
}

Status TextConverter::nextEncoding(ResourceData* resourceData)
{
    if (mFinish) {
        return STS_ERR_NOT_ALLOWED;
    }

    mFinish = true;

    int ret = 0;

    switch (resourceData->dataType) {
        case DATA_TYPE_INT:
            ret = snprintf(reinterpret_cast<char*>(mBuffer), sBufferSize + 1, "%ld", resourceData->intValue);
            break;

        case DATA_TYPE_UINT:
            ret = snprintf(reinterpret_cast<char*>(mBuffer), sBufferSize + 1, "%lu", resourceData->uintValue);
            break;

        case DATA_TYPE_STRING:
            ret = Utils::strCopy(reinterpret_cast<char*>(mBuffer), resourceData->strValue, sizeof(mBuffer));
            break;

        default:
            return STS_ERR;
    }

    if (ret < 0) {
        return STS_ERR_INVALID_VALUE;
    }

    if (ret >= sBufferSize) {
        return STS_ERR_NO_MEM;
    }

    if (static_cast<size_t>(ret) > mSize) {
        return STS_ERR_NO_MEM;
    }

    mSize = ret;

    LOG_DEBUG("Next encoding, value: %s, size: %zu", reinterpret_cast<char*>(mBuffer), mSize);

    return STS_OK;
}

Status TextConverter::finishEncoding(size_t* size)
{
    if (mFinish) {
        memcpy(mData, mBuffer, mSize);
        *size = mSize;
    }
    else {
        *size = 0;
    }

    return STS_OK;
}

/*******************************************************************************
 * Private
 ******************************************************************************/

}  // namespace openlwm2m