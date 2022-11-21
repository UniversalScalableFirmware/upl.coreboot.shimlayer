#include "BaseLib.h"

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
  )
{
  return Operand << Count;
}

/**
  Converts a lowercase Ascii character to upper one.

  If Chr is lowercase Ascii character, then converts it to upper one.

  If Value >= 0xA0, then ASSERT().
  If (Value & 0x0F) >= 0x0A, then ASSERT().

  @param  Chr   one Ascii character

  @return The uppercase value of Ascii character

**/
CHAR8
AsciiCharToUpper (
  IN      CHAR8  Chr
  )
{
  return (UINT8)((Chr >= 'a' && Chr <= 'z') ? Chr - ('a' - 'A') : Chr);
}

/**
  Check if a ASCII character is a decimal character.

  This internal function checks if a Unicode character is a
  decimal character. The valid decimal character is from
  '0' to '9'.

  @param  Char  The character to check against.

  @retval TRUE  If the Char is a decmial character.
  @retval FALSE If the Char is not a decmial character.

**/
BOOLEAN
InternalAsciiIsDecimalDigitCharacter (
  IN      CHAR8  Char
  )
{
  return (BOOLEAN)(Char >= '0' && Char <= '9');
}

/**
  Check if a ASCII character is a hexadecimal character.

  This internal function checks if a ASCII character is a
  decimal character.  The valid hexadecimal character is
  L'0' to L'9', L'a' to L'f', or L'A' to L'F'.


  @param  Char  The character to check against.

  @retval TRUE  If the Char is a hexadecmial character.
  @retval FALSE If the Char is not a hexadecmial character.

**/
BOOLEAN
InternalAsciiIsHexaDecimalDigitCharacter (
  IN      CHAR8  Char
  )
{
  return (BOOLEAN)(InternalAsciiIsDecimalDigitCharacter (Char) ||
                   (Char >= 'A' && Char <= 'F') ||
                   (Char >= 'a' && Char <= 'f'));
}

/**
  Check if a Unicode character is a decimal character.

  This internal function checks if a Unicode character is a
  decimal character. The valid decimal character is from
  L'0' to L'9'.

  @param  Char  The character to check against.

  @retval TRUE  If the Char is a decmial character.
  @retval FALSE If the Char is not a decmial character.

**/
BOOLEAN
InternalIsDecimalDigitCharacter (
  IN      CHAR16  Char
  )
{
  return (BOOLEAN)(Char >= L'0' && Char <= L'9');
}

