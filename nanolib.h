#ifndef _DBJ_NANOLIB_INC_
#define _DBJ_NANOLIB_INC_

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#ifndef _DBJ_LOG
#define _DBJ_LOG(...)                                                            \
    do                                                                           \
    {                                                                            \
        fprintf(stderr, "\n[%s] %s(%d) -- ", __TIMESTAMP__, __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__);                                            \
    } while (false)

#endif // _DBJ_LOG

#ifndef _TERROR
#define _DBJ_TERROR(...)       \
    do                         \
    {                          \
        _DBJ_LOG(__VA_ARGS__); \
        exit(EXIT_FAILURE);    \
    } while (0)
#endif // _DBJ_TERROR

#ifndef DBJ_ASSERT

#define DBJ_ASSERT(x) \
    if (!(x))         \
    _DBJ_TERROR(#x " -- FAILED !")

#endif // DBJ_ASSERT

/*
---------------------------------------------------------------------------------
*/

#define dbj_FAST_FAIL_POLICY

// stolen from simdbjson
namespace dbj
{
// portable version of  posix_memalign
inline void *aligned_malloc(size_t size)
{
    const size_t alignment = 32; // CAUTION! MAGICAL CONSTANT

    // always initialize on declaration
    void *p{};
#ifdef _MSC_VER
    p = _aligned_malloc(size, alignment);
#elif defined(__MINGW32__) || defined(__MINGW64__)
    p = __mingw_aligned_malloc(size, alignment);
#else
    // somehow, if this is used before including "x86intrin.h", it creates an
    // implicit defined warning.
    if (posix_memalign(&p, alignment, size) != 0)
    {
        return nullptr;
    }
#endif

#ifdef dbj_FAST_FAIL_POLICY
    if (p == nullptr)
    {
        _DBJ_TERROR("aligned_malloc() failed! ");
    }
#endif
    return p;
}

inline char *aligned_malloc_char(size_t alignment, size_t size)
{
    return (char *)aligned_malloc(size);
}

inline void aligned_free(void *mem_block)
{
    if (mem_block == nullptr)
    {
        return;
    }
#ifdef _MSC_VER
    _aligned_free(mem_block);
#elif defined(__MINGW32__) || defined(__MINGW64__)
    __mingw_aligned_free(mem_block);
#else
    free(mem_block);
#endif
}

inline void aligned_free_char(char *mem_block)
{
    aligned_free((void *)mem_block);
}

} // namespace dbj

#endif // _DBJ_NANOLIB_INC_