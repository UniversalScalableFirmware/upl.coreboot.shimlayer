/** @file
  Base Memory Library.

  Copyright (c) 2022, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __BASE_LIB__
#define __BASE_LIB__

#include <Base.h>

///
/// Maximum legal AARCH64  address
///
#define MAX_ADDRESS   0xFFFFFFFFFFFFFFFFULL

///
/// Maximum legal AARCH64  address
///
#define MAX_ASCII_STRING_LENGTH  1000000

#define SAFE_STRING_CONSTRAINT_CHECK(Expression, Status)  \
  do { \
    if (!(Expression)) { \
      return Status; \
    } \
  } while (FALSE)

/**
  Copies a source buffer to a destination buffer, and returns the destination buffer.

  This function copies Length bytes from SourceBuffer to DestinationBuffer, and returns
  DestinationBuffer.  The implementation must be reentrant, and it must handle the case
  where SourceBuffer overlaps DestinationBuffer.

  If Length is greater than (MAX_ADDRESS - DestinationBuffer + 1), then ASSERT().
  If Length is greater than (MAX_ADDRESS - SourceBuffer + 1), then ASSERT().

  @param  DestinationBuffer   The pointer to the destination buffer of the memory copy.
  @param  SourceBuffer        The pointer to the source buffer of the memory copy.
  @param  Length              The number of bytes to copy from SourceBuffer to DestinationBuffer.

  @return DestinationBuffer.

**/
VOID *
CopyMem (
  OUT VOID       *DestinationBuffer,
  IN CONST VOID  *SourceBuffer,
  IN UINTN       Length
  );

/**
  Fills a target buffer with zeros, and returns the target buffer.

  This function fills Length bytes of Buffer with zeros, and returns Buffer.

  If Length > 0 and Buffer is NULL, then ASSERT().
  If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().

  @param  Buffer      The pointer to the target buffer to fill with zeros.
  @param  Length      The number of bytes in Buffer to fill with zeros.

  @return Buffer.

**/
VOID *
ZeroMem (
  OUT VOID  *Buffer,
  IN UINTN  Length
  );

/**
  Multiples a 64-bit unsigned integer by a 64-bit unsigned integer and
  generates a 64-bit unsigned result.

  This function multiples the 64-bit unsigned value Multiplicand by the 64-bit
  unsigned value Multiplier and generates a 64-bit unsigned result. This 64-
  bit unsigned result is returned.

  @param  Multiplicand  A 64-bit unsigned value.
  @param  Multiplier    A 64-bit unsigned value.

  @return Multiplicand * Multiplier.

**/
UINT64
MultU64x64 (
  IN      UINT64                    Multiplicand,
  IN      UINT64                    Multiplier
  );

/**
  Divides a 64-bit unsigned integer by a 64-bit unsigned integer and generates
  a 64-bit unsigned result and an optional 64-bit unsigned remainder.

  This function divides the 64-bit unsigned value Dividend by the 64-bit
  unsigned value Divisor and generates a 64-bit unsigned quotient. If Remainder
  is not NULL, then the 64-bit unsigned remainder is returned in Remainder.
  This function returns the 64-bit unsigned quotient.

  If Divisor is 0, then ASSERT().

  @param  Dividend  A 64-bit unsigned value.
  @param  Divisor   A 64-bit unsigned value.
  @param  Remainder A pointer to a 64-bit unsigned value. This parameter is
                    optional and may be NULL.

  @return Dividend / Divisor.

**/
UINT64
DivU64x64Remainder (
  IN      UINT64                    Dividend,
  IN      UINT64                    Divisor,
  OUT     UINT64                    *Remainder  OPTIONAL
  );

/**
  Shifts a 64-bit integer left between 0 and 63 bits. The low bits are filled
  with zeros. The shifted value is returned.

  This function shifts the 64-bit value Operand to the left by Count bits. The
  low Count bits are set to zero. The shifted value is returned.

  If Count is greater than 63, then ASSERT().

  @param  Operand The 64-bit operand to shift left.
  @param  Count   The number of bits to shift left.

  @return Operand << Count.

**/
UINT64
LShiftU64 (
  IN      UINT64  Operand,
  IN      UINTN   Count
  );

