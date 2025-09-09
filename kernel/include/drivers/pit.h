#ifndef PIT_H
#define PIT_H

#include <stdint.h>

#define PIT_F_REF 1193182
#define PIT_MAX_FREQUENCY PIT_F_REF
#define PIT_MIN_FREQUENCY PIT_F_REF / UINT16_MAX

#define PIT_COMMAND 0x43
#define PIT_CHANNEL_0 0x40
#define PIT_CHANNEL_2 0x42

#define PIT_SC_COUNTER_0 0
#define PIT_SC_COUNTER_2 (1 << 7)
#define PIT_SC_READ_BACK ((1 << 7) | (1 << 6))

#define PIT_RW_LOW_HIGH ((1 << 5) | (1 << 4))
#define PIT_RW_LOW_ONLY (1 << 4)
#define PIT_RW_HIGH_ONLY (1 << 5)
#define PIT_RW_COUNTER_LATCH 0

#define PIT_MODE_INT_ON_TERMINAL_COUNT 0
#define PIT_MODE_HARDWARE_TRIG_ONESHOT (1 << 1)
#define PIT_MODE_RATE_GEN (1 << 2)
#define PIT_MODE_SQUARE_WAVE ((1 << 1) | (1 << 2))
#define PIT_MODE_SOFTWARE_TRIG_STROBE (1 << 3)
#define PIT_MODE_HARDWARE_TRIG_STROBE ((1 << 3) | (1 << 1))

#define PIT_BCD_BINARY 0
#define PIT_BCD_BCD 1

#define PIT_ERROR_FREQUENCY_OUT_OF_BOUNDS 1

uint8_t pit_init_channel(uint8_t channel, uint64_t frequency, uint8_t mode);

#endif