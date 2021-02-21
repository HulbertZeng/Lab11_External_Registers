 /* Author: Hulbert Zeng
 * Partner(s) Name (if applicable):  
 * Lab Section: 021
 * Assignment: Lab #11  Exercise #1
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 *
 *  Demo Link: Youtube URL>
 */ 
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

void transmit_data(unsigned char data) {
    int i;
    for(i = 0; i < 8; ++i) {
        PORTC = 0x08;
        PORTC |= ((data >> i) & 0x01);
        PORTC |= 0x02;
    }
    PORTC |= 0x04;
    PORTC = 0x00;
}

enum display_states { display_start } display_state;
unsigned char num = 0xF0;

void TickFct() {
    unsigned char inc = (~PINA) & 0x01;
    unsigned char dec = (~PINA) & 0x02;

    switch(display_state) {
        case display_start:
            if(inc & dec) {
                num = 0;
            } else if(inc & (num < 0xFF)) {
                ++num;
            } else if(dec & (num < 0x00)) {
                --num;
            } else {
                transmit_data(num);
            }
            break;
        default: display_state = display_start; break;
    }
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRC = 0xFF; PORTC = 0x00;
    DDRA = 0x00; PORTA = 0xFF;
    /* Insert your solution below */
    while (1) {
        TickFct();
        
    }
    return 1;
}
