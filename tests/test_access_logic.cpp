// Host unit tests for access_logic.h:  g++ -std=c++11 -Wall -Wextra -o t tests/test_access_logic.cpp && ./t
#include <cstdio>
#include "../rfid_access_control/access_logic.h"

static int failures = 0, total = 0;
#define CHECK(name, cond) do { total++; if (cond) std::printf("PASS  %s\n", name); \
  else { failures++; std::printf("FAIL  %s\n", name); } } while (0)

int main() {
  const CardUid allow[] = {{4, {0xDE, 0xAD, 0xBE, 0xEF}}, {7, {1, 2, 3, 4, 5, 6, 7}}};
  const CardUid good4 = {4, {0xDE, 0xAD, 0xBE, 0xEF}};
  const CardUid good7 = {7, {1, 2, 3, 4, 5, 6, 7}};
  const CardUid bad = {4, {0xDE, 0xAD, 0xBE, 0xEE}};
  const CardUid prefix = {3, {0xDE, 0xAD, 0xBE}};
  const CardUid empty = {0, {0}};
  const CardUid tooLong = {11, {0}};

  CHECK("authorised 4-byte UID", isAuthorized(good4, allow, 2));
  CHECK("authorised 7-byte UID", isAuthorized(good7, allow, 2));
  CHECK("one byte different is refused", !isAuthorized(bad, allow, 2));
  CHECK("UID prefix is refused", !isAuthorized(prefix, allow, 2));
  CHECK("empty UID is refused", !isAuthorized(empty, allow, 2));
  CHECK("invalid length is refused", !uidEquals(tooLong, tooLong));
  CHECK("empty allow-list refuses all", !isAuthorized(good4, allow, 0));

  Lockout l = {0, 0};
  recordResult(l, false, 1000); recordResult(l, false, 1100);
  CHECK("2 failures: not locked", !isLocked(l, 1200));
  recordResult(l, false, 1300);
  CHECK("3 failures: locked", isLocked(l, 1400));
  CHECK("lock expires after 30 s", !isLocked(l, 1300 + LOCKOUT_MS));
  Lockout l2 = {0, 0};
  recordResult(l2, false, 0); recordResult(l2, false, 10); recordResult(l2, true, 20);
  recordResult(l2, false, 30);
  CHECK("success resets the failure count", !isLocked(l2, 40));

  CHECK("same card within 1.5 s is a repeat", isRepeatRead(good4, good4, 2000, 1000));
  CHECK("same card after 1.5 s is a new read", !isRepeatRead(good4, good4, 2600, 1000));
  CHECK("different card is not a repeat", !isRepeatRead(bad, good4, 1100, 1000));

  std::printf("%d/%d tests passed\n", total - failures, total);
  return failures != 0;
}
