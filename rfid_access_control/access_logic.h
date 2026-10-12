// Pure access-control logic, no Arduino dependency, so it can be unit-tested on a PC.
#ifndef ACCESS_LOGIC_H
#define ACCESS_LOGIC_H

#include <stdint.h>
#include <stddef.h>

#define MAX_UID_LEN 10   // MIFARE UIDs are 4, 7 or 10 bytes

struct CardUid {
  uint8_t len;
  uint8_t bytes[MAX_UID_LEN];
};

// Constant-time comparison of two UIDs (same work whatever byte differs).
inline bool uidEquals(const CardUid &a, const CardUid &b) {
  if (a.len != b.len || a.len == 0 || a.len > MAX_UID_LEN) return false;
  uint8_t diff = 0;
  for (uint8_t i = 0; i < a.len; i++) diff |= (uint8_t)(a.bytes[i] ^ b.bytes[i]);
  return diff == 0;
}

inline bool isAuthorized(const CardUid &uid, const CardUid *allowList, size_t count) {
  bool found = false;
  for (size_t i = 0; i < count; i++) found |= uidEquals(uid, allowList[i]);  // no early exit
  return found;
}

// Lockout after too many refused cards in a row, to slow down brute force with random cards.
struct Lockout {
  uint8_t failures;
  unsigned long lockedUntil;  // millis()
};

const uint8_t MAX_FAILURES = 3;
const unsigned long LOCKOUT_MS = 30000UL;

inline bool isLocked(const Lockout &l, unsigned long now) { return now < l.lockedUntil; }

inline void recordResult(Lockout &l, bool granted, unsigned long now) {
  if (granted) { l.failures = 0; return; }
  if (++l.failures >= MAX_FAILURES) { l.failures = 0; l.lockedUntil = now + LOCKOUT_MS; }
}

// Ignore the same card read again within this window (the reader sees it several times).
const unsigned long DEBOUNCE_MS = 1500UL;

inline bool isRepeatRead(const CardUid &uid, const CardUid &last, unsigned long now, unsigned long lastTime) {
  return uidEquals(uid, last) && (now - lastTime) < DEBOUNCE_MS;
}

#endif
