/** \file dataformat.hpp
 * LWM2M data format.
 */

#ifndef OPENLWM2M_DATAFORMAT_HPP_
#define OPENLWM2M_DATAFORMAT_HPP_

#include <cstddef>

#include "itembase.hpp"
#include "status.hpp"
#include "storage.hpp"

namespace openlwm2m {

enum DataFormat {
    DATA_FMT_TEXT = 0,
    DATA_FMT_CORE = 40,
    DATA_FMT_OPAQUE = 42,
    DATA_FMT_TLV = 11542,
    DATA_FMT_JSON = 11543,
    DATA_FMT_SENML_JSON = 110,
    DATA_FMT_SENML_CBOR = 112
};

enum DataType {
    DATA_TYPE_NONE,
    DATA_TYPE_STRING,
    DATA_TYPE_INT,
    DATA_TYPE_UINT,
    DATA_TYPE_FLOAT,
    DATA_TYPE_BOOL,
    DATA_TYPE_OPAQUE,
    DATA_TYPE_TIME,
    DATA_TYPE_OBJLINK,
    DATA_TYPE_CORELINK
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
    typedef Lwm2mStorage<DataConverter, void*> Storage;

    struct ResourceData {
        uint16_t objectId;
        uint16_t objectInstanceId;
        uint16_t resourceId;
        uint16_t resourceInstanceId;
        DataType dataType;
        uint64_t timestamp;
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
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_DATAFORMAT_HPP_ */
