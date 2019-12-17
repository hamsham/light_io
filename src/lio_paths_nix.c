
// expose the UNIX98 standard AND enable "ntfw(...)"
#define _XOPEN_SOURCE 500
#include <ftw.h> // ftw, nftw(...)
#include <errno.h>
#include <wordexp.h>
#include <unistd.h> // rmdir(...)
#include <dirent.h> // DIR, dirent(), readdir(), closedir()
#include <sys/types.h> // mode_t

#include <stdio.h>
#include <string.h> // strlen
#include <stdlib.h> // size_t, realpath(...) (POSIX)

#include "light_io/lio_config.h"
#include "light_io/lio_utils.h"
#include "light_io/lio_paths.h"


/*-----------------------------------------------------------------------------
 * Determine what architecture is available
-----------------------------------------------------------------------------*/
#if LIGHT_IO_SYS_ARCH == 32
    #define STAT stat64
    #define LSTAT lstat64
#elif LIGHT_IO_SYS_ARCH == 64
    #define STAT stat
    #define LSTAT lstat
#else
    #error "Unknown Architecture!"
#endif



/*-----------------------------------------------------------------------------
 * Check for a path on the filesystem
-----------------------------------------------------------------------------*/
bool lio_path_does_exist(
    const char *const restrict path,
    const enum LioPathType pathType)
{
    const size_t numChars = strlen(path);
    
    if (!path || !numChars)
    {
        fprintf(stderr, "Unable to locate a file at a non-existent directory.\n");
        return false;
    }
    
    struct STAT info;
    const int result = LSTAT(path, &info);

    // The path doesn't exist.
    if (result != 0)
    {
        //const int errVal = errno;
        //const char* const errStr = strerror(errVal);
        //fprintf(stderr, "Unable to read the file \"%s\": %s.\n", path, errStr);
        return false;
    }
    
    if ((info.st_mode & S_IRUSR) == 0)
    {
        fprintf(stderr, "Unable to read the file \"%s\". Permission Denied.\n", path);
        return -1;
    }
    
    const mode_t fileMode = info.st_mode;
    
    switch (pathType)
    {
        case LIO_PATH_TYPE_REGULAR:
            return S_ISREG(fileMode) != 0;
        
        case LIO_PATH_TYPE_FILE:
             return S_ISREG(fileMode) != 0 || S_ISLNK(fileMode) != 0 || S_ISBLK(fileMode) || S_ISFIFO(fileMode) != 0 || S_ISCHR(fileMode) != 0;
        
        case LIO_PATH_TYPE_LINK:
            return S_ISLNK(fileMode) != 0;
        
        case LIO_PATH_TYPE_FOLDER:
            return S_ISDIR(fileMode) != 0;
        
        case LIO_PATH_TYPE_ANY:
        default:
            break;
    }
    
    return true;
}



/*-----------------------------------------------------------------------------
 * Error handler for expanding a path
-----------------------------------------------------------------------------*/
static void _lio_path_expand_error(const char* const restrict path, const int errCode)
{
    const char* errStr = NULL;
    
    switch (errCode)
    {
        case WRDE_BADCHAR:
            errStr = "ERROR: Illegal character located in path expansion: \"%s\"\n";
            break;
    
        case WRDE_BADVAL:
            errStr = "ERROR: Failed expansion of a shell variable in: \"%s\"\n";
            break;
            
        case WRDE_CMDSUB:
            errStr = "ERROR: Failed command substitution in path expansion: \"%s\"\n";
            break;
            
        case WRDE_NOSPACE:
            errStr = "ERROR: Ran out of memory while expanding path: \"%s\"\n";
            break;
            
        case WRDE_SYNTAX:
            errStr = "ERROR: Bad syntax encountered while expanding path: \"%s\"\n";
            break;
            
        default:
            errStr = "ERROR: Unknown error occurred while expanding the path: \"%s\"\n";
            break;
    }
    
    fprintf(stderr, errStr, path);
}



/*-----------------------------------------------------------------------------
 * Function to expand paths (symlinks, environment vars, relative paths).
-----------------------------------------------------------------------------*/
char* lio_path_resolve(const char *const restrict pInPath)
{
    // Expand all variables in the input path
    wordexp_t wxp;
    const int errCode = wordexp(pInPath, &wxp, 0);
    size_t bytesToAlloc = 0;
    
    if (errCode != 0)
    {
        _lio_path_expand_error(pInPath, errCode);
        wordfree(&wxp);
        return NULL;
    }
    
    bytesToAlloc += sizeof(char) * (wxp.we_wordc-1); // leave room for spaces
    
    for (size_t i = 0; i < wxp.we_wordc; ++i)
    {
        bytesToAlloc += strlen(wxp.we_wordv[i]) * sizeof(char);
    }
    
    // create a temporary string to hold the expanded path
    char* const tempPath = (char*)malloc(bytesToAlloc+1);
    if (!tempPath)
    {
        fprintf(stderr, "Failed to allocate memory while expanding the file path \"%s\".\n", pInPath);
        wordfree(&wxp);
        return NULL;
    }

    unsigned lastCharIndex = 0;
    tempPath[bytesToAlloc] = '\0';
    
    // if the expanded string turned into multiple arguments, concatenate the
    // strings into a single path.
    for (unsigned i = 0; i < wxp.we_wordc; ++i)
    {
        const char* const currStr = wxp.we_wordv[i];
        const size_t numCurrChars = strlen(currStr);

        // add a space in between all expansions
        if (i != 0)
        {
            tempPath[lastCharIndex++] = ' ';
        }
        
        memcpy(tempPath+lastCharIndex, currStr, numCurrChars*sizeof(char));
        lastCharIndex += numCurrChars;
    }

    char* const pOutPath = realpath(tempPath, NULL);
    
    wordfree(&wxp);
    free(tempPath);
    
    if (!pOutPath)
    {
        perror("Unable to resolve a file path");
    }
    
    return pOutPath;
}