/**
  Convert a ASCII character to numerical value.

  This internal function only deal with Unicode character
  which maps to a valid hexadecimal ASII character, i.e.
  '0' to '9', 'a' to 'f' or 'A' to 'F'. For other
  ASCII character, the value returned does not make sense.

  @param  Char  The character to convert.

  @return The numerical value converted.

**/
UINTN
InternalAsciiHexCharToUintn (
  IN      CHAR8  Char
  )
{
  if (InternalIsDecimalDigitCharacter (Char)) {
    return Char - '0';
  }

  return (10 + AsciiCharToUpper (Char) - 'A');
}

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
  VOID        *DestinationBuffer,
  CONST VOID  *SourceBuffer,
  UINTN       Length
  )
{
  //
  // Declare the local variables that actually move the data elements as
  // volatile to prevent the optimizer from replacing this function with
  // the intrinsic memcpy()
  //
  volatile UINT8   *Destination8;
  CONST UINT8      *Source8;
  volatile UINT32  *Destination32;
  CONST UINT32     *Source32;
  volatile UINT64  *Destination64;
  CONST UINT64     *Source64;
  UINTN            Alignment;
  if (Length == 0) {
    return DestinationBuffer;
  }

  if (DestinationBuffer == SourceBuffer) {
    return DestinationBuffer;
  }

  if ((((UINTN)DestinationBuffer & 0x7) == 0) && (((UINTN)SourceBuffer & 0x7) == 0) && (Length >= 8)) {
    if (SourceBuffer > DestinationBuffer) {
      Destination64 = (UINT64 *)DestinationBuffer;
      Source64      = (CONST UINT64 *)SourceBuffer;
      while (Length >= 8) {
        *(Destination64++) = *(Source64++);
        Length            -= 8;
      }

      // Finish if there are still some bytes to copy
      Destination8 = (UINT8 *)Destination64;
      Source8      = (CONST UINT8 *)Source64;
      while (Length-- != 0) {
        *(Destination8++) = *(Source8++);
      }
    } else if (SourceBuffer < DestinationBuffer) {
      Destination64 = (UINT64 *)((UINTN)DestinationBuffer + Length);
      Source64      = (CONST UINT64 *)((UINTN)SourceBuffer + Length);

      // Destination64 and Source64 were aligned on a 64-bit boundary
      // but if length is not a multiple of 8 bytes then they won't be
      // anymore.

      Alignment = Length & 0x7;
      if (Alignment != 0) {
        Destination8 = (UINT8 *)Destination64;
        Source8      = (CONST UINT8 *)Source64;

        while (Alignment-- != 0) {
          *(--Destination8) = *(--Source8);
          --Length;
        }

        Destination64 = (UINT64 *)Destination8;
        Source64      = (CONST UINT64 *)Source8;
      }

      while (Length > 0) {
        *(--Destination64) = *(--Source64);
        Length            -= 8;
      }
    }
  } else if ((((UINTN)DestinationBuffer & 0x3) == 0) && (((UINTN)SourceBuffer & 0x3) == 0) && (Length >= 4)) {
    if (SourceBuffer > DestinationBuffer) {
      Destination32 = (UINT32 *)DestinationBuffer;
      Source32      = (CONST UINT32 *)SourceBuffer;
      while (Length >= 4) {
        *(Destination32++) = *(Source32++);
        Length            -= 4;
      }

      // Finish if there are still some bytes to copy
      Destination8 = (UINT8 *)Destination32;
      Source8      = (CONST UINT8 *)Source32;
      while (Length-- != 0) {
        *(Destination8++) = *(Source8++);
      }
    } else if (SourceBuffer < DestinationBuffer) {
      Destination32 = (UINT32 *)((UINTN)DestinationBuffer + Length);
      Source32      = (CONST UINT32 *)((UINTN)SourceBuffer + Length);

      // Destination32 and Source32 were aligned on a 32-bit boundary
      // but if length is not a multiple of 4 bytes then they won't be
      // anymore.

      Alignment = Length & 0x3;
      if (Alignment != 0) {
        Destination8 = (UINT8 *)Destination32;
        Source8      = (CONST UINT8 *)Source32;

        while (Alignment-- != 0) {
          *(--Destination8) = *(--Source8);
          --Length;
        }

        Destination32 = (UINT32 *)Destination8;
        Source32      = (CONST UINT32 *)Source8;
      }

      while (Length > 0) {
        *(--Destination32) = *(--Source32);
        Length            -= 4;
      }
    }
  } else {
    if (SourceBuffer > DestinationBuffer) {
      Destination8 = (UINT8 *)DestinationBuffer;
      Source8      = (CONST UINT8 *)SourceBuffer;
      while (Length-- != 0) {
        *(Destination8++) = *(Source8++);
      }
    } else if (SourceBuffer < DestinationBuffer) {
      Destination8 = (UINT8 *)DestinationBuffer + (Length - 1);
      Source8      = (CONST UINT8 *)SourceBuffer + (Length - 1);
      while (Length-- != 0) {
        *(Destination8--) = *(Source8--);
      }
    }
  }

  return DestinationBuffer;
}

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
  )
{
  if (Length == 0) {
    return Buffer;
  }

  //
  // Declare the local variables that actually move the data elements as
  // volatile to prevent the optimizer from replacing this function with
  // the intrinsic memset()
  //
  volatile UINT8   *Pointer8;
  volatile UINT32  *Pointer32;
  volatile UINT64  *Pointer64;
  UINT32           Value32;
  UINT64           Value64;

  if ((((UINTN)Buffer & 0x7) == 0) && (Length >= 8)) {
    // Generate the 64bit value
    Value64 = 0;

    Pointer64 = (UINT64 *)Buffer;
    while (Length >= 8) {
      *(Pointer64++) = Value64;
      Length        -= 8;
    }

    // Finish with bytes if needed
    Pointer8 = (UINT8 *)Pointer64;
  } else if ((((UINTN)Buffer & 0x3) == 0) && (Length >= 4)) {
    // Generate the 32bit value
    Value32 = 0;

    Pointer32 = (UINT32 *)Buffer;
    while (Length >= 4) {
      *(Pointer32++) = Value32;
      Length        -= 4;
    }

    // Finish with bytes if needed
    Pointer8 = (UINT8 *)Pointer32;
  } else {
    Pointer8 = (UINT8 *)Buffer;
  }

  while (Length-- > 0) {
    *(Pointer8++) = 0;
  }

  return Buffer;
}

