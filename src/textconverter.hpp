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

    Status startDecoding(const char* path, void* data, size_t size);
    Status nextDecoding(ResourceData* resourceData);

    Status startEncoding(void* data, size_t size);
    Status nextEncoding(ResourceData* resourceData);
    Status finishEncoding(size_t* size);

private:
    static const int sStringSize = 64;

    char mPath[sStringSize];
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_TEXTCONVERTER_HPP_ */
