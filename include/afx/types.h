#pragma once

#ifdef __cplusplus
#define export extern "C"
#else
#define export
#endif

#define bit64 INTPTR_MAX == INT64_MAX
#define bit32 INTPTR_MAX == INT32_MAX

typedef signed char    s8;
typedef unsigned char  u8;
typedef signed short   s16;
typedef unsigned short u16;
typedef signed int     s32;
typedef unsigned int   u32;
typedef signed long    s64;
typedef unsigned long  u64;

#if bit64
typedef s64 ssz;
typedef u64 usz;
#else
typedef s32 ssz;
typedef u32 usz;
#endif

typedef usz addr_t;
typedef usz off_t;

typedef u32 addr32;
typedef u32 off32;
typedef u64 addr64;
typedef u64 off64;