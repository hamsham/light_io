
#ifndef VTK_UTILS_PATHS_H
#define VTK_UTILS_PATHS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif



#ifdef _WIN32
    #define PATH_SEPARATOR '\\'
#else
    #define PATH_SEPARATOR '/'
#endif



/**
 * @brief Enumeration which can be used to filter or request certain types of
 * paths from the path functions.
 */
enum PathType
{
    PATH_TYPE_ANY,     // Any type of thing that exists on the file system
    PATH_TYPE_REGULAR, // Files only, no links
    PATH_TYPE_FILE,    // Files or links to files
    PATH_TYPE_LINK,    // Links only
    PATH_TYPE_FOLDER   // directories
};



/**
 * @brief Determine if a path exists on the local filesystem.
 *
 * @param path
 * A pointer to a constant string which contains a full, relative, or linked
 * path to some entry on the local filesystem.
 *
 * @param pathType
 * An enumeration which can be used to check for folders, files, symlinks, or
 * any other type of file.
 *
 * @return TRUE if the file entry represented by "path" exists on the local
 * filesystem, FALSE if not.
 */
bool path_does_exist(
    const char* const path,
    const enum PathType pathType);



/**
 * @brief
 *
 * @param pInPath
 * A pointer to a constant string, containing a full, relative, or linked path
 * which will be expanded to a full path with all symlinks removed.
 *
 * @return A dynamically allocated string which contains the full, expanded path
 * if one exists on the local filesystem. Returns NULL if the path was not
 * found. The returned value will be no longer than PATH_MAX and must be freed
 * with "path_destroy*()".
 */
char* path_resolve(const char* const pInPath);



/**
 * @brief Copy the path from the input string into another dynamically
 * allocated string.
 *
 * @param pInPath
 * A string, containing a path to a file or folder on the local filesystem.
 *
 * @return A dynamically-allocated string which contains the exact same path as
 * the input string.
 */
char* path_copy(const char* const pInPath);



/**
 * @brief Release any dynamically allocated path strings back to the operating
 * system.
 *
 * @param pPath
 * A dynamically allocated string which was returned from one of the path
 * utility functions.
 */
void path_destroy(char* const pPath);



/**
 * @brief Release any dynamically allocated path strings back to the operating
 * system.
 *
 * @param pPaths
 * A dynamically allocated array of dynamically created string which were
 * returned from one of the path utility functions.
 *
 * @param
 * Contains the number of paths in the array to destroy.
 */
void paths_destroy(char** const pPaths, unsigned numPaths);



/**
 * @brief Delete an entry from the local filesystem.
 *
 * This function can be used to remove individual files, folders, or entire
 * directory trees.
 *
 * @param path
 * A pointer to a constant string which represents a path to be removed from
 * the local filesystem.
 *
 * @param recurse
 * Attempt to recursivly delete files within a directory tree.
 *
 * @param followLinks
 * (*NIX only) Determines if symbolic links should be followed.
 *
 * @return TRUE if the path could successfully be removed, FALSE if not.
 */
bool path_remove(
    const char* const path,
    const bool recurse,
    bool followLinks);



/**
 * @brief Recursively create a directory structure.
 *
 * @param pPath
 * The fully qualified path to a directory which should exist on the local
 * filesystem.
 *
 * @return
 * TRUE if the directory tree could be made, FALSE if not.
 */
bool path_mkdirs(const char* const pPath);



/**
 * @brief Retrieve the base file/folder name of a path.
 *
 * This fucntion will return the last component in a path. All leading path
 * names will be removed and a newly allocated string is returned. The return
 * value should be deleted with 'path_destroy()' when it is no longer being
 * used.
 *
 * @param pPath
 * A full or relative path to an entry on the local filesystem.
 *
 * @return A newly allocated string, containing the basename of a path (or NULL
 * if an error occurred). This string should be deleted with 'path_destroy()'
 * when it is no longer being used.
 */
char* path_basename(const char* const pPath);



