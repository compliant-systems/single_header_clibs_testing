#ifndef DBJ_NANOLIB_INC_
#define DBJ_NANOLIB_INC_

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#ifndef DBJ_LOG
#define DBJ_LOG(...)                                                            \
    {                                                                            \
        fprintf(stderr, "\n[%s] %s(%d) -- ", __TIMESTAMP__, __FILE__, __LINE__); \
        fprintf(stderr, __VA_ARGS__);                                            \
    }

#endif // DBJ_LOG

#ifndef DBJ_TERROR
inline auto DBJ_TERROR = [](auto... args_) {
    DBJ_LOG(args_...);
    exit(EXIT_FAILURE);
};
#endif // DBJ_TERROR

#ifndef DBJ_ASSERT

#define DBJ_ASSERT(x_) (void)((!!(x_)) || (DBJ_TERROR((#x_)), 0))

#endif // DBJ_ASSERT

/*
---------------------------------------------------------------------------------
*/

#define dbj_FAST_FAIL_POLICY

namespace dbj
{
using void_void_function_ptr = void (*)(void);
// yes I am aware of: https://ricab.github.io/scope_guard/
// but I do not see the point ;)
template <typename Function_PTR = void_void_function_ptr>
struct on_leaving final
{
    static Function_PTR null_call() {}
    // much faster + cleaner vs giving nullptr
    // no if() in destructor required
    const Function_PTR callable_{null_call};

    explicit on_leaving(Function_PTR fun_) noexcept : callable_(fun_) {}

    ~on_leaving()
    {
        // no if in destructor required
        callable_();
    }
};

// stolen from simdbjson
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
    DBJ_ASSERT(p != nullptr);
#endif
    return p;
}

inline char *aligned_malloc_char(size_t size)
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

#endif // DBJ_NANOLIB_INC_