#include "jsonconverter.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "log.hpp"
#include "utils.hpp"

#define LOG_MODULE "JsonConverter"

namespace openlwm2m {

/*******************************************************************************
 * Public
 ******************************************************************************/

JsonConverter::JsonConverter() : DataConverter(DATA_FMT_SENML_JSON), mDecodingPos(NULL), mDecodingEndPos(NULL)
{
}

JsonConverter::~JsonConverter()
{
}

Status JsonConverter::startDecoding(void* data, size_t size)
{
    Status status = STS_OK;

    LOG_DEBUG("Start decoding, size: %zu", size);

    mDecodingPos = static_cast<char*>(data);
    mDecodingEndPos = mDecodingPos + size;

    mDecodingBaseTime = 0;
    mDecodingBaseName[0] = '\0';

    if ((status = skipWhiteSpaces()) != STS_OK) {
        return status;
    }

    if (*mDecodingPos != JSON_TOKEN_BEGIN_ARRAY) {
        return STS_ERR_INVALID_VALUE;
    }

    mDecodingPos++;

    return STS_OK;
}

Status JsonConverter::nextDecoding(ResourceData* resourceData)
{
    Status status = STS_OK;

    LOG_DEBUG("Next decoding");

    *resourceData = (ResourceData){INVALID_ID, INVALID_ID, INVALID_ID, INVALID_ID, DATA_TYPE_NONE};
    mDecodingNameFound = false;

    if (mDecodingPos == NULL) {
        return STS_ERR_NOT_ALLOWED;
    }

    if (mDecodingPos == mDecodingEndPos) {
        return STS_ERR_NOT_FOUND;
    }

    if ((status = skipWhiteSpaces()) != STS_OK) {
        return status;
    }

    if (*mDecodingPos != JSON_TOKEN_BEGIN_ITEM) {
        return STS_ERR_INVALID_VALUE;
    }

    mDecodingPos++;

    bool lastElement = false;

    while (!lastElement) {
        if ((status = skipWhiteSpaces()) != STS_OK) {
            return status;
        }

        if (*mDecodingPos == JSON_TOKEN_STRING) {
            mDecodingPos++;

            char* field = mDecodingPos;

            if ((status = skipSymbolsTill(JSON_TOKEN_STRING)) != STS_OK) {
                return status;
            }

            if ((status = decodeItem(resourceData, field, mDecodingPos - field)) != STS_OK) {
                return status;
            }
        }

        if ((status = isLastElement(JSON_TOKEN_END_ITEM, &lastElement)) != STS_OK) {
            return status;
        }

        mDecodingPos++;
    }

    if ((status = isLastElement(JSON_TOKEN_END_ARRAY, &lastElement)) != STS_OK) {
        return status;
    }

    mDecodingPos++;

    if (lastElement) {
        mDecodingPos = mDecodingEndPos;
    }

    if (!mDecodingNameFound) {
        if (Utils::convertPath(mDecodingBaseName, &resourceData->objectId, &resourceData->objectInstanceId,
                               &resourceData->resourceId, &resourceData->resourceInstanceId) < 0) {
            return STS_ERR_INVALID_VALUE;
        }
    }

    LOG_DEBUG("Name, objectId: %d, objectInstanceId: %d, resourseId: %d, resourceInstanceId: %d",
              resourceData->objectId, resourceData->objectInstanceId, resourceData->resourceId,
              resourceData->resourceInstanceId);

    return STS_OK;
}

Status JsonConverter::startEncoding(void* data, size_t size)
{
    LOG_DEBUG("Start encoding");

    mEncodingBeginPos = static_cast<char*>(data);
    mEncodingPos = mEncodingBeginPos;
    mEncodingEndPos = mEncodingBeginPos + size;

    mEncodingBaseTime = 0;
    mEncodingBaseName[0] = '\0';
    mHasPrevData = false;

    if (size == 0) {
        return STS_ERR_NO_MEM;
    }

    mHasItems = false;

    return STS_OK;
}

Status JsonConverter::nextEncoding(ResourceData* resourceData)
{
    Status status = STS_OK;
    char name[sStringSize + 1];

    LOG_DEBUG("Next encoding");

    if (!mHasItems) {
        *mEncodingPos++ = JSON_TOKEN_BEGIN_ARRAY;
        mHasItems = true;
    }

    if (Utils::makePath(name, sStringSize, resourceData->objectId, resourceData->objectInstanceId,
                        resourceData->resourceId, resourceData->resourceInstanceId) < 0) {
        return STS_ERR_NO_MEM;
    }

    if (mHasPrevData) {
        // Determine base name
        char prevName[sStringSize + 1];
        char* baseNamePos = mEncodingBaseName;
        char* namePos = name;

        while (*baseNamePos != '\0' && *namePos != '\0' && *baseNamePos == *namePos) {
            baseNamePos++;
            namePos++;
        }

        if (Utils::strCopy(prevName, baseNamePos, sStringSize) < 0) {
            return STS_ERR_NO_MEM;
        }

        *baseNamePos = '\0';

        if ((status = encodeItem(mEncodingBaseName, prevName, &mPrevResourceData)) != STS_OK) {
            return status;
        }

        mHasPrevData = false;
    }
    else if (mEncodingBaseName[0] == '\0' || strncmp(mEncodingBaseName, name, strlen(mEncodingBaseName)) != 0) {
        // Store resource data to determine basename
        if (Utils::strCopy(mEncodingBaseName, name, sStringSize) < 0) {
            return STS_ERR_NO_MEM;
        }

        mPrevResourceData = *resourceData;
        mHasPrevData = true;

        return STS_OK;
    }

    if ((status = encodeItem(NULL, &name[strlen(mEncodingBaseName)], resourceData)) != STS_OK) {
        return status;
    }

    return STS_OK;
}

Status JsonConverter::finishEncoding(size_t* size)
{
    Status status = STS_OK;

    if (mHasItems) {
        if (mHasPrevData) {
            if ((status = encodeItem(mEncodingBaseName, NULL, &mPrevResourceData)) != STS_OK) {
                return status;
            }

            mHasPrevData = false;
        }

        if (mEncodingEndPos - mEncodingPos <= 0) {
            return STS_ERR_NO_MEM;
        }

        *mEncodingPos++ = JSON_TOKEN_END_ARRAY;

        *size = mEncodingPos - mEncodingBeginPos;
    }
    else {
        *size = 0;
    }

    LOG_DEBUG("Finish encoding, size: %d", *size);

    return STS_OK;
}

/*******************************************************************************
 * Private
 ******************************************************************************/

Status JsonConverter::skipWhiteSpaces()
{
    while (isspace(*mDecodingPos) && mDecodingPos != mDecodingEndPos) {
        mDecodingPos++;
    }

    if (mDecodingPos == mDecodingEndPos) {
        return STS_ERR_INVALID_VALUE;
    }

    return STS_OK;
}

Status JsonConverter::skipSymbolsTill(char expectedChar)
{
    while (*mDecodingPos != expectedChar && mDecodingPos != mDecodingEndPos) {
        mDecodingPos++;
    }

    if (mDecodingPos == mDecodingEndPos) {
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
    while (!isspace(*mDecodingPos) && !isSymbolInStr(*mDecodingPos, (const char[]){TOKEN_SET}) &&
           mDecodingPos != mDecodingEndPos) {
        mDecodingPos++;
    }

    if (mDecodingPos == mDecodingEndPos) {
        return STS_ERR_INVALID_VALUE;
    }

    return STS_OK;
}

Status JsonConverter::isLastElement(char endChar, bool* lastElement)
{
    while (isspace(*mDecodingPos) && mDecodingPos != mDecodingEndPos) {
        mDecodingPos++;
    }

    if (mDecodingPos == mDecodingEndPos) {
        return STS_ERR_INVALID_VALUE;
    }

    if (*mDecodingPos == JSON_TOKEN_SPLIT_ITEM) {
        *lastElement = false;

        return STS_OK;
    }

    if (*mDecodingPos == endChar) {
        *lastElement = true;

        return STS_OK;
    }

    return STS_ERR_INVALID_VALUE;
}

Status JsonConverter::decodeItem(ResourceData* resourceData, char* field, size_t size)
{
    Status status = STS_OK;

    mDecodingPos++;

    if ((status = skipWhiteSpaces()) != STS_OK) {
        return status;
    }

    if (*mDecodingPos != JSON_TOKEN_SPLIT_VALUE) {
        return STS_ERR_INVALID_VALUE;
    }

    mDecodingPos++;

    if ((status = skipWhiteSpaces()) != STS_OK) {
        return status;
    }

    if (strncmp(JSON_ITEM_BASE_NAME, field, size) == 0) {
        status = decodeBaseName(resourceData);
    }
    else if (strncmp(JSON_ITEM_BASE_TIME, field, size) == 0) {
        status = decodeBaseTime(resourceData);
    }
    else if (strncmp(JSON_ITEM_NAME, field, size) == 0) {
        status = decodeName(resourceData);
    }
    else if (strncmp(JSON_ITEM_TIME, field, size) == 0) {
        status = decodeTime(resourceData);
    }
    else if (strncmp(JSON_ITEM_FLOAT_VALUE, field, size) == 0) {
        status = decodeFloatValue(resourceData);
    }
    else if (strncmp(JSON_ITEM_BOOLEAN_VALUE, field, size) == 0) {
        status = decodeBooleanValue(resourceData);
    }
    else if (strncmp(JSON_ITEM_OBJECT_LINK_VALUE, field, size) == 0) {
        status = decodeObjectLinkValue(resourceData);
    }
    else if (strncmp(JSON_ITEM_OPAQUE_VALUE, field, size) == 0) {
        status = decodeOpaqueValue(resourceData);
    }
    else if (strncmp(JSON_ITEM_STRING_VALUE, field, size) == 0) {
        status = decodeStringValue(resourceData);
    }
    else {
        status = STS_ERR_INVALID_VALUE;
    }

    return status;
}

Status JsonConverter::getString(char* src, size_t size)
{
    Status status = STS_OK;

    if (*mDecodingPos != JSON_TOKEN_STRING) {
        return STS_ERR_INVALID_VALUE;
    }

    mDecodingPos++;

    char* beginStr = mDecodingPos;

    if ((status = skipSymbolsTill(JSON_TOKEN_STRING)) != STS_OK) {
        return status;
    }

    if (static_cast<size_t>(mDecodingPos - beginStr + 1) > size) {
        return STS_ERR_NO_MEM;
    }

    int i = 0;

    while (beginStr != mDecodingPos) {
        src[i++] = *beginStr++;
    }

    src[i] = '\0';

    mDecodingPos++;

    return STS_OK;
}

Status JsonConverter::getFloat(double* value)
{
    Status status = STS_OK;

    char* beginStr = mDecodingPos;

    if ((status = skipTillEndValue()) != STS_OK) {
        return status;
    }

    char* endStr;

    *value = strtod(beginStr, &endStr);

    if (mDecodingPos != endStr) {
        return STS_ERR_INVALID_VALUE;
    }

    return STS_OK;
}

Status JsonConverter::decodeBaseName(ResourceData* resourceData)
{
    Status status = STS_OK;

    if ((status = getString(mDecodingBaseName, sStringSize)) != STS_OK) {
        return status;
    }

    LOG_DEBUG("Base name: %s", mDecodingBaseName);

    return STS_OK;
}

Status JsonConverter::decodeBaseTime(ResourceData* resourceData)
{
    Status status = STS_OK;
    double value;

    if ((status = getFloat(&value)) != STS_OK) {
        return status;
    }

    mDecodingBaseTime = value;

    if (value != mDecodingBaseTime) {
        return STS_ERR_INVALID_VALUE;
    }

    LOG_DEBUG("Base time: %ld", mDecodingBaseTime);

    return STS_OK;
}

Status JsonConverter::decodeName(ResourceData* resourceData)
{
    Status status = STS_OK;
    char name[sStringSize];

    int len = Utils::strCopy(name, mDecodingBaseName, sStringSize);
    if (len < 0) {
        return STS_ERR_NO_MEM;
    }

    if ((status = getString(&name[len], sStringSize - len)) != STS_OK) {
        return status;
    }

    if (Utils::convertPath(name, &resourceData->objectId, &resourceData->objectInstanceId, &resourceData->resourceId,
                           &resourceData->resourceInstanceId) < 0) {
        return STS_ERR_INVALID_VALUE;
    }

    mDecodingNameFound = true;

    return STS_OK;
}

Status JsonConverter::decodeTime(ResourceData* resourceData)
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

    resourceData->timestamp = mDecodingBaseTime + time;

    LOG_DEBUG("Timestamp: %ld", resourceData->timestamp);

    return STS_OK;
}

Status JsonConverter::decodeFloatValue(ResourceData* resourceData)
{
    Status status = STS_OK;

    if ((status = getFloat(&resourceData->floatValue)) != STS_OK) {
        return status;
    }

    resourceData->dataType = DATA_TYPE_FLOAT;

    LOG_DEBUG("Float value: %.16g", resourceData->floatValue);

    return STS_OK;
}

Status JsonConverter::decodeBooleanValue(ResourceData* resourceData)
{
    Status status = STS_OK;

    char* beginStr = mDecodingPos;

    if ((status = skipTillEndValue()) != STS_OK) {
        return status;
    }

    if (strncmp("true", beginStr, mDecodingPos - beginStr) == 0) {
        resourceData->boolValue = 1;
    }
    else if (strncmp("false", beginStr, mDecodingPos - beginStr) == 0) {
        resourceData->boolValue = 0;
    }
    else {
        return STS_ERR_INVALID_VALUE;
    }

    resourceData->dataType = DATA_TYPE_BOOL;

    LOG_DEBUG("Boolean value: %d", resourceData->boolValue);

    return STS_OK;
}

Status JsonConverter::decodeObjectLinkValue(ResourceData* resourceData)
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

Status JsonConverter::decodeOpaqueValue(ResourceData* resourceData)
{
    Status status = STS_OK;

    // TODO
    LOG_WARNING("Opaque value is not implemented");

    if ((status = skipTillEndValue()) != STS_OK) {
        return status;
    }

    return STS_ERR;
}

Status JsonConverter::decodeStringValue(ResourceData* resourceData)
{
    Status status = STS_OK;

    if ((status = getString(reinterpret_cast<char*>(mDecodingBuffer), sBufferSize)) != STS_OK) {
        return status;
    }

    resourceData->strValue = reinterpret_cast<char*>(mDecodingBuffer);
    resourceData->dataType = DATA_TYPE_STRING;

    LOG_DEBUG("String value: %s", reinterpret_cast<char*>(mDecodingBuffer));

    return STS_OK;
}

Status JsonConverter::checkFirstItem()
{
    if (*(mEncodingPos - 1) != JSON_TOKEN_BEGIN_ITEM) {
        if (mEncodingEndPos - mEncodingPos <= 0) {
            return STS_ERR_NO_MEM;
        }

        *mEncodingPos++ = JSON_TOKEN_SPLIT_ITEM;
    }

    return STS_OK;
}

Status JsonConverter::writeBool(const char* item, uint8_t value)
{
    Status status = STS_OK;

    if ((status = checkFirstItem()) != STS_OK) {
        return status;
    }

    int ret = 0;

    const char* strValue = "true";

    if (!value) {
        strValue = "false";
    }

    if ((ret = snprintf(mEncodingPos, mEncodingEndPos - mEncodingPos, "\"%s\":%s", item, strValue)) < 0) {
        return STS_ERR_NO_MEM;
    }

    mEncodingPos += ret;

    return STS_OK;
}

Status JsonConverter::writeObjlink(const char* item, Objlnk value)
{
    Status status = STS_OK;

    if ((status = checkFirstItem()) != STS_OK) {
        return status;
    }

    int ret = 0;

    if ((ret = snprintf(mEncodingPos, mEncodingEndPos - mEncodingPos, "\"%s\":\"%d:%d\"", item, value.objectId,
                        value.objectInstanceId)) < 0) {
        return STS_ERR_NO_MEM;
    }

    mEncodingPos += ret;

    return STS_OK;
}

Status JsonConverter::writeFloat(const char* item, double value)
{
    Status status = STS_OK;

    if ((status = checkFirstItem()) != STS_OK) {
        return status;
    }

    int ret = 0;

    if ((ret = snprintf(mEncodingPos, mEncodingEndPos - mEncodingPos, "\"%s\":%.16g", item, value)) < 0) {
        return STS_ERR_NO_MEM;
    }

    mEncodingPos += ret;

    return STS_OK;
}

Status JsonConverter::writeString(const char* item, char* value)
{
    Status status = STS_OK;

    if ((status = checkFirstItem()) != STS_OK) {
        return status;
    }

    int ret = 0;

    if ((ret = snprintf(mEncodingPos, mEncodingEndPos - mEncodingPos, "\"%s\":\"%s\"", item, value)) < 0) {
        return STS_ERR_NO_MEM;
    }

    mEncodingPos += ret;

    return STS_OK;
}

Status JsonConverter::encodeValue(ResourceData* resourceData)
{
    if (resourceData->dataType == DATA_TYPE_NONE) {
        return STS_OK;
    }

    switch (resourceData->dataType) {
        case DATA_TYPE_INT:
            return writeFloat(JSON_ITEM_FLOAT_VALUE, resourceData->intValue);

        case DATA_TYPE_UINT:
            return writeFloat(JSON_ITEM_FLOAT_VALUE, resourceData->uintValue);

        case DATA_TYPE_FLOAT:
            return writeFloat(JSON_ITEM_FLOAT_VALUE, resourceData->floatValue);

        case DATA_TYPE_STRING:
            return writeString(JSON_ITEM_STRING_VALUE, resourceData->strValue);

        case DATA_TYPE_BOOL:
            return writeBool(JSON_ITEM_BOOLEAN_VALUE, resourceData->boolValue);

        case DATA_TYPE_OBJLINK:
            return writeObjlink(JSON_ITEM_OBJECT_LINK_VALUE, resourceData->objlnkValue);

        default:
            return STS_ERR_INVALID_VALUE;
    }
}

Status JsonConverter::encodeItem(char* baseName, char* name, ResourceData* resourceData)
{
    Status status = STS_OK;

    if (*(mEncodingPos - 1) != JSON_TOKEN_BEGIN_ARRAY) {
        if (mEncodingEndPos - mEncodingPos <= 0) {
            return STS_ERR_NO_MEM;
        }

        *mEncodingPos++ = JSON_TOKEN_SPLIT_ITEM;
    }

    if (mEncodingEndPos - mEncodingPos <= 0) {
        return STS_ERR_NO_MEM;
    }

    *mEncodingPos++ = JSON_TOKEN_BEGIN_ITEM;

    if (baseName && baseName[0] != '\0') {
        LOG_DEBUG("Base name: %s", baseName);

        if ((status = writeString(JSON_ITEM_BASE_NAME, baseName)) != STS_OK) {
            return status;
        }
    }

    if (mEncodingBaseTime == 0 && resourceData->timestamp != 0) {
        mEncodingBaseTime = resourceData->timestamp;

        LOG_DEBUG("Base time: %ld", mDecodingBaseTime);

        if ((status = writeFloat(JSON_ITEM_BASE_TIME, mEncodingBaseTime)) != STS_OK) {
            return status;
        }
    }

    if (name && name[0] != '\0') {
        LOG_DEBUG("Name: %s", name);

        if ((status = writeString(JSON_ITEM_NAME, name)) != STS_OK) {
            return status;
        }
    }

    if ((status = encodeValue(resourceData)) != STS_OK) {
        return status;
    }

    if (resourceData->timestamp != 0) {
        LOG_DEBUG("Time: %ld", resourceData->timestamp - mEncodingBaseTime);

        if ((status = writeFloat(JSON_ITEM_TIME, resourceData->timestamp - mEncodingBaseTime)) != STS_OK) {
            return status;
        }
    }

    if (mEncodingEndPos - mEncodingPos <= 0) {
        return STS_ERR_NO_MEM;
    }

    *mEncodingPos++ = JSON_TOKEN_END_ITEM;

    return STS_OK;
}

}  // namespace openlwm2m
