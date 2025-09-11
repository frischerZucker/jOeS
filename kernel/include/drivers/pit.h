/*!
    @file pit.h

    @brief Driver for the 8254 Programmable Interrupt Timer (PIT).
    
    Provides an interface for setting frequency and mode of channels of the PIT.

    @author frischerZucker
 */

#ifndef PIT_H
#define PIT_H

#include <stdint.h>

#define PIT_F_REF 1193182
#define PIT_MAX_FREQUENCY PIT_F_REF
#define PIT_MIN_FREQUENCY PIT_F_REF / UINT16_MAX

#define PIT_CHANNEL_0 0x40
#define PIT_CHANNEL_2 0x42

#define PIT_SC_COUNTER_0 0
#define PIT_SC_COUNTER_2 (1 << 7)
#define PIT_SC_READ_BACK ((1 << 7) | (1 << 6))

#define PIT_MODE_INT_ON_TERMINAL_COUNT 0
#define PIT_MODE_HARDWARE_TRIG_ONESHOT (1 << 1)
#define PIT_MODE_RATE_GEN (1 << 2)
#define PIT_MODE_SQUARE_WAVE ((1 << 1) | (1 << 2))
#define PIT_MODE_SOFTWARE_TRIG_STROBE (1 << 3)
#define PIT_MODE_HARDWARE_TRIG_STROBE ((1 << 3) | (1 << 1))

/*!
    @brief Error codes used by the driver.
*/
typedef enum pit_error_codes
{
    PIT_OK = 0,
    PIT_ERROR_FREQUENCY_OUT_OF_BOUNDS
} pit_error_codes;

/*!
    @brief Initializes a channel of the PIT.

    Initializes a channel of the PIT so that it runs at the wanted frequency and mode.
    The frecuency can be somewhere between 18 Hz and 1.19 MHz, otherwise an error is returned.
    Possible modes are:
    - PIT_MODE_INT_ON_TERMINAL_COUNT: Output starts LOW. Starts counting down after reload register is set by software. 
                                      Output changes to HIGH when the terminal count is reached and stays there, until a new mode is set .
    - PIT_MODE_HARDWARE_TRIG_ONESHOT: Same as PIT_MODE_INT_ON_TERMINAL_COUNT, but counting starts when a rising edge is detected at the gate input.
    - PIT_MODE_RATE_GEN: Acts as a frequency devider. Creates a HIGH output, that drops LOW for one input signal cycle.
    - PIT_MODE_SQUARE_WAVE: Like PIT_MODE_RATE_GEN, but the output is fed into a flip flop, to produce an square wave instead of short LOW-pulses.
    - PIT_MODE_SOFTWARE_TRIG_STROBE: Software triggered delay.
    - PIT_MODE_HARDWARE_TRIG_STROBE: Hardware triggered delay.

    @param channel PIT channel to initialize.
    @param frequency Frequency the PIT should run at.
    @param mode Mode the PIT should run in.
    @returns PIT_OK on success, PIT_ERROR_FREQUENCY_OUT_OF_BOUNDS if frequency is too large / small.
*/
pit_error_codes pit_init_channel(uint8_t channel, uint64_t frequency, uint8_t mode);

#endif // PIT_H