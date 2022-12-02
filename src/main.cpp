#include <avr/interrupt.h>
#include <Arduino.h>

volatile uint16_t c_timer;
volatile uint16_t f_time;
volatile uint16_t p_time;
volatile char flag = 0;

uint16_t freq;
uint16_t pw;

int main() {

    init();
    Serial.begin(9600);

    TCCR1A = TCCR1B = 0;                            // reset timer 1
    TIMSK1 |= (1 << ICIE1);

    sei();


    while (1) {
       Serial.print("F: ");
       Serial.println(15900000 / f_time);
       delay(1000);
    }



}

/*
 * Fang bei erster Flanke das Zählen an
 * Zweite Flanke -> Counter ist Frequenz (Formel)
 * Reset Timer
 *
 * Fang bei erster Flanke das Zählen an und Wechsel Flanke
 * Zweite Flanke -> Counter ist Tastgrad (Formel)
 * Reset Timer
 *
 *
 */

ISR(TIMER1_CAPT_vect) {
    c_timer = ICR1;
    if(c_timer == 0) {
        TCCR1B |= (1 << CS10) | (1 << ICES1);
    }
    else if(TCCR1B & (1 << ICES1)) {
        p_time = c_timer;
        TCCR1B &= ~(1 << ICES1);
        TIFR1 |= (1 << ICF1);
    }
    else {
        TCCR1B &= ~(1 << CS10);
        TCNT1 = 0;
        f_time = c_timer;
    }
}

/**
* #include <avr/interrupt.h>
#include <Arduino.h>

volatile uint16_t current_count[17][2];
volatile uint8_t cnt = 0;

uint16_t freq;
uint16_t pw;

int main() {

    init();
    Serial.begin(9600);

    TCCR1A = TCCR1B = 0;                            // reset timer 1
    TCCR1B |= (1 << CS00);                          // Prescale 1
//    TCCR1C |= (1 << ICES1);    //   // | (1 <<ICNC) = Noise Canceler
    TIMSK1 |= (1 << ICIE1);

    sei();

    while (1) {

        if(cnt == 4) {
            cli();
            freq = 0;
            pw = 0;
            for (int i = 0; i < 16; ++i) {
                freq += (current_count[i][0] - current_count[i+1][0]);
                pw += (current_count[i][0] - current_count[i][1]);
                Serial.print("Rising ");
                Serial.print(i);
                Serial.print(": ");
                Serial.println(current_count[i][0]);
                Serial.print("Falling ");
                Serial.print(i);
                Serial.print(": ");
                Serial.println(current_count[i][1]);
            }
            Serial.print("T on 0: ");
            Serial.println(current_count[0][0] - current_count[0][1]);

            Serial.print("T on 1: ");
            Serial.println(current_count[1][0] - current_count[1][1]);

            Serial.print("T0 0 - 1: ");
            Serial.println(current_count[1][0] - current_count[0][0]);

            Serial.print("T1 0 - 1: ");
            Serial.println(current_count[1][1] - current_count[0][1]);

            Serial.print("Freq: ");
            Serial.println(freq / 16);
            Serial.print("PW: ");
            Serial.println(pw / 16);

            Serial.println("--------------");

            cnt = 0;

            sei();

            delay(2000);
        }

    }



}

ISR(TIMER1_CAPT_vect) {
    current_count[cnt%4][!!(TCCR1B & (1 << ICES1))] = ICR1;
    TCCR1B ^= (1 << ICES1);
    TIFR1 |= (1 << ICF1);
    cnt += !!(TCCR1B & (1 << ICES1));
    if(cnt >= 17) {
        cli();
    }
}
*/