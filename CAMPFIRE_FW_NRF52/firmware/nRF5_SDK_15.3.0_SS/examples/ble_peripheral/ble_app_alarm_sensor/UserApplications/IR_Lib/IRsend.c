
#include "IRsend.h"
#include "stdbool.h"


/// Generic method for sending data that is common to most protocols.
/// Will send leading or trailing 0's if the nbits is larger than the number
/// of bits in data.
/// @param[in] onemark Nr. of usecs for the led to be pulsed for a '1' bit.
/// @param[in] onespace Nr. of usecs for the led to be fully off for a '1' bit.
/// @param[in] zeromark Nr. of usecs for the led to be pulsed for a '0' bit.
/// @param[in] zerospace Nr. of usecs for the led to be fully off for a '0' bit.
/// @param[in] data The data to be transmitted.
/// @param[in] nbits Nr. of bits of data to be sent.
/// @param[in] MSBfirst Flag for bit transmission order.
///   Defaults to MSB->LSB order.
uint16_t dataToRaw(uint16_t onemark, uint32_t onespace, uint16_t zeromark,
                      uint32_t zerospace, uint64_t data, uint16_t nbits,
                      bool MSBfirst, 
                      int16_t *IR_raw) {
  uint8_t idx = 0;
  if (nbits == 0)  // If we are asked to send nothing, just return.
    return 0;
  if (MSBfirst) {  // Send the MSB first.
    // Send 0's until we get down to a bit size we can actually manage.
    while (nbits > sizeof(data) * 8) {
      IR_raw[idx++] = zeromark;
      IR_raw[idx++] = -zerospace;
      nbits--;
    }
    // Send the supplied data.
    for (uint64_t mask = 1ULL << (nbits - 1); mask; mask >>= 1)
      if (data & mask) {  // Send a 1
        IR_raw[idx++] = onemark;
        IR_raw[idx++] = -onespace;
      } else {  // Send a 0
        IR_raw[idx++] = zeromark;
        IR_raw[idx++] = -zerospace;
      }
  } else {  // Send the Least Significant Bit (LSB) first / MSB last.
    for (uint16_t bit = 0; bit < nbits; bit++, data >>= 1)
      if (data & 1) {  // Send a 1
        IR_raw[idx++] = onemark;
        IR_raw[idx++] = -onespace;
      } else {  // Send a 0
        IR_raw[idx++] = zeromark;
        IR_raw[idx++] = -zerospace;
      }
  }
  return idx;
}

/// Generic method for sending simple protocol messages.
/// @param[in] headermark Nr. of usecs for the led to be pulsed for the header
///   mark. A value of 0 means no header mark.
/// @param[in] headerspace Nr. of usecs for the led to be off after the header
///   mark. A value of 0 means no header space.
/// @param[in] onemark Nr. of usecs for the led to be pulsed for a '1' bit.
/// @param[in] onespace Nr. of usecs for the led to be fully off for a '1' bit.
/// @param[in] zeromark Nr. of usecs for the led to be pulsed for a '0' bit.
/// @param[in] zerospace Nr. of usecs for the led to be fully off for a '0' bit.
/// @param[in] footermark Nr. of usecs for the led to be pulsed for the footer
///   mark. A value of 0 means no footer mark.
/// @param[in] gap Nr. of usecs for the led to be off after the footer mark.
///   This is effectively the gap between messages.
///   A value of 0 means no gap space.
/// @param[in] dataptr Pointer to the data to be transmitted.
/// @param[in] nbytes Nr. of bytes of data to be sent.
/// @param[in] frequency The frequency we want to modulate at. (Hz/kHz)
/// @param[in] MSBfirst Flag for bit transmission order.
///   Defaults to MSB->LSB order.
/// @param[in] repeat Nr. of extra times the message will be sent.
///   e.g. 0 = 1 message sent, 1 = 1 initial + 1 repeat = 2 messages
/// @param[in] dutycycle Percentage duty cycle of the LED.
///   e.g. 25 = 25% = 1/4 on, 3/4 off.
///   If you are not sure, try 50 percent.
/// @note Assumes a frequency < 1000 means kHz otherwise it is in Hz.
///   Most common value is 38000 or 38, for 38kHz.
void sendGeneric_8(  const uint16_t headermark,  const uint16_t headerspace,
                      const uint16_t onemark,     const uint16_t onespace,
                      const uint16_t zeromark,    const uint16_t zerospace,
                      const uint16_t footermark,  const uint16_t gap,
                      const uint8_t *dataptr,     const uint16_t nbytes,
                      const uint16_t frequency,   const bool     MSBfirst,
                      const uint16_t repeat,      const uint8_t  dutycycle,
                      int16_t *IR_raw) {
  // Setup
  uint16_t idx = 0;

  // We always send a message, even for repeat=0, hence '<= repeat'.
  for (uint16_t r = 0; r <= repeat; r++) {
    // Header
    if (headermark)  IR_raw[idx++] = headermark;
    if (headerspace) IR_raw[idx++] = -headerspace;

    // Data
    for (uint16_t i = 0; i < nbytes; i++)
      idx += dataToRaw(onemark, onespace, zeromark, zerospace, *(dataptr + i), 8, MSBfirst, &IR_raw[idx]);

    // Footer
    if (footermark){
      IR_raw[idx++] = footermark;
    }
    if(gap)
      IR_raw[idx++] = -gap;
  }
}


