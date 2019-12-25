
#include "nanolib.h"

#define ASSETSYS_IMPLEMENTATION
#define ASSETSYS_MALLOC(ctx, size) (dbj::aligned_malloc(size))
#define ASSETSYS_FREE(ctx, ptr) (dbj::aligned_free(ptr))
// #define ASSETSYS_ASSERT(condition) (DBJ_ASSERT(condition))

#define STRPOOL_IMPLEMENTATION
#define STRPOOL_MALLOC(ctx, size) (dbj::aligned_malloc(size))
#define STRPOOL_FREE(ctx, ptr) (dbj::aligned_free(ptr))
// #define STRPOOL_ASSERT(condition) (DBJ_ASSERT(condition))

#include "../single_header_clibs/assetsys.h"

#define ASSYS_CALL(x)                                               \
    do                                                              \
    {                                                               \
        assetsys_error_t ass_err_ = (x);                            \
        if (ass_err_ != assetsys_error_t::ASSETSYS_SUCCESS)         \
        {                                                           \
            _DBJ_TERROR(#x " has failed, with code: %d", ass_err_); \
        }                                                           \
    } while (0)

void list_assets(assetsys_t *assetsys, char const *path, int indent)
{
    const int subdir_count = assetsys_subdir_count(assetsys, path);
    // Print folder names and recursively list assets
    for (int i = 0; i < subdir_count; ++i)
    {
        char const *subdir_name = assetsys_subdir_name(assetsys, path, i);
        for (int j = 0; j < indent; ++j)
            printf("  ");
        printf("%s/\n", subdir_name);

        char const *subdir_path = assetsys_subdir_path(assetsys, path, i);
        list_assets(assetsys, subdir_path, indent + 1);
    }

    const int file_count = assetsys_file_count(assetsys, path);
    // Print file names
    for (int i = 0; i < file_count; ++i)
    {
        char const *file_name = assetsys_file_name(assetsys, path, i);
        for (int j = 0; j < indent; ++j)
            printf("  ");
        printf("%s\n", file_name);
    }
}

int main(int, char **)
{
    assetsys_t *assetsys = assetsys_create(0);
    dbj::on_leaving leaver_{[&]() { assetsys_destroy(assetsys); }};

    // Mount ./json_samples folder as a virtual "/data" path
    ASSYS_CALL(assetsys_mount(assetsys, "./json_samples", "/data"));

    // Print all files and subfolders
    list_assets(assetsys, "/", 0); // Start at root

    // Load a file
    assetsys_file_t file;
    ASSYS_CALL(assetsys_file(assetsys, "/data/twitter.json", &file));
    int size = assetsys_file_size(assetsys, file);

    // app exits on alloc failure
    // extra space for '\0'
    char *content = dbj::aligned_malloc_char(size + 1);
    dbj::on_leaving liberate_{[&]() { dbj::aligned_free_char(content); }};

    int loaded_size = 0;
    ASSYS_CALL(assetsys_file_load(assetsys, file, &loaded_size, content, size));
    content[size] = '\0'; // zero terminate the text file
    printf("%s\n", content);
}