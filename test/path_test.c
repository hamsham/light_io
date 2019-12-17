
#include <stdio.h>

#include "light_io/lio_utils.h"
#include "light_io/lio_paths.h"


int main(int argc, char* argv[])
{
    int ret = 0;
    int testId = 0;
    char* pCwd = NULL;
    char* pExc = NULL;
    unsigned numChildPaths = 0u;
    char** pSiblings = NULL;
    unsigned i = 0u;
    char* tmpDirStr = NULL;
    char* superDir = NULL;
    char* duperDir = NULL;
    char* newSubDir = NULL;

    // Test that files can be found on the local file system
    ++testId;
    if (!lio_path_does_exist(argv[0], LIO_PATH_TYPE_FILE))
    {
        fprintf(stderr, "Unable to validate the current program path \"%s.\"\n", argv[0]);
        ret = testId;
        goto end;
    }
    else
    {
        printf("Current program path:\n\t%s\n", argv[0]);
    }

    // Test that directories can be extracted from a path.
    ++testId;
    pCwd = lio_path_dirname(argv[0]);
    if (!pCwd)
    {
        fprintf(stderr, "Unable to determine the directory name of \"%s.\"\n", argv[0]);
        ret = testId;
        goto end;
    }
    else
    {
        printf("Current working program directory:\n\t%s\n", pCwd);
    }

    // Test that base names can be extracted from a path.
    ++testId;
    pExc = lio_path_basename(argv[0]);
    if (!pExc)
    {
        fprintf(stderr, "Unable to determine the base name of \"%s.\"\n", argv[0]);
        ret = testId;
        goto end;
    }
    else
    {
        printf("Current executable:\n\t%s\n", pExc);
    }


    // Test that directory trees can be enumerated
    ++testId;
    numChildPaths = lio_path_count_entries(pCwd, false, NULL);
    printf("File Listing of %u items:", numChildPaths);
    pSiblings = lio_path_list(pCwd, false, NULL, &numChildPaths);
    for (i = 0u; i < numChildPaths; ++i)
    {
        printf("\n\t%s", pSiblings[i]);
    }
    printf("\n");

    if (i != numChildPaths)
    {
        fprintf(stderr, "The number of expected directories within \"%s\" does not match the expected count (%u/%u).\n", pCwd, i, numChildPaths);
        ret = testId;
        goto end;
    }

    // Test that paths can be concatenated
    ++testId;
    tmpDirStr = lio_utils_str_fmt("super%ccali%cfragi%clistic%cexpi%cali%cdocious", LIO_PATH_SEP, LIO_PATH_SEP, LIO_PATH_SEP, LIO_PATH_SEP, LIO_PATH_SEP, LIO_PATH_SEP);
    if (!tmpDirStr)
    {
        fprintf(stderr, "Unable to create a new path string.\n");
        ret = testId;
        goto end;
    }
    else
    {
        printf("Sub-directories to be created:\n\t%s\n", tmpDirStr);
    }

    // Test that path strings can be concatenated
    ++testId;
    newSubDir = lio_path_join(pCwd, tmpDirStr);
    if (!newSubDir)
    {
        fprintf(stderr, "Unable to join the file paths \"%s\" and \"%s.\"\n", pCwd, tmpDirStr);
        ret = testId;
        goto end;
    }
    else
    {
        printf("Successfully joined subdirectory string:\n\t%s\n", newSubDir);
    }

    // Test that paths can be recursively created
    ++testId;
    if (!lio_path_mkdirs(newSubDir))
    {
        fprintf(stderr, "Unable to create the directory tree \"%s.\"", newSubDir);
        ret = testId;
        goto end;
    }
    else
    {
        printf("Successfully created the directory tree:\n\t%s\n", newSubDir);
    }

    // Test that paths can be identified on the local file system
    ++testId;
    if (!lio_path_does_exist(newSubDir, LIO_PATH_TYPE_FOLDER))
    {
        fprintf(stderr, "Unable to validate the directory tree \"%s.\"\n", newSubDir);
        ret = testId;
        goto end;
    }
    else
    {
        printf("Successfully validated the directory tree:\n\t%s\n", newSubDir);
    }

    // Test that paths can be moved
    ++testId;
    superDir = lio_path_join(pCwd, "super");
    duperDir = lio_path_join(pCwd, "duper");
    if (!superDir || !duperDir || lio_path_move(superDir, duperDir, false) != 0)
    {
        fprintf(stderr, "Unable to move a directory: \"%s%c%s\" -> \"%s%c%s\"\n", pCwd, LIO_PATH_SEP, "super", pCwd, LIO_PATH_SEP, "duper");
        ret = testId;
        goto end;
    }
    else
    {
        printf("Successfully moved a directory:\n\t\"%s%c%s\" -> \"%s%c%s\"\n", pCwd, LIO_PATH_SEP, "super", pCwd, LIO_PATH_SEP, "duper");
    }

    // Test that paths can be removed from the local file system
    ++testId;
    if (!lio_path_remove(duperDir, true, false))
    {
        fprintf(stderr, "Unable to remove the directory tree \"%s.\"\n", duperDir);
        ret = testId;
        goto end;
    }
    else
    {
        printf("Successfully removed a directory tree:\n\t%s\n", duperDir);
    }

    end:
    lio_utils_str_destroy(tmpDirStr);
    lio_path_destroy(duperDir);
    lio_path_destroy(superDir);
    lio_path_destroy(newSubDir);
    lio_paths_destroy(pSiblings, numChildPaths);
    lio_path_destroy(pExc);
    lio_path_destroy(pCwd);

    return ret;
}
