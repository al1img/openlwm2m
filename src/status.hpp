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
    STS_OK,          ///< OK.
    STS_ERR_MEM,     ///< Not enough memory.
    STS_ERR_ACCESS,  ///< Access error.
    STS_ERR_EXIST,   ///< Item doesn't exist or unavailable.
    STS_ERR_STATE,   ///< Wrong state.
    STS_ERR_VALUE    ///< Wrong value.
};

}  // namespace openlwm2m

#endif /* OPENLWM2M_STATUS_HPP_ */