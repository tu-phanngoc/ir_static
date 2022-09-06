

#include "checksum.h"
#include "IRutils.h"

/// Verify the checksum is valid for a given state.
/// @param[in] state The array to verify the checksum of.
/// @param[in] length The length of the state array.
/// @return true, if the state has a valid checksum. Otherwise, false.
bool validChecksum_sumBytes(const uint8_t *state, const uint16_t length, const uint8_t init) {
  if (length < 2) return false;  // 1 byte of data can't have a checksum.
  return (state[length - 1] == sumBytes(state, length - 1, init));
}


/// Calculate the checksum for a given state.
/// @param[in] state The value to calc the checksum of.
/// @param[in] length The size/length of the state.
/// @return The calculated checksum value.
uint8_t calcChecksum_sumBytes(const uint8_t *state, const uint16_t length, const uint8_t init) {
  return sumBytes(state, length - 1, init);
}

