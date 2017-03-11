#define ALSOFT_VERSION "1.17.1"

#if defined (__linux__)
	#define AL_API  __attribute__((visibility("protected")))
	#define ALC_API __attribute__((visibility("protected")))
	#define ALIGN(x) __attribute__((aligned(x)))
#elif defined(_MSC_VER)
	#define AL_API  __declspec(dllexport)
	#define ALC_API __declspec(dllexport)
	#define ALIGN(x) __declspec(align(x))
#else
	#define AL_API  __declspec(dllexport)
	#define ALC_API __declspec(dllexport)
	#define ALIGN(x) __attribute__((aligned(x)))
#endif // defined(_MSC_VER)
