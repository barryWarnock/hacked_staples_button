#define F_CPU 1000000UL
#define true 1
#define false 0

#include "sound.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

void setup_pwm() {
    //set up timer1 
    TCCR1 = 1 << CTC1 | //clear on compare
        1 << CS10; //no prescaler

    PLLCSR = 1 << PCKE | 1 << PLLE; //use the PLL clock (64Mhz)

    //general timer/counter 1 control register
    GTCCR = 1 << PWM1B | //enable PWM B
        1 << COM1B0; //clear OC1B on OCR1B match, set on 0 (stay on for OCR1B/255), output an inverted signal on another pin

    DDRB |= 1 << PB4 | 1 << PB3; //set OC1B to output along with its inverse

    // Set up Timer/Counter0 for 8kHz interrupt to output samples.
    //set up timer 0 to switch the samples
    TCCR0A = 1 << WGM01;// clear on compare
    TCCR0B = 1 << CS01; // prescale the 8Mhz clock down to 1Mhz
    //OCR0A = 124; //clear every 124 clock ticks, 1M/124 = 8k
    OCR0A = 50;

    ADCSRA = ADCSRA & ~(1<<ADEN);           // Disable ADC to save power
    sei(); //enable interrupts
    TIMSK = 1 << OCIE0A; //enable output compare interupt
}

void finish() {
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_bod_disable();
    sleep_mode();
}

volatile char stop = false;

int main(void) {
    setup_pwm();

    while(!stop) {};

    finish();

    return 0;
}

volatile int count = 0;

ISR(TIM0_COMPA_vect) {
    OCR1B = pgm_read_byte(&sound[count]);
    count++;
    if (count == sound_len) {
        stop = true;
    }
}
