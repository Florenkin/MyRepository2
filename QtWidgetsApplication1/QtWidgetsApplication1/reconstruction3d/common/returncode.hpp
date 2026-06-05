/*! 
 * @file       returncode.hpp
 * @ingroup    group_Common
 * @brief      Defines ReturnCode class for all SDK modules for errors and warning messages
 * @copyright  Multi-view_IpS
 */

#ifndef DLP_SDK_RETURNCODE_HPP
#define DLP_SDK_RETURNCODE_HPP

// Compiler-specific warning suppression for external includes
#if defined(_MSC_VER)
#pragma warning(push, 0)  // ignore all warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"  // ignore all warnings
#endif

#include <string>
#include <vector>

// Restore compiler warnings after external includes
#if defined(_MSC_VER)
#pragma warning(pop)  // restore warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop  // restore warnings
#endif

#include <reconstruction3d_global.h>
#include <common/other.hpp>

/** @brief  Contains all reconstruction 3D library classes, functions, etc. */
namespace dlp{

/**
 * @class      ReturnCode
 * @ingroup    group_Common
 * @brief      Comprehensive return type for SDK methods with error and warning tracking
 *
 * The ReturnCode class provides a unified mechanism for method return values that
 * can carry multiple errors and warnings. It supports:
 * - Accumulation of multiple error and warning messages
 * - Easy status checking via boolean conversion
 * - Message containment checking
 * - Human-readable string output
 *
 * @warning This class is NOT functional with switch() statements due to its
 *          boolean conversion operator design.
 *
 * @example
 * @code
 * dlp::ReturnCode result = SomeFunction();
 * if (!result) {
 *     std::cerr << "Operation failed: " << result.ToString() << std::endl;
 * }
 * @endcode
 */
class ReturnCode{
public:
    /**
     * @brief Clears all errors and warnings from the object
     */
    RECONSTRUCTION3D_EXPORT void Clear();

    /**
     * @brief      Adds an error message to the object
     * @param[in]  msg  Error message to add
     * @return     Reference to this object for method chaining
     *
     * @note Empty messages are ignored
     */
    RECONSTRUCTION3D_EXPORT ReturnCode& AddError(const std::string &msg);

    /**
     * @brief      Adds a warning message to the object
     * @param[in]  msg  Warning message to add
     * @return     Reference to this object for method chaining
     *
     * @note Empty messages are ignored
     */
    RECONSTRUCTION3D_EXPORT ReturnCode& AddWarning(const std::string &msg);

    /**
     * @brief      Merges errors and warnings from another ReturnCode
     * @param[in]  source  Source ReturnCode to copy from
     * @return     Reference to this object for method chaining
     */
    RECONSTRUCTION3D_EXPORT ReturnCode& Add(const ReturnCode &source);

    /**
     * @brief Checks if the object contains any errors
     * @return true if errors are present, false otherwise
     */
    RECONSTRUCTION3D_EXPORT bool hasErrors() const;

    /**
     * @brief Checks if the object contains any warnings
     * @return true if warnings are present, false otherwise
     */
    RECONSTRUCTION3D_EXPORT bool hasWarnings() const;

    /**
     * @brief      Checks if a specific error message exists
     * @param[in]  msg  Error message to search for (exact match)
     * @return     true if the exact error message is found, false otherwise
     */
    RECONSTRUCTION3D_EXPORT bool ContainsError(const std::string &msg) const;

    /**
     * @brief      Checks if a specific warning message exists
     * @param[in]  msg  Warning message to search for (exact match)
     * @return     true if the exact warning message is found, false otherwise
     */
    RECONSTRUCTION3D_EXPORT bool ContainsWarning(const std::string &msg) const;

    /**
     * @brief Gets all error messages
     * @return Vector containing all error messages
     */
    RECONSTRUCTION3D_EXPORT std::vector<std::string> GetErrors() const;

    /**
     * @brief Gets all warning messages
     * @return Vector containing all warning messages
     */
    RECONSTRUCTION3D_EXPORT std::vector<std::string> GetWarnings() const;

    /**
     * @brief Gets the number of error messages
     * @return Count of error messages
     */
    RECONSTRUCTION3D_EXPORT unsigned int GetErrorCount() const;

    /**
     * @brief Gets the number of warning messages
     * @return Count of warning messages
     */
    RECONSTRUCTION3D_EXPORT unsigned int GetWarningCount() const;

    /**
     * @brief Converts the object to a human-readable string
     * @return Formatted string with error and warning summary
     *
     * The output format includes:
     * - First line: Error and warning counts
     * - Subsequent lines: Individual error and warning messages
     */
    RECONSTRUCTION3D_EXPORT std::string ToString() const;

    /**
     * @brief Boolean conversion operator for easy error checking
     * @return true if no errors are present, false otherwise
     *
     * This operator allows intuitive usage in conditional statements.
     * Note that warnings do not affect the boolean value.
     *
     * @example
     * @code
     * dlp::ReturnCode retval;
     *
     * if (retval) {
     *     // Operation successful (no errors)
     *     // May still have warnings - check with hasWarnings()
     * } else {
     *     // Operation failed (has errors)
     *     // May also have warnings
     * }
     * @endcode
     */
    RECONSTRUCTION3D_EXPORT operator bool() const {
        return this->errors_.empty();
    }

private:
    std::vector<std::string> errors_;   ///< Collection of error messages
    std::vector<std::string> warnings_; ///< Collection of warning messages
};

} // namespace dlp

#endif // DLP_SDK_RETURNCODE_HPP