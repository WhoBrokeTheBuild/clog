//
// clog version 1.0.0
//
// MIT License
//
// Copyright (c) 2021 Stephen Lane-Walsh
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifndef CLOG_H
#define CLOG_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__MSC_VER)
#   define CLOG_RESTRICT __restrict
#else
#   define CLOG_RESTRICT __restrict__
#endif

#if !defined(CLOG_SOURCE_PATH)
    #define CLOG_SOURCE_PATH ""
#endif

#define CLOG_FILENAME (&__FILE__[sizeof(CLOG_SOURCE_PATH) - 1])

#if !defined(CLOG_MAX_LOG_MESSAGE_LENGTH)
    #define CLOG_MAX_LOG_MESSAGE_LENGTH 200
#endif

#if !defined(CLOG_MAX_LOG_FILES)
    #define CLOG_MAX_LOG_FILES 10
#endif

#if !defined(CLOG_MAX_LOG_CALLBACKS)
    #define CLOG_MAX_LOG_CALLBACKS 10
#endif

typedef enum clog_color
{
    CLOG_COLOR_DEFAULT = 0,
    CLOG_COLOR_BLACK,
    CLOG_COLOR_RED,
    CLOG_COLOR_GREEN,
    CLOG_COLOR_YELLOW,
    CLOG_COLOR_BLUE,
    CLOG_COLOR_MAGENTA,
    CLOG_COLOR_CYAN,
    CLOG_COLOR_WHITE,

} clog_color_t;

typedef void (*clog_func_t)(clog_color_t, const char *, void *);

void clog_init();

void clog_term();

bool clog_add_file(const char * filename, bool append);

bool clog_add_callback(clog_func_t func, void * userData);

bool clog_remove_callback(clog_func_t func, void * userData);

void clog_log(clog_color_t color, const char * CLOG_RESTRICT format, ...);

// Run the expression once and only once
#define CLogOnce(EXPR)                                                        \
    do {                                                                      \
        static bool _CLogOnce##__COUNTER__ = false;                           \
        if (!_CLogOnce##__COUNTER__) {                                        \
            EXPR;                                                             \
            _CLogOnce##__COUNTER__ = true;                                    \
        }                                                                     \
    } while (0)

// Run the expression once every COUNT times
#define CLogEvery(COUNT, EXPR)                                                \
    do {                                                                      \
        static int _CLogEvery##__COUNTER__ = 0;                               \
        if (++_CLogEvery##__COUNTER__ >= (COUNT)) {                           \
            EXPR;                                                             \
            _CLogEvery##__COUNTER__ = 0;                                      \
        }                                                                     \
    } while (0)

// Run the expression when TEST is true
#define CLogWhen(TEST, EXPR)                                                  \
    if (TEST) {                                                               \
        EXPR;                                                                 \
    }


