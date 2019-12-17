
#ifndef LIGHT_IO_FILE_IO_H
#define LIGHT_IO_FILE_IO_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif



enum LioFileLimitsType
{
    LIO_FILE_DEFAULT_CHUNK_SIZE = 4096 // KB
};



bool lio_file_copy(
    const char* const pFrom,
    const char* const pTo,
    const bool overwrite);



bool lio_file_concat(
    const char* const fileA,
    const char* const fileB,
    const char* const outFile,
    const bool overwrite);



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LIGHT_IO_FILE_IO_H */