/*-----------------------------------------------------------------------------
 * File Removal
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Callback function to remove a path recursively
------------------------------------*/
static int _lio_path_recursive_remove(
    const char* inPath,
    const struct stat* pUnusedStat,
    int fileType, struct FTW* pUnusedFtw)
{
    (void)pUnusedStat;
    (void)pUnusedFtw;
    
    // non-recursive call
    if (fileType == FTW_D)
    {
        const int ret = rmdir(inPath);

        if (ret != 0)
        {
            fprintf(stderr, "Cannot recursively remove non-empty directory: %s\n", inPath);
        }
        
        return ret;
    }
    
    // bad permissions
    if (fileType == FTW_DNR || fileType == FTW_NS)
    {
        fprintf(stderr, "Cannot remove path due to bad permissions: %s\n", inPath);
        return -1;
    }
    
    const int ret = remove(inPath);
    if (ret != 0)
    {
        fprintf(stderr, "Cannot remove path: %s\n", inPath);
    }
    
    return ret;
}




/*-------------------------------------
 * Recursively remove a path
------------------------------------*/
bool lio_path_remove(
    const char* const restrict path,
    const bool recurse,
    bool followLinks)
{
    if (lio_path_does_exist(path, LIO_PATH_TYPE_FILE))
    {
        if (!remove(path))
        {
            fprintf(stderr, "Cannot remove single file: %s\n", path);
            return false;
        }
        return true;
    }
    
    if (!recurse && lio_path_does_exist(path, LIO_PATH_TYPE_FOLDER))
    {
        if (rmdir(path) != 0)
        {
            fprintf(stderr, "Cannot recursively remove directory: %s\n", path);
            return false;
        }
    }
    
    const int walkFlags = FTW_DEPTH | (followLinks ? 0 : FTW_PHYS);
    
    return nftw(path, &_lio_path_recursive_remove, 1, walkFlags) == 0;
}



/*-----------------------------------------------------------------------------
    MKDIR
-----------------------------------------------------------------------------*/
bool lio_path_mkdirs(const char* const restrict pPath)
{
    static const mode_t permissions = 0 \
        | S_IRGRP | S_IWGRP | S_IXGRP \
        | S_IRUSR | S_IWUSR | S_IXUSR \
        | S_IROTH;

    if (!pPath)
    {
        return false;
    }

    char* const pTmpPath = lio_utils_str_copy(pPath, 0); //lio_path_resolve(pPath);
    /*
    if (!pTmpPath)
    {
        fprintf(stderr, "Unable to allocate memory before recursively creating directories: %s\n", pPath);
        return false;
    }
    */
    
    char* const pDir = pTmpPath;
    
    for(char* p = pDir+1; *p; p++)
    {
        // Detect if a path needs to be made by checking for a trailing slash
        if(*p != LIO_PATH_SEP)
        {
            continue;
        }
        
        // replace the current char with a '\0' so mkdir will think that's
        // a null-termination and only create a path up to that point.
        *p = '\0';
        
        if (!lio_path_does_exist(pDir, LIO_PATH_TYPE_FOLDER))
        {
            if (mkdir(pDir, permissions) != 0)
            {
                fprintf(stderr, "Cannot create parent directory: %s\n", pDir);
                lio_path_destroy(pTmpPath);
                return false;
            }
        }
        
        // return the trailing slash to its normal state.
        *p = LIO_PATH_SEP;
    }
    
    // create the final directory in a path
    const int ret = mkdir(pDir, permissions);
    
    if (ret != 0)
    {
        fprintf(stderr, "Cannot create directory: %s\n", pDir);
    }
    
    //lio_path_destroy(pTmpPath);
    lio_utils_str_destroy(pTmpPath);
    
    return ret == 0;
}



