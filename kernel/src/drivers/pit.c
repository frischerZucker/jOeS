#include "drivers/pit.h"

#include "stdio.h"

#include "cpu/port_io.h"
#include "logging.h"

#define PIT_COMMAND 0x43

#define PIT_RW_LOW_HIGH ((1 << 5) | (1 << 4))
#define PIT_RW_LOW_ONLY (1 << 4)
#define PIT_RW_HIGH_ONLY (1 << 5)
#define PIT_RW_COUNTER_LATCH 0

#define PIT_BCD_BINARY 0
#define PIT_BCD_BCD 1

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
pit_error_codes pit_init_channel(uint8_t channel, uint64_t frequency, uint8_t mode)
{
    asm("cli");

    if (frequency < PIT_MIN_FREQUENCY || frequency > PIT_MAX_FREQUENCY)
    {
        LOG_ERROR("Frequency out of bounds! (f=%dHz)", frequency);
        return PIT_ERROR_FREQUENCY_OUT_OF_BOUNDS;
    }
    
    // When using a 16 bit integer a frequency of 18 Hz would cause an overflow and therefor a wrong divisor.
    // By using a 32 bit integer there's no overflow and the divisor can be limited to UINT16_MAX. 
    uint32_t divisor = PIT_F_REF / frequency;
    if (divisor > UINT16_MAX)
    {
        divisor = UINT16_MAX;
    }
    
    // Send the control word.
    uint8_t select_counter = (channel == PIT_CHANNEL_0) ? PIT_SC_COUNTER_0 : PIT_SC_COUNTER_2;
    port_write_byte(PIT_COMMAND, select_counter | PIT_RW_LOW_HIGH | mode | PIT_BCD_BINARY);
    // Set the frequency.
    port_write_byte(channel, divisor & 0x00ff);
    port_write_byte(channel, ((divisor & 0xff00) >> 8));

    asm("sti");

    return PIT_OK;
}