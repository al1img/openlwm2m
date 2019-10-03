/** \file textconverter.hpp
 * LWM2M text data converter.
 */

#ifndef OPENLWM2M_TEXTCONVERTER_HPP_
#define OPENLWM2M_TEXTCONVERTER_HPP_

#include "dataformat.hpp"

namespace openlwm2m {

class TextConverter : public DataConverter {
public:
    TextConverter();
    ~TextConverter();

    Status startDecoding(void* data, size_t size);
    Status nextDecoding(ResourceData* resourceData);

    Status startEncoding(void* data, size_t size);
    Status nextEncoding(ResourceData* resourceData);
    Status finishEncoding(size_t* size);

private:
    static const int sBufferSize = CONFIG_DEFAULT_STRING_LEN;

    uint8_t mBuffer[sBufferSize + 1];

    bool mFinish;
    void* mData;
    size_t mSize;
    uint16_t mObjectId;
    uint16_t mObjectInstanceId;
    uint16_t mResourceId;
    uint16_t mResourceInstanceId;
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_TEXTCONVERTER_HPP_ */
