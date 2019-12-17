
#include <stdio.h>
#include <string.h> // strlen, memset()
#include <stdlib.h> // size_t, realpath(...) (POSIX)

#include "light_io/lio_config.h"
#include "light_io/lio_utils.h"
#include "light_io/lio_paths.h"



// Thanks Windows
#ifndef restrict
    #ifdef __restrict
        #define restrict __restrict
    #else
        #define restrict
    #endif
#endif



/*-----------------------------------------------------------------------------
 * Copy a path string
-----------------------------------------------------------------------------*/
char* lio_path_copy(const char* const restrict pPath)
{
    return lio_utils_str_copy(pPath, 0);
}



/*-----------------------------------------------------------------------------
 * Destroy dynamically created paths
-----------------------------------------------------------------------------*/
void lio_path_destroy(char* const pPath)
{
    lio_utils_str_destroy(pPath);
}



/*-----------------------------------------------------------------------------
 * Destroy dynamically created arrays of paths
-----------------------------------------------------------------------------*/
void lio_paths_destroy(char** const restrict pPaths, unsigned numPaths)
{
    while (numPaths --> 0)
    {
        free(pPaths[numPaths]);
    }

    free(pPaths);
}



/*-----------------------------------------------------------------------------
 * Basename
-----------------------------------------------------------------------------*/
char* lio_path_basename(const char* const restrict pPath)
{
    if (!pPath)
    {
        return NULL;
    }

    const size_t numChars = strlen(pPath);
    size_t iter = numChars;

    while (iter --> 0)
    {
        const char c = pPath[iter];

        if (c == LIO_PATH_SEP)
        {
            break;
        }
    }

    ++iter;

    if (iter == numChars)
    {
        return lio_utils_str_fmt("\0");
    }

    return lio_utils_str_copy(pPath+iter, 0);
}



/*-----------------------------------------------------------------------------
 * dirname
-----------------------------------------------------------------------------*/
char* lio_path_dirname(const char* const restrict pPath)
{
    if (!pPath)
    {
        return NULL;
    }

    const size_t numChars = strlen(pPath);
    size_t iter = numChars;

    while (iter --> 0)
    {
        const char c = pPath[iter];

        if (c == LIO_PATH_SEP)
        {
            break;
        }
    }

    if (iter == numChars)
    {
        return lio_utils_str_fmt("\0");
    }

    char* const ret = (char*) malloc(iter+1);
    if (!ret)
    {
        return NULL;
    }

    ret[iter] = '\0';

    return strncpy(ret, pPath, iter);
}



/*-----------------------------------------------------------------------------
 * Join two paths
-----------------------------------------------------------------------------*/
char* lio_path_join(
    const char* const restrict pDirName,
    const char* const restrict pBaseName)
{
    // don't trust the caller.
    if (!pDirName)
    {
        return !pBaseName ? lio_utils_str_copy("", 0) : lio_utils_str_copy(pBaseName, 0);
    }

    if (!pBaseName)
    {
        return lio_utils_str_copy("", 0);
    }

    char* const pTemp = lio_utils_str_fmt("%s%c%s", pDirName, LIO_PATH_SEP, pBaseName);
    if (!pTemp)
    {
        fprintf(stderr, "Unable to join the paths \"%s\" and \"%s\".\n", pDirName, pBaseName);
        return NULL;
    }

    return pTemp;
}