/**
 * @brief Retrieve all leading components of a path string. The final component
 * of the path is removed.
 *
 * @param pPath
 * A full or relative path to an entry on the local filesystem.
 *
 * @return A newly allocated string which contains all but the final component
 * of the input path string. A value of NULL is returned if an error occurred.
 * The return value should be deleted with "path_destroy()" when it is no
 * longer being used.
 */
char* path_dirname(const char* const pPath);


/**
 * @brief Retrieve a list of all immediate files or folders contained within a
 * parent folder.
 *
 * @param baseDir
 * The base folder to query for child entries.
 *
 * @param listHidden
 * Determine if hidden files or folder should be placed into the returned path
 * array.
 *
 * @param filter
 * A pointer to a function which will be used to determine if certain path
 * entries should be kept or removed from the returned array. This function
 * accepts a string, containing the full path to an entry on the local
 * filesystem, and should return TRUE if the entry should be returned or FALSE
 * removed to not be returned.
 *
 * @param pOutNumEntries
 * A pointer to an unsigned integer which will provide the calling function
 * with the number of entries which were returned from this function.
 *
 * @return
 * An array of strings, each containing the full path to a file or folder on
 * the local filesystem which is an immediate descendant of the input path.
 */
char** path_list(
    const char* const baseDir,
    const bool listHidden,
    bool (*filter)(const char* const),
    unsigned* const pOutNumEntries);


/**
 * @brief Retrieve the number of child files/folders are contained within a
 * folder on the local filesystem.
 *
 * @param baseDir
 * The base folder to query for child entries.
 *
 * @param listHidden
 * Determine if hidden files or folder should be counted.
 *
 * @param filter
 * A pointer to a function which will be used to determine if certain path
 * entries should be added to the return value. This function accepts a string,
 * containing the full path to an entry on the local filesystem, and should
 * return TRUE if the entry should be counted or FALSE if not.
 *
 * @return
 * An unsigned integer, representing the number of sub-paths which are
 * contained within 'baseDir'. A value of UINT_MAX is returned in the event of
 * an error.
 */
unsigned path_count_entries(
    const char* const baseDir,
    const bool listHidden,
    bool (*filter)(const char* const));



/**
 * @brief Concatenate two paths to represent a longer path to an entry on the
 * local filesystem.
 *
 * This function does not check for valid paths. It is simply meant to provide
 * a cross-platform way to concatenate paths.
 *
 * @param pDirName
 * A string which represents the full or relative path to a directory on the
 * local filesystem.
 *
 * @param pBaseName
 * A string which represents a file, folder, or folders on the local
 * filesystem.
 *
 * @return A dynamically allocated string, containing the concatenation of the
 * input parameters.
 */
char* path_join(
    const char* const pDirName,
    const char* const pBaseName);



/*-----------------------------------------------------------------------------
 * Filters for path iteration functions
-----------------------------------------------------------------------------*/
/**
 * @brief Include all path entries when iterating through a directory.
 *
 * @param fullPath
 * A fully-qualified path to a location on the filesystem.
 *
 * @return TRUE if the path exists, FALSE if not.
 */
static inline bool path_filter_all(const char* const fullPath)
{
    return path_does_exist(fullPath, PATH_TYPE_ANY);
}

/**
 * @brief Include all file/link entries when iterating through a directory.
 *
 * @param filePath
 * A fully-qualified path to a file on the filesystem.
 *
 * @return TRUE if the file exists, FALSE if not.
 */
static inline bool path_filter_files(const char* const filePath)
{
    return path_does_exist(filePath, PATH_TYPE_FILE);
}

/**
 * @brief Include all folder entries when iterating through a directory.
 *
 * @param dirPath
 * A fully-qualified path to a directory on the filesystem.
 *
 * @return TRUE if the folder exists, FALSE if not.
 */
static inline bool path_filter_dirs(const char* const dirPath)
{
    return path_does_exist(dirPath, PATH_TYPE_FOLDER);
}



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* VTK_UTILS_PATHS_H */
