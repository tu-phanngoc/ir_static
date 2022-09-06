// Copyright 2009 Ken Shirriff
// Copyright 2015 Mark Szabo
// Copyright 2015 Sebastien Warin
// Copyright 2017, 2019 David Conran

#include <string.h>
#include "IRrecv.h"
#include <stddef.h>
#include "IRutils.h"
#include "stdlib.h"


#define MAX_TOLERANCE   100


decode_results gDecodeResult;

void initDecodeData(int16_t *src, uint16_t length) {
	
	/* TODO: Them () vao Macro, Tu Phan */
  memset(gDecodeResult.rawbuf, 0, kStateBitsMax);
  memset(gDecodeResult.state, 0, kStateSizeMax);

  // Copy contents of src[] to dst[]
  for (uint16_t i = 0; i < length; i++) gDecodeResult.rawbuf[i] = abs(src[i]);

  gDecodeResult.rawlen       = length;
  gDecodeResult.overflow     = 0;
  gDecodeResult.decode_type  = UNKNOWN;
  gDecodeResult.bits         = 0;
  gDecodeResult.address = 0;
  gDecodeResult.address = 0;
  gDecodeResult.command = 0;
  gDecodeResult.repeat       = false;
}


void getRawBuf(int16_t *src, decode_results *dst, uint16_t length){

  memset(dst->rawbuf, 0, kStateBitsMax);
  memset(dst->state, 0, kStateSizeMax);

  // Copy contents of src[] to dst[]
  for (uint16_t i = 0; i < length; i++) dst->rawbuf[i] = abs(src[i]);

  dst->rawlen       = length;
  dst->overflow     = 0;
  dst->decode_type  = UNKNOWN;
  dst->bits         = 0;
  dst->address      = 0;
  dst->address      = 0;
  dst->command      = 0;
  dst->repeat       = false;
}


/// Convert the tolerance percentage into something valid.
/// @param[in] percentage An integer percentage.
uint8_t _validTolerance(const uint8_t percentage) {
    return (percentage > 100) ? MAX_TOLERANCE : percentage;
}

/// Calculate the lower bound of the nr. of ticks.
/// @param[in] usecs Nr. of uSeconds.
/// @param[in] tolerance Percent as an integer. e.g. 10 is 10%
/// @param[in] delta A non-scaling amount to reduce usecs by.
/// @return Nr. of ticks.
uint32_t ticksLow(const uint32_t usecs, const uint8_t tolerance) {
  // MAX() used to ensure the result can't drop below 0 before the cast.
  return ((uint32_t)MAX((int32_t)(usecs * (1.0 - _validTolerance(tolerance) / 100.0)), 0));
}

/// Calculate the upper bound of the nr. of ticks.
/// @param[in] usecs Nr. of uSeconds.
/// @param[in] tolerance Percent as an integer. e.g. 10 is 10%
/// @return Nr. of ticks.
uint32_t ticksHigh(const uint32_t usecs, const uint8_t tolerance) {
  return ((uint32_t)(usecs * (1.0 + _validTolerance(tolerance) / 100.0)));
}

/// Check if we match a pulse(measured) with the desired within
///   +/-tolerance percent and/or +/- a fixed delta range.
/// @param[in] measured The recorded period of the signal pulse.
/// @param[in] desired The expected period (in usecs) we are matching against.
/// @param[in] tolerance A percentage expressed as an integer. e.g. 10 is 10%.
/// @param[in] delta A non-scaling (+/-) error margin (in useconds).
/// @return A Boolean. true if it matches, false if it doesn't.
bool match(uint32_t measured, uint32_t desired, uint8_t tolerance) {
  return (measured >= ticksLow(desired, tolerance) &&
          measured <= ticksHigh(desired, tolerance));
}


/// Check if we match a pulse(measured) of at least desired within
///   tolerance percent and/or a fixed delta margin.
/// @param[in] measured The recorded period of the signal pulse.
/// @param[in] desired The expected period (in usecs) we are matching against.
/// @param[in] tolerance A percentage expressed as an integer. e.g. 10 is 10%.
/// @return A Boolean. true if it matches, false if it doesn't.
bool matchAtLeast(uint32_t measured, uint32_t desired, uint8_t tolerance) {
  // We really should never get a value of 0, except as the last value
  // in the buffer. If that is the case, then assume infinity and return true.
  if (measured == 0) return true;
  return measured >= ticksLow(desired, tolerance);
}

