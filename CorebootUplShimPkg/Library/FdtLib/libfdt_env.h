/** @file
*
*  Copyright (c) 2011-2014, ARM Limited. All rights reserved.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#ifndef _LIBFDT_ENV_H
#define _LIBFDT_ENV_H

#include <Base.h>
#include <BaseLib.h>

typedef UINT16  fdt16_t;
typedef UINT32  fdt32_t;
typedef UINT64  fdt64_t;

typedef BOOLEAN bool;
typedef UINT8   uint8_t;
typedef UINT16  uint16_t;
typedef UINT32  uint32_t;
typedef UINT64  uint64_t;
typedef UINTN   uintptr_t;
typedef UINTN   size_t;

typedef INT8    int8_t;
typedef INT16   int16_t;
typedef INT32   int32_t;
typedef INT64   int64_t;

#define true        ((unsigned char)(1==1))
#define false       ((unsigned char)(0==1))
#define UINT32_MAX  ((uint32_t)0xFFFFFFFF)
#define INT_MAX     ((int32_t)0x7FFFFFFF)
#define INT32_MAX   ((int32_t)0x7FFFFFFF)

extern unsigned long strtoul (
  const char  *nptr,
  char        **endptr,
  int         base
  );

static inline uint16_t
SwapBytes16 (
  UINT16                    Value
  )
{
  return (UINT16) ((Value<< 8) | (Value>> 8));
}

static inline uint16_t
fdt16_to_cpu (
  fdt16_t  x
  )
{
  return SwapBytes16 (x);
}

static inline uint32_t
SwapBytes32 (
  UINT32                    Value
  )
{
  UINT32  LowerBytes;
  UINT32  HigherBytes;

  LowerBytes  = (UINT32) SwapBytes16 ((UINT16) Value);
  HigherBytes = (UINT32) SwapBytes16 ((UINT16) (Value >> 16));
  return (LowerBytes << 16 | HigherBytes);
}

static inline uint64_t
SwapBytes64 (
  UINT64                    Value
  )
{
  UINT64  LowerBytes;
  UINT64  HigherBytes;

  LowerBytes  = (UINT64) SwapBytes32 ((UINT32) Value);
  HigherBytes = (UINT64) SwapBytes32 ((UINT32) (Value >> 32));

  return (LowerBytes << 32 | HigherBytes);
}

#define cpu_to_fdt16(x)  fdt16_to_cpu(x)

static inline uint32_t
fdt32_to_cpu (
  fdt32_t  x
  )
{
  return SwapBytes32 (x);
}

#define cpu_to_fdt32(x)  fdt32_to_cpu(x)

static inline uint64_t
fdt64_to_cpu (
  fdt64_t  x
  )
{
  return SwapBytes64 (x);
}

#define cpu_to_fdt64(x)  fdt64_to_cpu(x)

static inline void *
memcpy (
  void        *dest,
  const void  *src,
  size_t      len
  )
{
  return CopyMem (dest, src, len);
}

static inline void *
memmove (
  void        *dest,
  const void  *src,
  size_t      n
  )
{
  return CopyMem (dest, src, n);
}

static inline void *
memset (
  void    *s,
  int     c,
  size_t  n
  )
{
  return SetMem (s, n, c);
}

static inline int
memcmp (
  const void  *dest,
  const void  *src,
  int         len
  )
{
  return CompareMem (dest, src, len);
}

static inline void *
memchr (
  const void  *s,
  int         c,
  size_t      n
  )
{
  return ScanMem8 (s, n, c);
}

static inline size_t
strlen (
  const char  *str
  )
{
  return AsciiStrLen (str);
}

static inline char *
strchr (
  const char  *s,
  int         c
  )
{
  char  pattern[2];

  pattern[0] = c;
  pattern[1] = 0;
  return AsciiStrStr (s, pattern);
}

static inline char *
strrchr (
  const char  *s,
  int         c
  )
{
  return AsciiStrrStr (s, c);
}

static inline size_t
strnlen (
  const char  *str,
  size_t      strsz
  )
{
  return AsciiStrnLenS (str, strsz);
}

static inline size_t
strcmp (
  const char  *str1,
  const char  *str2
  )
{
  return AsciiStrCmp (str1, str2);
}

static inline size_t
strncmp (
  const char  *str1,
  const char  *str2,
  size_t      strsz
  )
{
  return AsciiStrnCmp (str1, str2, strsz);
}

static inline size_t
strncpy (
  char        *dest,
  const char  *source,
  size_t      dest_max
  )
{
  return AsciiStrCpyS (dest, dest_max, source);
}

#endif /* _LIBFDT_ENV_H */
