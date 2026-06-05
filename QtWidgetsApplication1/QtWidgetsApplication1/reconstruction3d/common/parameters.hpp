/** @file       parameters.hpp
 *  @ingroup    group_Common
 *  @brief      Contains \ref dlp::Parameters and \ref dlp::Parameters::Entry classes
 *  @copyright  Multi-view_IpS
 */

#ifndef DLP_SDK_PARAMETERS_HPP
#define DLP_SDK_PARAMETERS_HPP

#include <reconstruction3d_global.h>

#include <common/debug.hpp>
#include <common/returncode.hpp>
#include <common/other.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <map>

// Error code definitions for parameter operations
#define PARAMETERS_EMPTY                        "PARAMETERS_EMPTY"                       ///< Parameter list is empty
#define PARAMETERS_SOURCE_EMPTY                 "PARAMETERS_SOURCE_EMPTY"                ///< Source parameter list is empty
#define PARAMETERS_DESTINATION_MISSING_ENTRY    "PARAMETERS_DESTINATION_MISSING_ENTRY"   ///< Destination missing required entry
#define PARAMETERS_NO_NAME                      "PARAMETERS_NO_NAME"                     ///< Parameter name is empty
#define PARAMETERS_NO_NAME_SUPPLIED             "PARAMETERS_NO_NAME_SUPPLIED"            ///< No parameter name supplied
#define PARAMETERS_NOT_FOUND                    "PARAMETERS_NOT_FOUND"                   ///< Parameter not found in list
#define PARAMETERS_INDEX_OUT_OF_RANGE           "PARAMETERS_INDEX_OUT_OF_RANGE"          ///< Index out of valid range
#define PARAMETERS_ILLEGAL_CHARACTER            "PARAMETERS_ILLEGAL_CHARACTER"           ///< Illegal character in name/value
#define PARAMETERS_MISSING_VALUE                "PARAMETERS_MISSING_VALUE"               ///< Missing value in parameter entry
#define PARAMETERS_NULL_POINTER                 "PARAMETERS_NULL_POINTER"                ///< Null pointer provided
#define PARAMETERS_FILE_DOES_NOT_EXIST          "PARAMETERS_FILE_DOES_NOT_EXIST"         ///< Parameter file does not exist
#define PARAMETERS_FILE_OPEN_FAILED             "PARAMETERS_FILE_OPEN_FAILED"            ///< Failed to open parameter file
#define PARAMETERS_FILE_PROCESSING_FAILED       "PARAMETERS_FILE_PROCESSING_FAILED"      ///< Error processing parameter file

/**
 * @def         DLP_NEW_PARAMETERS_ENTRY(name, string, type, default_value)
 * @brief       Macro to create a new parameter entry class
 * @param[in]   name           Name of the new parameter class
 * @param[in]   string         String identifier for the parameter
 * @param[in]   type           Data type of the parameter
 * @param[in]   default_value  Default value for the parameter
 * 
 * This macro generates a complete parameter entry class with constructors
 * and automatic setup functionality.
 */
#define DLP_NEW_PARAMETERS_ENTRY(name,string,type,default_value) \
    class name: public dlp::Parameters::Entry<type>{  \
        public:\
        RECONSTRUCTION3D_EXPORT name(){\
            this->Setup(string,default_value);\
        }\
        RECONSTRUCTION3D_EXPORT name(type value){\
            this->Setup(string,default_value);\
            this->Set(value);\
        }\
    }

/** @brief  Contains all reconstruction 3D library classes, functions, etc. */
namespace dlp{

/**
 * @class      Parameters
 * @brief      Container for transferring module setup information
 * @ingroup    group_Common
 * 
 * This class provides a flexible container for storing and managing
 * configuration parameters. All parameters are stored as string pairs
 * (name-value) with support for type conversion and file persistence.
 * 
 * @note All names and values are stored in std::string vectors internally.
 */
class Parameters{
public:

     /**
     * @class      Entry
     * @brief      Template class for automating parameter setup, storage and retrieval
     * @ingroup    group_Common
     * @tparam     T  Type of the parameter value
     * 
     * This template class provides type-safe access to parameter values
     * while maintaining string-based storage compatibility.
     */
    template <class T>
    class Entry{
    public:
        /**
         * @brief      Sets the parameter value
         * @param[in]  value  New value to set
         */
        RECONSTRUCTION3D_EXPORT void Set(const T &value){
            this->value_ = value;
        }

        /**
         * @brief      Gets the current parameter value
         * @return     Current value of the parameter
         */
        RECONSTRUCTION3D_EXPORT T Get() const{
            return this->value_;
        }