/// Check if we match a mark signal(measured) with the desired within
///  +/-tolerance percent, after an expected is excess is added.
/// @param[in] measured The recorded period of the signal pulse.
/// @param[in] desired The expected period (in usecs) we are matching against.
/// @param[in] tolerance A percentage expressed as an integer. e.g. 10 is 10%.
/// @return A Boolean. true if it matches, false if it doesn't.
bool matchMark(uint32_t measured, uint32_t desired, uint8_t tolerance) {
  return match(measured, desired, tolerance);
}

/// Check if we match a space signal(measured) with the desired within
///  +/-tolerance percent, after an expected is excess is removed.
/// @param[in] measured The recorded period of the signal pulse.
/// @param[in] desired The expected period (in usecs) we are matching against.
/// @param[in] tolerance A percentage expressed as an integer. e.g. 10 is 10%.
/// @return A Boolean. true if it matches, false if it doesn't.
bool matchSpace(uint32_t measured, uint32_t desired, uint8_t tolerance) {
  return match(measured, desired, tolerance);
}


/// Match & decode the typical data section of an IR message.
/// The data value is stored in the least significant bits reguardless of the
/// bit ordering requested.
/// @param[in] data_ptr A pointer to where we are at in the capture buffer.
/// @param[in] nbits Nr. of data bits we expect.
/// @param[in] onemark Nr. of uSeconds in an expected mark signal for a '1' bit.
/// @param[in] onespace Nr. of uSecs in an expected space signal for a '1' bit.
/// @param[in] zeromark Nr. of uSecs in an expected mark signal for a '0' bit.
/// @param[in] zerospace Nr. of uSecs in an expected space signal for a '0' bit.
/// @param[in] tolerance Percentage error margin to allow. (Default: kUseDefTol)
/// @param[in] excess Nr. of uSeconds. (Def: kMarkExcess)
/// @param[in] MSBfirst Bit order to save the data in. (Def: true)
///   true is Most Significant Bit First Order, false is Least Significant First
/// @param[in] expectlastspace Do we expect a space at the end of the message?
/// @return A match_result_t structure containing the success (or not), the
///   data value, and how many buffer entries were used.
match_result_t matchData(
    uint16_t *data_ptr,  const uint16_t nbits,
    const uint16_t onemark,       const uint16_t onespace,
    const uint16_t zeromark,      const uint16_t zerospace,
    const uint8_t tolerance,      const int16_t excess,
		const bool MSBfirst,					const bool expectlastspace) {
  match_result_t result;
  result.success = false;  // Fail by default.
  result.data = 0;

  if (expectlastspace) {  // We are expecting data with a final space.
    for (result.used = 0; result.used < nbits * 2;
         result.used += 2, data_ptr += 2) {
      // Is the bit a '1'?
      if (matchMark(*data_ptr, onemark, tolerance) &&
          matchSpace(*(data_ptr + 1), onespace, tolerance)) {
        result.data = (result.data << 1) | 1;
      }
      else if ( matchMark(*data_ptr, zeromark, tolerance) &&
                matchSpace(*(data_ptr + 1), zerospace, tolerance)) {
        result.data <<= 1;  // The bit is a '0'.
      }
      else {
        if (!MSBfirst) result.data = reverseBits(result.data, result.used / 2);
        return result;  // It's neither, so fail.
      }
    }
    result.success = true;
  } else {  // We are expecting data without a final space.
    // Match all but the last bit, as it may not match easily.
    result = matchData( data_ptr, nbits ? nbits - 1 : 0, 
                        onemark, onespace,
                        zeromark, zerospace, 
                        tolerance, kMarkExcess, 
                        true, true);
    if (result.success) {
      // Is the bit a '1'?
      if (matchMark(*(data_ptr + result.used), onemark, tolerance))
        result.data = (result.data << 1) | 1;
      else if (matchMark(*(data_ptr + result.used), zeromark, tolerance))
        result.data <<= 1;  // The bit is a '0'.
      else
        result.success = false;
      if (result.success) result.used++;
    }
  }

  if (!MSBfirst) result.data = reverseBits(result.data, nbits);
  return result;

}

