#define ALSOFT_VERSION "1.17.1"

#if defined(_MSC_VER)
	/* API declaration export attribute */
	#define AL_API  __declspec(dllexport)
	#define ALC_API __declspec(dllexport)
	/* Define any available alignment declaration */
	#define ALIGN(x) __declspec(align(x))
#else
	#define AL_API  __attribute__((visibility("protected")))
	#define ALC_API __attribute__((visibility("protected")))
#endif // defined(_MSC_VER)
