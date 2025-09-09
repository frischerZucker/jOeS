#include "pit.h"

#include "stdio.h"

#include "port_io.h"

/*
    Initializes a channel of the PIT.

    Initializes a channel of the PIT so that ir runs at the desired frequency in the desired mode.

    @param channel PIT channel to initialize.
    @param frequency Frequency the PIT should run at.
    @param mode Mode the PIT should run in.
    @returns 0 if success, PIT_ERROR_FREQUENCY_OUT_OF_BOUNDS (1) if frequency is too large / small.
*/
uint8_t pit_init_channel(uint8_t channel, uint64_t frequency, uint8_t mode)
{
    asm("cli");

    if (frequency < PIT_MIN_FREQUENCY || frequency > PIT_MAX_FREQUENCY)
    {
        printf("PIT: Frequency out of bounds! (f=%dHz)\n", frequency);
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

    return 0;
}