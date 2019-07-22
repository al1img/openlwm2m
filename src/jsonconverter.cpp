#include "jsonconverter.hpp"

#include <cstdlib>
#include <cstring>

#include "log.hpp"
#include "utils.hpp"

#define LOG_MODULE "JsonConverter"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

JsonConverter::JsonConverter() : DataConverter(DATA_FMT_SENML_JSON), mCurPos(NULL), mEndPos(NULL)
{
}

JsonConverter::~JsonConverter()
{
}

Status JsonConverter::startDecoding(const char* path, void* data, size_t size)
{
    Status status = STS_OK;

    LOG_DEBUG("Start decoding, path: %s, size: %zu", path, size);

    mCurPos = static_cast<char*>(data);
    mEndPos = mCurPos + size;

    mBaseTime = 0;

    if (Utils::strCopy(mBaseName, path, sStringSize) < 0) {
        return STS_ERR_NO_MEM;
    }

    if ((status = skipWhiteSpaces()) != STS_OK) {
        return status;
    }

    if (*mCurPos != JSON_TOKEN_BEGIN_ARRAY) {
        return STS_ERR_INVALID_VALUE;
    }

    mCurPos++;

    return STS_OK;
}

Status JsonConverter::nextDecoding(ResourceData* resourceData)
{
    Status status = STS_OK;

    LOG_DEBUG("Next decoding");

    *resourceData = (ResourceData){INVALID_ID, INVALID_ID, INVALID_ID, INVALID_ID, DATA_TYPE_NONE};
    mNameFound = false;

    if (mCurPos == NULL) {
        return STS_ERR_INVALID_STATE;
    }

    if (mCurPos == mEndPos) {
        return STS_ERR_NOT_EXIST;
    }

    if ((status = skipWhiteSpaces()) != STS_OK) {
        return status;
    }

    if (*mCurPos != JSON_TOKEN_BEGIN_ITEM) {
        return STS_ERR_INVALID_VALUE;
    }

    mCurPos++;

    bool lastElement = false;

    while (!lastElement) {
        if ((status = skipWhiteSpaces()) != STS_OK) {
            return status;
        }

        if (*mCurPos == JSON_TOKEN_STRING) {
            mCurPos++;

            char* field = mCurPos;

            if ((status = skipSymbolsTill(JSON_TOKEN_STRING)) != STS_OK) {
                return status;
            }

            if ((status = processItem(resourceData, field, mCurPos - field)) != STS_OK) {
                return status;
            }
        }

        if ((status = isLastElement(JSON_TOKEN_END_ITEM, &lastElement)) != STS_OK) {
            return status;
        }

        mCurPos++;
    }

    if ((status = isLastElement(JSON_TOKEN_END_ARRAY, &lastElement)) != STS_OK) {
        return status;
    }

    mCurPos++;

    if (lastElement) {
        mCurPos = mEndPos;
    }

    if (!mNameFound) {
        if ((convertName(mBaseName, resourceData)) != STS_OK) {
            return status;
        }
    }

    LOG_DEBUG("Name, objectId: %d, objectInstanceId: %d, resourseId: %d, resourceInstanceId: %d",
              resourceData->objectId, resourceData->objectInstanceId, resourceData->resourceId,
              resourceData->resourceInstanceId);

    return STS_OK;
}

/*******************************************************************************
 * Private
 ******************************************************************************/

Status JsonConverter::skipWhiteSpaces()
{
    while (isspace(*mCurPos) && mCurPos != mEndPos) {
        mCurPos++;
    }

    if (mCurPos == mEndPos) {
        return STS_ERR_INVALID_VALUE;
    }

    return STS_OK;
}

Status JsonConverter::skipSymbolsTill(char expectedChar)
{
    while (*mCurPos != expectedChar && mCurPos != mEndPos) {
        mCurPos++;
    }

    if (mCurPos == mEndPos) {
        return STS_ERR_INVALID_VALUE;
    }

    return STS_OK;
}

bool JsonConverter::isSymbolInStr(char c, const char* str)
{
    while (*str) {
        if (c == *str) {
            return true;
        }

        str++;
    }

    return false;
}

Status JsonConverter::skipTillEndValue()
{
    while (!isspace(*mCurPos) && !isSymbolInStr(*mCurPos, (const char[]){TOKEN_SET}) && mCurPos != mEndPos) {
        mCurPos++;
    }

    if (mCurPos == mEndPos) {
        return STS_ERR_INVALID_VALUE;
    }

    return STS_OK;
}

