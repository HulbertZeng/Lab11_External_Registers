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

#include "timer.h"
#include "scheduler.h"

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

enum display_states { display_start, display_inc, display_dec, display_reset, display_buffer };
unsigned char num = 0xF0;

void displaySMTick(state) {
    unsigned char inc = (~PINA) & 0x01;
    unsigned char dec = (~PINA) & 0x02;

    switch(state) {
        case display_start:
            if(inc & dec) {
                state = display_reset;
            } else if(inc & (num < 0xFF)) {
                state = display_inc;
            } else if(dec & (num < 0x00)) {
                state = display_dec;
            } else {
                transmit_data(num);
            }
            break;
        case display_reset: num = 0; state = display_buffer; break;
        case display_inc: ++num; state = display_buffer; break;
        case display_dec: --num; state = display_buffer; break;
        case display_buffer: if(!(inc | dec)) state = display_start; break;
        default: state = display_start; break;
    }

    return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRC = 0xFF; PORTC = 0x00;
    DDRA = 0x00; PORTA = 0xFF;
    /* Insert your solution below */
    static task task1;
    task *tasks[] = { &task1 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(*tasks);
    const char start = -1;
    // increment, decrement, and reset displayed value
    task1.state = start;
    task1.period = 50;
    task1.elapsedTime = task1.period;
    task1.TickFct = &displaySMTick;

    unsigned short i;

    unsigned long GCD = tasks[0]->period;
    for(i = 0; i < numTasks; i++) {
        GCD = findGCD(GCD, tasks[i]->period);
    }

    TimerSet(GCD);
    TimerOn();
    while (1) {
        for(i = 0; i < numTasks; i++) {
            if(tasks[i]->elapsedTime == tasks[i]->period) {
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                tasks[i]->elapsedTime = 0;
            }
            tasks[i]->elapsedTime += GCD;
        }
        while(!TimerFlag);
        TimerFlag = 0;
    }
    return 1;
}
