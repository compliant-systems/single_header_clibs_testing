
#include "nanolib.h"
#include <assert.h>

// we define our aligned alloca/free to be used

#define ASSETSYS_IMPLEMENTATION
#define ASSETSYS_MALLOC(ctx, size) (dbj::aligned_malloc(size))
#define ASSETSYS_FREE(ctx, ptr) (dbj::aligned_free(ptr))
#define ASSETSYS_ASSERT(condition) (DBJ_ASSERT(condition))

// asset sys include strpool 

#define STRPOOL_IMPLEMENTATION
#define STRPOOL_MALLOC(ctx, size) (dbj::aligned_malloc(size))
#define STRPOOL_FREE(ctx, ptr) (dbj::aligned_free(ptr))
#define STRPOOL_ASSERT(condition) (DBJ_ASSERT(condition))

namespace dbj
{
// libs is git submodule here
#include "libs/assetsys.h"
} // namespace dbj

/*
-------------------------------------------------------------------------
*/
#define ASSYS_CALL(x)                                               \
    do                                                              \
    {                                                               \
        dbj::assetsys_error_t ass_err_ = (x);                       \
        if (ass_err_ != dbj::assetsys_error_t::ASSETSYS_SUCCESS)    \
        {                                                           \
            DBJ_TERROR(#x " has failed, with code: %d", ass_err_); \
        }                                                           \
    } while (0)

/*
-------------------------------------------------------------------------
*/
void list_assets(
    dbj::assetsys_t *assetsys, char const *path, int indent)
{
    const int subdir_count = dbj::assetsys_subdir_count(assetsys, path);
    // Print folder names and recursively list assets
    for (int i = 0; i < subdir_count; ++i)
    {
        char const *subdir_name = dbj::assetsys_subdir_name(assetsys, path, i);
        for (int j = 0; j < indent; ++j)
            printf("  ");
        printf("%s/\n", subdir_name);

        char const *subdir_path = dbj::assetsys_subdir_path(assetsys, path, i);
        list_assets(assetsys, subdir_path, indent + 1);
    }

    const int file_count = dbj::assetsys_file_count(assetsys, path);
    // Print file names
    for (int i = 0; i < file_count; ++i)
    {
        char const *file_name = dbj::assetsys_file_name(assetsys, path, i);
        for (int j = 0; j < indent; ++j)
            printf("  ");
        printf("%s\n", file_name);
    }
}
/*
-------------------------------------------------------------------------
*/
int main(int argc, char **argv)
{
    printf("\n\n%s , version [%s]", argv[0], __TIMESTAMP__);
    // no file dump requested
    if (argc < 2)
    {
        printf("\n\n%s usage:", argv[0]);
        printf("\n\nFirst mandatory arg is folder you want listed");
        printf("\nSecond optional arg is file from that folder you want dumped\n\n");
        exit(EXIT_SUCCESS);
    }

    dbj::assetsys_t *assetsys = dbj::assetsys_create(0);
    dbj::on_leaving leaver_{[&]() { dbj::assetsys_destroy(assetsys); }};

    // Try to mount required folder as a virtual "/data" path
    ASSYS_CALL(dbj::assetsys_mount(assetsys, argv[1], "/data"));

    printf("\n\n%s is mounted as: %s\n\n", argv[1], "/data");

    // Print all files and subfolders
    list_assets(assetsys, "/", 0); // Start at root

    // no file dump requested
    if (argc > 2)
    {
        printf("\n\n------------------------------------------------------------------\n"
               "File requested to be dumped: %s\n\n",
               argv[2]);

        // Load a file
        dbj::assetsys_file_t file;
        ASSYS_CALL(dbj::assetsys_file(assetsys, "/data/twitter.json", &file));
        int size = dbj::assetsys_file_size(assetsys, file);

        // app exits on alloc failure
        // extra space for '\0'
        char *content = dbj::aligned_malloc_char(size + 1);
        dbj::on_leaving liberate_{[&]() { dbj::aligned_free_char(content); }};

        int loaded_size = 0;
        ASSYS_CALL(dbj::assetsys_file_load(assetsys, file, &loaded_size, content, size));
        content[size] = '\0'; // zero terminate the text file
        printf("%s\n", content);
    }
    printf("\n\n");
    exit(EXIT_SUCCESS);
}