/// Generic method for sending simple protocol messages.
/// Will send leading or trailing 0's if the nbits is larger than the number
/// of bits in data.
/// @param[in] headermark Nr. of usecs for the led to be pulsed for the header
///   mark. A value of 0 means no header mark.
/// @param[in] headerspace Nr. of usecs for the led to be off after the header
///   mark. A value of 0 means no header space.
/// @param[in] onemark Nr. of usecs for the led to be pulsed for a '1' bit.
/// @param[in] onespace Nr. of usecs for the led to be fully off for a '1' bit.
/// @param[in] zeromark Nr. of usecs for the led to be pulsed for a '0' bit.
/// @param[in] zerospace Nr. of usecs for the led to be fully off for a '0' bit.
/// @param[in] footermark Nr. of usecs for the led to be pulsed for the footer
///   mark. A value of 0 means no footer mark.
/// @param[in] gap Nr. of usecs for the led to be off after the footer mark.
///   This is effectively the gap between messages.
///   A value of 0 means no gap space.
/// @param[in] mesgtime Min. nr. of usecs a single message needs to be.
///   This is effectively the min. total length of a single message.
/// @param[in] data The data to be transmitted.
/// @param[in] nbits Nr. of bits of data to be sent.
/// @param[in] frequency The frequency we want to modulate at. (Hz/kHz)
/// @param[in] MSBfirst Flag for bit transmission order.
///   Defaults to MSB->LSB order.
/// @param[in] repeat Nr. of extra times the message will be sent.
///   e.g. 0 = 1 message sent, 1 = 1 initial + 1 repeat = 2 messages
/// @param[in] dutycycle Percentage duty cycle of the LED.
///   e.g. 25 = 25% = 1/4 on, 3/4 off.
///   If you are not sure, try 50 percent.
/// @note Assumes a frequency < 1000 means kHz otherwise it is in Hz.
///   Most common value is 38000 or 38, for 38kHz.
void sendGeneric_64( const uint16_t headermark,  const uint32_t headerspace,
                      const uint16_t onemark,     const uint32_t onespace,
                      const uint16_t zeromark,    const uint32_t zerospace,
                      const uint16_t footermark,  const uint32_t gap,
                      const uint32_t mesgtime,    const uint64_t data,
                      const uint16_t nbits,       const uint16_t frequency,
                      const bool MSBfirst,        const uint16_t repeat,
                      const uint8_t dutycycle,
                      int16_t *IR_raw){

  // Setup
  uint16_t idx = 0;

  // We always send a message, even for repeat=0, hence '<= repeat'.
  for (uint16_t r = 0; r <= repeat; r++) {

    // Header
    if (headermark)  IR_raw[idx++] = headermark;
    if (headerspace) IR_raw[idx++] = -headerspace;

    // Data
    idx += dataToRaw(onemark, onespace, zeromark, zerospace, data, nbits, MSBfirst, &IR_raw[idx]);

    // Footer
    if (footermark){
      IR_raw[idx++] = footermark;
    }
    if(gap)
      IR_raw[idx++] = -gap;
  }

}

