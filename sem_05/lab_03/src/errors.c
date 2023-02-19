/* 
 * err_quit не является стандартной функцией.
 * err_quit определяется в include/apue.h, реализация находится в lib/error.c. 
 * (но у нас его нет, поэтому определяем здесь руками)
 */


#include "apue.h"

#include <stdarg.h>

void err_quit(const char *fmt, ...)
{
    va_list args;
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}