Status JsonConverter::isLastElement(char endChar, bool* lastElement)
{
    while (isspace(*mCurPos) && mCurPos != mEndPos) {
        mCurPos++;
    }

    if (mCurPos == mEndPos) {
        return STS_ERR_INVALID_VALUE;
    }

    if (*mCurPos == JSON_TOKEN_SPLIT_ITEM) {
        *lastElement = false;

        return STS_OK;
    }

    if (*mCurPos == endChar) {
        *lastElement = true;

        return STS_OK;
    }

    return STS_ERR_INVALID_VALUE;
}

Status JsonConverter::processItem(ResourceData* resourceData, char* field, size_t size)
{
    Status status = STS_OK;

    mCurPos++;

    if ((status = skipWhiteSpaces()) != STS_OK) {
        return status;
    }

    if (*mCurPos != JSON_TOKEN_SPLIT_VALUE) {
        return STS_ERR_INVALID_VALUE;
    }

    mCurPos++;

    if ((status = skipWhiteSpaces()) != STS_OK) {
        return status;
    }

    if (strncmp(JSON_ITEM_BASE_NAME, field, size) == 0) {
        status = processBaseName(resourceData);
    }
    else if (strncmp(JSON_ITEM_BASE_TIME, field, size) == 0) {
        status = processBaseTime(resourceData);
    }
    else if (strncmp(JSON_ITEM_NAME, field, size) == 0) {
        status = processName(resourceData);
    }
    else if (strncmp(JSON_ITEM_TIME, field, size) == 0) {
        status = processTime(resourceData);
    }
    else if (strncmp(JSON_ITEM_FLOAT_VALUE, field, size) == 0) {
        status = processFloatValue(resourceData);
    }
    else if (strncmp(JSON_ITEM_BOOLEAN_VALUE, field, size) == 0) {
        status = processBooleanValue(resourceData);
    }
    else if (strncmp(JSON_ITEM_OBJECT_LINK_VALUE, field, size) == 0) {
        status = processObjectLinkValue(resourceData);
    }
    else if (strncmp(JSON_ITEM_OPAQUE_VALUE, field, size) == 0) {
        status = processOpaqueValue(resourceData);
    }
    else if (strncmp(JSON_ITEM_STRING_VALUE, field, size) == 0) {
        status = processStringValue(resourceData);
    }
    else {
        status = STS_ERR_INVALID_VALUE;
    }

    return status;
}

Status JsonConverter::getString(char* src, size_t size)
{
    Status status = STS_OK;

    if (*mCurPos != JSON_TOKEN_STRING) {
        return STS_ERR_INVALID_VALUE;
    }

    mCurPos++;

    char* beginStr = mCurPos;

    if ((status = skipSymbolsTill(JSON_TOKEN_STRING)) != STS_OK) {
        return status;
    }

    if (static_cast<size_t>(mCurPos - beginStr + 1) > size) {
        return STS_ERR_NO_MEM;
    }

    int i = 0;

    while (beginStr != mCurPos) {
        src[i++] = *beginStr++;
    }

    src[i] = '\0';

    mCurPos++;

    return STS_OK;
}

Status JsonConverter::getFloat(double* value)
{
    Status status = STS_OK;

    char* beginStr = mCurPos;

    if ((status = skipTillEndValue()) != STS_OK) {
        return status;
    }

    char* endStr;

    *value = strtod(beginStr, &endStr);

    if (mCurPos != endStr) {
        return STS_ERR_INVALID_VALUE;
    }

    return STS_OK;
}

Status JsonConverter::convertName(char* name, ResourceData* resourceData)
{
    uint16_t* setValues[4] = {&resourceData->objectId, &resourceData->objectInstanceId, &resourceData->resourceId,
                              &resourceData->resourceInstanceId};

    for (int i = 0; i < 4; i++) {
        if (name == NULL || *name == '\0') {
            return STS_OK;
        }

        if (*name != '/') {
            return STS_ERR_INVALID_VALUE;
        }

        name++;

        uint64_t value = strtol(name, &name, 0);

        if (value > UINT16_MAX) {
            return STS_ERR_INVALID_VALUE;
        }

        *setValues[i] = value;
    }

    return STS_OK;
}

Status JsonConverter::processBaseName(ResourceData* resourceData)
{
    Status status = STS_OK;

    if ((status = getString(mBaseName, sStringSize)) != STS_OK) {
        return status;
    }

    LOG_DEBUG("Base name: %s", mBaseName);

    return STS_OK;
}