/**
  Retrieves CPUID information.

  Executes the CPUID instruction with EAX set to the value specified by Index.
  This function always returns Index.
  If Eax is not NULL, then the value of EAX after CPUID is returned in Eax.
  If Ebx is not NULL, then the value of EBX after CPUID is returned in Ebx.
  If Ecx is not NULL, then the value of ECX after CPUID is returned in Ecx.
  If Edx is not NULL, then the value of EDX after CPUID is returned in Edx.
  This function is only available on IA-32 and x64.

  @param  Index The 32-bit value to load into EAX prior to invoking the CPUID
                instruction.
  @param  Eax   The pointer to the 32-bit EAX value returned by the CPUID
                instruction. This is an optional parameter that may be NULL.
  @param  Ebx   The pointer to the 32-bit EBX value returned by the CPUID
                instruction. This is an optional parameter that may be NULL.
  @param  Ecx   The pointer to the 32-bit ECX value returned by the CPUID
                instruction. This is an optional parameter that may be NULL.
  @param  Edx   The pointer to the 32-bit EDX value returned by the CPUID
                instruction. This is an optional parameter that may be NULL.

  @return Index.

**/
UINT32

AsmCpuid (
  IN      UINT32                    Index,
  OUT     UINT32                    *Eax,  OPTIONAL
  OUT     UINT32                    *Ebx,  OPTIONAL
  OUT     UINT32                    *Ecx,  OPTIONAL
  OUT     UINT32                    *Edx   OPTIONAL
  );

GUID *
CopyGuid (
   GUID        *DestinationGuid,
   CONST GUID  *SourceGuid
  );

unsigned char
CompareGuid (
  IN CONST GUID     *Guid1,
  IN CONST GUID     *Guid2
  );

/**
  Returns the length of a Null-terminated Ascii string.

  This function is similar as strlen_s defined in C11.

  @param  String   A pointer to a Null-terminated Ascii string.
  @param  MaxSize  The maximum number of Destination Ascii
                   char, including terminating null char.

  @retval 0        If String is NULL.
  @retval MaxSize  If there is no null character in the first MaxSize characters of String.
  @return The number of characters that percede the terminating null character.

**/
UINTN
AsciiStrnLenS (
  const CHAR8  *String,
  UINTN        MaxSize
  );

/**
  Compares two Null-terminated ASCII strings, and returns the difference
  between the first mismatched ASCII characters.

  This function compares the Null-terminated ASCII string FirstString to the
  Null-terminated ASCII string SecondString. If FirstString is identical to
  SecondString, then 0 is returned. Otherwise, the value returned is the first
  mismatched ASCII character in SecondString subtracted from the first
  mismatched ASCII character in FirstString.

  If FirstString is NULL, then ASSERT().
  If SecondString is NULL, then ASSERT().
  If PcdMaximumAsciiStringLength is not zero and FirstString contains more than
  PcdMaximumAsciiStringLength ASCII characters, not including the Null-terminator,
  then ASSERT().
  If PcdMaximumAsciiStringLength is not zero and SecondString contains more
  than PcdMaximumAsciiStringLength ASCII characters, not including the
  Null-terminator, then ASSERT().

  @param  FirstString   A pointer to a Null-terminated ASCII string.
  @param  SecondString  A pointer to a Null-terminated ASCII string.

  @retval ==0      FirstString is identical to SecondString.
  @retval !=0      FirstString is not identical to SecondString.

**/
INTN
AsciiStrCmp (
     const CHAR8  *FirstString,
     const CHAR8  *SecondString
  );

/**
  Compares two Null-terminated ASCII strings with maximum lengths, and returns
  the difference between the first mismatched ASCII characters.

  This function compares the Null-terminated ASCII string FirstString to the
  Null-terminated ASCII  string SecondString. At most, Length ASCII characters
  will be compared. If Length is 0, then 0 is returned. If FirstString is
  identical to SecondString, then 0 is returned. Otherwise, the value returned
  is the first mismatched ASCII character in SecondString subtracted from the
  first mismatched ASCII character in FirstString.

  If Length > 0 and FirstString is NULL, then ASSERT().
  If Length > 0 and SecondString is NULL, then ASSERT().
  If PcdMaximumAsciiStringLength is not zero, and Length is greater than
  PcdMaximumAsciiStringLength, then ASSERT().
  If PcdMaximumAsciiStringLength is not zero, and FirstString contains more than
  PcdMaximumAsciiStringLength ASCII characters, not including the Null-terminator,
  then ASSERT().
  If PcdMaximumAsciiStringLength is not zero, and SecondString contains more than
  PcdMaximumAsciiStringLength ASCII characters, not including the Null-terminator,
  then ASSERT().

  @param  FirstString   A pointer to a Null-terminated ASCII string.
  @param  SecondString  A pointer to a Null-terminated ASCII string.
  @param  Length        The maximum number of ASCII characters for compare.

  @retval ==0       FirstString is identical to SecondString.
  @retval !=0       FirstString is not identical to SecondString.

**/
INTN
AsciiStrnCmp (
  const CHAR8  *FirstString,
  const CHAR8  *SecondString,
  UINTN        Length
  );