/**
 * fls - find last (most-significant) bit set
 * @x: the word to search
 *
 * This is defined the same way as ffs.
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */

STATIC int fls(UINT32 x)
{
	int r = 32;

	if (!x)
		return 0;
	if (!(x & 0xffff0000u)) {
		x <<= 16;
		r -= 16;
	}
	if (!(x & 0xff000000u)) {
		x <<= 8;
		r -= 8;
	}
	if (!(x & 0xf0000000u)) {
		x <<= 4;
		r -= 4;
	}
	if (!(x & 0xc0000000u)) {
		x <<= 2;
		r -= 2;
	}
	if (!(x & 0x80000000u)) {
		x <<= 1;
		r -= 1;
	}
	return r;
}

/**
  Multiplies a 64-bit unsigned integer by a 64-bit unsigned integer and
  generates a 64-bit unsigned result.

  This function multiplies the 64-bit unsigned value Multiplicand by the 64-bit
  unsigned value Multiplier and generates a 64-bit unsigned result. This 64-
  bit unsigned result is returned.

  @param  Multiplicand  A 64-bit unsigned value.
  @param  Multiplier    A 64-bit unsigned value.

  @return Multiplicand * Multiplier.

**/
UINT64
MultU64x64 (
  IN      UINT64  Multiplicand,
  IN      UINT64  Multiplier
  )
{
  UINT64  Result;

  Result = Multiplicand * Multiplier;

  return Result;
}

/**
 * div_u64_rem - unsigned 64bit divide with 32bit divisor with remainder
 * @dividend: unsigned 64bit dividend
 * @divisor: unsigned 32bit divisor
 * @remainder: pointer to unsigned 32bit remainder
 *
 * Return: sets ``*remainder``, then returns dividend / divisor
 *
 * This is commonly provided by 32bit archs to provide an optimized 64bit
 * divide.
 */
STATIC inline UINT64 div_u64_rem(UINT64 dividend, UINT32 divisor, UINT32 *remainder)
{
	*remainder = dividend % divisor;
	return dividend / divisor;
}

/**
 * div_u64 - unsigned 64bit divide with 32bit divisor
 * @dividend: unsigned 64bit dividend
 * @divisor: unsigned 32bit divisor
 *
 * This is the most common 64bit divide and should be used if possible,
 * as many 32bit archs can optimize this variant better than a full 64bit
 * divide.
 */
STATIC inline UINT64 div_u64(UINT64 dividend, UINT32 divisor)
{
	UINT32 remainder;
	return div_u64_rem(dividend, divisor, &remainder);
}

UINT64 div64_u64(UINT64 dividend, UINT64 divisor)
{
  UINT32 high = divisor >> 32;
  UINT64 quot;

  if (high == 0) {
    quot = div_u64(dividend, divisor);
  } else {
    int n = 1 + fls(high);
    quot = div_u64(dividend >> n, divisor >> n);

    if (quot != 0)
      quot--;
    if ((dividend - quot * divisor) >= divisor)
      quot++;
  }

  return quot;
}

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

  @return Dividend / Divisor

**/
UINT64
DivU64x64Remainder (
  IN      UINT64  Dividend,
  IN      UINT64  Divisor,
  OUT     UINT64  *Remainder  OPTIONAL
  )
{
  if (Remainder != NULL) {
    *Remainder = Dividend % Divisor;
  }

  div64_u64 (Dividend, Divisor);

  return Dividend;
}

UINT64
ReadUnaligned64 (
   CONST UINT64              *Buffer
  )
{
  return *Buffer;
}

UINT64
WriteUnaligned64 (
   UINT64                    *Buffer,
   UINT64                    Value
  )
{
  return *Buffer = Value;
}


GUID *
CopyGuid (
   GUID        *DestinationGuid,
   CONST GUID  *SourceGuid
  )
{
  WriteUnaligned64 (
    (UINT64*)DestinationGuid,
    ReadUnaligned64 ((CONST UINT64*)SourceGuid)
    );
  WriteUnaligned64 (
    (UINT64*)DestinationGuid + 1,
    ReadUnaligned64 ((CONST UINT64*)SourceGuid + 1)
    );
  return DestinationGuid;
}

