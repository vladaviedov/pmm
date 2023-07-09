#pragma once

#include <stdint.h>

// uint128_t
#ifdef __GNUC__
typedef unsigned __int128 uint128_t;
#endif // __GNUC__

#define PAGE_SIZE 4096
#define INVALID_VAL UINT32_MAX
#define INVALID_EXT UINT64_MAX

// Pkg table info
#define PKG_TABLE "pkg.pmm"
#define PKG_TABLE_VER 1

// Btree Entry Types
typedef uint32_t page_t;
#ifdef uint128_t
typedef uint128_t md5_t;
#else
typedef uint8_t md5_t[16];
#endif // uint128_t

// External page pointer/locator
typedef struct {
	uint64_t ptr;
	uint64_t len;
} ext_t;

// MD5 Comparison Macros
#ifdef uint128_t
#define md5_eq(val, ref) (val == ref)
#define md5_ls(val, ref) (val < ref)
#define md5_gr(val, ref) (val > ref)
#define md5_cp(dest, src) *dest = *src
#define md5_zero(ptr) *ptr = 0
#else
#include <string.h>
#define md5_eq(val, ref) (memcmp(val, ref, 16) == 0)
#define md5_ls(val, ref) (memcmp(val, ref, 16) < 0)
#define md5_gr(val, ref) (memcmp(val, ref, 16) > 0)
#define md5_cp(dest, src) memcpy(dest, src, 16)
#define md5_zero(ptr) memset(ptr, 0, 16)
#endif // uint128_t