/// Match & decode the typical data section of an IR message.
/// The bytes are stored at result_ptr. The first byte in the result equates to
/// the first byte encountered, and so on.
/// @param[in] data_ptr A pointer to where we are at in the capture buffer.
/// @param[out] result_ptr A ptr to where to start storing the bytes we decoded.
/// @param[in] remaining The size of the capture buffer remaining.
/// @param[in] nbytes Nr. of data bytes we expect.
/// @param[in] onemark Nr. of uSeconds in an expected mark signal for a '1' bit.
/// @param[in] onespace Nr. of uSecs in an expected space signal for a '1' bit.
/// @param[in] zeromark Nr. of uSecs in an expected mark signal for a '0' bit.
/// @param[in] zerospace Nr. of uSecs in an expected space signal for a '0' bit.
/// @param[in] tolerance Percentage error margin to allow. (Default: kUseDefTol)
/// @param[in] excess Nr. of uSeconds. (Def: kMarkExcess)
/// @param[in] MSBfirst Bit order to save the data in. (Def: true)
///   true is Most Significant Bit First Order, false is Least Significant First
/// @param[in] expectlastspace Do we expect a space at the end of the message?
/// @return If successful, how many buffer entries were used. Otherwise 0.
uint16_t matchBytes(uint16_t *data_ptr,       uint8_t *result_ptr,
                    const uint16_t remaining, const uint16_t nbytes,
                    const uint16_t onemark,   const uint16_t onespace,
                    const uint16_t zeromark,  const uint16_t zerospace,
                    const uint8_t tolerance,  const int16_t excess,
                    const bool MSBfirst, 			const bool expectlastspace) {
  // Check if there is enough capture buffer to possibly have the desired bytes.
  if (remaining + expectlastspace < (nbytes * 8 * 2) + 1)
    return 0;  // Nope, so abort.
  uint16_t offset = 0;
  for (uint16_t byte_pos = 0; byte_pos < nbytes; byte_pos++) {
    bool lastspace = (byte_pos + 1 == nbytes) ? expectlastspace : true;
    match_result_t result = matchData(data_ptr + offset, 8, 
                                      onemark, onespace,
                                      zeromark, zerospace,
                                      tolerance, kMarkExcess,
                                      MSBfirst, lastspace);
    if (result.success == false)
      return 0;  // Fail
    result_ptr[byte_pos] = (uint8_t)result.data;
    offset += result.used;
  }
  return offset;
}