/**
  Compares two GUIDs.

  This function compares Guid1 to Guid2.  If the GUIDs are identical then TRUE is returned.
  If there are any bit differences in the two GUIDs, then FALSE is returned.

  If Guid1 is NULL, then ASSERT().
  If Guid2 is NULL, then ASSERT().

  @param  Guid1       A pointer to a 128 bit GUID.
  @param  Guid2       A pointer to a 128 bit GUID.

  @retval TRUE        Guid1 and Guid2 are identical.
  @retval FALSE       Guid1 and Guid2 are not identical.

**/

unsigned char
CompareGuid (
    CONST GUID  *Guid1,
    CONST GUID  *Guid2
  )
{
  UINT64  LowPartOfGuid1;
  UINT64  LowPartOfGuid2;
  UINT64  HighPartOfGuid1;
  UINT64  HighPartOfGuid2;

  LowPartOfGuid1  = ReadUnaligned64 ((CONST UINT64 *)Guid1);
  LowPartOfGuid2  = ReadUnaligned64 ((CONST UINT64 *)Guid2);
  HighPartOfGuid1 = ReadUnaligned64 ((CONST UINT64 *)Guid1 + 1);
  HighPartOfGuid2 = ReadUnaligned64 ((CONST UINT64 *)Guid2 + 1);

  return (unsigned char)(LowPartOfGuid1 == LowPartOfGuid2 && HighPartOfGuid1 == HighPartOfGuid2);
}

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
  CONST CHAR8  *String,
  UINTN        MaxSize
  )
{
  UINTN  Length;

  //
  // If String is a null pointer or MaxSize is 0, then the AsciiStrnLenS function returns zero.
  //
  if ((String == NULL) || (MaxSize == 0)) {
    return 0;
  }

  //
  // Otherwise, the AsciiStrnLenS function returns the number of characters that precede the
  // terminating null character. If there is no null character in the first MaxSize characters of
  // String then AsciiStrnLenS returns MaxSize. At most the first MaxSize characters of String shall
  // be accessed by AsciiStrnLenS.
  //
  Length = 0;
  while (String[Length] != 0) {
    if (Length >= MaxSize - 1) {
      return MaxSize;
    }

    Length++;
  }

  return Length;
}

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
     CONST CHAR8  *FirstString,
     CONST CHAR8  *SecondString
  )
{
  while ((*FirstString != '\0') && (*FirstString == *SecondString)) {
    FirstString++;
    SecondString++;
  }
  return *FirstString - *SecondString;
}

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
  CONST CHAR8  *FirstString,
  CONST CHAR8  *SecondString,
  UINTN        Length
  )
{
  if (Length == 0) {
    return 0;
  }

  while ((*FirstString != '\0') &&
         (*SecondString != '\0') &&
         (*FirstString == *SecondString) &&
         (Length > 1))
  {
    FirstString++;
    SecondString++;
    Length--;
  }

  return *FirstString - *SecondString;
}

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
  CONST CHAR8  *Source
  )
{
  UINTN  SourceLen;

  //
  // 1. Neither Destination nor Source shall be a null pointer.
  //
  if (Destination == NULL || Source == NULL) {
    return RETURN_NOT_FOUND;
  }

  //
  // 2. DestMax shall not be greater than ASCII_RSIZE_MAX.
  //
  if (ASCII_RSIZE_MAX != 0) {
    if (DestMax > ASCII_RSIZE_MAX) {
      return RETURN_UNSUPPORTED;
    }
  }

  //
  // 3. DestMax shall not equal zero.
  //
  if (DestMax == 0) {
    return RETURN_NOT_FOUND;
  }

  //
  // 4. DestMax shall be greater than AsciiStrnLenS(Source, DestMax).
  //
  SourceLen = AsciiStrnLenS (Source, DestMax);
  if (DestMax <= SourceLen) {
    return RETURN_UNSUPPORTED;
  }

  //
  // The AsciiStrCpyS function copies the string pointed to by Source (including the terminating
  // null character) into the array pointed to by Destination.
  //
  while (*Source != 0) {
    *(Destination++) = *(Source++);
  }

  *Destination = 0;

  return RETURN_SUCCESS;
}

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
  )
{
  if (Length == 0) {
    return Buffer;
  }

  //
  // Declare the local variables that actually move the data elements as
  // volatile to prevent the optimizer from replacing this function with
  // the intrinsic memset()
  //
  volatile UINT8   *Pointer8;
  volatile UINT32  *Pointer32;
  volatile UINT64  *Pointer64;
  UINT32           Value32;
  UINT64           Value64;

  if ((((UINTN)Buffer & 0x7) == 0) && (Length >= 8)) {
    // Generate the 64bit value
    Value32 = (Value << 24) | (Value << 16) | (Value << 8) | Value;
    Value64 = LShiftU64 (Value32, 32) | Value32;

    Pointer64 = (UINT64 *)Buffer;
    while (Length >= 8) {
      *(Pointer64++) = Value64;
      Length        -= 8;
    }

    // Finish with bytes if needed
    Pointer8 = (UINT8 *)Pointer64;
  } else if ((((UINTN)Buffer & 0x3) == 0) && (Length >= 4)) {
    // Generate the 32bit value
    Value32 = (Value << 24) | (Value << 16) | (Value << 8) | Value;

    Pointer32 = (UINT32 *)Buffer;
    while (Length >= 4) {
      *(Pointer32++) = Value32;
      Length        -= 4;
    }

    // Finish with bytes if needed
    Pointer8 = (UINT8 *)Pointer32;
  } else {
    Pointer8 = (UINT8 *)Buffer;
  }

  while (Length-- > 0) {
    *(Pointer8++) = Value;
  }

  return Buffer;
}

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
  )
{
  if ((Length == 0) || (DestinationBuffer == SourceBuffer)) {
    return 0;
  }

  while ((--Length != 0) &&
         (*(INT8 *)DestinationBuffer == *(INT8 *)SourceBuffer))
  {
    DestinationBuffer = (INT8 *)DestinationBuffer + 1;
    SourceBuffer      = (INT8 *)SourceBuffer + 1;
  }

  return (INTN)*(UINT8 *)DestinationBuffer - (INTN)*(UINT8 *)SourceBuffer;
}

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
  )
{
  if (Length == 0) {
    return NULL;
  }

  CONST UINT8  *Pointer;

  Pointer = (CONST UINT8 *)Buffer;
  do {
    if (*Pointer == Value) {
      return (VOID *)Pointer;
    }

    ++Pointer;
  } while (--Length != 0);

  return NULL;
}

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
  )
{
  UINTN  Length;

  for (Length = 0; *String != '\0'; String++, Length++) {
    //
    // If PcdMaximumUnicodeStringLength is not zero,
    // length should not more than PcdMaximumUnicodeStringLength
    //
    if ( Length > MAX_ASCII_STRING_LENGTH) {
      return Length - 1;
    }
  }

  return Length;
}

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
  )
{
  CONST CHAR8  *FirstMatch;
  CONST CHAR8  *SearchStringTmp;

  if (*SearchString == '\0') {
    return (CHAR8 *)String;
  }

  while (*String != '\0') {
    SearchStringTmp = SearchString;
    FirstMatch      = String;

    while (  (*String == *SearchStringTmp)
          && (*String != '\0'))
    {
      String++;
      SearchStringTmp++;
    }

    if (*SearchStringTmp == '\0') {
      return (CHAR8 *)FirstMatch;
    }

    if (*String == '\0') {
      return NULL;
    }

    String = FirstMatch + 1;
  }

  return NULL;
}

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
  )
{
  //
  // 1. Neither String nor Data shall be a null pointer.
  //
  SAFE_STRING_CONSTRAINT_CHECK ((String != NULL), RETURN_INVALID_PARAMETER);
  SAFE_STRING_CONSTRAINT_CHECK ((Data != NULL), RETURN_INVALID_PARAMETER);

  //
  // 2. The length of String shall not be greater than ASCII_RSIZE_MAX.
  //
  if (ASCII_RSIZE_MAX != 0) {
    SAFE_STRING_CONSTRAINT_CHECK ((AsciiStrnLenS (String, ASCII_RSIZE_MAX + 1) <= ASCII_RSIZE_MAX), RETURN_INVALID_PARAMETER);
  }

  if (EndPointer != NULL) {
    *EndPointer = (CHAR8 *)String;
  }

  //
  // Ignore the pad spaces (space or tab)
  //
  while ((*String == ' ') || (*String == '\t')) {
    String++;
  }

  //
  // Ignore leading Zeros after the spaces
  //
  while (*String == '0') {
    String++;
  }

  *Data = 0;

  while (InternalAsciiIsDecimalDigitCharacter (*String)) {
    //
    // If the number represented by String overflows according to the range
    // defined by UINTN, then MAX_UINTN is stored in *Data and
    // RETURN_UNSUPPORTED is returned.
    //
    if (*Data > ((MAX_UINTN - (*String - '0')) / 10)) {
      *Data = MAX_UINTN;
      if (EndPointer != NULL) {
        *EndPointer = (CHAR8 *)String;
      }

      return RETURN_UNSUPPORTED;
    }

    *Data = *Data * 10 + (*String - '0');
    String++;
  }

  if (EndPointer != NULL) {
    *EndPointer = (CHAR8 *)String;
  }

  return RETURN_SUCCESS;
}

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
  )
{
  //
  // 1. Neither String nor Data shall be a null pointer.
  //
  SAFE_STRING_CONSTRAINT_CHECK ((String != NULL), RETURN_INVALID_PARAMETER);
  SAFE_STRING_CONSTRAINT_CHECK ((Data != NULL), RETURN_INVALID_PARAMETER);

  //
  // 2. The length of String shall not be greater than ASCII_RSIZE_MAX.
  //
  if (ASCII_RSIZE_MAX != 0) {
    SAFE_STRING_CONSTRAINT_CHECK ((AsciiStrnLenS (String, ASCII_RSIZE_MAX + 1) <= ASCII_RSIZE_MAX), RETURN_INVALID_PARAMETER);
  }

  if (EndPointer != NULL) {
    *EndPointer = (CHAR8 *)String;
  }

  //
  // Ignore the pad spaces (space or tab)
  //
  while ((*String == ' ') || (*String == '\t')) {
    String++;
  }

  //
  // Ignore leading Zeros after the spaces
  //
  while (*String == '0') {
    String++;
  }

  if (AsciiCharToUpper (*String) == 'X') {
    if (*(String - 1) != '0') {
      *Data = 0;
      return RETURN_SUCCESS;
    }

    //
    // Skip the 'X'
    //
    String++;
  }

  *Data = 0;

  while (InternalAsciiIsHexaDecimalDigitCharacter (*String)) {
    //
    // If the number represented by String overflows according to the range
    // defined by UINTN, then MAX_UINTN is stored in *Data and
    // RETURN_UNSUPPORTED is returned.
    //
    if (*Data > ((MAX_UINTN - InternalAsciiHexCharToUintn (*String)) >> 4)) {
      *Data = MAX_UINTN;
      if (EndPointer != NULL) {
        *EndPointer = (CHAR8 *)String;
      }

      return RETURN_UNSUPPORTED;
    }

    *Data = (*Data << 4) + InternalAsciiHexCharToUintn (*String);
    String++;
  }

  if (EndPointer != NULL) {
    *EndPointer = (CHAR8 *)String;
  }

  return RETURN_SUCCESS;
}

