
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
    char* newPath = NULL;
    char* tmpDir = NULL;

    // Test that files can be found on the local file system
    ++testId;
    if (!path_does_exist(argv[0], PATH_TYPE_FILE))
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
    pCwd = path_dirname(argv[0]);
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
    pExc = path_basename(argv[0]);
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
    numChildPaths = path_count_entries(pCwd, false, NULL);
    printf("File Listing of %u items:", numChildPaths);
    pSiblings = path_list(pCwd, false, NULL, &numChildPaths);
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
    newPath = utils_str_fmt("super%ccali%cfragi%clistic%cexpi%cali%cdocious", PATH_SEPARATOR, PATH_SEPARATOR, PATH_SEPARATOR, PATH_SEPARATOR, PATH_SEPARATOR, PATH_SEPARATOR);
    if (!newPath)
    {
        fprintf(stderr, "Unable to create a new path string.\n");
        ret = testId;
        goto end;
    }
    else
    {
        printf("Sub-directories to be created:\n\t%s\n", newPath);
    }

    // Test that path strings can be concatenated
    ++testId;
    tmpDir = path_join(pCwd, newPath);
    if (!tmpDir)
    {
        fprintf(stderr, "Unable to join the file paths \"%s\" and \"%s.\"\n", pCwd, newPath);
        ret = testId;
        goto end;
    }
    else
    {
        printf("Successfully created subdirectories:\n\t%s\n", tmpDir);
    }

    // Test that paths can be recursively created
    ++testId;
    if (!path_mkdirs(tmpDir))
    {
        fprintf(stderr, "Unable to create the directory tree \"%s.\"", tmpDir);
        ret = testId;
        goto end;
    }
    else
    {
        printf("Successfully created the directory tree:\n\t%s\n", tmpDir);
    }

    // Test that paths can be identified on the local file system
    ++testId;
    if (!path_does_exist(tmpDir, PATH_TYPE_FOLDER))
    {
        fprintf(stderr, "Unable to validate the directory tree \"%s.\"\n", tmpDir);
        ret = testId;
        goto end;
    }
    else
    {
        printf("Successfully validated the directory tree:\n\t%s\n", tmpDir);
    }

    // Test that paths can be removed from the local file system
    ++testId;
    if (!path_remove("super", true, false))
    {
        fprintf(stderr, "Unable to remove the directory tree \"%s.\"\n", tmpDir);
        ret = testId;
        goto end;
    }
    else
    {
        printf("Successfully removed a directory tree:\n\t%s\n", tmpDir);
    }

    end:
    utils_str_destroy(newPath);
    path_destroy(tmpDir);
    paths_destroy(pSiblings, numChildPaths);
    path_destroy(pExc);
    path_destroy(pCwd);

    return ret;
}