/// Match & decode a generic/typical IR message.
/// The data is stored in result_bits_ptr or result_bytes_ptr depending on flag
/// `use_bits`.
/// @note Values of 0 for hdrmark, hdrspace, footermark, or footerspace mean
/// skip that requirement.
///
/// @param[in] data_ptr A pointer to where we are at in the capture buffer.
/// @param[out] result_bits_ptr A pointer to where to start storing the bits we
///    decoded.
/// @param[out] result_bytes_ptr A pointer to where to start storing the bytes
///    we decoded.
/// @param[in] use_bits A flag indicating if we are to decode bits or bytes.
/// @param[in] remaining The size of the capture buffer remaining.
/// @param[in] nbits Nr. of data bits we expect.
/// @param[in] hdrmark Nr. of uSeconds for the expected header mark signal.
/// @param[in] hdrspace Nr. of uSeconds for the expected header space signal.
/// @param[in] onemark Nr. of uSeconds in an expected mark signal for a '1' bit.
/// @param[in] onespace Nr. of uSecs in an expected space signal for a '1' bit.
/// @param[in] zeromark Nr. of uSecs in an expected mark signal for a '0' bit.
/// @param[in] zerospace Nr. of uSecs in an expected space signal for a '0' bit.
/// @param[in] footermark Nr. of uSeconds for the expected footer mark signal.
/// @param[in] footerspace Nr. of uSeconds for the expected footer space/gap
///   signal.
/// @param[in] atleast Is the match on the footerspace a matchAtLeast or
///   matchSpace?
/// @param[in] tolerance Percentage error margin to allow. (Default: kUseDefTol)
/// @param[in] excess Nr. of uSeconds. (Def: kMarkExcess)
/// @param[in] MSBfirst Bit order to save the data in. (Def: true)
///   true is Most Significant Bit First Order, false is Least Significant First
/// @return If successful, how many buffer entries were used. Otherwise 0.
uint16_t _matchGeneric( uint16_t *data_ptr,         uint64_t *result_bits_ptr,
                        uint8_t *result_bytes_ptr,  const bool use_bits,
                        const uint16_t remaining,   const uint16_t nbits,
                        const uint16_t hdrmark,     const uint32_t hdrspace,
                        const uint16_t onemark,     const uint32_t onespace,
                        const uint16_t zeromark,    const uint32_t zerospace,
                        const uint16_t footermark,  const uint32_t footerspace,
                        const bool atleast,         const uint8_t tolerance,
                        const int16_t excess,       const bool MSBfirst){
  // If we are expecting byte sizes, check it's a factor of 8 or fail.
  if (!use_bits && nbits % 8 != 0)  return 0;
  // Calculate if we expect a trailing space in the data section.
  const bool kexpectspace = footermark || (onespace != zerospace);
  // Calculate how much remaining buffer is required.
  uint16_t min_remaining = nbits * 2 - (kexpectspace ? 0 : 1);

  if (hdrmark) min_remaining++;
  if (hdrspace) min_remaining++;
  if (footermark) min_remaining++;
  // Don't need to extend for footerspace because it could be the end of message

  // Check if there is enough capture buffer to possibly have the message.
  if (remaining < min_remaining) return 0;  // Nope, so abort.
  uint16_t offset = 0;

  // Header
  if (hdrmark && !matchMark(*(data_ptr + offset++), hdrmark, tolerance))
    return 0;
  if (hdrspace && !matchSpace(*(data_ptr + offset++), hdrspace, tolerance))
    return 0;

  // Data
  if (use_bits) {  // Bits.
    match_result_t result = matchData(data_ptr + offset, nbits,
                                      onemark, onespace,
                                      zeromark, zerospace, tolerance,
                                      excess, MSBfirst, kexpectspace);
    if (!result.success) return 0;
    *result_bits_ptr = result.data;
    offset += result.used;
  } else {  // bytes
    uint16_t data_used = matchBytes(data_ptr + offset, result_bytes_ptr,
                                            remaining - offset, nbits / 8,
                                            onemark, onespace,
                                            zeromark, zerospace, tolerance,
                                            excess, MSBfirst, kexpectspace);
    if (!data_used) return 0;
    offset += data_used;
  }
  // Footer
  if (footermark && !matchMark(*(data_ptr + offset++), footermark, tolerance))
    return 0;
  // If we have something still to match & haven't reached the end of the buffer
  if (footerspace && offset < remaining) {
      if (atleast) {
        if (!matchAtLeast(*(data_ptr + offset), footerspace, tolerance))
          return 0;
      } else {
        if (!matchSpace(*(data_ptr + offset), footerspace, tolerance))
          return 0;
      }
      offset++;
  }
  return offset;
}

/// Match & decode a generic/typical <= 64bit IR message.
/// The data is stored at result_ptr.
/// @note Values of 0 for hdrmark, hdrspace, footermark, or footerspace mean
///   skip that requirement.
///
/// @param[in] data_ptr: A pointer to where we are at in the capture buffer.
/// @param[out] result_ptr A ptr to where to start storing the bits we decoded.
/// @param[in] remaining The size of the capture buffer remaining.
/// @param[in] nbits Nr. of data bits we expect.
/// @param[in] hdrmark Nr. of uSeconds for the expected header mark signal.
/// @param[in] hdrspace Nr. of uSeconds for the expected header space signal.
/// @param[in] onemark Nr. of uSeconds in an expected mark signal for a '1' bit.
/// @param[in] onespace Nr. of uSecs in an expected space signal for a '1' bit.
/// @param[in] zeromark Nr. of uSecs in an expected mark signal for a '0' bit.
/// @param[in] zerospace Nr. of uSecs in an expected space signal for a '0' bit.
/// @param[in] footermark Nr. of uSeconds for the expected footer mark signal.
/// @param[in] footerspace Nr. of uSeconds for the expected footer space/gap
///   signal.
/// @param[in] atleast Is the match on the footerspace a matchAtLeast or
///   matchSpace?
/// @param[in] tolerance Percentage error margin to allow. (Default: kUseDefTol)
/// @param[in] excess Nr. of uSeconds. (Def: kMarkExcess)
/// @param[in] MSBfirst Bit order to save the data in. (Def: true)
///   true is Most Significant Bit First Order, false is Least Significant First
/// @return If successful, how many buffer entries were used. Otherwise 0.
uint16_t matchGeneric_64( uint16_t *data_ptr,  uint64_t *result_ptr,
                          const uint16_t remaining,     const uint16_t nbits,
                          const uint16_t hdrmark,       const uint16_t hdrspace,
                          const uint16_t onemark,       const uint16_t onespace,
                          const uint16_t zeromark,      const uint16_t zerospace,
                          const uint16_t footermark,    const uint16_t footerspace,
                          const bool atleast,           const uint8_t tolerance,
                          const int16_t excess,         const bool MSBfirst) {
  return _matchGeneric( data_ptr, result_ptr,
                        NULL, true,
                        remaining, nbits,
                        hdrmark, hdrspace,
                        onemark, onespace,
                        zeromark, zerospace,
                        footermark, footerspace,
                        atleast, tolerance, 
                        excess, MSBfirst);
}

