// To make gtkcolorpicker* compile.

#ifndef GTK_COMPILATION
#	define GTK_COMPILATION
#endif

#if defined(_WIN32) || defined(_WIN64)
#	if !defined(G_OS_WIN32)
#		define G_OS_WIN32 1
#	endif
#elif defined(__linux__)
#	if !defined(G_OS_UNIX)
#		define G_OS_UNIX 1
#	endif
#else
#	error "Unknown platform."
#endif
