
#include <stdlib.h>
#include <stdio.h>

#include "light_io/lio_paths.h"
#include "light_io/lio_files.h"

// Thanks Windows
#ifndef restrict
    #ifdef __restrict
        #define restrict __restrict
    #else
        #define restrict
    #endif
#endif



/*-----------------------------------------------------------------------------
 * Stream data into a file
-----------------------------------------------------------------------------*/
static bool _lio_file_stream_into_file(
    const char* const from,
    const char* const to,
    const size_t chunkSize,
    const bool append
)
{
    const char writeMode[3] = {(char)(append ? 'a' : 'w'), 'b', '\0'};
    char* const buffer = (char*) malloc(sizeof(char) * chunkSize);

    if (!buffer)
    {
        return false;
    }

    FILE* const pFrom = fopen(from, "rb");
    if (!pFrom)
    {
        fprintf(stderr, "Unable to open the file \"%s\" for reading.\n", from);
        free(buffer);
        return false;
    }

    FILE* const pTo = fopen(to, writeMode);
    if (!pTo)
    {
        fprintf(stderr, "Unable to open the file \"%s\" for writing.\n", to);
        free(buffer);
        fclose(pFrom);
        return false;
    }

    size_t chunksRead = 0;
    do
    {
        chunksRead = fread(buffer, 1, chunkSize, pFrom);

        if (chunksRead != fwrite(buffer, 1, chunksRead, pTo))
        {
            fprintf(stderr, "Failed to stream %zu bytes into \"%s\"\n.", chunksRead, to);
            free(buffer);
            fclose(pFrom);
            fclose(pTo);
            return false;
        }
    }
    while (chunksRead == chunkSize);

    free(buffer);
    fclose(pFrom);
    fclose(pTo);
    return true;
}



/*-----------------------------------------------------------------------------
 * Copy data from one file to another
-----------------------------------------------------------------------------*/
bool lio_file_copy(
    const char* const restrict from,
    const char* const restrict to,
    const bool overwrite)
{
    if (!from || !to)
    {
        fprintf(stderr, "Unable to copy paths. Invalid file names.\n");
        return false;
    }

    if (!lio_path_does_exist(from, LIO_PATH_TYPE_FILE))
    {
        fprintf(stderr, "Unable to copy file \"%s\". File does not exist.\n", from);
        return false;
    }

    if (lio_path_does_exist(to, LIO_PATH_TYPE_FILE) && !overwrite)
    {
        fprintf(stderr, "Cannot copy \"%s\" to \"%s\". File exists at destination.\n", from, to);
        return false;
    }

    return _lio_file_stream_into_file(from, to, LIO_FILE_DEFAULT_CHUNK_SIZE, false);
}



/*-----------------------------------------------------------------------------
 * Concatenate two files
-----------------------------------------------------------------------------*/
bool lio_file_concat(
    const char* const restrict fileA,
    const char* const restrict fileB,
    const char* const restrict outFile,
    const bool overwrite)
{
    if (!fileA || !fileB)
    {
        fprintf(stderr, "Unable to concatenate files. Invalid file names.\n");
        return false;
    }

    if (!lio_path_does_exist(fileA, LIO_PATH_TYPE_FILE))
    {
        fprintf(stderr, "Unable to read file for concatenation \"%s\". File does not exist.\n", fileA);
        return false;
    }

    if (!lio_path_does_exist(fileB, LIO_PATH_TYPE_FILE))
    {
        fprintf(stderr, "Unable to read file for concatenation \"%s\". File does not exist.\n", fileB);
        return false;
    }

    if (lio_path_does_exist(outFile, LIO_PATH_TYPE_FILE) && !overwrite)
    {
        fprintf(stderr, "Unable to write concatenated files to \"%s\". File already exists.\n", outFile);
        return false;
    }

    return
        (_lio_file_stream_into_file(fileA, outFile, LIO_FILE_DEFAULT_CHUNK_SIZE, false) &&
        _lio_file_stream_into_file(fileB, outFile, LIO_FILE_DEFAULT_CHUNK_SIZE, true))
        || lio_path_remove(outFile, false, false);
}
