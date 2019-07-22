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
        JSON_TOKEN_SPLIT_VALUE, JSON_TOKEN_STRING

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

    Status startDecoding(const char* path, void* data, size_t size);
    Status nextDecoding(ResourceData* resourceData);

private:
    static const int sStringSize = 64;
    static const int sBufferSize = CONFIG_DEFAULT_STRING_LEN;

    char* mCurPos;
    char* mEndPos;
    char mBaseName[sStringSize];
    uint64_t mBaseTime;
    uint8_t mBuffer[sBufferSize];
    bool mNameFound;

    Status skipWhiteSpaces();
    Status skipSymbolsTill(char expectedChar);
    bool isSymbolInStr(char c, const char* str);
    Status skipTillEndValue();
    Status isLastElement(char endChar, bool* lastElement);

    Status processItem(ResourceData* resourceData, char* field, size_t size);
    Status getString(char* src, size_t size);
    Status getFloat(double* value);

    Status convertName(char* name, ResourceData* resourceData);

    Status processBaseName(ResourceData* resourceData);
    Status processBaseTime(ResourceData* resourceData);
    Status processName(ResourceData* resourceData);
    Status processTime(ResourceData* resourceData);
    Status processFloatValue(ResourceData* resourceData);
    Status processBooleanValue(ResourceData* resourceData);
    Status processObjectLinkValue(ResourceData* resourceData);
    Status processOpaqueValue(ResourceData* resourceData);
    Status processStringValue(ResourceData* resourceData);
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_JSONCONVERTER_HPP_ */
