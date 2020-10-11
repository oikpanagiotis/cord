#ifndef LOG_H
#define LOG_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define debug(M, ...) fprintf(stderr, "[DEBUG](%s:%d): " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define log_info(M, ...) fprintf(stderr, "[INFO](%s:%d): " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define log_warning(M, ...) fprintf(stderr, "[WARNING](%s:%d): " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define log_error(M, ...) fprintf(stderr, "[ERROR](%s:%d): " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#endif
