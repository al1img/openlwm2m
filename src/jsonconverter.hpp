/** \file jsonconverter.hpp
 * LWM2M JSON data converter.
 */

#ifndef OPENLWM2M_JSONCONVERTER_HPP_
#define OPENLWM2M_JSONCONVERTER_HPP_

#include <cctype>

#include "dataformat.hpp"

#define JSON_TOKEN_BEGIN_ARRAY '['
#define JSON_TOKEN_END_ARRAY ']'
#define JSON_TOKEN_BEGIN_ITEM '{'
#define JSON_TOKEN_END_ITEM '}'
#define JSON_TOKEN_SPLIT_ITEM ','
#define JSON_TOKEN_SPLIT_VALUE ':'
#define JSON_TOKEN_STRING '"'

#define TOKEN_SET                                                                                                    \
    JSON_TOKEN_BEGIN_ARRAY, JSON_TOKEN_END_ARRAY, JSON_TOKEN_BEGIN_ITEM, JSON_TOKEN_END_ITEM, JSON_TOKEN_SPLIT_ITEM, \
        JSON_TOKEN_SPLIT_VALUE, JSON_TOKEN_STRING, '\0'

#define JSON_ITEM_BASE_NAME "bn"
#define JSON_ITEM_BASE_TIME "bt"
#define JSON_ITEM_NAME "n"
#define JSON_ITEM_TIME "t"
#define JSON_ITEM_FLOAT_VALUE "v"
#define JSON_ITEM_BOOLEAN_VALUE "vb"
#define JSON_ITEM_OBJECT_LINK_VALUE "vlo"
#define JSON_ITEM_OPAQUE_VALUE "vd"
#define JSON_ITEM_STRING_VALUE "vs"

namespace openlwm2m {

class JsonConverter : public DataConverter {
public:
    JsonConverter();
    ~JsonConverter();

    Status startDecoding(void* data, size_t size);
    Status nextDecoding(ResourceData* resourceData);

    Status startEncoding(void* data, size_t size);
    Status nextEncoding(ResourceData* resourceData);
    Status finishEncoding(size_t* size);

private:
    static const int sStringSize = 64;
    static const int sBufferSize = CONFIG_DEFAULT_STRING_LEN;

    char* mDecodingPos;
    char* mDecodingEndPos;
    char mDecodingBaseName[sStringSize + 1];
    int64_t mDecodingBaseTime;
    uint8_t mDecodingBuffer[sBufferSize + 1];
    bool mDecodingNameFound;

    char* mEncodingBeginPos;
    char* mEncodingPos;
    char* mEncodingEndPos;

    char mEncodingBaseName[sStringSize + 1];
    int64_t mEncodingBaseTime;
    ResourceData mPrevResourceData;
    bool mHasPrevData;
    bool mHasItems;

    Status skipWhiteSpaces();
    Status skipSymbolsTill(char expectedChar);
    bool isSymbolInStr(char c, const char* str);
    Status skipTillEndValue();
    Status isLastElement(char endChar, bool* lastElement);

    Status decodeItem(ResourceData* resourceData, char* field, size_t size);
    Status getString(char* src, size_t size);
    Status getFloat(double* value);

    Status decodeBaseName(ResourceData* resourceData);
    Status decodeBaseTime(ResourceData* resourceData);
    Status decodeName(ResourceData* resourceData);
    Status decodeTime(ResourceData* resourceData);
    Status decodeFloatValue(ResourceData* resourceData);
    Status decodeBooleanValue(ResourceData* resourceData);
    Status decodeObjectLinkValue(ResourceData* resourceData);
    Status decodeOpaqueValue(ResourceData* resourceData);
    Status decodeStringValue(ResourceData* resourceData);

    Status checkFirstItem();
    Status writeBool(const char* item, uint8_t value);
    Status writeObjlink(const char* item, Objlnk value);
    Status writeFloat(const char* item, double value);
    Status writeString(const char* item, char* value);
    Status encodeValue(ResourceData* resourceData);
    Status encodeItem(char* baseName, char* name, ResourceData* resourceData);
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_JSONCONVERTER_HPP_ */
