/*** Debug header. ***/

/** Version 1 + Functional Model Modification **/
/** Redundance check. **/
#ifndef DEBUG_HEADER
#define DEBUG_HEADER
#include <stdint.h>
/** Included files. **/
#include <stdio.h>                      /* Standard I/O operations. E.g. vprintf() */
#include <stdarg.h>                     /* Standard argument operations. E.g. va_list */
#include <sys/time.h>                   /* Time functions. E.g. gettimeofday() */


#if 1
#define TRACE_LOG(format, ...) (fprintf(stdout, "#%s(%d)-<%s>#\n"##format, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__))
#else
#define TRACE_LOG() ()
//空宏
#endif


#define RED_PRINT(str) printf("\033[0;31m%s\033[0m", str);
#define GREEN_PRINT(str) printf("\033[0;42;1m%s\033[0m\n", str);

/** Defninitions. **/
#define MAX_FORMAT_LEN 255
/*
#define DEBUG true
#define TITLE true
#define TIMER true
#define CUR  true
*/

#define DEBUG false
#define TITLE false
#define TIMER true
#define CUR  false

/** Classes. **/
class Debug
{
private:
    static long startTime;              /* Last start time in milliseconds. */

public:
    static void debugTitle(const char *str); /* Print debug title string. */
    static void debugItem(const char *format, ...); /* Print debug item string. */
    static void debugCur(const char *format, ...); /* Print debug item string. */
    static void notifyInfo(const char *format, ...); /* Print normal notification. */
    static void notifyError(const char *format, ...); /* Print error information. */
    static void startTimer(const char*); /* Start timer and display information. */
    static void endTimer(const char*); /* End timer and display information. */
    static uint64_t NowMicros(); /* get microsecs */
};

/** Redundance check. **/
#endif
