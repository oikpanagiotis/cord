#ifndef LOG_H
#define LOG_H

#include "typedefs.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// GCC only to retrieve the filename
#ifndef __FILENAME__
#define __FILENAME__                                                           \
    (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1   \
                                      : __FILE__)
#endif

// TODO: Add support for logger_get(name) so we can log the module that produces
// the log

#define MAX_LOGGER_OUTPUT_STREAMS 8

typedef enum cord_log_level {
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
} cord_log_level;

typedef struct cord_logger_t {
    FILE *stream[MAX_LOGGER_OUTPUT_STREAMS];
    bool verbose;
    i32 num_streams;
    i32 log_level;
} cord_logger_t;

cord_logger_t *logger_create(FILE *stream, i32 log_level, bool verbose);
void logger_add_stream(cord_logger_t *logger, FILE *stream);
void logger_destroy(cord_logger_t *logger);

void logger_use(cord_logger_t *logger);
void global_logger_init(void);
void global_logger_destroy(void);

void logger_info(const char *fmt, ...);
void logger_debug(const char *fmt, ...);
void logger_warn(const char *fmt, ...);
void logger_error(const char *fmt, ...);

#endif
