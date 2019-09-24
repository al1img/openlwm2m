/** \file dataformat.hpp
 * LWM2M data format.
 */

#ifndef OPENLWM2M_DATAFORMAT_HPP_
#define OPENLWM2M_DATAFORMAT_HPP_

#include <cstddef>

#include "itembase.hpp"
#include "lwm2m.hpp"
#include "resource.hpp"
#include "storage.hpp"

namespace openlwm2m {

enum DataFormat {
    DATA_FMT_TEXT = 0,
    DATA_FMT_CORE = 40,
    DATA_FMT_OPAQUE = 42,
    DATA_FMT_TLV = 11542,
    DATA_FMT_JSON = 11543,
    DATA_FMT_SENML_JSON = 110,
    DATA_FMT_SENML_CBOR = 112,
    DATA_FMT_ANY = 0xFFFF
};

struct Objlnk {
    uint16_t objectId;
    uint16_t objectInstanceId;
};

struct Opaque {
    size_t size;
    uint8_t* data;
};

class DataConverter : public ItemBase {
public:
    typedef Lwm2mStaticStorage<DataConverter> Storage;

    struct ResourceData {
        uint16_t objectId;
        uint16_t objectInstanceId;
        uint16_t resourceId;
        uint16_t resourceInstanceId;
        DataType dataType;
        int64_t timestamp;
        union {
            uint64_t uintValue;
            int64_t intValue;
            double floatValue;
            uint8_t boolValue;
            char* strValue;
            Objlnk objlnkValue;
            Opaque opaqueValue;
        };
    };

    DataConverter(DataFormat format) : ItemBase(NULL) { setId(format); }
    virtual ~DataConverter() {}

    virtual Status startDecoding(const char* path, void* data, size_t size) = 0;
    virtual Status nextDecoding(ResourceData* resourceData) = 0;

    virtual Status startEncoding(void* data, size_t size) = 0;
    virtual Status nextEncoding(ResourceData* resourceData) = 0;
    virtual Status finishEncoding(size_t* size) = 0;

    void init() {}
    void release() {}
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_DATAFORMAT_HPP_ */
