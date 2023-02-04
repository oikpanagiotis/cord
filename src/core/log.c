#include "log.h"
#include "typedefs.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>

#define tnormal "\x1B[0m"
#define tgreen "\x1B[32m"
#define tblue "\x1B[34m"
#define tpurple "\x1B[35m"
#define tyellow "\x1B[33m"
#define tred "\x1B[31m"

static cord_logger_t *g_logger = NULL;

cord_logger_t *logger_create(FILE *stream, i32 log_level, bool verbose) {
    cord_logger_t *logger = malloc(sizeof(cord_logger_t));
    if (logger) {
        logger->stream[0] = stream;
        logger->num_streams = 1;
        logger->log_level = log_level;
        logger->verbose = verbose;
        return logger;
    }

    return NULL;
}

void logger_add_stream(cord_logger_t *logger, FILE *stream) {
    if (logger->num_streams < MAX_LOGGER_OUTPUT_STREAMS) {
        logger->stream[logger->num_streams++] = stream;
    }
}

void logger_destroy(cord_logger_t *logger) {
	if (logger) {
		for (i32 i = 0; i < logger->num_streams; i++) {
			FILE *stream = logger->stream[i];
			if (stream) {
				fclose(stream);
			}
		}
        free(logger);
	}
}

void logger_use(cord_logger_t *logger) {
    if (!logger) {
        fprintf(stderr, "logger is null\n");
        return;
    }

    g_logger = logger;
}

static void check_global_logger(void) {
    if (!g_logger) {
        fprintf(stderr, "failed: Global Logger is null\n");
        return;
    }
}

static void set_date_time(char *buffer) {
    time_t timer;
    time(&timer);
    struct tm tm = *localtime(&timer);
    sprintf(buffer, "%02d/%02d/%02d %02d:%02d:%02d",
        tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void logger_info(const char *fmt, ...) {
    check_global_logger();

    if (g_logger->log_level >= LOG_LEVEL_INFO) {
        for (int i = 0; i < g_logger->num_streams; i++) {
            if (g_logger->verbose) {
                fprintf(g_logger->stream[i], tblue "[ " tgreen "INFO  " tblue "]" tnormal " %s:%d: ", __FILE__, __LINE__);
            } else {
                fprintf(g_logger->stream[i], tblue "[ " tgreen "INFO  " tblue "]" tnormal " ");
            }
            va_list args;
            va_start(args, fmt);
            vfprintf(g_logger->stream[i], fmt, args);
            va_end(args);
            fprintf(g_logger->stream[i], "\n");
        }
    }
}

static void log_date_time(FILE *stream) {
    if (stream) {
        char date_time[24] = {0};
        set_date_time(date_time);
        fprintf(stream, tyellow "[" tnormal "%s" tyellow "]" tnormal , date_time);
    }
}

static void log_debug_label(FILE *stream) {
    if (stream) {
        fprintf(stream, tblue "[ " tpurple "DEBUG " tblue "]" tnormal " ");
    }
}

static void log_debug_label_vewrbose(FILE *stream) {
    if (stream) {
        fprintf(stream, tblue "[ " tpurple "DEBUG " tblue "]" tnormal " %s:%d: ", __FILE__, __LINE__);
    }
}

void logger_debug(const char *fmt, ...) {
#if NDEBUG
// Do not log debug logs on release builds
#else
    check_global_logger();
    

    if (g_logger->log_level >= LOG_LEVEL_DEBUG) {
        for (int i = 0; i < g_logger->num_streams; i++) {
            if (g_logger->verbose) {
                log_date_time(g_logger->stream[i]);
                log_debug_label_vewrbose(g_logger->stream[i]);
            } else {
                log_date_time(g_logger->stream[i]);
                log_debug_label(g_logger->stream[i]);
            }
            va_list args;
            va_start(args, fmt);
            vfprintf(g_logger->stream[i], fmt, args);
            va_end(args);
            fprintf(g_logger->stream[i], "\n");
        }
    }
#endif
}

void logger_warn(const char *fmt, ...) {
    check_global_logger();

    if (g_logger->log_level >= LOG_LEVEL_ERROR) {
        for (int i = 0; i < g_logger->num_streams; i++) {
            if (g_logger->verbose) {
                fprintf(g_logger->stream[i], tblue "[ " tyellow "WARN  " tblue "]" tnormal " %s:%d: ", __FILE__, __LINE__);
            } else {
                fprintf(g_logger->stream[i], tblue "[ " tyellow "WARN  " tblue "]" tnormal " ");
            }
            va_list args;
            va_start(args, fmt);
            vfprintf(g_logger->stream[i], fmt, args);
            va_end(args);
            fprintf(g_logger->stream[i], "\n");
        }
    }
}

void logger_error(const char *fmt, ...) {
    check_global_logger();

    if (g_logger->log_level >= LOG_LEVEL_ERROR) {
        for (int i = 0; i < g_logger->num_streams; i++) {
            fprintf(g_logger->stream[i], tblue "[ " tred "ERROR " tblue "]" tnormal " (%s:%d): ", __FILE__, __LINE__);
            va_list args;
            va_start(args, fmt);
            vfprintf(g_logger->stream[i], fmt, args);
            va_end(args);
            fprintf(g_logger->stream[i], "\n");
        }
    }
}

void global_logger_init(void) {
    cord_logger_t *logger = logger_create(stdout, LOG_LEVEL_INFO, false);
    logger_use(logger);
}

void global_logger_destroy(void) {
    logger_destroy(g_logger);
}