/// Match & decode a generic/typical > 64bit IR message.
/// The bytes are stored at result_ptr. The first byte in the result equates to
/// the first byte encountered, and so on.
/// @note Values of 0 for hdrmark, hdrspace, footermark, or footerspace mean
///   skip that requirement.
/// @param[in] data_ptr: A pointer to where we are at in the capture buffer.
/// @param[out] result_ptr A ptr to where to start storing the bytes we decoded.
/// @param[in] remaining The size of the capture buffer remaining.
/// @param[in] nbits Nr. of data bits we expect.
/// @param[in] hdrmark Nr. of uSeconds for the expected header mark signal.
/// @param[in] hdrspace Nr. of uSeconds for the expected header space signal.
/// @param[in] onemark Nr. of uSeconds in an expected mark signal for a '1' bit.
/// @param[in] onespace Nr. of uSecs in an expected space signal for a '1' bit.
/// @param[in] zeromark Nr. of uSecs in an expected mark signal for a '0' bit.
/// @param[in] zerospace Nr. of uSecs in an expected space signal for a '0' bit.
/// @param[in] footermark Nr. of uSeconds for the expected footer mark signal.
/// @param[in] footerspace Nr. of uSeconds for the expected footer space/gap
///   signal.
/// @param[in] atleast Is the match on the footerspace a matchAtLeast or
///   matchSpace?
/// @param[in] tolerance Percentage error margin to allow. (Default: kUseDefTol)
/// @param[in] excess Nr. of uSeconds. (Def: kMarkExcess)
/// @param[in] MSBfirst Bit order to save the data in. (Def: true)
///   true is Most Significant Bit First Order, false is Least Significant First
/// @return If successful, how many buffer entries were used. Otherwise 0.
uint16_t matchGeneric_8(uint16_t *data_ptr,             uint8_t *result_ptr,
                        const uint16_t remaining,       const uint16_t nbits,
                        const uint16_t hdrmark,         const uint16_t hdrspace,
                        const uint16_t onemark,         const uint16_t onespace,
                        const uint16_t zeromark,        const uint16_t zerospace,
                        const uint16_t footermark,      const uint16_t footerspace,
                        const bool atleast,             const uint8_t tolerance,
                        const int16_t excess,           const bool MSBfirst) {
  return _matchGeneric( data_ptr, NULL,
                        result_ptr, false,
                        remaining, nbits,
                        hdrmark, hdrspace,
                        onemark, onespace,
                        zeromark, zerospace, 
                        footermark, footerspace, 
                        atleast, tolerance,
                        excess, MSBfirst);
}