/// Generic method for sending Manchester code data.
/// Will send leading or trailing 0's if the nbits is larger than the number
/// of bits in data.
/// @param[in] half_period Nr. of uSeconds for half the clock's period.
///   (1/2 wavelength)
/// @param[in] data The data to be transmitted.
/// @param[in] nbits Nr. of bits of data to be sent.
/// @param[in] MSBfirst Flag for bit transmission order.
///   Defaults to MSB->LSB order.
/// @param[in] GEThomas Use G.E. Thomas (true/default) or IEEE 802.3 (false).
void sendManchesterData(const uint16_t half_period,
                                const uint64_t data,
                                const uint16_t nbits, const bool MSBfirst,
                                const bool GEThomas) {
  if (nbits == 0) return;  // Nothing to send.
  uint16_t bits = nbits;
  uint64_t copy = (GEThomas) ? data : ~data;

  if (MSBfirst) {  // Send the MSB first.
    // Send 0's until we get down to a bit size we can actually manage.
    if (bits > (sizeof(data) * 8)) {
      sendManchesterData(half_period, 0ULL, bits - sizeof(data) * 8, MSBfirst,
                         GEThomas);
      bits = sizeof(data) * 8;
    }
    // Send the supplied data.
    for (uint64_t mask = 1ULL << (bits - 1); mask; mask >>= 1)
      if (copy & mask) {
        // mark(half_period);
        // space(half_period);
      } else {
        // space(half_period);
        // mark(half_period);
      }
  } else {  // Send the Least Significant Bit (LSB) first / MSB last.
    for (bits = 0; bits < nbits; bits++, copy >>= 1)
      if (copy & 1) {
        // mark(half_period);
        // space(half_period);
      } else {
        // space(half_period);
        // mark(half_period);
      }
  }
}

/// Generic method for sending Manchester code messages.
/// Will send leading or trailing 0's if the nbits is larger than the number
/// @param[in] headermark Nr. of usecs for the led to be pulsed for the header
///   mark. A value of 0 means no header mark.
/// @param[in] headerspace Nr. of usecs for the led to be off after the header
///   mark. A value of 0 means no header space.
/// @param[in] half_period Nr. of uSeconds for half the clock's period.
///   (1/2 wavelength)
/// @param[in] footermark Nr. of usecs for the led to be pulsed for the footer
///   mark. A value of 0 means no footer mark.
/// @param[in] gap Min. nr. of usecs for the led to be off after the footer
///   mark. This is effectively the absolute minimum gap between messages.
/// @param[in] data The data to be transmitted.
/// @param[in] nbits Nr. of bits of data to be sent.
/// @param[in] frequency The frequency we want to modulate at. (Hz/kHz)
/// @param[in] MSBfirst Flag for bit transmission order.
///   Defaults to MSB->LSB order.
/// @param[in] repeat Nr. of extra times the message will be sent.
///   e.g. 0 = 1 message sent, 1 = 1 initial + 1 repeat = 2 messages
/// @param[in] dutycycle Percentage duty cycle of the LED.
///   e.g. 25 = 25% = 1/4 on, 3/4 off.
///   If you are not sure, try 50 percent.
/// @param[in] GEThomas Use G.E. Thomas (true/default) or IEEE 802.3 (false).
/// @note Assumes a frequency < 1000 means kHz otherwise it is in Hz.
///   Most common value is 38000 or 38, for 38kHz.
void sendManchester(const uint16_t headermark,
                            const uint32_t headerspace,
                            const uint16_t half_period,
                            const uint16_t footermark, const uint32_t gap,
                            const uint64_t data, const uint16_t nbits,
                            const uint16_t frequency, const bool MSBfirst,
                            const uint16_t repeat, const uint8_t dutycycle,
                            const bool GEThomas) {
  // Setup


  // We always send a message, even for repeat=0, hence '<= repeat'.
  for (uint16_t r = 0; r <= repeat; r++) {
    // Header
    // if (headermark) mark(headermark);
    // if (headerspace) space(headerspace);
    // Data
    sendManchesterData(half_period, data, nbits, MSBfirst, GEThomas);
    // Footer
    // if (footermark) mark(footermark);
    // if (gap) space(gap);
  }
}
