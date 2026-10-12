# RFID Access Control with Arduino (RC522)

![Arduino](https://img.shields.io/badge/Arduino-Uno-00979D?logo=arduino&logoColor=white)
![C++](https://img.shields.io/badge/C%2B%2B-sketch-00599C?logo=cplusplus&logoColor=white)
![Tests](https://img.shields.io/badge/logic%20tests-14%20passing-brightgreen)
![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)

A physical access-control prototype: an **MFRC522 (RC522)** reader identifies a badge, the Arduino compares its UID with an allow-list, then opens a servo "door" with a green LED, or refuses with a red LED and a buzzer.

> Academic project, **Innovation** module, Master in AI & Cybersecurity (Faculté des Sciences de Kénitra).
> This repository was **rebuilt in October 2026** from the project report, with a few security improvements added (listed below). The decision logic is unit-tested on a PC (`docs/test-output.txt`); the sketch compiles against stub Arduino headers, and must be flashed to real hardware to be tested end to end.

## How it works

```text
badge ──► RC522 (SPI) ──► read UID ──► repeat read? ──yes──► ignore
                                          │no
                                          ▼
                                   locked out? ──yes──► 1 beep, refuse
                                          │no
                                          ▼
                              UID in allow-list? ──yes──► green LED + servo open 3 s
                                          │no
                                          ▼
                               red LED + beep (3 refusals in a row → 30 s lockout)
```

## Security improvements over the original version

- **Constant-time UID comparison** and no early exit when searching the allow-list
- **Lockout**: 3 refused badges in a row block the reader for 30 s (slows down trying random cards)
- **Debounce**: the same badge read again within 1.5 s is ignored
- UID length checked (4, 7 or 10 bytes); empty or oversized UIDs are refused
- Maximum antenna gain, because reading was sensitive to distance and orientation in the original tests

**Limit:** a UID alone is not a secret; it can be cloned onto "magic" cards. A real deployment would use cards with cryptographic authentication (e.g. MIFARE DESFire EV2/EV3) instead of the UID.

## Wiring (Arduino Uno)

| RC522 | Uno | | Component | Uno pin |
|---|---|---|---|---|
| SDA (SS) | D10 | | Green LED (+220 Ω) | D4 |
| SCK | D13 | | Red LED (+220 Ω) | D5 |
| MOSI | D11 | | Buzzer | D6 |
| MISO | D12 | | Servo signal | D3 |
| RST | D9 | | Servo power | 5 V / GND |
| 3.3V | **3.3V** (not 5 V) | | | |
| GND | GND | | | |

## Usage

1. Install the **MFRC522** library (Arduino IDE ▸ Library Manager).
2. Open `rfid_access_control/rfid_access_control.ino`, upload, open the Serial Monitor at 9600 baud.
3. Present your badges: their UIDs are printed. Put the authorised ones in `ALLOW_LIST`, upload again.

Run the logic tests on a PC:

```bash
g++ -std=c++11 -Wall -Wextra -o test tests/test_access_logic.cpp && ./test
```

## Skills

Embedded programming (Arduino C/C++) · SPI peripherals · RFID / MIFARE · physical access control · unit-testing embedded logic on a host.

## License

MIT. See [LICENSE](LICENSE).