/// Match & decode a generic/typical constant bit time <= 64bit IR message.
/// The data is stored at result_ptr.
/// @note Values of 0 for hdrmark, hdrspace, footermark, or footerspace mean
///   skip that requirement.
/// @param[in] data_ptr A pointer to where we are at in the capture buffer.
/// @note `data_ptr` is assumed to be pointing to a "Mark", not a "Space".
/// @param[out] result_ptr A ptr to where to start storing the bits we decoded.
/// @param[in] remaining The size of the capture buffer remaining.
/// @param[in] nbits Nr. of data bits we expect.
/// @param[in] hdrmark Nr. of uSeconds for the expected header mark signal.
/// @param[in] hdrspace Nr. of uSeconds for the expected header space signal.
/// @param[in] one Nr. of uSeconds in an expected mark signal for a '1' bit.
/// @param[in] zero Nr. of uSeconds in an expected mark signal for a '0' bit.
/// @param[in] footermark Nr. of uSeconds for the expected footer mark signal.
/// @param[in] footerspace Nr. of uSeconds for the expected footer space/gap
///   signal.
/// @param[in] atleast Is the match on the footerspace a matchAtLeast or
///   matchSpace?
/// @param[in] tolerance Percentage error margin to allow. (Default: kUseDefTol)
/// @param[in] excess Nr. of uSeconds. (Def: kMarkExcess)
/// @param[in] MSBfirst Bit order to save the data in. (Def: true)
///   true is Most Significant Bit First Order, false is Least Significant First
/// @return If successful, how many buffer entries were used. Otherwise 0.
/// @note Parameters one + zero add up to the total time for a bit.
///   e.g. mark(one) + space(zero) is a `1`, mark(zero) + space(one) is a `0`.
uint16_t matchGenericConstBitTime(uint16_t *data_ptr,
                                          uint64_t *result_ptr,
                                          const uint16_t remaining,
                                          const uint16_t nbits,
                                          const uint16_t hdrmark,
                                          const uint16_t hdrspace,
                                          const uint16_t one,
                                          const uint16_t zero,
                                          const uint16_t footermark,
                                          const uint16_t footerspace,
                                          const bool atleast,
                                          const uint8_t tolerance,
                                          const int16_t excess,
                                          const bool MSBfirst) {
  uint16_t offset = 0;
  uint64_t result = 0;
  // If we expect a footermark, then this can be processed like normal.
  if (footermark)
    return _matchGeneric(data_ptr, result_ptr, NULL, true, remaining, nbits,
                         hdrmark, hdrspace, one, zero, zero, one,
                         footermark, footerspace,
                         atleast, tolerance,
                         excess, MSBfirst);
  // Overwise handle like normal, except for the last bit. and no footer.
  uint16_t bits = (nbits > 0) ? nbits - 1 : 0;  // Make sure we don't underflow.
  offset = _matchGeneric( data_ptr, &result, NULL, true, 
                          remaining, bits,
                          hdrmark, hdrspace, one, zero, zero, one, 0, 0,
                          false, tolerance,
                          excess, true);
  if (!offset) return 0;  // Didn't match.
  // Now for the last bit.
  if (remaining <= offset) return 0;  // Not enough buffer.
  result <<= 1;
  bool last_bit = 0;
  // Is the mark a '1' or a `0`?
  if (matchMark(*(data_ptr + offset), one, tolerance)) {  // 1
    last_bit = 1;
    result |= 1;
  } else if (matchMark(*(data_ptr + offset), zero, tolerance)) {  // 0
    last_bit = 0;
  } else {
    return 0;  // It's neither, so fail.
  }
  offset++;
  uint32_t expected_space = (last_bit ? zero : one) + footerspace;
  // If we are not at the end of the buffer, check for at least the expected
  // space value.
  if (remaining > offset) {
    if (atleast) {
      if (!matchAtLeast(*(data_ptr + offset), expected_space, tolerance))
        return false;
    } else {
      if (!matchSpace(*(data_ptr + offset), expected_space, tolerance))
        return false;
    }
    offset++;
  }
  if (!MSBfirst) result = reverseBits(result, nbits);
  *result_ptr = result;
  return offset;
}