        /**
         * @brief      Gets the default parameter value
         * @return     Default value of the parameter
         */
        RECONSTRUCTION3D_EXPORT T GetDefault() const{
            return this->default_;
        }

        /**
         * @brief      Gets the parameter name
         * @return     Name identifier of the parameter
         */
        RECONSTRUCTION3D_EXPORT std::string GetEntryName() const{
            return this->name_;
        }

        /**
         * @brief      Gets the parameter value as string
         * @return     String representation of the current value
         */
        RECONSTRUCTION3D_EXPORT std::string GetEntryValue() const{
            return dlp::Number::ToString(this->value_);
        }

        /**
         * @brief      Gets the default value as string
         * @return     String representation of the default value
         */
        RECONSTRUCTION3D_EXPORT std::string GetEntryDefault() const{
            return dlp::Number::ToString(this->default_);
        }

        /**
         * @brief      Sets the parameter value from string
         * @param[in]  value  String representation of the new value
         */
        RECONSTRUCTION3D_EXPORT void SetEntryValue(const std::string &value){
            this->value_ = dlp::String::ToNumber<T>(value);
        }

        /**
         * @brief      Conversion operator to Parameters object
         * @return     Parameters object containing this entry
         * 
         * Allows automatic conversion of Entry to Parameters for
         * easy integration with the Parameters container.
         */
        RECONSTRUCTION3D_EXPORT operator Parameters(){
            Parameters ret;
            ret.Clear();
            ret.Set(this->GetEntryName(), this->GetEntryValue());
            return ret;
        }

    protected:
        /**
         * @brief      Initializes the parameter entry
         * @param[in]  name           Parameter name
         * @param[in]  default_value  Default parameter value
         */
        RECONSTRUCTION3D_EXPORT void Setup(std::string name,T default_value){
            this->name_     = name;
            this->value_    = default_value;
            this->default_  = default_value;
        }

    private:
        std::string name_;     ///< Parameter name identifier
        T value_;              ///< Current parameter value
        T default_;            ///< Default parameter value
    };

    /**
     * @brief      Sets a parameter from an Entry object
     * @tparam     T       Type of the parameter value
     * @param[in]  option  Entry object containing parameter data
     * @return     ReturnCode indicating success or specific error
     */
    template <typename T>
    ReturnCode Set(const Entry<T> &option){
        return this->Set(option.GetEntryName(),option.GetEntryValue());
    }

    /**
     * @brief      Retrieves a parameter value into an Entry object
     * @tparam     T       Type of the parameter value
     * @param[out] option  Pointer to Entry object to receive the value
     * @return     ReturnCode indicating success or specific error
     * @retval     PARAMETERS_NULL_POINTER  Output pointer is null
     */
    template <typename T>
    ReturnCode Get(Entry<T> *option) const{
        // Initialize a ReturnCode object to track operation status
        ReturnCode ret;
        // String to store the retrieved parameter value
        std::string return_value;
        // Validate input pointer is not null
        if(!option) {
            // Early return with null pointer error if option is invalid
            return ret.AddError(PARAMETERS_NULL_POINTER);
        }            
        // Attempt to get parameter value using:
        // 1. The option's name (via GetEntryName())
        // 2. The option's default value (via GetEntryDefault())
        // 3. Store result in return_value
        ret = this->Get(option->GetEntryName(),option->GetEntryDefault(),&return_value);
        // Update the option object with retrieved value
        option->SetEntryValue(return_value);
        // Return operation status (contains success/failure information)
        return ret;
    }

    /**
     * @brief      Checks if an Entry exists in parameters
     * @tparam     T       Type of the parameter value
     * @param[in]  option  Entry object to check for
     * @return     true if parameter exists, false otherwise
     */
    template <typename T>
    bool Contains(const Entry<T> &option) const{
        return this->Contains(option.GetEntryName());
    }

    /**
     * @brief      Creates or updates a parameter entry with numerical value
     * @tparam     T       Type of the numerical value
     * @param[in]  name    Name of the parameter
     * @param[in]  value   Numerical value to store (converted to string)
     * @return     ReturnCode indicating success or specific error
     * 
     * @note The method converts the name to upper case and removes all whitespace.
     */
    template <typename T>
    ReturnCode Set(const std::string &name, const T &value){
        return (this->Set(name,dlp::Number::ToString(value)));
    }

