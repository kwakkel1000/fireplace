#include "aux_globals.h"
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "avr_compat.h"
#include "LCD/HD44780.h"
#include "DHT/dht.h"
#include <util/delay.h>

#define LED_MASK   ((1 << 7) | (1 << 6) | (1 << 5))


int milis = 0,sec = 0;//, dht11_temp=0, dht11_humidity=0;
int dht11_temp = 0;
int dht11_humidity = 0;
dht DHT;



// CTC interrupt for Timer 1
volatile int interval1;
volatile int counter1 = 0;
float sec1 = 0.2;

ISR(TIMER1_COMPA_vect)
{
    PORTD ^= (1 << 5);
    interval1 = (int) (sec1 * 160);
    if (counter1 == interval1)
    {
        counter1 = 0;
        if (DHT.read() == 0)
        {
            dht11_temp = DHT.temperature;
            dht11_humidity = DHT.humidity;
            PORTD ^= (1 << 7);
        }
        PORTD ^= (1 << 6);
    }
    counter1++;
}


/*
 * Main entry point
 */
int main(void) {
    DDRD = LED_MASK;

    HD44780 lcd;

    lcd.lcd_init();                                                         // init the LCD screen
    lcd.lcd_clrscr();                                                       // initial screen cleanup
    lcd.lcd_home();
    char szDisp[255] = {0};
    sprintf(szDisp,"booting\n");
    lcd.lcd_string(szDisp);


    // setup timer 1 for CTC
    TCCR1B |= (1 << WGM12); // MAX counter = value OCR1A (Mode 4 / CTC)

    //TCCR1B |= 0x01; // prescaler = 1;      // TCCR1B |= (1 << CS10);
    //TCCR1B |= 0x02; // prescaler = 8;      // TCCR1B |= (1 << CS11);
    TCCR1B |= 0x03; // prescaler = 64;   // TCCR1B |= (1 << CS11) | (1 << CS10);
    //TCCR1B |= 0x04; // prescaler = 256;   // TCCR1B |= (1 << CS12);
    //TCCR1B |= 0x05; // prescaler = 1024;   // TCCR1B |= (1 << CS12) | (1 << CS10);

    // setup period
    // when OCR1A = 2400 and prescaler = 8, TIMER1_COMPA_vect interrupt is triggered 1000 times/sec
    // because: 12000000 / 8 / 2400 = 1000;
    OCR1A = 25000; // OCR1A is 16 bit, so max 65535

    // trigger interrupt when Timer1 == OCR1A
    TIMSK1 = 1 << OCIE1A;


    // start timer and interrupts
    sei();


    sprintf(szDisp,"timers start\n");
    lcd.lcd_string(szDisp);
    fcpu_delay_ms(200);
    lcd.lcd_clrscr();
    while (1)
    {
        fcpu_delay_ms(500);
        char szDisp[255] = {0};
        sprintf(szDisp,"T:%d H:%d\n", dht11_temp, dht11_humidity);
        lcd.lcd_home();
        lcd.lcd_string(szDisp);
    }
    return (0);
}
