#include "simple_logger.h"
#include <stdarg.h>
#include <stdio.h>

FILE * __log_file = NULL;

void close_logger()
{
    if (__log_file != NULL)
    {
        fclose(__log_file);
        __log_file = NULL;
    }
}

void init_logger()
{
    __log_file = fopen("system/log.txt","a");
}

void slog(char *msg,...)
{
    va_list ap;
    /*echo all logging to stdout*/
    va_start(ap,msg);
    vfprintf(stdout,msg,ap);
    va_end(ap);
    fprintf(stdout,"\n");
    if (__log_file != NULL)
    {
        va_start(ap,msg);
        vfprintf(__log_file,msg,ap);
        fprintf(__log_file,"\n");
        va_end(ap);
    }
}


/*eol@eof*/
