#include <avr/interrupt.h>
#include <Arduino.h>

volatile uint16_t timings[4];
volatile uint8_t state = 0;

uint32_t current_time;

double frequency;
double pulsewidth;

int main() {

    init();                                         // Für Serial
    Serial.begin(9600);


    TCCR1A = TCCR1B = 0;                            // reset timer 1
    TIMSK1 |= (1 << ICIE1);                         // edge detection
    TCCR1B |= (1 << CS10);                          // prescaler = 1

    sei();                                          // enable interrupts

    while (1) {

/**
 * Zeitabfrage mit millis bzw. _delay_ms können die Flankenerkennung stören. delay verursacht am wenigsten Probleme.
 * Nutzung von Timer0 zur Ausgabe mit ca. 1 Hz kann auch die Erkennung stören, falls wir bei einer Flanke in der ISR sind...
 *
 * Für delay:
 *  - Zeile 39 auskommentieren
 *  - Zeilen 40, 64 einkommentieren
 *
 *  Für millis:
 *  - Zeile 39 einkommentieren
 *  - Zeilen 40,64 auskommentieren
 */

//        if ((millis() > current_time + 1000)) {
        if (1) {                                                                                            // Nur zum testen

            if ((state == 3) && (timings[0] != 0)) {                                                        // Wenn alle 3 Werte aktualisiert sind und der Zeitpunkt der ersten Flanke nicht bei 0 liegt (bug?)

                cli();                                                                                      // Interrupts ausschalten

                frequency = (16000000.0 / (timings[2] - timings[0]));                                       // Berechnung der Frequenz aus den beiden Steigenden Flanken
                pulsewidth = 100 * (1.0 * (timings[1] - timings[0]) / (timings[2] - timings[0]));           // Berechnung des Tastgrades aus Verhältnis von Ton zu Toff

                /* Ausgabe */
                Serial.print("Frequenz: ");
                Serial.print(frequency);
                Serial.println(" Hz");
                Serial.print("Tastgrad: ");
                Serial.print(pulsewidth);
                Serial.println(" %");
                Serial.println("------------------");

                state = 0;                                                                                  // Status der Messung zurücksetzen
                current_time = millis();

                TCCR1B |= (1 << ICES1);                                                                     // Flankenerkennung festlegen (sonst gibt es Fehler beim Ausgeben des Tastgrades)
                sei();                                                                                      // Interrupts aktivieren

                delay(1000);                                                                            // delay ist schlecht (blockiert), macht aber bei hohen Frequenzen die wenigsten Probleme

            }

        }

    }

}

ISR(TIMER1_CAPT_vect) {

    timings[state] = ICR1;                                                                                  // Liest den aktuellen Timerstand aus. state == 1: erste Flanke, state == 2: zweite Flanke, state == 3: dritte Flanke
    TCCR1B ^= (1 << ICES1);                                                                                 // Ändert die Flankenerkennung

    if (state != 3) {
        state++;
    } else {                                                                                                // Wenn die drei Flanken gemessen wurden
        state = 0;                                                                                          // Status zurücksetzen (damit es keine Ausgabe bei unvollständiger Messung gibt)
        TCNT1 = 0;                                                                                          // Reset Timerstand. Weniger Fehler bei hohen Frequenzen, eine Flanke wird trotzdem ab und zu übersprungen
    }

}