#ifndef DEBUGGING_H
#define DEBUGGING_H

#ifdef DEBUG
	int printf(const char *format, ...);
	#define DEBUG_PRINTF(format, ...) printf (format, __VA_ARGS__)
	#define DEBUG_PRINT(format) printf(format)
#else
	#define DEBUG_PRINTF(format, ...)
	#define DEBUG_PRINT(format)
#endif

#endif

