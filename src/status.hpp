/** \file status.hpp
 * lwm2m statuses.
 */

#ifndef OPENLWM2M_STATUS_HPP_
#define OPENLWM2M_STATUS_HPP_

namespace openlwm2m {

/**
 * Status.
 */
enum Status {
    STS_OK,                 ///< OK.
    STS_ERR,                ///< Generic error.
    STS_ERR_NO_MEM,         ///< Not enough memory.
    STS_ERR_NO_ACCESS,      ///< Access error.
    STS_ERR_NOT_EXIST,      ///< Item doesn't exist or unavailable.
    STS_ERR_EXIST,          ///< Item already exists.
    STS_ERR_INVALID_STATE,  ///< Invalid state.
    STS_ERR_INVALID_VALUE   ///< Invalid value.
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_STATUS_HPP_ */