/**
  Copies the string pointed to by Source (including the terminating null char)
  to the array pointed to by Destination.

  This function is similar as strcpy_s defined in C11.

  If an error is returned, then the Destination is unmodified.

  @param  Destination              A pointer to a Null-terminated Ascii string.
  @param  DestMax                  The maximum number of Destination Ascii
                                   char, including terminating null char.
  @param  Source                   A pointer to a Null-terminated Ascii string.

  @retval RETURN_SUCCESS           String is copied.
  @retval RETURN_UNSUPPORTED       If DestMax is NOT greater than StrLen(Source).
                                   If DestMax is greater than ASCII_RSIZE_MAX.
  @retval RETURN_NOT_FOUND         If Destination is NULL.
                                   If Source is NULL.
                                   If DestMax is 0.
**/
RETURN_STATUS
AsciiStrCpyS (
  CHAR8        *Destination,
  UINTN        DestMax,
  const CHAR8  *Source
  );

/**
  Fills a target buffer with a byte value, and returns the target buffer.
  This function fills Length bytes of Buffer with Value, and returns Buffer.
  If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().
  @param  Buffer    The memory to set.
  @param  Length    The number of bytes to set.
  @param  Value     The value with which to fill Length bytes of Buffer.
  @return Buffer.
**/
VOID *
SetMem (
  OUT VOID  *Buffer,
  IN UINTN  Length,
  IN UINT8  Value
  );

/**
  Compares the contents of two buffers.
  This function compares Length bytes of SourceBuffer to Length bytes of DestinationBuffer.
  If all Length bytes of the two buffers are identical, then 0 is returned.  Otherwise, the
  value returned is the first mismatched byte in SourceBuffer subtracted from the first
  mismatched byte in DestinationBuffer.
  If Length > 0 and DestinationBuffer is NULL, then ASSERT().
  If Length > 0 and SourceBuffer is NULL, then ASSERT().
  If Length is greater than (MAX_ADDRESS - DestinationBuffer + 1), then ASSERT().
  If Length is greater than (MAX_ADDRESS - SourceBuffer + 1), then ASSERT().
  @param  DestinationBuffer A pointer to the destination buffer to compare.
  @param  SourceBuffer      A pointer to the source buffer to compare.
  @param  Length            The number of bytes to compare.
  @return 0                 All Length bytes of the two buffers are identical.
  @retval Non-zero          The first mismatched byte in SourceBuffer subtracted from the first
                            mismatched byte in DestinationBuffer.
**/
INTN
CompareMem (
  IN CONST VOID  *DestinationBuffer,
  IN CONST VOID  *SourceBuffer,
  IN UINTN       Length
  );

/**
  Scans a target buffer for an 8-bit value, and returns a pointer to the matching 8-bit value
  in the target buffer.
  This function searches the target buffer specified by Buffer and Length from the lowest
  address to the highest address for an 8-bit value that matches Value.  If a match is found,
  then a pointer to the matching byte in the target buffer is returned.  If no match is found,
  then NULL is returned.  If Length is 0, then NULL is returned.
  If Length > 0 and Buffer is NULL, then ASSERT().
  If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().
  @param  Buffer      The pointer to the target buffer to scan.
  @param  Length      The number of bytes in Buffer to scan.
  @param  Value       The value to search for in the target buffer.
  @return A pointer to the matching byte in the target buffer, or NULL otherwise.
**/
VOID *
ScanMem8 (
  IN CONST VOID  *Buffer,
  IN UINTN       Length,
  IN UINT8       Value
  );

/**
  Returns the length of a Null-terminated ASCII string.
  This function returns the number of ASCII characters in the Null-terminated
  ASCII string specified by String.
  If Length > 0 and Destination is NULL, then ASSERT().
  If Length > 0 and Source is NULL, then ASSERT().
  If PcdMaximumAsciiStringLength is not zero and String contains more than
  PcdMaximumAsciiStringLength ASCII characters, not including the Null-terminator,
  then ASSERT().
  @param  String  A pointer to a Null-terminated ASCII string.
  @return The length of String.
**/
UINTN
AsciiStrLen (
  IN      CONST CHAR8  *String
  );

/**
  Returns the first occurrence of a Null-terminated ASCII sub-string
  in a Null-terminated ASCII string.
  This function scans the contents of the ASCII string specified by String
  and returns the first occurrence of SearchString. If SearchString is not
  found in String, then NULL is returned. If the length of SearchString is zero,
  then String is returned.
  If String is NULL, then ASSERT().
  If SearchString is NULL, then ASSERT().
  If PcdMaximumAsciiStringLength is not zero, and SearchString or
  String contains more than PcdMaximumAsciiStringLength Unicode characters
  not including the Null-terminator, then ASSERT().
  @param  String          A pointer to a Null-terminated ASCII string.
  @param  SearchString    A pointer to a Null-terminated ASCII string to search for.
  @retval NULL            If the SearchString does not appear in String.
  @retval others          If there is a match return the first occurrence of SearchingString.
                          If the length of SearchString is zero,return String.
**/
CHAR8 *
AsciiStrStr (
  IN      CONST CHAR8  *String,
  IN      CONST CHAR8  *SearchString
  );

