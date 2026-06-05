/** @file       module.hpp
 *  @ingroup    group_Common
 *  @brief      Defines dlp::Module base class for minimum reconstruction 3D library module requirements
 *  @copyright  Multi-view_IpS
 */

#ifndef DLP_SDK_MODULE_HPP
#define DLP_SDK_MODULE_HPP

#include <common/returncode.hpp>
#include <common/parameters.hpp>

namespace dlp{

/**
 * @class  Module
 * @brief  Base class for reconstruction 3D library modules providing common functionality
 * 
 * The Module class serves as the foundation for all reconstruction 3D library modules,
 * providing essential features including:
 * - Configuration management via Parameters
 * - Debug output control
 * - Setup state tracking
 * - Static helper methods for module initialization
 * 
 * Derived classes must implement the pure virtual Setup() and GetSetup() methods
 * to handle module-specific configuration.
 * 
 * @see Parameters
 * @see Debug
 * @see ReturnCode
 */
class Module{
public:
    /**
     * @brief Virtual destructor for proper polymorphic destruction
     */
    RECONSTRUCTION3D_EXPORT virtual ~Module();
    
    /**
     * @brief      Configures the module with the specified settings
     * @param[in]  settings  Configuration parameters for the module
     * @return     ReturnCode indicating success or failure of configuration
     * 
     * Pure virtual method that must be implemented by derived classes to
     * handle module-specific configuration logic.
     */
    RECONSTRUCTION3D_EXPORT virtual ReturnCode Setup(const dlp::Parameters &settings) = 0;
    
    /**
     * @brief      Retrieves the current module configuration
     * @param[out] settings  Pointer to Parameters object to receive current settings
     * @return     ReturnCode indicating success or failure of operation
     * 
     * Pure virtual method that must be implemented by derived classes to
     * return the current configuration state of the module.
     */
    RECONSTRUCTION3D_EXPORT virtual ReturnCode GetSetup(dlp::Parameters *settings) const = 0;

    /**
     * @brief      Checks if the module has been successfully configured
     * @return     true if the module has been setup, false otherwise
     * 
     * This method indicates whether the module has undergone successful
     * configuration via the Setup() method.
     */
    RECONSTRUCTION3D_EXPORT bool isSetup() const;
    
    /**
     * @brief      Enables or disables debug output for the module
     * @param[in]  enable  true to enable debug messages, false to disable
     * 
     * Controls the debug output for this module instance. When enabled,
     * the module will output debug information through its Debug interface.
     */
    RECONSTRUCTION3D_EXPORT void SetDebugEnable(const bool &enable);
    
    /**
     * @brief      Sets the debug output level for the module
     * @param[in]  level  Maximum level for debug messages
     * 
     * Messages with levels equal to or less than this value will be output.
     * Typical level usage:
     * - 1: Critical errors only
     * - 2: Warnings and errors
     * - 3: Informational messages
     * - 4+: Verbose debugging information
     */
    RECONSTRUCTION3D_EXPORT void SetDebugLevel(const unsigned int &level);
    
    /**
     * @brief      Sets the output stream for debug messages
     * @param[out] output  Pointer to output stream for debug messages
     * 
     * @note The Module class does not manage the lifetime of the output stream.
     *       The caller is responsible for ensuring the stream remains valid.
     * 
     * @warning Passing a pointer to a temporary or locally scoped stream object
     *          will cause undefined behavior.
     */
    RECONSTRUCTION3D_EXPORT void SetDebugOutput(std::ostream* output);

    /**
     * @brief      Configures a module using parameters from a file
     * @param[in]  module           Reference to the module to configure
     * @param[in]  parameters_file  Path to the parameters configuration file
     * @param[in]  output_cmdline   If true, outputs configuration to command line
     * @return     ReturnCode indicating success or failure of configuration
     * 
     * This static helper method simplifies module configuration by loading
     * parameters from a file and applying them to the module.
     * 
     * @see Parameters::Load()
     */
    RECONSTRUCTION3D_EXPORT static ReturnCode Setup(Module &module, std::string parameters_file, bool output_cmdline = false);
    
    /**
     * @brief      Configures a module using provided parameters
     * @param[in]  module         Reference to the module to configure
     * @param[in]  settings       Configuration parameters to apply
     * @param[in]  output_cmdline If true, outputs configuration to command line
     * @return     ReturnCode indicating success or failure of configuration
     * 
     * This static helper method provides a convenient way to configure
     * modules with pre-existing parameter sets.
     */
    RECONSTRUCTION3D_EXPORT static ReturnCode Setup(Module &module, const dlp::Parameters &settings, bool output_cmdline = false);

protected:
    Debug   debug_;      ///< Debug interface for module output messages
    bool    is_setup_;   ///< Flag indicating whether module has been configured
};

} // namespace dlp

#endif // DLP_SDK_MODULE_HPP