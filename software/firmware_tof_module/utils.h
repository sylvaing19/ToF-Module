#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

#define PIN_DEBUG_A 6
#define PIN_DEBUG_B 7
#define PIN_DEBUG_C 8
#define PIN_DEBUG_D 9

static long read_vcc()
{
#ifdef __AVR_ATmega328P__
    long result; // Read 1.1V reference against AVcc
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
    delay(2); // Wait for Vref to settle
    ADCSRA |= _BV(ADSC); // Convert
    while (bit_is_set(ADCSRA, ADSC));
    result = ADCL;
    result |= ADCH << 8;
    result = 1126400L / result; // Back-calculate AVcc in mV
    return result;
#else
    return 3300;
#endif
}

static int free_ram()
{
    extern int __heap_start, *__brkval;
    int v;
    return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

static bool check_buffer_intersect(size_t b1_start, size_t b1_size,
    size_t b2_start, size_t b2_size)
{
    if (b1_start < b2_start) {
        return b1_start + b1_size > b2_start;
    }
    else if (b1_start > b2_start) {
        return b1_start < b2_start + b2_size;
    }
    else {
        return b1_size > 0 && b2_size > 0;
    }
}

static uint32_t baudrate(uint8_t stored_baudrate)
{
    return 2000000 / ((uint32_t)stored_baudrate + 1);
}

#endif // !UTILS_H
