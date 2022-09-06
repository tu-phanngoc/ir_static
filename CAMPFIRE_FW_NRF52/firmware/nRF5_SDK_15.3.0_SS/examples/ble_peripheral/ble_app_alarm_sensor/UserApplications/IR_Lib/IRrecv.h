

#ifndef IRRECV_H_
#define IRRECV_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "IRcommon.h"



// Constants
#define kHeader             2             // Usual nr. of header entries.
#define kFooter             2             // Usual nr. of footer (stop bits) entries.
#define kStartOffset        1             // Usual rawbuf entry to start from.
#define MS_TO_USEC(x)       ((x) * 1000U) // Convert milli-Seconds to micro-Seconds.
// Marks tend to be 100us too long, and spaces 100us too short
// when received due to sensor lag.
#define kMarkExcess				50		
#define kRawBuf				100		  // Default length of raw capture buffer
#define kRepeat				0xffffffffffffffff		
// Default min size of reported UNKNOWN messages.
#define kUnknownThreshold				6		

// receiver states
#define kIdleState				2		
#define kMarkState				3		
#define kSpaceState				4		
#define kStopState				5		
#define kTolerance				50		   // default percent tolerance in measurements.
#define kUseDefTol				255		  // Indicate to use the class default tolerance.
#define kRawTick				2		     // Capture tick to uSec factor.
#define RAWTICK kRawTick  // Deprecated. For legacy user code support only.
// How long (ms) before we give up wait for more data?
// Don't exceed kMaxTimeoutMs without a good reason.
// That is the capture buffers maximum value size. (UINT16_MAX / kRawTick)
// Typically messages/protocols tend to repeat around the 100ms timeframe,
// thus we should timeout before that to give us some time to try to decode
// before we need to start capturing a possible new message.
// Typically 15ms suits most applications. However, some protocols demand a
// higher value. e.g. 90ms for XMP-1 and some aircon units.
#define kTimeoutMs				15		  // In MilliSeconds.
#define TIMEOUT_MS kTimeoutMs   // For legacy documentation.
#define kMaxTimeoutMs				kRawTick * (UINT16_MAX / MS_TO_USEC(1))		

// Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
#define kFnvPrime32				16777619UL		
#define kFnvBasis32				2166136261UL		

// Which of the ESP32 timers to use by default. (0-3)
#define kDefaultESP32Timer				3		

// Just define something (a uint64_t)
#define kStateSizeMax				80
#define kStateBitsMax				kStateSizeMax * 8

// Types

/// Information for the interrupt handler
typedef struct {
  uint8_t recvpin;   // pin for IR data from detector
  uint8_t rcvstate;  // state machine
  uint16_t timer;    // state timer, counts 50uS ticks.
  uint16_t bufsize;  // max. nr. of entries in the capture buffer.
  uint16_t *rawbuf;  // raw data
  // uint16_t is used for rawlen as it saves 3 bytes of iram in the interrupt
  // handler. Don't ask why, I don't know. It just does.
  uint16_t rawlen;   // counter of entries in rawbuf.
  uint8_t overflow;  // Buffer overflow indicator.
  uint8_t timeout;   // Nr. of milliSeconds before we give up.
} irparams_t;

/// Results from a data match
typedef struct {
  bool success;   // Was the match successful?
  uint64_t data;  // The data found.
  uint16_t used;  // How many buffer positions were used.
} match_result_t;


/// Results returned from the decoder
typedef struct {
  decode_type_t decode_type;  // NEC, SONY, RC5, UNKNOWN
  // value, address, & command are all mutually exclusive with state.
  // i.e. They MUST NOT be used at the same time as state, so we can use a union
  // structure to save us a handful of valuable bytes of memory.
  union {
    struct {
      uint64_t value;    // Decoded value
      uint32_t address;  // Decoded device address.
      uint32_t command;  // Decoded command.
    };
    uint8_t state[kStateSizeMax];  // Multi-byte results.
  };
  uint16_t bits;                        // Number of bits in decoded value
  uint16_t rawbuf[kStateBitsMax];     // Raw intervals in .5 us ticks
  uint16_t rawlen;                      // Number of records in rawbuf.
  bool overflow;
  bool repeat;  // Is the result a repeat code?
} decode_results;