/// Match & decode a Manchester Code <= 64bit IR message.
/// The data is stored at result_ptr.
/// @note Values of 0 for hdrmark, hdrspace, footermark, or footerspace mean
///   skip that requirement.
/// @param[in] data_ptr A pointer to where we are at in the capture buffer.
/// @note `data_ptr` is assumed to be pointing to a "Mark", not a "Space".
/// @param[out] result_ptr A ptr to where to start storing the bits we decoded.
/// @param[in] remaining The size of the capture buffer remaining.
/// @param[in] nbits Nr. of data bits we expect.
/// @param[in] hdrmark Nr. of uSeconds for the expected header mark signal.
/// @param[in] hdrspace Nr. of uSeconds for the expected header space signal.
/// @param[in] half_period Nr. of uSeconds for half the clock's period.
///   i.e. 1/2 wavelength
/// @param[in] footermark Nr. of uSeconds for the expected footer mark signal.
/// @param[in] footerspace Nr. of uSeconds for the expected footer space/gap
///   signal.
/// @param[in] atleast Is the match on the footerspace a matchAtLeast or
///   matchSpace?
/// @param[in] tolerance Percentage error margin to allow. (Default: kUseDefTol)
/// @param[in] excess Nr. of uSeconds. (Def: kMarkExcess)
/// @param[in] MSBfirst Bit order to save the data in. (Def: true)
///   true is Most Significant Bit First Order, false is Least Significant First
/// @param[in] GEThomas Use G.E. Thomas (true) or IEEE 802.3 (false) convention?
/// @return If successful, how many buffer entries were used. Otherwise 0.
/// @see https://en.wikipedia.org/wiki/Manchester_code
/// @see http://ww1.microchip.com/downloads/en/AppNotes/Atmel-9164-Manchester-Coding-Basics_Application-Note.pdf
uint16_t matchManchester(const uint16_t *data_ptr,
                                 uint64_t *result_ptr,
                                 const uint16_t remaining,
                                 const uint16_t nbits,
                                 const uint16_t hdrmark,
                                 const uint16_t hdrspace,
                                 const uint16_t half_period,
                                 const uint16_t footermark,
                                 const uint16_t footerspace,
                                 const bool atleast,
                                 const uint8_t tolerance,
                                 const int16_t excess,
                                 const bool MSBfirst,
                                 const bool GEThomas) {
  uint16_t offset = 0;
  uint16_t bank = 0;
  uint16_t entry = 0;

  // Calculate how much remaining buffer is required.
  // Shortest case is nbits. Longest case is 2 * nbits.
  uint16_t min_remaining = nbits;

  if (hdrmark) min_remaining++;
  if (hdrspace) min_remaining++;
  if (footermark) min_remaining++;
  // Don't need to extend for footerspace because it could be the end of message

  // Check if there is enough capture buffer to possibly have the message.
  if (remaining < min_remaining) return 0;  // Nope, so abort.

  // Header
  if (hdrmark) {
    entry = *(data_ptr + offset++);
    if (!hdrspace) {  // If we have no Header Space ...
      // Do we have a data 'mark' half period merged with the header mark?
      if (matchMark(entry, hdrmark + half_period,
                    tolerance)) {
        // Looks like we do.
        bank = entry * kRawTick - hdrmark;
      } else if (!matchMark(entry, hdrmark, tolerance)) {
        return 0;  // It's not a normal header mark, so fail.
      }
    } else if (!matchMark(entry, hdrmark, tolerance)) {
      return 0;  // It's not a normal header mark, so fail.
    }
  }
  if (hdrspace) {
    entry = *(data_ptr + offset++);
    // Check to see if the header space has merged with a data space half period
    if (matchSpace(entry, hdrspace + half_period, tolerance)) {
      // Looks like we do.
      bank = entry * kRawTick - hdrspace;
    } else if (!matchSpace(entry, hdrspace, tolerance)) {
      return 0;  // It's not a normal header space, so fail.
    }
  }

  if (!match(bank / kRawTick, half_period, tolerance)) bank = 0;
  // Data
  uint16_t used = matchManchesterData(data_ptr + offset, result_ptr,
                                      remaining - offset, nbits, half_period,
                                      bank, tolerance, excess, MSBfirst,
                                      GEThomas);
  if (!used) return 0;  // Data did match.
  offset += used;
  // Footer
  if (footermark &&
      !(matchMark(*(data_ptr + offset), footermark + half_period,
                  tolerance) ||
        matchMark(*(data_ptr + offset), footermark, tolerance)))
    return 0;
  offset++;
  // If we have something still to match & haven't reached the end of the buffer
  if (footerspace && offset < remaining) {
    if (atleast) {
      if (!matchAtLeast(*(data_ptr + offset), footerspace, tolerance))
        return 0;
    } else {
      if (!matchSpace(*(data_ptr + offset), footerspace, tolerance) &&
          !matchSpace(*(data_ptr + offset), footerspace + half_period,
                      tolerance))
        return 0;
    }
    offset++;
  }
  return offset;
}

