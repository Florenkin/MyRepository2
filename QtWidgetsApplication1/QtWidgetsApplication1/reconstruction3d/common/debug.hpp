/** @file       debug.hpp
 *  @ingroup    group_Common
 *  @brief      Defines the Debug class for terminal output messages
 *  @copyright  Multi-view_IpS
 */

#ifndef DLP_SDK_DEBUG_HPP
#define DLP_SDK_DEBUG_HPP

// Compiler-specific warning suppression for external includes
#if defined(_MSC_VER)
#pragma warning(push, 0)  // ignore all warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"  // ignore all warnings
#endif

#include <iostream>
#include <fstream>
#include <string>

// Restore compiler warnings after external includes
#if defined(_MSC_VER)
#pragma warning(pop)  // restore warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop  // restore warnings
#endif

#include <reconstruction3d_global.h>

/** @brief  Contains all reconstruction 3D library classes, functions, etc. */
namespace dlp{

/**
 * @class      Debug
 * @ingroup    group_Common
 * @brief      Runtime debug message interface with enable, level, and output controls.
 *
 * The Debug class provides a flexible and configurable logging system that allows
 * for runtime control of debug message output. Key features include:
 * - Enable/disable debug output at runtime
 * - Message level filtering (e.g., error, warning, info, debug)
 * - Customizable output streams
 * - Instance naming for message identification
 *
 * Debug messages are printed in the following format:
 * @code
 * {Debug instance name} + {message} + std::endl
 * @endcode
 *
 * @warning The Debug class does NOT open, close, or control its output stream. If
 *          the stream closes, the messages will automatically fall back to std::cerr.
 *
 * @example
 * @code
 * dlp::Debug debug(true, "MyModule", 3);  // Enabled, name "MyModule", level 3
 * debug.Msg("This is a debug message");    // Output: "MyModule This is a debug message"
 * debug.Msg(2, "Level 2 message");         // Only shows if current level >= 2
 * @endcode
 */
class Debug{
public:
    /**
     * @brief      Default constructor
     * 
     * Creates a Debug instance with default settings:
     * - Enabled: false
     * - Name: empty string
     * - Level: 0
     * - Output: std::cerr
     */
    RECONSTRUCTION3D_EXPORT Debug();
    
    /**
     * @brief      Parameterized constructor
     * @param[in]  enable  Initial enable state
     * @param[in]  name    Name identifier for this debug instance
     * @param[in]  level   Maximum debug level for message filtering
     */
    RECONSTRUCTION3D_EXPORT Debug(
        const bool &enable, 
        const std::string &name,
        const unsigned int &level);

    // Configuration Methods

    /**
     * @brief      Enables or disables the debug message output
     * @param[in]  enable  true to enable debug output, false to disable
     */
    RECONSTRUCTION3D_EXPORT void SetEnable(const bool &enable);
    
    /**
     * @brief      Sets the maximum level of debug messages to output
     * @param[in]  level   Maximum level of debug messages to output
     *
     * Messages with a level less than or equal to this value will be
     * sent to the output stream. Messages with higher levels will be
     * filtered out.
     *
     * Typical level usage:
     * - 1: Critical errors
     * - 2: Warnings
     * - 3: Informational messages
     * - 4+: Verbose debugging
     */
    RECONSTRUCTION3D_EXPORT void SetLevel(const unsigned int &level);
    
    /**
     * @brief      Sets the name identifier for this debug instance
     * @param[in]  name    Name to identify this debug instance in output
     *
     * The name is prepended to all debug messages to help identify
     * the source of the message in the output.
     */
    RECONSTRUCTION3D_EXPORT void SetName(const std::string &name);
    
    /**
     * @brief      Sets the output stream for debug messages
     * @param[in]  output  Pointer to the output stream
     *
     * @note The Debug class does NOT open, close, or control its output stream.
     *       If the provided stream becomes invalid or closes, messages will
     *       automatically fall back to std::cerr.
     *
     * @warning The caller is responsible for managing the lifetime of the
     *          output stream. Do not pass a pointer to a temporary or
     *          locally scoped stream object.
     */
    RECONSTRUCTION3D_EXPORT void SetOutput(std::ostream *output);

    // Accessor Methods

    /**
     * @brief      Gets the current enable state
     * @return     true if debug output is enabled, false otherwise
     */
    RECONSTRUCTION3D_EXPORT bool          GetEnable() const;
    
    /**
     * @brief      Gets the current maximum message level
     * @return     Current maximum level for message filtering
     */
    RECONSTRUCTION3D_EXPORT unsigned int  GetLevel()  const;
    
    /**
     * @brief      Gets the current instance name
     * @return     Name identifier for this debug instance
     */
    RECONSTRUCTION3D_EXPORT std::string   GetName()   const;
    
    /**
     * @brief      Gets the current output stream
     * @return     Pointer to the current output stream
     *
     * @note Returns nullptr if no custom output stream is set and
     *       the default std::cerr is being used.
     */
    RECONSTRUCTION3D_EXPORT std::ostream* GetOutput() const;

    // Message Output Methods

    /**
     * @brief      Outputs a debug message at default level (level 1)
     * @param[in]  msg  Message string to output
     *
     * @note If the configured output stream is unavailable, the message
     *       will be sent to std::cerr as a fallback.
     *
     * Output format: `{Debug instance name} {message}`
     */
    RECONSTRUCTION3D_EXPORT void Msg(const std::string       &msg) const;
    
    /**
     * @brief      Outputs a debug message at default level (level 1)
     * @param[in]  msg  Message stringstream to output
     *
     * @note If the configured output stream is unavailable, the message
     *       will be sent to std::cerr as a fallback.
     *
     * Output format: `{Debug instance name} {message}`
     */
    RECONSTRUCTION3D_EXPORT void Msg(const std::stringstream &msg) const;
    
    /**
     * @brief      Outputs a debug message if the specified level is enabled
     * @param[in]  level Minimum level required for this message to be output
     * @param[in]  msg   Message string to output
     *
     * The message will only be output if:
     * - Debug instance is enabled
     * - Specified level <= current maximum level
     *
     * @note If the configured output stream is unavailable, the message
     *       will be sent to std::cerr as a fallback.
     *
     * Output format: `{Debug instance name} {message}`
     */
    RECONSTRUCTION3D_EXPORT void Msg(const unsigned int &level, const std::string       &msg) const;
    
    /**
     * @brief      Outputs a debug message if the specified level is enabled
     * @param[in]  level Minimum level required for this message to be output
     * @param[in]  msg   Message stringstream to output
     *
     * The message will only be output if:
     * - Debug instance is enabled
     * - Specified level <= current maximum level
     *
     * @note If the configured output stream is unavailable, the message
     *       will be sent to std::cerr as a fallback.
     *
     * Output format: `{Debug instance name} {message}`
     */
    RECONSTRUCTION3D_EXPORT void Msg(const unsigned int &level, const std::stringstream &msg) const;

private:
    bool          enable_;  ///< Enable flag for debug output (true = enabled, false = disabled)
    unsigned int  level_;   ///< Maximum debug level for message filtering (higher = more verbose)
    std::string   name_;    ///< Name identifier for this debug instance
    std::ostream  *output_; ///< Pointer to output stream (nullptr = use std::cerr)
};

} // namespace dlp

#endif // DLP_SDK_DEBUG_HPP