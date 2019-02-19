

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif /* WIN32_LEAN_AND_MEAN */
#include <windows.h>
#include <shlwapi.h> // PathCanonicalizeA()
#include <shobjidl.h> // SHFILEOPSTRUCT
#include <shellapi.h> // SHFileOperationA
#include <direct.h> // _mkdir()

#include <errno.h>
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
 * Check for a path on the filesystem
-----------------------------------------------------------------------------*/
bool path_does_exist(
    const char *const restrict path,
    const enum PathType pathType)
{
    const size_t numChars = strlen(path);
    
    if (!path || !numChars)
    {
        fprintf(stderr, "Unable to locate a file at a non-existent directory.\n");
        return false;
    }

    DWORD attribs = GetFileAttributesA(path);
    
    // The path doesn't exist.
    if (attribs == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }

    switch (pathType)
    {
        case PATH_TYPE_REGULAR:
            return (attribs & (FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE)) != 0;
        
        case PATH_TYPE_FILE:
            return (attribs & (FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_ARCHIVE)) != 0;
        
        case PATH_TYPE_LINK:
            return (attribs & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
        
        case PATH_TYPE_FOLDER:
            return (attribs & FILE_ATTRIBUTE_DIRECTORY) != 0;
        
        case PATH_TYPE_ANY:
        default:
            break;
    }
    
    return true;
}



/*-----------------------------------------------------------------------------
 * Function to expand paths (symlinks, environment vars, relative paths).
-----------------------------------------------------------------------------*/
char* path_resolve(const char *const restrict pInPath)
{
    const size_t bytesToAlloc = (sizeof(unsigned char)*MAX_PATH) + 1;

    if (!pInPath || pInPath[0] == '\0')
    {
        fprintf(stderr, "Cannot expand a NULL file path.\n");
        return NULL;
    }
    
    // create a temporary string to hold the expanded path
    char* pOutPath = (char*)malloc(bytesToAlloc);
    if (!pOutPath)
    {
        fprintf(stderr, "Failed to allocate memory while expanding the file path \"%s\".\n", pInPath);
        return NULL;
    }
    else
    {
        memset(pOutPath, '\0', bytesToAlloc);
    }

    BOOL ret = PathCanonicalizeA(pOutPath, pInPath);
        
    if (ret == FALSE)
    {
        fprintf(stderr, "Unable to resolve the file path \"%s\".", pInPath);
        free(pOutPath);
        pOutPath = NULL;
    }
    
    return pOutPath;
}



/*-----------------------------------------------------------------------------
 * File Removal
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Recursively remove a path
------------------------------------*/
bool path_remove(
    const char* const restrict path,
    const bool recurse,
    bool followLinks)
{
    (void)followLinks;

    if (path_does_exist(path, PATH_TYPE_FILE))
    {
        if (!remove(path))
        {
            fprintf(stderr, "Cannot remove single file: %s\n", path);
            return false;
        }
        return true;
    }

    size_t dirLen = strlen(path) + 2; // +2 NULL characters are required for SHFileOperation.
    char* tmpDir = (char*)malloc(dirLen);
    
    memcpy(tmpDir, path, dirLen-2);
    tmpDir[dirLen-1] = '\0';
    tmpDir[dirLen-2] = '\0';

    FILEOP_FLAGS opFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
    if (!recurse)
    {
        opFlags |= FOF_NORECURSION;
    }

  SHFILEOPSTRUCT pathOp = {
    NULL,
    FO_DELETE,
    tmpDir,
    "",
    opFlags,
    FALSE,
    NULL,
    ""
  };
  
  int ret = SHFileOperation(&pathOp);
  free(tmpDir);

  if (ret != 0)
  {
      fprintf(stderr, "Encountered error %X while removing the directory \"%s\".\n", ret, path);
      return false;
  }
  
  return true;
}



/*-----------------------------------------------------------------------------
    MKDIR
-----------------------------------------------------------------------------*/
bool path_mkdirs(const char* const restrict pPath)
{
    if (!pPath)
    {
        return false;
    }

    char* const pTmpPath = utils_str_copy(pPath, 0); //path_resolve(pPath);
    if (!pTmpPath)
    {
        fprintf(stderr, "Unable to allocate memory before recursively creating directories: %s\n", pPath);
        return false;
    }
    
    char* const pDir = pTmpPath;
    
    for(char* p = pDir+1; *p; p++)
    {
        // Detect if a path needs to be made by checking for a trailing slash
        if(*p != PATH_SEPARATOR)
        {
            continue;
        }
        
        // replace the current char with a '\0' so mkdir will think that's
        // a null-termination and only create a path up to that point.
        *p = '\0';
        
        if (!path_does_exist(pDir, PATH_TYPE_FOLDER))
        {
            if (_mkdir(pDir) != 0)
            {
                fprintf(stderr, "Encountered error %X while creating a directory: %s\n", errno, pDir);
                path_destroy(pTmpPath);
                return false;
            }
        }
        
        // return the trailing slash to its normal state.
        *p = PATH_SEPARATOR;
    }
    
    // create the final directory in a path
    const int ret = _mkdir(pDir);
    
    if (ret != 0)
    {
        fprintf(stderr, "Encountered error %X while creating a directory: %s\n", errno, pDir);
    }
    
    //path_destroy(pTmpPath);
    utils_str_destroy(pTmpPath);
    
    return ret == 0;
}



/*-----------------------------------------------------------------------------
 * get a path listing
-----------------------------------------------------------------------------*/
char** path_list(
    const char* const baseDir,
    const bool listHidden,
    bool (*filter)(const char* const),
    unsigned* const pOutNumEntries)
{
    char* baseDirectory = NULL;
    char* searchDirectory = NULL;
    WIN32_FIND_DATA pData;
    HANDLE pEntry = INVALID_HANDLE_VALUE;
    unsigned iter = 0;
    const unsigned numEntries = path_count_entries(baseDir, listHidden, filter);
    char** const ret = (char**)calloc(numEntries, sizeof(char*));

    // make sure we have the full path to avoid errors in enumeration
    if (ret == NULL
    || (baseDirectory = path_resolve(baseDir)) == NULL
    || path_does_exist(baseDirectory, PATH_TYPE_FOLDER) == false
    || (searchDirectory = utils_str_fmt("%s%c*", baseDirectory, PATH_SEPARATOR)) == NULL)
    {
        fprintf(stderr, "Unable to resolve the directory \"%s\" for reading.\n", baseDir);
        path_destroy(baseDirectory);
        return false;
    }

    if ((pEntry = FindFirstFile(searchDirectory, &pData)) == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "Failed to open the directory \"%s\" for reading.\n", baseDir);
        utils_str_destroy(searchDirectory);
        path_destroy(baseDirectory);
        return false;
    }

    while (FindNextFile(pEntry, &pData) != 0)
    {
        // Portability: "dotfiles" are *NIX only, but we'll block them in Windows
        if ((!listHidden && (pData.cFileName[0] == '.' || (GetFileAttributesA(pData.cFileName) & FILE_ATTRIBUTE_HIDDEN)))
        || strcmp(pData.cFileName, ".") == 0
        || strcmp(pData.cFileName, "..") == 0)
        {
            continue;
        }

        // concatenate full paths to avoid read errors
        char* const fullPath = utils_str_fmt("%s%c%s", baseDirectory, PATH_SEPARATOR, pData.cFileName);
        if (!fullPath)
        {
            fprintf(stderr, "Failed to concatenate the paths \"%s\" and \"%s\".\n", baseDirectory, pData.cFileName);
            continue;
        }

        // user-defined entry filters
        if (!filter || filter(fullPath))
        {
            ret[iter++] = fullPath;
        }
    }
    FindClose(pEntry);

    utils_str_destroy(searchDirectory);
    path_destroy(baseDirectory);

    *pOutNumEntries = iter;

    return ret;
}