    /**
     * @brief      Retrieves a numerical parameter value
     * @tparam     R               Type of the return value
     * @param[in]  name            Parameter name to retrieve
     * @param[in]  default_value   Fallback value if parameter doesn't exist
     * @param[out] value           Pointer to numerical variable to store result
     * @return     ReturnCode indicating success or specific error
     * @retval     PARAMETERS_NO_NAME      Supplied name is empty
     * @retval     PARAMETERS_NULL_POINTER Output pointer is null
     * @retval     PARAMETERS_NOT_FOUND    Parameter not found, default value used
     */
    template <typename R>
    ReturnCode Get( const std::string &name, const R &default_value, R* value) const{
        ReturnCode ret;
        std::string value_str;
        std::string default_value_str;

        // Convert the default value to a string
        default_value_str = dlp::Number::ToString(default_value);

        // Get the parameter as its string value
        ret = this->Get(name,default_value_str,&value_str);

        // Convert the string value to the template type
        (*value) = String::ToNumber<R>(value_str);

        return ret;
    }

    // String-based parameter operations (implemented in .cpp)

    /**
     * @brief      Creates or updates a parameter entry with string value
     * @param[in]  name    Name of the parameter
     * @param[in]  value   String value to store
     * @return     ReturnCode indicating success or specific error
     * @retval     PARAMETERS_NO_NAME              Supplied name is empty
     * @retval     PARAMETERS_ILLEGAL_CHARACTER    Name or value contains '=' character
     */
    RECONSTRUCTION3D_EXPORT ReturnCode Set(const std::string &name, const std::string &value);
    /**
     * @brief      Retrieves a string parameter value
     * @param[in]  name          Parameter name to retrieve
     * @param[in]  default_value Fallback value if parameter doesn't exist
     * @param[out] value         Pointer to store the retrieved value
     * @return     ReturnCode indicating success or specific error
     * @retval     PARAMETERS_NO_NAME      Supplied name is empty
     * @retval     PARAMETERS_NULL_POINTER Output pointer is null
     * @retval     PARAMETERS_NOT_FOUND    Parameter not found, default value used
     */
    RECONSTRUCTION3D_EXPORT ReturnCode Get(const std::string &name, const std::string &default_value, std::string* value) const;

    /**
     * @brief      Retrieves parameter name by index
     * @param[in]  index     Index of the parameter (0-based)
     * @param[out] ret_name  Pointer to store the parameter name
     * @return     ReturnCode indicating success or specific error
     * @retval     PARAMETERS_INDEX_OUT_OF_RANGE   Index is invalid
     * @retval     PARAMETERS_NULL_POINTER         Output pointer is null
     */
    RECONSTRUCTION3D_EXPORT ReturnCode GetName(const int &index, std::string* ret_name) const;
    
    /**
     * @brief      Removes a parameter from the list
     * @param[in]  name    Name of parameter to remove
     * @return     ReturnCode indicating success or specific error
     * @retval     PARAMETERS_NO_NAME      Supplied name is empty
     * @retval     PARAMETERS_NOT_FOUND    Parameter not found in list
     */
    RECONSTRUCTION3D_EXPORT ReturnCode Remove(const std::string &name);

    /**
     * @brief      Checks if parameter exists in list
     * @param[in]  name    Parameter name to search for
     * @return     true if parameter exists, false otherwise
     */
    RECONSTRUCTION3D_EXPORT bool Contains(const std::string &name) const;
    
    /**
     * @brief      Checks if parameter exists and returns its index
     * @param[in]  name        Parameter name to search for
     * @param[out] ret_index   Pointer to store parameter index if found
     * @return     true if parameter exists, false otherwise
     */
    RECONSTRUCTION3D_EXPORT bool Contains(const std::string &name, int *ret_index) const;

    /**
     * @brief      Adds a comment for a specific parameter
     * @param[in]  param_name  Name of the parameter to add comment for
     * @param[in]  comment     Comment text to associate with the parameter
     * @return     ReturnCode indicating success or specific error
     * @retval     PARAMETERS_NO_NAME  Supplied parameter name is empty
     * 
     * This method allows multiple comments to be associated with a single parameter.
     * Comments are stored in insertion order and will be displayed before the parameter
     * when saving with comments enabled.
     * 
     * @note The parameter name is standardized (uppercase + trimmed) before storage.
     * 
     * @example
     * @code
     * dlp::Parameters params;
     * params.AddComment("ITERATION_COUNT", "Optimization Control");
     * params.AddComment("ITERATION_COUNT", "Maximum number of iterations");
     * params.AddComment("ITERATION_COUNT", "Range: 10-1000");
     * @endcode
     */
    RECONSTRUCTION3D_EXPORT ReturnCode AddComment(const std::string &param_name, const std::string &comment);
    