RETURN_STATUS
AddrToAsciiS (
  IN   UINTN  Address,
  OUT  UINT8  *String
  )
{
  UINT8  TmpString[9] = {0};
  INT8   Position = 0;
  INT8   ValidPos = 0;
  INT8   Key;
  INT8   i;

  while (Position < 8) {
    Key = (Address >> (Position*4)) & 0xf;
    switch (Key) {
    case 0:
      TmpString[Position] = '0';
      break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
      ValidPos = Position;
      TmpString[Position] = '0' + Key;
      break;
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
      ValidPos = Position;
      TmpString[Position] = 'A' + Key - 10;
      break;
    default:
      break;
    }
    Position++;
  }

  for(i=0;i<=ValidPos;i++) {
    String[i] = TmpString[ValidPos - i];
  }
  return RETURN_SUCCESS;
}

RETURN_STATUS
AsciiStrCat (
  IN OUT CHAR8        *Destination,
  IN     UINTN        DestMax,
  IN     CONST CHAR8  *Source
  )
{
  INT16 i;
  INT16 SourceLen;
  INT16 DestLen;

  DestLen   = AsciiStrnLenS(Destination, 32);
  SourceLen = AsciiStrnLenS(Source, 32);
  for(i = DestLen; i < DestMax || (i - DestLen) < SourceLen; i++) {
    Destination[i] = Source[i - DestLen];
  }

  return RETURN_SUCCESS;
}