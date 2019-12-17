
#ifndef LIGHT_IO_CONFIG_H
#define LIGHT_IO_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif



/*
 * Architecture Support
 */
#if defined(__x86_64__) \
 || defined(__ppc64__) \
 || defined(__amd64__) \
 || defined(_M_X64) \
 || defined(_M_IA64) \
 || defined(__ia64__) \
 || defined(__aarch64__)
    #define LIGHT_IO_SYS_ARCH 64
#else
    #define LIGHT_IO_SYS_ARCH 32
#endif



/**
 * @brief Shared library setup
 */
#if defined(LIGHT_IO_BUILD_SHARED)
    #if defined(__GNUC__)
        #define LIO_API __attribute__((__visibility__("default")))
    #elif defined(_MSC_VER)
        #define LIO_API __declspec(dllexport)
    #endif
#elif defined(LIGHT_IO_API_SHARED)
    #if defined(_MSC_VER)
        #define LIO_API __declspec(dllimport)
    #else
        #define LIO_API
    #endif
#else
    #define LIO_API
#endif



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LIGHT_IO_CONFIG_H */
