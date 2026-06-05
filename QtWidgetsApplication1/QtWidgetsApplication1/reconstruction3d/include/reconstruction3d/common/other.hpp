/** @file       other.hpp
 *  @ingroup    group_Common
 *  @brief      Contains common functions relating to strings, numbers, and time
 *  @copyright  Multi-view_IpS
 */

#ifndef DLP_SDK_OTHER_HPP
#define DLP_SDK_OTHER_HPP

#if defined(_MSC_VER)
#pragma warning(push, 0)  // ignore all warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"  // ignore all warnings
#endif

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <limits>   // for std::numeric_limits
#include <iomanip>  // for setPrecision()

#if defined(_MSC_VER)
#pragma warning(pop)  // restore warnings
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop  // restore warnings
#endif

#define NUM_TO_STRING_PRECISION     16
#define FILE_DOES_NOT_EXIST         "FILE_DOES_NOT_EXIST"


/** @brief  Contains all reconstruction 3D library classes, functions, etc. */
namespace dlp{

/* DISALLOW_COPY_AND_ASSIGN Macro from google
 * http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml?showone=Copy_Constructors#Copy_Constructors
 *
 */
// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

#define DLP_STD_CIN_GET(value) std::cin >> value; \
	std::cin.ignore( std::numeric_limits<std::streamsize>::max(), '\n' );

void PressEnterToContinue(std::string msg = "Press ENTER to continue...");


namespace CmdLine{

template <typename Type = std::string>
void Print(const std::string &msg = "", const Type &value = "", const std::string &units = ""){
	std::cout << msg << value << units << std::endl;
}

template <typename Type>
bool Get(Type &value, const std::string &msg = ""){
	bool ret = true;

	std::cout << msg;
	std::cin >> value;

	if(std::cin.fail()){
		std::cin.clear();
		ret = false;
	}
	std::cin.ignore( std::numeric_limits<std::streamsize>::max(), '\n' );
	return ret;
}

void PressEnterToContinue(const std::string &msg = "Press ENTER to continue...");

} //CmdLine

/** @brief  Contains sleep functions and time tracking \ref dlp::Time::Chronograph class
 *  @ingroup group_Common
 */
namespace Time{

	/** @brief  Contains methods to pause program or thread execution */
	namespace Sleep{
		void Microseconds(unsigned int time);
		void Milliseconds(unsigned int time);
		void Seconds(unsigned int time);
	} //Sleep

	/** @class Chronograph
	 *  @brief  Measures time between laps and total time in milliseconds
	 *  @ingroup group_Common
	 */
	class Chronograph{
	public:
		Chronograph();
		Chronograph(bool start);

		unsigned long long Reset();
		unsigned long long Lap();

		std::vector<unsigned long long> GetLapTimes();
		unsigned long long              GetTotalTime();

	private:
		unsigned long long start_;
		unsigned long long last_lap_;
		std::vector<unsigned long long> laps_;
	};
} //Time

/** @brief  Contains common functions related to files
 *  @ingroup group_Common
 */
namespace File{
	bool Exists( std::string filename );
	unsigned long long GetSize(std::string filename);
	std::vector<std::string> ReadLines(std::string filename);
} //File

/** @brief  Contains common functions related to string manipulation
 *  @ingroup group_Common
 */
namespace String{

	std::string Trim(const std::string &string);
	std::string ToUpperCase(const std::string &string);
	std::string ToLowerCase(const std::string &string);

	std::vector<std::string> SeparateDelimited(const std::string &string, const char &delimiter);

