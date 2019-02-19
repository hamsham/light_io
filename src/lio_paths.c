
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
char* path_copy(const char* const restrict pPath)
{
    return utils_str_copy(pPath, 0);
}



/*-----------------------------------------------------------------------------
 * Destroy dynamically created paths
-----------------------------------------------------------------------------*/
void path_destroy(char* const pPath)
{
    utils_str_destroy(pPath);
}



/*-----------------------------------------------------------------------------
 * Destroy dynamically created arrays of paths
-----------------------------------------------------------------------------*/
void paths_destroy(char** const restrict pPaths, unsigned numPaths)
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
char* path_basename(const char* const restrict pPath)
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

        if (c == PATH_SEPARATOR)
        {
            break;
        }
    }

    ++iter;

    if (iter == numChars)
    {
        return utils_str_fmt("\0");
    }

    return utils_str_copy(pPath+iter, 0);
}



/*-----------------------------------------------------------------------------
 * dirname
-----------------------------------------------------------------------------*/
char* path_dirname(const char* const restrict pPath)
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

        if (c == PATH_SEPARATOR)
        {
            break;
        }
    }

    if (iter == numChars)
    {
        return utils_str_fmt("\0");
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
char* path_join(
    const char* const restrict pDirName,
    const char* const restrict pBaseName)
{
    // don't trust the caller.
    if (!pDirName)
    {
        return !pBaseName ? utils_str_copy("", 0) : utils_str_copy(pBaseName, 0);
    }

    if (!pBaseName)
    {
        return utils_str_copy("", 0);
    }

    char* const pTemp = utils_str_fmt("%s%c%s", pDirName, PATH_SEPARATOR, pBaseName);
    if (!pTemp)
    {
        fprintf(stderr, "Unable to join the paths \"%s\" and \"%s\".\n", pDirName, pBaseName);
        return NULL;
    }

    return pTemp;
}
