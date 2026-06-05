/** @file       point_cloud.hpp
 *  @ingroup    group_Common
 *  @brief      Contains definitions for the point cloud class
 *  @copyright  Multi-view_IpS
 */

#ifndef DLP_SDK_POINTCLOUD_HPP
#define DLP_SDK_POINTCLOUD_HPP

#if defined(_MSC_VER)
#pragma warning(push, 0)  // ignore all warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"  // ignore all warnings
#endif

#include <string>
#include <vector>
#include <atomic>

#if defined(_MSC_VER)
#pragma warning(pop)  // restore warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop  // restore warnings
#endif

#include <reconstruction3d_global.h>

#include <common/debug.hpp>
#include <common/other.hpp>
#include <common/returncode.hpp>

#define POINT_CLOUD_EMPTY                       "POINT_CLOUD_EMPTY"
#define POINT_CLOUD_INDEX_OUT_OF_RANGE          "POINT_CLOUD_INDEX_OUT_OF_RANGE"
#define POINT_CLOUD_FILE_SAVE_FAILED            "POINT_CLOUD_FILE_SAVE_FAILED"
#define POINT_CLOUD_NULL_POINTER_ARGUMENT       "POINT_CLOUD_NULL_POINTER_ARGUMENT"
#define POINT_CLOUD_FILENAME_EMPTY              "POINT_CLOUD_FILENAME_EMPTY"
#define POINT_CLOUD_GLFW_INIT_FAILED            "POINT_CLOUD_GLFW_INIT_FAILED"
#define POINT_CLOUD_GLFW_WINDOW_FAILED          "POINT_CLOUD_GLFW_FAILED"
#define POINT_CLOUD_FILE_DOES_NOT_EXIST         "POINT_CLOUD_FILE_DOES_NOT_EXIST"
#define POINT_CLOUD_FILE_OPEN_FAILED            "POINT_CLOUD_FILE_OPEN_FAILED"
#define POINT_CLOUD_FILE_MISSING_DIMENSION      "POINT_CLOUD_FILE_MISSING_DIMENSION"
#define POINT_CLOUD_FILE_MISSING_WIDTH_HEIGHT   "POINT_CLOUD_FILE_MISSING_WIDTH_HEIGHT"

/** @brief  Contains all reconstruction 3D library classes, functions, etc. */
namespace dlp{

/** @struct Point
 *  @brief  Stores a point in real space in x, y, z format
 */
struct RECONSTRUCTION3D_EXPORT Point{

    union {// This adds the members x,y,z which can also be accessed using the point (which is float[4])
        float data[4];
        struct {
            float x;
            float y;
            float z;
            float distance;
        };
    };
    union {
        float dataColor[4];
        struct {
            union {
                struct {
                    uint8_t b;
                    uint8_t g;
                    uint8_t r;
                    uint8_t a;
                };
                float rgb;
            };
            float padding_1;
            float padding_2;
            float padding_3;
        };
    };

    /** @brief  Constructs point at 0,0,0 */
    inline Point() {
        x = 0.0;
        y = 0.0;
        z = 0.0;
        distance = 0.0;

        r=255;
        g=255;
        b=255;
        a=255;
    }
    /** @brief  Constructs point at supplied x, y, z location */
    inline Point(const float &x_in, const float &y_in, const float &z_in) {
        x = x_in;
        y = y_in;
        z = z_in;
        distance = 0.0;

        r=255;
        g=255;
        b=255;
        a=255;
    }
    inline Point(const float &x_in, const float &y_in, const float &z_in, const float &distance_in) {
        x = x_in;
        y = y_in;
        z = z_in;
        distance = distance_in;

        r=255;
        g=255;
        b=255;
        a=255;
    }

    inline void SetGrayValue(unsigned char &value){
        r=value;
        g=value;
        b=value;
    }
    inline void SetRGBValue(unsigned char red, unsigned char green, unsigned char blue) {
        r=red;
        g=green;
        b=blue;
    }
};

/** @class  Cloud
 *  @brief  Stores a collection of points and methods to clear the cloud and
 *          add individual points.
 */
class Cloud{
public:
    /** @brief Data type identifiers */
    enum DataType {
        XYZ = 0,        /**< XYZ coordinates only */
        XYZ_GRAY = 1,   /**< XYZ with grayscale */
        XYZRGB = 2      /**< XYZ with RGB color */
    };

    RECONSTRUCTION3D_EXPORT Cloud();
    RECONSTRUCTION3D_EXPORT Cloud(unsigned int size);
    RECONSTRUCTION3D_EXPORT ~Cloud();

    RECONSTRUCTION3D_EXPORT void Clear();
    RECONSTRUCTION3D_EXPORT void Resize(unsigned int number);
    RECONSTRUCTION3D_EXPORT void Add(Point new_point);
    RECONSTRUCTION3D_EXPORT void Add(Cloud new_point_cloud);

    RECONSTRUCTION3D_EXPORT void modify(Point &new_point, unsigned int index);

    RECONSTRUCTION3D_EXPORT unsigned long long GetCount() const;

    RECONSTRUCTION3D_EXPORT ReturnCode Get(unsigned long long index, Point *ret_point)const;
    RECONSTRUCTION3D_EXPORT ReturnCode Remove(unsigned long long index);

    RECONSTRUCTION3D_EXPORT ReturnCode SaveXYZ(const std::string &filename, const unsigned char &delimiter = ' ')const;
    RECONSTRUCTION3D_EXPORT ReturnCode LoadXYZ(const std::string &filename, const unsigned char &delimiter = ' ');
    RECONSTRUCTION3D_EXPORT ReturnCode SaveXYZRGB(const std::string &filename, const unsigned char &delimiter = ' ')const;
    RECONSTRUCTION3D_EXPORT ReturnCode LoadXYZRGB(const std::string &filename, const unsigned char &delimiter = ' ');

    RECONSTRUCTION3D_EXPORT std::vector<dlp::Point>* GetPointsVec();

    DataType dataType; //0--xyz; 1--xyz_gray; 2-xyzrgb
    uint32_t  width;
    uint32_t  height;

private:
    std::vector<dlp::Point> points_;
};


}


#endif // DLP_SDK_POINTCLOUD_HPP