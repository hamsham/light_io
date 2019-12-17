
#ifndef LIGHT_IO_C_UTILS_H
#define LIGHT_IO_C_UTILS_H

#include <stddef.h> // size_t
#include <float.h> // FLT_MIN, FLT_MAX
#include <limits.h> // ..._MIN, ..._MAX macros
#include <stdint.h> // fixed-width integers

#ifdef __cplusplus
extern "C" {
#endif



/**
 * Get the length of static arrays
 */
#ifndef LIO_UTILS_ARRAY_LENGTH
    #define LIO_UTILS_ARRAY_LENGTH( x ) (sizeof(x) / sizeof(x[0]))
#endif /* LIO_UTILS_ARRAY_LENGTH */



#ifndef LIO_UTILS_STRINGIFY
    #define LIO_UTILS_STRINGIFY( x ) (#x)
#endif /* LIO_UTILS_STRINGIFY */



#ifndef LIO_UTILS_CONCAT
    #define LIO_UTILS_CONCAT( x, y ) x##y
#endif /* LIO_UTILS_CONCAT */



#ifndef LIO_UTILS_MIN
    #define LIO_UTILS_MIN( a, b ) ((a) <= (b) ? (a) : (b))
#endif /* LIO_UTILS_MIN */



#ifndef LIO_UTILS_MAX
    #define LIO_UTILS_MAX( a, b ) ((a) >= (b) ? (a) : (b))
#endif /* LIO_UTILS_MAX */



static const uint32_t LIO_UTILS_LITTLE_ENDIAN  = 0x00000001;
static const uint32_t LIO_UTILS_BIG_ENDIAN     = 0x01000000;
static const uint32_t LIO_UTILS_PDP_ENDIAN     = 0x00010000;
static const uint32_t LIO_UTILS_UNKNOWN_ENDIAN = 0xFFFFFFFF;



/**
 * A simple function that can be used to help determine a program's endianness
 * at compile-time.
 */
static inline uint32_t lio_utils_get_endian_order()
{
    if ((0xFFFFFFFF & 1) == LIO_UTILS_LITTLE_ENDIAN)
    {
        return LIO_UTILS_LITTLE_ENDIAN;
    }
    else if ((0xFFFFFFFF & 1) == LIO_UTILS_BIG_ENDIAN)
    {
        return LIO_UTILS_BIG_ENDIAN;
    }
    else if ((0xFFFFFFFF & 1) == LIO_UTILS_PDP_ENDIAN)
    {
        return LIO_UTILS_PDP_ENDIAN;
    }

    return LIO_UTILS_UNKNOWN_ENDIAN;
}



/**
 * Swap the bytes of an unsigned 16-bit integral type between big and little
 * endian representation.
 *
 * @param n
 * an unsigned integral type.
 *
 * @return uint16_t
 * The value of the input parameter with its bytes swapped between big & little
 * endian representation.
 */
static inline uint16_t lio_utils_btol_u16(uint16_t n)
{
    return (n >> 8) ^ (n << 8);
}



/**
 * Swap the bytes of a signed 16-bit integral type between big and little
 * endian representation.
 *
 * @param n
 * a signed integral type.
 *
 * @return int16_t
 * The value of the input parameter with its bytes swapped between big & little
 * endian representation.
 */
static inline int16_t lio_utils_btol_s16(int16_t n)
{
    return (int16_t) lio_utils_btol_u16((uint16_t) n);
}



/**
 * Swap the bytes of an unsigned 32-bit integral type between big and little
 * endian representation.
 *
 * @param n
 * an unsigned integral type.
 *
 * @return uint32_t
 * The value of the input parameter with its bytes swapped between big & little
 * endian representation.
 */
static inline uint32_t lio_utils_btol_u32(uint32_t n)
{
    return
        (0x000000FF & (n >> 24)) ^
        (0x0000FF00 & (n >> 8)) ^
        (0x00FF0000 & (n << 8)) ^
        (0xFF000000 & (n << 24));
}



/**
 * Swap the bytes of a signed 32-bit integral type between big and little
 * endian representation.
 *
 * @param n
 * a signed integral type.
 *
 * @return int32_t
 * The value of the input parameter with its bytes swapped between big & little
 * endian representation.
 */
static inline int32_t lio_utils_btol_s32(int32_t n)
{
    return (int32_t) lio_utils_btol_u32((uint32_t) n);
}



/**
 * Swap the bytes of an unsigned 64-bit integral type between big and little
 * endian representation.
 *
 * @param n
 * an unsigned integral type.
 *
 * @return uint64_t
 * The value of the input parameter with its bytes swapped between big & little
 * endian representation.
 */
static inline uint64_t lio_utils_btol_u64(uint64_t n)
{
    return
        (0x00000000000000FF & (n >> 56)) ^
        (0x000000000000FF00 & (n >> 40)) ^
        (0x0000000000FF0000 & (n >> 24)) ^
        (0x00000000FF000000 & (n >> 8)) ^
        (0x000000FF00000000 & (n << 8)) ^
        (0x0000FF0000000000 & (n << 24)) ^
        (0x00FF000000000000 & (n << 40)) ^
        (0xFF00000000000000 & (n << 56));
}



/**
 * Swap the bytes of a signed 64-bit integral type between big and little
 * endian representation.
 *
 * @param n
 * a signed integral type.
 *
 * @return int64_t
 * The value of the input parameter with its bytes swapped between big & little
 * endian representation.
 */
static inline int64_t lio_utils_btol_s64(int64_t n)
{
    return (int64_t) lio_utils_btol_u64((uint64_t) n);
}



/**
 * Swap the bytes of a 32-bit floating-point type between big and little
 * endian representation.
 *
 * @param f
 * A 32-bit float
 *
 * @return float
 * The value of the input parameter with its bytes swapped between big & little
 * endian representation.
 */
static inline float lio_utils_btol_f(float f)
{
    float ret;
    char* pF = (char*) & f;
    char* pR = (char*) & ret;

    pR[0] = pF[3];
    pR[1] = pF[2];
    pR[2] = pF[1];
    pR[3] = pF[0];

    return ret;
}



/**
 * Swap the bytes of a 64-bit floating-point type between big and little
 * endian representation.
 *
 * @param f
 * A 64-bit double
 *
 * @return double
 * The value of the input parameter with its bytes swapped between big & little
 * endian representation.
 */
static inline double lio_utils_btol_d(double d)
{
    float ret;
    char* pF = (char*) & d;
    char* pR = (char*) & ret;

    pR[0] = pF[3];
    pR[1] = pF[2];
    pR[2] = pF[1];
    pR[3] = pF[0];

    return ret;
}



char* lio_utils_str_fmt(const char* fmt, ...);



char* lio_utils_str_copy(const char* str, const size_t maxChars);



char* lio_utils_str_concat(const char* const str1, const char* const str2);



void lio_utils_str_destroy(char* const pStr);



#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LIGHT_IO_C_UTILS_H */