/**
  Convert a Null-terminated Ascii decimal string to a value of type UINTN.
  This function outputs a value of type UINTN by interpreting the contents of
  the Ascii string specified by String as a decimal number. The format of the
  input Ascii string String is:
                  [spaces] [decimal digits].
  The valid decimal digit character is in the range [0-9]. The function will
  ignore the pad space, which includes spaces or tab characters, before
  [decimal digits]. The running zero in the beginning of [decimal digits] will
  be ignored. Then, the function stops at the first character that is a not a
  valid decimal character or a Null-terminator, whichever one comes first.
  If String has no valid decimal digits in the above format, then 0 is stored
  at the location pointed to by Data.
  If the number represented by String exceeds the range defined by UINTN, then
  MAX_UINTN is stored at the location pointed to by Data.
  If EndPointer is not NULL, a pointer to the character that stopped the scan
  is stored at the location pointed to by EndPointer. If String has no valid
  decimal digits right after the optional pad spaces, the value of String is
  stored at the location pointed to by EndPointer.
  @param  String                   Pointer to a Null-terminated Ascii string.
  @param  EndPointer               Pointer to character that stops scan.
  @param  Data                     Pointer to the converted value.
  @retval RETURN_SUCCESS           Value is translated from String.
  @retval RETURN_INVALID_PARAMETER If String is NULL.
                                   If Data is NULL.
                                   If PcdMaximumAsciiStringLength is not zero,
                                   and String contains more than
                                   PcdMaximumAsciiStringLength Ascii
                                   characters, not including the
                                   Null-terminator.
  @retval RETURN_UNSUPPORTED       If the number represented by String exceeds
                                   the range defined by UINTN.
**/
RETURN_STATUS
AsciiStrDecimalToUintnS (
  IN  CONST CHAR8  *String,
  OUT       CHAR8  **EndPointer   OPTIONAL,
  OUT       UINTN  *Data
  );

/**
  Convert a Null-terminated Ascii hexadecimal string to a value of type UINTN.
  This function outputs a value of type UINTN by interpreting the contents of
  the Ascii string specified by String as a hexadecimal number. The format of
  the input Ascii string String is:
                  [spaces][zeros][x][hexadecimal digits].
  The valid hexadecimal digit character is in the range [0-9], [a-f] and [A-F].
  The prefix "0x" is optional. Both "x" and "X" is allowed in "0x" prefix. If
  "x" appears in the input string, it must be prefixed with at least one 0. The
  function will ignore the pad space, which includes spaces or tab characters,
  before [zeros], [x] or [hexadecimal digits]. The running zero before [x] or
  [hexadecimal digits] will be ignored. Then, the decoding starts after [x] or
  the first valid hexadecimal digit. Then, the function stops at the first
  character that is a not a valid hexadecimal character or Null-terminator,
  whichever on comes first.
  If String has no valid hexadecimal digits in the above format, then 0 is
  stored at the location pointed to by Data.
  If the number represented by String exceeds the range defined by UINTN, then
  MAX_UINTN is stored at the location pointed to by Data.
  If EndPointer is not NULL, a pointer to the character that stopped the scan
  is stored at the location pointed to by EndPointer. If String has no valid
  hexadecimal digits right after the optional pad spaces, the value of String
  is stored at the location pointed to by EndPointer.
  @param  String                   Pointer to a Null-terminated Ascii string.
  @param  EndPointer               Pointer to character that stops scan.
  @param  Data                     Pointer to the converted value.
  @retval RETURN_SUCCESS           Value is translated from String.
  @retval RETURN_INVALID_PARAMETER If String is NULL.
                                   If Data is NULL.
                                   If PcdMaximumAsciiStringLength is not zero,
                                   and String contains more than
                                   PcdMaximumAsciiStringLength Ascii
                                   characters, not including the
                                   Null-terminator.
  @retval RETURN_UNSUPPORTED       If the number represented by String exceeds
                                   the range defined by UINTN.
**/
RETURN_STATUS
AsciiStrHexToUintnS (
  IN  CONST CHAR8  *String,
  OUT       CHAR8  **EndPointer   OPTIONAL,
  OUT       UINTN  *Data
  );

RETURN_STATUS
AddrToAsciiS (
  IN   UINTN  Address,
  OUT  UINT8  *String
  );

RETURN_STATUS
AsciiStrCat (
  IN OUT CHAR8        *Destination,
  IN     UINTN        DestMax,
  IN     CONST CHAR8  *Source
  );

#endif // __BASE_LIB__