#if defined(CLOG_USE_DEFAULTS)

    #define CLogFatal(M, ...)                                                 \
        do {                                                                  \
            clog_log(CLOG_COLOR_RED, "[FATA](%s:%d) " M "\n",                 \
                CLOG_FILENAME, __LINE__, ##__VA_ARGS__);                      \
            exit(1);                                                          \
        } while (0)

    #define CLogError(M, ...)                                                 \
        do {                                                                  \
            clog_log(CLOG_COLOR_RED, "[ERRO](%s:%d) " M "\n",                 \
                CLOG_FILENAME, __LINE__, ##__VA_ARGS__);                      \
        } while (0)

    #define CLogWarn(M, ...)                                                  \
        do {                                                                  \
            clog_log(CLOG_COLOR_YELLOW, "[WARN](%s:%d) " M "\n",              \
                CLOG_FILENAME, __LINE__, ##__VA_ARGS__);                      \
        } while (0)

    #define CLogInfo(M, ...)                                                  \
        do {                                                                  \
            clog_log(CLOG_COLOR_DEFAULT, "[INFO](%s:%d) " M "\n",             \
                CLOG_FILENAME, __LINE__, ##__VA_ARGS__);                      \
        } while (0)

    #define CLogVerb(M, ...)                                                  \
        do {                                                                  \
            clog_log(CLOG_COLOR_DEFAULT, "[VERB](%s:%d) " M "\n",             \
                CLOG_FILENAME, __LINE__, ##__VA_ARGS__);                      \
        } while (0)

#endif

#ifdef __cplusplus
} // extern "C"
#endif

#endif // CLOG_H

#if defined(CLOG_IMPLEMENTATION)

#include <stdio.h>
#include <stdarg.h>

#if defined(_WIN32)

    #include <Windows.h>

#elif defined(__unix__)

    #include <unistd.h>

#endif

#ifdef __cplusplus
extern "C" {
#endif

FILE * _clog_files[CLOG_MAX_LOG_FILES];

typedef struct clog_callback
{
    clog_func_t func;

    void * userData;

} clog_callback_t;

clog_callback_t _clog_callbacks[CLOG_MAX_LOG_CALLBACKS];

#if defined(_WIN32)

    HCONSOLE _clog_hconsole;

#elif defined(__unix__)

    bool _clog_is_term;

#endif

void clog_init()
{
    for (int i = 0; i < CLOG_MAX_LOG_FILES; ++i) {
        _clog_files[i] = NULL;
    }

    for (int i = 0; i < CLOG_MAX_LOG_CALLBACKS; ++i) {
        _clog_callbacks[i].func = NULL;
        _clog_callbacks[i].userData = NULL;
    }

    #if defined(_WIN32)

        _clog_hconsole = GetStdHandle(STD_OUTPUT_HANDLE);

    #elif defined(__unix__)

        _clog_is_term = isatty(fileno(stdout));

    #endif
}

void clog_term()
{
    for (int i = 0; i < CLOG_MAX_LOG_FILES; ++i) {
        if (_clog_files[i]) {
            fclose(_clog_files[i]);
            _clog_files[i] = NULL;
        }
    }

    for (int i = 0; i < CLOG_MAX_LOG_CALLBACKS; ++i) {
        _clog_callbacks[i].func = NULL;
        _clog_callbacks[i].userData = NULL;
    }
}

bool clog_add_file(const char * filename, bool append)
{
    for (int i = 0; i < CLOG_MAX_LOG_FILES; ++i) {
        if (!_clog_files[i]) {
            _clog_files[i] = fopen(filename, (append ? "at" : "wt"));
            
            if (!_clog_files[i]) {
                fprintf(stderr, "clog: unable to open '%s'\n", filename);
                return false;
            }

            return true;
        }
    }

    fprintf(stderr, "clog: max number of logfiles (%d) reached\n", 
        CLOG_MAX_LOG_FILES);

    return false;
}

bool clog_add_callback(clog_func_t func, void * userData)
{
    for (int i = 0; i < CLOG_MAX_LOG_CALLBACKS; ++i) {
        if (!_clog_callbacks[i].func && !_clog_callbacks[i].userData) {
            _clog_callbacks[i].func = func;
            _clog_callbacks[i].userData = userData;

            return 0;
        }
    }

    fprintf(stderr, "clog: max number of callbacks (%d) reached\n", 
        CLOG_MAX_LOG_CALLBACKS);
        
    return true;
}

bool clog_remove_callback(clog_func_t func, void * userData)
{
    for (int i = 0; i < CLOG_MAX_LOG_CALLBACKS; ++i) {
        if (_clog_callbacks[i].func == func && 
            _clog_callbacks[i].userData == userData) {

            _clog_callbacks[i].func = NULL;
            _clog_callbacks[i].userData = NULL;

            return 0;
        }
    }

    return 1;
}

void clog_log(clog_color_t color, const char * CLOG_RESTRICT format, ...)
{
    va_list args;
    char buffer[CLOG_MAX_LOG_MESSAGE_LENGTH];

    va_start(args, format);
    vsnprintf(buffer, CLOG_MAX_LOG_MESSAGE_LENGTH, format, args);

    if (color == CLOG_COLOR_DEFAULT) {
        printf("%s", buffer);
    }
    else {
        #if defined(_WIN32)

            if (_clog_hconsole != INVALID_HANDLE_VALUE) {
                int colorCode;

                switch (color) {
                case CLOG_COLOR_BLACK:
                    colorCode = 0;
                    break;
                case CLOG_COLOR_RED:
                    colorCode = 4;
                    break;
                case CLOG_COLOR_GREEN:
                    colorCode = 2;
                    break;
                case CLOG_COLOR_YELLOW:
                    colorCode = 6;
                    break;
                case CLOG_COLOR_BLUE:
                    colorCode = 1;
                    break;
                case CLOG_COLOR_MAGENTA:
                    colorCode = 5;
                    break;
                case CLOG_COLOR_CYAN:
                    colorCode = 3;
                    break;
                case CLOG_COLOR_WHITE:
                    colorCode = 7;
                    break;
                case CLOG_COLOR_DEFAULT:
                default:
                    colorCode = 7;
                    break;
                }

                SetConsoleTextAttribute(_clog_hconsole, colorCode);
                printf("%s", buffer);
                SetConsoleTextAttribute(_clog_hconsole, 7);
            }

        #elif defined(__unix__)

            if (_clog_is_term) {
                int colorCode;

                switch (color) {
                case CLOG_COLOR_BLACK:
                    colorCode = 30;
                    break;
                case CLOG_COLOR_RED:
                    colorCode = 31;
                    break;
                case CLOG_COLOR_GREEN:
                    colorCode = 32;
                    break;
                case CLOG_COLOR_YELLOW:
                    colorCode = 33;
                    break;
                case CLOG_COLOR_BLUE:
                    colorCode = 34;
                    break;
                case CLOG_COLOR_MAGENTA:
                    colorCode = 35;
                    break;
                case CLOG_COLOR_CYAN:
                    colorCode = 36;
                    break;
                case CLOG_COLOR_WHITE:
                    colorCode = 37;
                    break;
                case CLOG_COLOR_DEFAULT:
                default:
                    colorCode = 39;
                    break;
                }

                printf("\033[%dm%s\033[0m", colorCode, buffer);
            }

        #else

            printf("%s", buffer);

        #endif
    }

    for (int i = 0; i < CLOG_MAX_LOG_FILES; ++i) {
        if (_clog_files[i]) {
            fprintf(_clog_files[i], "%s", buffer);
        }
    }

    for (int i = 0; i < CLOG_MAX_LOG_CALLBACKS; ++i) {
        if (_clog_callbacks[i].func) {
            _clog_callbacks[i].func(color, buffer,
                _clog_callbacks[i].userData);
        }
    }
}

#ifdef __cplusplus
} // extern "C"
#endif

#endif