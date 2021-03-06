// Copyright (c) 2018-2020 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <string.h>
#include <libgen.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>

/// Builds a proper log message id from the filename.
#define LOG_MSGID static const char *__msgId(void) {    \
        static char *msgId = nullptr;                   \
        if (!msgId) {                                   \
            char *file = strdup(__FILE__);              \
            char *name = basename(file);                \
            char *sep = strrchr(name, '.');             \
            if (sep)                                    \
                *sep = '\0';                            \
            msgId = strdup(name);                       \
            for (size_t i = 0; i < strlen(msgId); ++i)  \
                msgId[i] = toupper(msgId[i]);           \
            free(file);                                 \
        }                                               \
        return msgId;                                   \
    }

/// Red shell output.
#define COLOR_RED "\33[0;31m"
/// Yellow shell output.
#define COLOR_YELLOW "\033[1;33m"
/// Green shell output.
#define COLOR_GREEN "\033[1;32m"
/// Blue shell output.
#define COLOR_BLUE "\033[1;34m"
/// Uncolored shell output.
#define COLOR_NC "\033[0m"


#if defined HAS_PMLOG
#include <PmLogLib.h>

PmLogContext getPmLogContext();
/// We may use this as function or value, so abstract function call
/// away.
#define logContext getPmLogContext()
#else
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>

/// Dummy variable for standalone substitution.
#define logContext 0



/// Helper macro for standalone mode.
#define PmLog(ctx, msgid, kvcount, fmt, ...)    \
    do {                                        \
        fprintf(stderr, fmt, ##__VA_ARGS__);    \
        fprintf(stderr, COLOR_NC "\n");         \
    } while (0);

/// Helper macro for standalone mode.
#define PmLogCritical(ctx, msgid, kvcount, ...)         \
    do {                                                \
        fprintf(stderr, COLOR_RED);                     \
        PmLog(ctx, msgid, kvcount, ##__VA_ARGS__);      \
    } while (0);

/// Helper macro for standalone mode.
#define PmLogError(ctx, msgid, kvcount, ...)            \
    do {                                                \
        fprintf(stderr, COLOR_RED);                     \
        PmLog(ctx, msgid, kvcount, ##__VA_ARGS__);      \
    } while (0);

/// Helper macro for standalone mode.
#define PmLogWarning(ctx, msgid, kvcount, ...);         \
    do {                                                \
        fprintf(stderr, COLOR_YELLOW);                  \
        PmLog(ctx, msgid, kvcount, ##__VA_ARGS__);      \
    } while (0);

/// Helper macro for standalone mode.
#define PmLogInfo(ctx, msgid, kvcount, ...)             \
    do {                                                \
        fprintf(stderr, COLOR_GREEN);                   \
        PmLog(ctx, msgid, kvcount, ##__VA_ARGS__);      \
    } while (0);

/// Helper macro for standalone mode.
#define PmLogDebug(ctx, fmt, file, func, ...)                   \
    do {                                                        \
        fprintf(stderr, fmt, file, func, ##__VA_ARGS__);        \
        fprintf(stderr, "\n");                                  \
    } while (0);
#endif

/// print thread ID.
#ifdef LOGGING_THREAD_ID
#define LOG_PERF(fmt, ...)                                    \
    PmLogCritical(logContext, "[PERF]", 0, COLOR_GREEN  "<%d> %s:%s() " fmt COLOR_NC, (pid_t) syscall(__NR_gettid), __FILE__, __FUNCTION__, ##__VA_ARGS__)

/// Log critical message.
#define LOG_CRITICAL(kvcount, fmt, ...)                       \
    PmLogCritical(logContext, __msgId(), kvcount,  "<%d> %s:%s() " fmt, (pid_t) syscall(__NR_gettid), __FILE__, __FUNCTION__, ##__VA_ARGS__)

/// Log error message.
#define LOG_ERROR(kvcount, fmt, ...)                          \
    PmLogError(logContext, __msgId(), kvcount, COLOR_RED "<%d> %s:%s() " fmt COLOR_NC, (pid_t) syscall(__NR_gettid), __FILE__, __FUNCTION__, ##__VA_ARGS__)

/// Log warning message.
#define LOG_WARNING(kvcount, fmt, ...)                        \
    PmLogWarning(logContext, __msgId(), kvcount, COLOR_YELLOW "<%d> %s:%s() " fmt COLOR_NC, (pid_t) syscall(__NR_gettid), __FILE__, __FUNCTION__, ##__VA_ARGS__)

/// Log info message.
#define LOG_INFO(kvcount, fmt, ...)                           \
    PmLogInfo(logContext, __msgId(), kvcount, COLOR_GREEN "<%d> %s:%s() " fmt COLOR_NC, (pid_t) syscall(__NR_gettid), __FILE__, __FUNCTION__, ##__VA_ARGS__)

/// Debug log.
#define LOG_DEBUG(fmt, ...)                                             \
    PmLogDebug(logContext, COLOR_BLUE "%s <%d> %s:%s() " fmt COLOR_NC, __msgId(), (pid_t) syscall(__NR_gettid), __FILE__, __FUNCTION__, ##__VA_ARGS__)
#else

#define LOG_PERF(fmt, ...)                                    \
    PmLogCritical(logContext, COLOR_GREEN "[PERF]", 0, "%s:%s() " fmt COLOR_NC, __FILE__, __FUNCTION__, ##__VA_ARGS__)

/// Log critical message.
#define LOG_CRITICAL(kvcount, fmt, ...)                       \
    PmLogCritical(logContext, __msgId(), kvcount,  "%s:%s() " fmt, __FILE__, __FUNCTION__, ##__VA_ARGS__)

/// Log error message.
#define LOG_ERROR(kvcount, fmt, ...)                          \
    PmLogError(logContext, __msgId(), kvcount, COLOR_RED "%s:%s() " fmt COLOR_NC, __FILE__, __FUNCTION__, ##__VA_ARGS__)

/// Log warning message.
#define LOG_WARNING(kvcount, fmt, ...)                        \
    PmLogWarning(logContext, __msgId(), kvcount, COLOR_YELLOW "%s:%s() " fmt COLOR_NC, __FILE__, __FUNCTION__, ##__VA_ARGS__)

/// Log info message.
#define LOG_INFO(kvcount, fmt, ...)                           \
    PmLogInfo(logContext, __msgId(), kvcount, COLOR_GREEN "%s:%s() " fmt COLOR_NC, __FILE__, __FUNCTION__, ##__VA_ARGS__)

/// Debug log.
#define LOG_DEBUG(fmt, ...)                                             \
    PmLogDebug(logContext, COLOR_BLUE "%s %s:%s() " fmt COLOR_NC, __msgId(), __FILE__, __FUNCTION__, ##__VA_ARGS__)
#endif