	#ifdef _MSC_VER  // If using Visual Studio (MSVC)
		#pragma warning(push)                // Save current warning state
		#pragma warning(disable : 4800)      // Disable warning C4800 (forcing bool conversion)
	#elif defined(__GNUC__) || defined(__clang__)  // If using GCC or Clang
		#pragma GCC diagnostic push          // Save current warning state
		#pragma GCC diagnostic ignored "-Wconversion"  // Disable conversion warning
	#endif
	/** @brief Converts an ASCII string number to a signed or unsigned numerical variable
	 *
	 *  For example, to convert a string to an int perform the following:
	 *  int value = dlp::String::ToNumber<int>("123");
	 */
	template <typename T>
	T ToNumber( const std::string &text, unsigned int base = 10){
		// Step 1: Clean the input string
		std::string trimmed = dlp::String::Trim(text);	// Remove leading/trailing whitespace		
		// // Check if the input represents a vector (contains comma)
		// if (trimmed.find(',') != std::string::npos) {
		// 	// Handle vector case			
		// 	std::istringstream ss(trimmed);
		// 	std::string item;			
		// 	std::vector<T> result;
		// 	while (std::getline(ss, item, ',')) {
		// 		// Process each item as a single number
		// 		result.push_back(ToNumber<T>(dlp::String::Trim(item)));
		// 	}			
		// 	return result;
		// }

		// Step 2: Detect hexadecimal notation
		std::size_t hex_0x = trimmed.find("0x");	// Standard 0x prefix
		std::size_t hex_x  = trimmed.find("x");		// Alternate x prefix
		// Handle hex prefixes
		if(hex_0x != std::string::npos){
			base = 16;	// Force hexadecimal base
			trimmed = trimmed.substr(hex_0x + 2);	// Remove "0x" prefix
		} else if(hex_x != std::string::npos){
			base = 16;	// Force hexadecimal base
			trimmed = trimmed.substr(hex_x + 1);	// Remove "x" prefix
		}
		// Step 3: String parsing setup
		std::istringstream ss(trimmed);	// Create string stream from cleaned input		
		T result;						// Stores final converted value
		long long number_int;			// Intermediate storage (handles all integer sizes)
		// Step 4: Base-specific conversion
		switch(base){
		case 8:		// Octal conversion
			ss >> std::oct >> number_int;          // Convert string to number
			result = (T) number_int;
			break;
		case 16:	// Hexadecimal conversion
			ss >> std::hex >> number_int;          // Convert string to number
			result = (T) number_int;
			break;
		case 10:	// Decimal conversion (default)
		default:
			ss >> std::dec >> number_int;          // Convert string to number
			result = (T) number_int;
		}

		return result;
	}
	#ifdef _MSC_VER  // If using Visual Studio (MSVC)
		#pragma warning(pop)                 // Restore previous warning state
	#elif defined(__GNUC__) || defined(__clang__)  // If using GCC or Clang
		#pragma GCC diagnostic pop           // Restore warning state
	#endif

	template <> std::string ToNumber( const std::string &string, unsigned int base);

	template <> float ToNumber(const std::string &string, unsigned int base);
	template <> double ToNumber(const std::string &string, unsigned int base);
	template <> long double ToNumber(const std::string &string, unsigned int base);

//    /** @brief Converts an ASCII string hexadecimal number to an unsigned numerical variable
//     *
//     *  For example, to convert a hexadecimal string to an int perform the following:
//     *  unsigned int value = dlp::String::HEXtoNumber_unsigned<unsigned int>("0x1A");
//     */
//    template <typename T>
//    T HEXtoNumber_unsigned(const std::string &text){
//        unsigned long long temp = strtoull(text, nullptr, 16);//std::stoull(text, nullptr, 16);
//        T result = (T) temp;
//        return result;
//    }

//    /** @brief Converts an ASCII string hexadecimal number to a signed numerical variable
//     *
//     * For example, to convert a hexadecimal string to an int perform the following:
//     *
//     * @code{.cpp}
//     * #include <stdlib.h>
//     * #include <stdio.h>
//     * int value = dlp::String::HEXtoNumber_unsigned<int>("0x1A");
//     * @endcode
//     *
//     * The expected output would be as follows.
//     *
//     * @verbatim
//     * user@ti.com : i = 23
//     * @endverbatim
//     *
//     */
//    template <typename T>
//    T HEXtoNumber_signed(const std::string &text){
//        long long temp =  strtoll(text, nullptr, 16); // std::stoll(text, nullptr, 16);
//        T result =  (T) temp;
//        return result;
//    }

} //String

/** @brief  Contains common functions to convert numbers to strings
 *  @ingroup group_Common
 */
namespace Number{

	/** @brief Converts a numerical variable to its ASCII string equivalent */
	template <typename T>
	std::string ToString( T number ){
		return " ";
	}

	template <> std::string ToString<std::string>( std::string string );
	template <> std::string ToString<char>( char number );
	template <> std::string ToString<unsigned char>( unsigned char number );
	template <> std::string ToString<int>(int number);
	template <> std::string ToString<unsigned int>(unsigned int number);
	template <> std::string ToString<long int>(long int number);
	template <> std::string ToString<unsigned long int>(unsigned long int number);
	template <> std::string ToString<unsigned long long>(unsigned long long number);
	template <> std::string ToString<float>(float number);
	template <> std::string ToString<double>(double number);
	template <> std::string ToString<long double>(long double number);
	template <> std::string ToString<bool>(bool number);
	// template <> std::string ToString<std::vector<bool>>(std::vector<bool> number);

} //Number

} //dlp

#endif // DLP_SDK_OTHER_HPP
