
#include <stdarg.h> // va_start, va_end, va_list
#include <stdlib.h> // calloc(...), NULL
#include <stdio.h> // vsnprintf(...)
#include <string.h> // strlen(...)

#include "light_io/lio_utils.h"



char* utils_str_fmt(const char* fmt, ...)
{
    va_list args;
    char *pStr = NULL;
    
    // vsnprintf(...) automatically includes a byte for NULL-termination.
    va_start(args, fmt);
    const int iNumBytes = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    
    if (iNumBytes <= 0)
    {
        return NULL;
    }
    
    const size_t numBytes = (size_t) iNumBytes+1;
    pStr = (char*) malloc(numBytes);

    if (pStr != NULL)
    {
        va_start(args, fmt);
        const long long bytesWritten = vsnprintf(pStr, numBytes, fmt, args) + 1;
        va_end(args);
    
        if ((size_t) bytesWritten != numBytes)
        {
            static const char err[] = "ERROR: Unable to create a formatted string (wrote %lld/%zu bytes).\n";
            fprintf(stderr, err, bytesWritten, numBytes);
            free(pStr);
            pStr = NULL;
        }
    }
    
    return pStr;
}



char* utils_str_copy(const char* str, const size_t maxChars)
{
    if (!str)
    {
        return NULL;
    }

    // unsafe warning: maxChars can be bigger than strlen(str)
    const size_t numChars = (maxChars > 0) ? maxChars : strlen(str);
    const size_t numBytes = sizeof(char) * (1 + numChars);
    char* const pNewStr = (char*)calloc(numChars+1, sizeof(char));
    
    if (!pNewStr)
    {
        return NULL;
    }
    
    return (char*)memcpy(pNewStr, str, numBytes);
}



char* utils_str_concat(const char* const str1, const char* const str2)
{
    if (!str1 || !str2)
    {
        return NULL;
    }
    
    const size_t str1Size = strlen(str1);
    const size_t str2Size = strlen(str2);
    const size_t numChars = str1Size + str2Size + 1;
    char* const pNewStr = (char*)calloc(numChars, sizeof(char));
    
    if (!pNewStr)
    {
        return NULL;
    }
    
    if (!(char*)memcpy(pNewStr, str1, str1Size*sizeof(char))
    || !(char*)memcpy(pNewStr+str1Size, str2, str2Size*sizeof(char)))
    {
        utils_str_destroy(pNewStr);
        return NULL;
    }
    
    return pNewStr;
}



void utils_str_destroy(char* const pStr)
{
    free(pStr);
}
