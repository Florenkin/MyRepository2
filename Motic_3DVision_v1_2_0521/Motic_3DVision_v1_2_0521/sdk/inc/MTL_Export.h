#ifndef __MOTIC_EXPORT_HEADER__
#define __MOTIC_EXPORT_HEADER__

#if defined(_WIN32)
	#ifdef MOTICSDK_EXPORTS
		#ifdef __cplusplus
			#define MTLAPI extern "C" __declspec(dllexport)
		#else
			#define MTLAPI __declspec(dllexport)
		#endif
	#else
		#ifdef __cplusplus
			#define MTLAPI extern "C" __declspec(dllimport)
		#else
			#define MTLAPI __declspec(dllimport)			
		#endif
		#ifdef _DEBUG
			//#pragma comment(lib, "MtLDetect.lib")
		#else
			//#pragma comment(lib, "MtLDetect.lib")
		#endif
	#endif
#else
	#ifndef MTLAPI
            #ifdef __cplusplus
                #define MTLAPI extern "C" __attribute__((visibility("default")))
            #else
                #define MTLAPI __attribute__((visibility("default")))
            #endif
	#endif
#endif

#endif