extern decode_results gDecodeResult;

void initDecodeData(int16_t *src, uint16_t length);
void getRawBuf(int16_t *src, decode_results *dst, uint16_t length);
bool match(const uint32_t measured, const uint32_t desired, const uint8_t tolerance);
bool matchMark(const uint32_t measured, const uint32_t desired, const uint8_t tolerance);
bool matchSpace(const uint32_t measured, const uint32_t desired, const uint8_t tolerance);
uint8_t _validTolerance(const uint8_t percentage);
uint32_t ticksLow(const uint32_t usecs, const uint8_t tolerance);
uint32_t ticksHigh(const uint32_t usecs, const uint8_t tolerance);
bool matchAtLeast(const uint32_t measured, const uint32_t desired, const uint8_t tolerance);
uint16_t _matchGeneric( uint16_t *data_ptr,           uint64_t *result_bits_ptr,
                        uint8_t *result_bytes_ptr,    const bool use_bits,
                        const uint16_t remaining,     const uint16_t nbits,
                        const uint16_t hdrmark,       const uint32_t hdrspace,
                        const uint16_t onemark,       const uint32_t onespace,
                        const uint16_t zeromark,      const uint32_t zerospace,
                        const uint16_t footermark,    const uint32_t footerspace,
                        const bool atleast,           const uint8_t tolerance,
                        const int16_t excess,         const bool MSBfirst);

match_result_t matchData( uint16_t *data_ptr,           const uint16_t nbits,
                          const uint16_t onemark,       const uint16_t onespace,
                          const uint16_t zeromark,      const uint16_t zerospace,
                          const uint8_t tolerance,      const int16_t excess,
                          const bool MSBfirst,          const bool expectlastspace);

uint16_t matchBytes(uint16_t *data_ptr,  uint8_t *result_ptr,
                    const uint16_t remaining,     const uint16_t nbytes,
                    const uint16_t onemark,       const uint16_t onespace,
                    const uint16_t zeromark,      const uint16_t zerospace,
                    const uint8_t tolerance,      const int16_t excess,
                    const bool MSBfirst,          const bool expectlastspace);

uint16_t matchGeneric_64( uint16_t *data_ptr,         uint64_t *result_ptr,
                          const uint16_t remaining,   const uint16_t nbits,
                          const uint16_t hdrmark,     const uint16_t hdrspace,
                          const uint16_t onemark,     const uint16_t onespace,
                          const uint16_t zeromark,    const uint16_t zerospace,
                          const uint16_t footermark,  const uint16_t footerspace,
                          const bool atleast,         const uint8_t tolerance,
                          const int16_t excess,       const bool MSBfirst);

uint16_t matchGeneric_8(uint16_t *data_ptr,             uint8_t *result_ptr,
                        const uint16_t remaining,       const uint16_t nbits,
                        const uint16_t hdrmark,         const uint16_t hdrspace,
                        const uint16_t onemark,         const uint16_t onespace,
                        const uint16_t zeromark,        const uint16_t zerospace,
                        const uint16_t footermark,      const uint16_t footerspace,
                        const bool atleast,             const uint8_t tolerance,
                        const int16_t excess,           const bool MSBfirst);

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
                                  const bool MSBfirst);
uint16_t matchManchesterData(const uint16_t *data_ptr,
                              uint64_t *result_ptr,
                              const uint16_t remaining,
                              const uint16_t nbits,
                              const uint16_t half_period,
                              const uint16_t starting_balance,
                              const uint8_t tolerance,
                              const int16_t excess,
                              const bool MSBfirst,
                              const bool GEThomas);
uint16_t matchManchester(const uint16_t *data_ptr,
                          uint64_t *result_ptr,
                          const uint16_t remaining,
                          const uint16_t nbits,
                          const uint16_t hdrmark,
                          const uint16_t hdrspace,
                          const uint16_t clock_period,
                          const uint16_t footermark,
                          const uint16_t footerspace,
                          const bool atleast,
                          const uint8_t tolerance,
                          const int16_t excess,
                          const bool MSBfirst,
                          const bool GEThomas);

#endif  // IRRECV_H_