/*-----------------------------------------------------------------------------
 * get a path listing
-----------------------------------------------------------------------------*/
char** lio_path_list(
    const char* const baseDir,
    const bool listHidden,
    bool (*filter)(const char* const),
    unsigned* const pOutNumEntries)
{
    char* baseDirectory;
    struct dirent* pEntry;
    DIR* pDir = NULL;
    unsigned iter = 0;
    const unsigned numEntries = lio_path_count_entries(baseDir, listHidden, filter);
    char** const ret = (char**)calloc(numEntries, sizeof(char*));

    // make sure we have the full path to avoid errors in enumeration
    if (ret == NULL
    || (baseDirectory = lio_path_resolve(baseDir)) == NULL
    || lio_path_does_exist(baseDirectory, LIO_PATH_TYPE_FOLDER) == false)
    {
        fprintf(stderr, "Unable to resolve the directory \"%s\" for reading.\n", baseDir);
        closedir(pDir);
        return false;
    }

    if ((pDir = opendir(baseDirectory)) == NULL)
    {
        fprintf(stderr, "Failed to open the directory \"%s\" for reading.\n", baseDir);
        closedir(pDir);
        lio_path_destroy(baseDirectory);
        return false;
    }

    while ((pEntry = readdir(pDir)) != NULL)
    {
        const char* const entry = pEntry->d_name;
        if (entry == NULL)
        {
            fprintf(stderr, "Unable to enumerate a path within \"%s\".\n", baseDir);
            continue;
        }

        // Portability: "dotfiles" are *NIX only
        if ((!listHidden && entry[0] == '.')
        || strcmp(entry, ".") == 0
        || strcmp(entry, "..") == 0)
        {
            continue;
        }

        // concatenate full paths to avoid read errors
        char* const fullPath = lio_utils_str_fmt("%s%c%s", baseDirectory, LIO_PATH_SEP, entry);
        if (!fullPath)
        {
            fprintf(stderr, "Failed to concatenate the paths \"%s\" and \"%s\".\n", baseDirectory, entry);
            continue;
        }

        // user-defined entry filters
        if (!filter || filter(fullPath))
        {
            ret[iter++] = fullPath;
        }
    }

    closedir(pDir);
    lio_path_destroy(baseDirectory);

    *pOutNumEntries = iter;

    return ret;
}



/*-----------------------------------------------------------------------------
 * Count the number of entries in a directory
-----------------------------------------------------------------------------*/
unsigned lio_path_count_entries(
    const char* const baseDir,
    const bool listHidden,
    bool (*filter)(const char* const))
{
    char* baseDirectory;
    struct dirent* pEntry;
    DIR* pDir = NULL;
    unsigned numEntries = 0;

    // make sure we have the full path to avoid errors in enumeration
    if ((baseDirectory = lio_path_resolve(baseDir)) == NULL
    || lio_path_does_exist(baseDirectory, LIO_PATH_TYPE_FOLDER) == false)
    {
        fprintf(stderr, "Unable to resolve the directory \"%s\" for reading.\n", baseDir);
        closedir(pDir);
        return UINT_MAX;
    }

    if ((pDir = opendir(baseDirectory)) == NULL)
    {
        fprintf(stderr, "Failed to open the directory \"%s\" for reading.\n", baseDir);
        closedir(pDir);
        lio_path_destroy(baseDirectory);
        return UINT_MAX;
    }

    while ((pEntry = readdir(pDir)) != NULL)
    {
        const char* const entry = pEntry->d_name;
        if (entry == NULL)
        {
            fprintf(stderr, "Unable to enumerate a path within \"%s\".\n", baseDir);
            continue;
        }

        // Portability: "dotfiles" are *NIX only
        if ((!listHidden && entry[0] == '.')
        || strcmp(entry, ".") == 0
        || strcmp(entry, "..") == 0)
        {
            continue;
        }

        // concatenate full paths to avoid read errors
        char* const fullPath = lio_utils_str_fmt("%s%c%s", baseDirectory, LIO_PATH_SEP, entry);
        if (!fullPath)
        {
            fprintf(stderr, "Failed to concatenate the paths \"%s\" and \"%s\".\n", baseDirectory, entry);
            continue;
        }

        // user-defined entry filters
        if (!filter || filter(fullPath))
        {
            ++numEntries;
        }

        lio_path_destroy(fullPath);
    }

    closedir(pDir);
    lio_path_destroy(baseDirectory);

    return numEntries;
}



/*-----------------------------------------------------------------------------
 * Move a file or folder
-----------------------------------------------------------------------------*/
int lio_path_move(
    const char* const restrict pFrom,
    const char* const restrict pTo,
    const bool overwrite)
{
    // move directories
    if (lio_path_does_exist(pFrom, LIO_PATH_TYPE_FOLDER))
    {
        if (lio_path_does_exist(pTo, LIO_PATH_TYPE_FOLDER))
        {
            if (overwrite)
            {
                lio_path_remove(pTo, true, false);
            }
            else
            {
                return -1;
            }
        }

        return rename(pFrom, pTo);
    }
    else if (lio_path_does_exist(pFrom, LIO_PATH_TYPE_FILE))
    {
        if (lio_path_does_exist(pTo, LIO_PATH_TYPE_FILE))
        {
            if (overwrite)
            {
                lio_path_remove(pTo, false, false);
            }
            else
            {
                return -2;
            }
        }

        return rename(pFrom, pTo);
    }

    fprintf(stderr, "Error: cannot move \"%s\" to \"%s\"", pFrom, pTo);
    return -3;
}