    /**
     * @brief      Saves parameters to a text file with optional comments
     * @param[in]  filename      Output file path
     * @param[in]  with_comments If true, includes parameter comments in output
     * @return     ReturnCode indicating success or specific error
     * @retval     PARAMETERS_EMPTY            No parameters to save
     * @retval     PARAMETERS_FILE_OPEN_FAILED Failed to open file for writing
     * 
     * The output file format includes:
     * - Parameter comments (if enabled) as lines starting with '#'
     * - Parameter entries in "NAME = VALUE" format
     * - Empty lines between parameter groups for better readability
     * 
     * @note Comments associated with a parameter are displayed before the parameter line.
     * Multiple comments for the same parameter are displayed in insertion order.
     * 
     * @example Output format:
     * @code
     * # Distortion Model Parameters
     * # ===========================
     * # Enable zero tangent distortion
     * ZERO_TANGENT_DISTORTION = true
     * 
     * # Iteration Control
     * # Maximum iterations: 100
     * ITERATION_COUNT = 100
     * @endcode
     */
    RECONSTRUCTION3D_EXPORT ReturnCode Save(const std::string &filename, bool with_comments) const;

    /**
     * @brief      Saves parameters to a text file
     * @param[in]  filename    Output file path
     * @return     ReturnCode indicating success or specific error
     * @retval     PARAMETERS_EMPTY            No parameters to save
     * @retval     PARAMETERS_FILE_OPEN_FAILED Failed to open file for writing
     * 
     * File format: Each line contains "NAME = VALUE" pairs
     */
    RECONSTRUCTION3D_EXPORT ReturnCode Save(const std::string &filename) const;

    /**
     * @brief      Loads parameters from file, updating existing ones
     * @param[in]  filename    Input file path
     * @return     ReturnCode indicating success or specific error/warning
     * @retval     PARAMETERS_FILE_DOES_NOT_EXIST  File not found
     * @retval     PARAMETERS_FILE_OPEN_FAILED     Failed to open file
     * @retval     PARAMETERS_MISSING_VALUE        Some entries had formatting issues
     */
    RECONSTRUCTION3D_EXPORT ReturnCode Load(const std::string &filename);
    
    /**
     * @brief      Loads parameters from file with update control
     * @param[in]  filename        Input file path
     * @param[in]  update_current  Whether to update existing parameters
     * @return     ReturnCode indicating success or specific error/warning
     * @retval     PARAMETERS_FILE_DOES_NOT_EXIST  File not found
     * @retval     PARAMETERS_FILE_OPEN_FAILED     Failed to open file
     * @retval     PARAMETERS_MISSING_VALUE        Some entries had formatting issues
     */
    RECONSTRUCTION3D_EXPORT ReturnCode Load(const std::string &filename, const bool &update_current);

    /**
     * @brief      Loads parameters from another Parameters object
     * @param[in]  source    Source Parameters object
     * @return     ReturnCode indicating success or specific error
     * @retval     PARAMETERS_SOURCE_EMPTY  Source object is empty
     */
    RECONSTRUCTION3D_EXPORT ReturnCode Load(const Parameters &source);
    
    /**
     * @brief      Loads parameters from another Parameters object with update control
     * @param[in]  source          Source Parameters object
     * @param[in]  update_current  Whether to update existing parameters
     * @return     ReturnCode indicating success or specific error
     * @retval     PARAMETERS_SOURCE_EMPTY  Source object is empty
     */
    RECONSTRUCTION3D_EXPORT ReturnCode Load(const Parameters &source, const bool &update_current);

    /**
     * @brief      Checks if the parameter list is empty
     * @return     true if no parameters are stored, false otherwise
     */
    RECONSTRUCTION3D_EXPORT bool isEmpty() const;
    
    /**
     * @brief      Gets the number of parameters in the list
     * @return     Number of parameters stored
     */
    RECONSTRUCTION3D_EXPORT unsigned int GetCount() const;
    
    /**
     * @brief      Removes all parameters from the list
     */
    RECONSTRUCTION3D_EXPORT void Clear();

    /**
     * @brief      Converts parameters to human-readable string
     * @return     String representation of all parameters in "NAME = VALUE" format
     */
    RECONSTRUCTION3D_EXPORT std::string ToString();

private:
    std::vector<std::string> names_;   ///< Vector storing parameter names
    std::vector<std::string> values_;  ///< Vector storing parameter values (corresponding to names)
    std::vector<std::string> comments_;   ///< Vector storing parameter comments
    std::multimap<std::string, size_t> comment_index_map_; ///< Map from param name to comment index
};

}

#endif // DLP_SDK_PARAMETERS_HPP