Status JsonConverter::processBaseTime(ResourceData* resourceData)
{
    Status status = STS_OK;
    double value;

    if ((status = getFloat(&value)) != STS_OK) {
        return status;
    }

    mBaseTime = value;

    if (value != mBaseTime) {
        return STS_ERR_INVALID_VALUE;
    }

    LOG_DEBUG("Base time: %lu", mBaseTime);

    return STS_OK;
}

Status JsonConverter::processName(ResourceData* resourceData)
{
    Status status = STS_OK;
    char name[sStringSize];

    int len = Utils::strCopy(name, mBaseName, sStringSize);
    if (len < 0) {
        return STS_ERR_NO_MEM;
    }

    if ((status = getString(&name[len], sStringSize - len)) != STS_OK) {
        return status;
    }

    if ((status = convertName(name, resourceData)) != STS_OK) {
        return status;
    }

    mNameFound = true;

    return STS_OK;
}

Status JsonConverter::processTime(ResourceData* resourceData)
{
    Status status = STS_OK;
    double value;

    if ((status = getFloat(&value)) != STS_OK) {
        return status;
    }

    int64_t time = value;

    if (value != time) {
        return STS_ERR_INVALID_VALUE;
    }

    resourceData->timestamp = mBaseTime + time;

    LOG_DEBUG("Timestamp: %lu", resourceData->timestamp);

    return STS_OK;
}

Status JsonConverter::processFloatValue(ResourceData* resourceData)
{
    Status status = STS_OK;

    if ((status = getFloat(&resourceData->floatValue)) != STS_OK) {
        return status;
    }

    resourceData->dataType = DATA_TYPE_FLOAT;

    LOG_DEBUG("Float value: %f", resourceData->floatValue);

    return STS_OK;
}

Status JsonConverter::processBooleanValue(ResourceData* resourceData)
{
    Status status = STS_OK;

    char* beginStr = mCurPos;

    if ((status = skipTillEndValue()) != STS_OK) {
        return status;
    }

    if (strncmp("true", beginStr, mCurPos - beginStr) == 0) {
        resourceData->boolValue = 1;
    }
    else if (strncmp("false", beginStr, mCurPos - beginStr) == 0) {
        resourceData->boolValue = 0;
    }
    else {
        return STS_ERR_INVALID_VALUE;
    }

    resourceData->dataType = DATA_TYPE_BOOL;

    LOG_DEBUG("Boolean value: %d", resourceData->boolValue);

    return STS_OK;
}

Status JsonConverter::processObjectLinkValue(ResourceData* resourceData)
{
    Status status = STS_OK;

    char tmpStr[sStringSize];

    if ((status = getString(tmpStr, sStringSize)) != STS_OK) {
        return status;
    }

    char* endStr;

    uint64_t value = strtol(tmpStr, &endStr, 0);

    if (value > UINT16_MAX) {
        return STS_ERR_INVALID_VALUE;
    }

    if (*endStr != ':') {
        return STS_ERR_INVALID_VALUE;
    }

    resourceData->objlnkValue.objectId = value;

    endStr++;

    value = strtol(endStr, &endStr, 0);

    if (value > UINT16_MAX) {
        return STS_ERR_INVALID_VALUE;
    }

    if (*endStr != '\0') {
        return STS_ERR_INVALID_VALUE;
    }

    resourceData->objlnkValue.objectInstanceId = value;

    resourceData->dataType = DATA_TYPE_OBJLINK;

    LOG_DEBUG("Object link value: %d:%d", resourceData->objlnkValue.objectId,
              resourceData->objlnkValue.objectInstanceId);

    return STS_OK;
}

Status JsonConverter::processOpaqueValue(ResourceData* resourceData)
{
    Status status = STS_OK;

    // TODO
    LOG_WARNING("Opaque value is not implemented");

    if ((status = skipTillEndValue()) != STS_OK) {
        return status;
    }

    return STS_ERR;
}

Status JsonConverter::processStringValue(ResourceData* resourceData)
{
    Status status = STS_OK;

    if ((status = getString(reinterpret_cast<char*>(mBuffer), sBufferSize)) != STS_OK) {
        return status;
    }

    resourceData->strValue = reinterpret_cast<char*>(mBuffer);
    resourceData->dataType = DATA_TYPE_STRING;

    LOG_DEBUG("String value: %s", reinterpret_cast<char*>(mBuffer));

    return STS_OK;
}

}  // namespace openlwm2m