/*-----------------------------------------------------------------------------
 * Count the number of entries in a directory
-----------------------------------------------------------------------------*/
unsigned path_count_entries(
    const char* const baseDir,
    const bool listHidden,
    bool (*filter)(const char* const))
{
    char* baseDirectory = NULL;
    char* searchDirectory = NULL;
    WIN32_FIND_DATA pData;
    HANDLE pEntry = INVALID_HANDLE_VALUE;
    unsigned numEntries = 0;

    // make sure we have the full path to avoid errors in enumeration
    if ((baseDirectory = path_resolve(baseDir)) == NULL
    || path_does_exist(baseDirectory, PATH_TYPE_FOLDER) == false
    || (searchDirectory = utils_str_fmt("%s%c*", baseDirectory, PATH_SEPARATOR)) == NULL)
    {
        fprintf(stderr, "Unable to resolve the directory \"%s\" for reading.\n", baseDir);
        return false;
    }

    if ((pEntry = FindFirstFile(searchDirectory, &pData)) == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "Failed to open the directory \"%s\" for reading.\n", baseDir);
        utils_str_destroy(searchDirectory);
        path_destroy(baseDirectory);
        return false;
    }

    while (FindNextFile(pEntry, &pData) != 0)
    {
        // Portability: "dotfiles" are *NIX only, but we'll block them in Windows
        if ((!listHidden && (pData.cFileName[0] == '.' || (GetFileAttributesA(pData.cFileName) & FILE_ATTRIBUTE_HIDDEN)))
        || strcmp(pData.cFileName, ".") == 0
        || strcmp(pData.cFileName, "..") == 0)
        {
            continue;
        }

        // concatenate full paths to avoid read errors
        char* const fullPath = utils_str_fmt("%s%c%s", baseDirectory, PATH_SEPARATOR, pData.cFileName);
        if (!fullPath)
        {
            fprintf(stderr, "Failed to concatenate the paths \"%s\" and \"%s\".\n", baseDirectory, pData.cFileName);
            continue;
        }

        // user-defined entry filters
        if (!filter || filter(fullPath))
        {
            ++numEntries;
        }
    }
    FindClose(pEntry);

    utils_str_destroy(searchDirectory);
    path_destroy(baseDirectory);

    return numEntries;
}



/*-----------------------------------------------------------------------------
 * Move a file or folder
-----------------------------------------------------------------------------*/
int path_move(
    const char* const restrict pFrom,
    const char* const restrict pTo,
    const bool overwrite)
{
    // move directories
    if (path_does_exist(pFrom, PATH_TYPE_FOLDER))
    {
        if (path_does_exist(pTo, PATH_TYPE_FOLDER))
        {
            if (overwrite)
            {
                path_remove(pTo, true, false);
            }
            else
            {
                return -1;
            }
        }

        return MoveFileEx(pFrom, pTo, MOVEFILE_WRITE_THROUGH | MOVEFILE_FAIL_IF_NOT_TRACKABLE | MOVEFILE_COPY_ALLOWED);
    }
    else if (path_does_exist(pFrom, PATH_TYPE_FILE))
    {
        if (path_does_exist(pTo, PATH_TYPE_FILE))
        {
            if (overwrite)
            {
                path_remove(pTo, false, false);
            }
            else
            {
                return -2;
            }
        }

        return MoveFileEx(pFrom, pTo, MOVEFILE_WRITE_THROUGH | MOVEFILE_FAIL_IF_NOT_TRACKABLE | MOVEFILE_COPY_ALLOWED);
    }

    fprintf(stderr, "Error: cannot move \"%s\" to \"%s\"", pFrom, pTo);
    return -3;
}
