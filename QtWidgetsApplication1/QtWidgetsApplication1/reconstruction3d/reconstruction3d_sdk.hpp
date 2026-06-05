/** @file       reconstruction3d_sdk.hpp
 *  @brief      Header file that includes all reconstruction 3D SDK header files
 *  @copyright  Multi-view_IpS
 */

#ifndef RECONSTRUCTION3D_SDK_HPP
#define RECONSTRUCTION3D_SDK_HPP

// Include SDK headers
#include <common/returncode.hpp>
#include <common/debug.hpp>
#include <common/other.hpp>
#include <common/parameters.hpp>
#include <common/point_cloud/point_cloud.hpp>
#include <common/module.hpp>

#include <calibration/calibration.hpp>
#include <geometry/geometry.hpp>

/** @defgroup   group_reconstruction3d_library reconstruction3d_library
 *  @brief      Contains All things of reconstruction 3D library.
 *
 * The reconstruction 3D library is a algorithm tool containing calibration and reconstruction.
 *
 * \note        Contains All things of reconstruction 3D library
 */

/** @defgroup   group_Common Common
 *  @ingroup    group_reconstruction3d_library
 *  @brief      Contains common functions relating to strings, numbers, and time
 */

/** @defgroup   group_Calibration Calibration
 *  @ingroup    group_reconstruction3d_library
 *  @brief      Contains calibration functions.
 */

/** @defgroup   group_Geometry Geometry
 *  @ingroup    group_reconstruction3d_library
 *  @brief      Contains geometry functions relating to get point cloud.
 */


#endif  //#ifndef RECONSTRUCTION3D_SDK_HPP


