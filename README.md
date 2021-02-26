
# clog

Log macros and output management in C

## Building

`clog` is a header only library with an implementation define.

To use it, simply copy `clog.h` to your project, or add it to your include path.

Then include it in a *single* source file like so:

```
#define CLOG_IMPLEMENTATION
#include <clog.h>
```

You may also install it using cmake, like so:

```
cmake path/to/source
sudo make install
```

This will install both CMake and pkg-config configuration files.

## Usage

You can either use the default log macros like so:

```c
#define CLOG_USE_DEFAULTS
#define CLOG_IMPLEMENTATION
#include <clog.h>

void callback_func(clog_color_t color, const char * message, void * userData) {
    // Do whatever
}

int main(int argc, char** argv)
{
    clog_init();

    clog_add_file("allruns.log", true);
    clog_add_file("lastrun.log", false);
    
    clog_add_callback(callback_func, NULL);

    CLogVerb("This will be verbose");
    CLogInfo("This is informational");
    CLogWarn("This is a warning");
    CLogError("This is an error");

    // CLogFatal("This is a fatal error, it will exit(1)");

    clog_term();

    return 0;
}
```

Or you can define your own log macros like so:

```c
#define CLOG_IMPLEMENTATION
#include <clog.h>

#define LogLevel1(M, ...)                                                 \
    do {                                                                  \
        clog_log(CLOG_COLOR_RED, "[LVL1](%s:%d) " M "\n",                 \
            CLOG_FILENAME, __LINE__, ##__VA_ARGS__);                      \
    } while (0)

#define LogLevel2(M, ...)                                                 \
    do {                                                                  \
        clog_log(CLOG_COLOR_GREEN, "[LVL2](%s:%d) " M "\n",               \
            CLOG_FILENAME, __LINE__, ##__VA_ARGS__);                      \
    } while (0)

#define LogLevel3(M, ...)                                                 \
    do {                                                                  \
        clog_log(CLOG_COLOR_BLUE, "[LVL3](%s:%d) " M "\n",                \
            CLOG_FILENAME, __LINE__, ##__VA_ARGS__);                      \
    } while (0)

int main(int argc, char** argv)
{
    clog_init();

    LogLevel1("This is level 1");
    LogLevel2("This is level 2");
    LogLevel3("This is level 3");

    clog_term();

    return 0;
}
```

Several helper macros exist for controlling when log statements are processed:

```c
#define CLOG_USE_DEFAULTS
#define CLOG_IMPLEMENTATION
#include <clog.h>

int main(int argc, char** argv)
{
    clog_init();

    for (int i = 0; i < 100; ++i) {
        CLogOnce(CLogInfo("This will only print once"));
        CLogEvery(10, CLogInfo("This will only print ten times"));
        CLogWhen(i % 2 == 0, CLogInfo,"This will print fifty times"));
    }

    clog_term();

    return 0;
}
```

In order to reduce the length of `__FILE__`, you can define the base of your source path like so:

```c
#define CLOG_SOURCE_PATH "/path/to/source/"
```

This is made easier using CMake:

```cmake
TARGET_COMPILE_DEFINITIONS(
    MyTarget
    PRIVATE
        CLOG_SOURCE_PATH="${CMAKE_SOURCE_DIR}/"
)
```

There are three sane limits that can be overriden if needed:

```c
#if !defined(CLOG_MAX_LOG_MESSAGE_LENGTH)
    #define CLOG_MAX_LOG_MESSAGE_LENGTH 200
#endif

#if !defined(CLOG_MAX_LOG_FILES)
    #define CLOG_MAX_LOG_FILES 10
#endif

#if !defined(CLOG_MAX_LOG_CALLBACKS)
    #define CLOG_MAX_LOG_CALLBACKS 10
#endif

```