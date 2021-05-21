#pragma once

#if defined(_WIN32)

#include <yvals.h>

// assert без проверки на DEBUG_MODE
#define ERROR_REPORT(condition, message) _STL_VERIFY((condition), message)

#else

#include <iostream>

#define ERROR_REPORT(condition, message)\
if(!(condition)){\
	std::cerr << "error:" << "\nmessage: " << (message) << "\nline: " << __LINE__ << "\nfile: " << __FILE__ << std::endl;\
	exit(EXIT_FAILURE);\
}

#endif

#ifdef DEBUG_MODE

#define EASSERT(condition, message) ERROR_REPORT(condition, message)

#else

#define EASSERT(condition, message) (condition)

#endif

