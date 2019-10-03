/** \file lwm2m.hpp
 * lwm2m structures.
 */

#ifndef OPENLWM2M_LWM2M_HPP_
#define OPENLWM2M_LWM2M_HPP_

namespace openlwm2m {

#define INVALID_ID 0xFFFF

/**
 * Status.
 */
enum Status {
    STS_OK,                 ///< OK.
    STS_ERR,                ///< Generic error.
    STS_ERR_NO_MEM,         ///< Not enough memory.
    STS_ERR_NO_ACCESS,      ///< Access error.
    STS_ERR_NOT_FOUND,      ///< Item doesn't exist or unavailable.
    STS_ERR_EXIST,          ///< Item already exists.
    STS_ERR_NOT_ALLOWED,    ///< Not allowed.
    STS_ERR_INVALID_VALUE,  ///< Invalid value.
    STS_ERR_TIMEOUT,        ///< Timeout occurs.
    STS_ERR_FORMAT          ///< Unsupported content format.
};

/**
 * Define LWM2M Interfaces.
 *
 * See: 6. Interfaces.
 */
enum Interface {
    ITF_CLIENT = 0x00,     ///< Client interface: this interface access all objects.
    ITF_BOOTSTRAP = 0x01,  ///< Bootstrap interface.
    ITF_REGISTER = 0x02,   ///< Client registration interface.
    ITF_DEVICE = 0x04,     ///< Device Management and Service Enablement Interface.
    ITF_REPORTTING = 0x08  ///< Information Reporting Interface.
};

/**
 * Defines all interfaces.
 */
#define ITF_ALL (ITF_BOOTSTRAP | ITF_REGISTER | ITF_DEVICE | ITF_REPORTTING)

enum Operation { OP_NONE = 0x00, OP_READ = 0x01, OP_WRITE = 0x02, OP_READWRITE = 0x03, OP_EXECUTE = 0x04 };

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

}  // namespace openlwm2m

#endif /* OPENLWM2M_LWM2M_HPP_ */
