#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

// Warning: GCC only to retrieve the filename
#ifndef __FILENAME__
#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define log_debug(M, ...) fprintf(stderr, "[DEBUG](%s:%d): " M "\n", __FILENAME__, __LINE__, ##__VA_ARGS__)
#define log_info(M, ...) fprintf(stderr, "[INFO]: " M "\n", ##__VA_ARGS__)
#define log_warning(M, ...) fprintf(stderr, "[WARNING](%s:%d): " M "\n", __FILENAME__, __LINE__, ##__VA_ARGS__)
#define log_error(M, ...) fprintf(stderr, "[ERROR](%s:%d): " M "\n", __FILENAME__, __LINE__, ##__VA_ARGS__)

#endif
