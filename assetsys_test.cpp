
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

#include <stdio.h> // for printf

void list_assets(assetsys_t *assetsys, char const *path, int indent)
{
    // Print folder names and recursively list assets
    for (int i = 0; i < assetsys_subdir_count(assetsys, path); ++i)
    {
        char const *subdir_name = assetsys_subdir_name(assetsys, path, i);
        for (int j = 0; j < indent; ++j)
            printf("  ");
        printf("%s/\n", subdir_name);

        char const *subdir_path = assetsys_subdir_path(assetsys, path, i);
        list_assets(assetsys, subdir_path, indent + 1);
    }

    // Print file names
    for (int i = 0; i < assetsys_file_count(assetsys, path); ++i)
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
    // Mount current working folder as a virtual "/data" path
    //    assetsys_mount(assetsys, ".", "/data");
    // or
    assetsys_error_t ass_err_ = assetsys_mount(assetsys, "./json_samples", "/data");

    if (ass_err_ != assetsys_error_t::ASSETSYS_SUCCESS)
    {
        _DBJ_TERROR("assetsys_mount() has failed, with code: %d", ass_err_);
    }

    // Print all files and subfolders
    list_assets(assetsys, "/", 0); // Start at root

    // Load a file
    assetsys_file_t file;
    assetsys_file(assetsys, "/data/twitter.json", &file);
    int size = assetsys_file_size(assetsys, file);

    // char *content = (char *)malloc(size + 1); // extra space for '\0'
    char *content = dbj::aligned_malloc_char(32, size + 1);

    int loaded_size = 0;
    assetsys_file_load(assetsys, file, &loaded_size, content, size);
    content[size] = '\0'; // zero terminate the text file
    printf("%s\n", content);
    free(content);

    assetsys_destroy(assetsys);
}