/// Match & decode a Manchester Code data (<= 64bits.
/// @param[in] data_ptr A pointer to where we are at in the capture buffer.
/// @note `data_ptr` is assumed to be pointing to a "Mark", not a "Space".
/// @param[out] result_ptr A ptr to where to start storing the bits we decoded.
/// @param[in] remaining The size of the capture buffer remaining.
/// @param[in] nbits Nr. of data bits we expect.
/// @param[in] half_period Nr. of uSeconds for half the clock's period.
///   i.e. 1/2 wavelength
/// @param[in] tolerance Percentage error margin to allow. (Default: kUseDefTol)
/// @param[in] starting_balance Amount of uSeconds to assume exists prior to
///   the current value pointed too.
/// @param[in] excess Nr. of uSeconds. (Def: kMarkExcess)
/// @param[in] MSBfirst Bit order to save the data in. (Def: true)
///   true is Most Significant Bit First Order, false is Least Significant First
/// @param[in] GEThomas Use G.E. Thomas (true) or IEEE 802.3 (false) convention?
/// @return If successful, how many buffer entries were used. Otherwise 0.
/// @see https://en.wikipedia.org/wiki/Manchester_code
/// @see http://ww1.microchip.com/downloads/en/AppNotes/Atmel-9164-Manchester-Coding-Basics_Application-Note.pdf
/// @todo Clean up and optimise this. It is just "get it working code" atm.
uint16_t matchManchesterData(const uint16_t *data_ptr,
                                     uint64_t *result_ptr,
                                     const uint16_t remaining,
                                     const uint16_t nbits,
                                     const uint16_t half_period,
                                     const uint16_t starting_balance,
                                     const uint8_t tolerance,
                                     const int16_t excess,
                                     const bool MSBfirst,
                                     const bool GEThomas) {
  uint16_t offset = 0;
  uint64_t data = 0;
  uint16_t nr_half_periods = 0;
  const uint16_t expected_half_periods = nbits * 2;
  // Flip the bit if we have a starting balance. ie. Carry over from the header.
  bool currentBit = starting_balance ? !GEThomas : GEThomas;
  const uint16_t raw_half_period = half_period / kRawTick;

  // Calculate how much remaining buffer is required.
  // Shortest case is nbits. Longest case is 2 * nbits.
  uint16_t min_remaining = nbits;

  // Check if there is enough capture buffer to possibly have the message.
  if (remaining < min_remaining) {
    return 0;  // Nope, so abort.
  }

  // Convert to ticks. Optimisation: Saves on math/extra instructions later.
  uint16_t bank = starting_balance / kRawTick;

  // Data
  // Loop through the buffer till we run out of buffer, or nr of half periods.
  // Possible patterns are:
  // short + short = 1 bit (Add the value of the previous bit again)
  // short + long + short = 2 bits (Add the previous bit again, then flip & add)
  // short + long + long + short = 3 bits (add prev, flip & add, flip & add)
  // We can't start with a long.
  //
  // The general approach is thus:
  //   Check we have a short interval, next or in the bank.
  //   If the next timing value is long, act according and reset the bank to
  //     a short balance.
  //   or
  //   If it is short, act accordingly and declare the bank empty.
  //   Repeat.
  while ((offset < remaining || bank) &&
         nr_half_periods < expected_half_periods) {
    // Get the next entry if we haven't anything existing to process.
    if (!bank) bank = *(data_ptr + offset++);
    // Check if we don't have a short interval.
    if (!match(bank, half_period, tolerance)) {
      return 0;  // Not valid.
    }
    // We've succeeded in matching half a period, so count it.
    nr_half_periods++;
    // We've now used up our bank, so refill it with the next item, unless we
    // are at the end of the capture buffer.
    // If we are assume a single half period of "space".
    if (offset < remaining) {
      bank = *(data_ptr + offset++);
    } else if (offset == remaining) {
      bank = raw_half_period;
    } else {
      return 0;  // We are out of buffer, so abort!
    }

    // Shift the data along and add our new bit.
    data <<= 1;
    data |= currentBit;

    // Check if we have a long interval.
    if (match(bank, half_period * 2, tolerance)) {
      // It is, so flip the bit we need to append, and remove a half_period of
      // time from the bank.
      currentBit = !currentBit;
      bank -= raw_half_period;
    } else if (match(bank, half_period, tolerance)) {
      // It is a short interval, so eat up all the time and move on.
      bank = 0;
    } else if (nr_half_periods == expected_half_periods - 1 &&
               matchAtLeast(bank, half_period, tolerance)) {
      // We are at the end of the data & it is a short interval, so eat up all
      // the time and move on.
      bank = 0;
      // Reduce the offset as we are at the end of the data doing a
      // matchAtLeast() because  we could be processing part of a footer.
      offset--;
    } else {
      // The length isn't what we expected (neither long or short), so bail.
      return 0;
    }
    nr_half_periods++;
  }

  // Clean up and process the data.
  if (!MSBfirst) data = reverseBits(data, nbits);
  // Trim the data to size.
  *result_ptr = GETBITS64(data, 0, nbits);
  return offset;
}


