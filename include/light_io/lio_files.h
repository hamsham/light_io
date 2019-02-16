
#ifndef VTK_UTILS_FILE_IO_H
#define VTK_UTILS_FILE_IO_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif



enum FileIOLimitsType
{
    FILE_IO_DEFAULT_CHUNK_SIZE = 4096 // KB
};



bool fileio_copy(
    const char* const pFrom,
    const char* const pTo,
    const bool overwrite);



bool fileio_concat(
    const char* const fileA,
    const char* const fileB,
    const char* const outFile,
    const bool overwrite);



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* VTK_UTILS_FILE_IO_H */
