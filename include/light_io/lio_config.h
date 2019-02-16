
#ifndef VTK_UTILS_CONFIG_H
#define VTK_UTILS_CONFIG_H

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
    #define VTK_UTILS_SYS_ARCH 64
#else
    #define VTK_UTILS_SYS_ARCH 32
#endif



/**
 * @brief Shared library setup
 */
#if defined(VTK_UTILS_BUILD_SHARED)
    #define VTK_API __attribute__((__visibility__("default")))
#else
    #define VTK_API
#endif



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* VTK_UTILS_CONFIG_H */
