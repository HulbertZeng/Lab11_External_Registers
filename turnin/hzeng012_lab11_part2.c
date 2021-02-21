 /* Author: Hulbert Zeng
 * Partner(s) Name (if applicable):  
 * Lab Section: 021
 * Assignment: Lab #11  Exercise #2
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 *
 *  Demo Link: https://youtu.be/hGaqAINm1C8
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

// shared variables
unsigned char go = 0;

enum FC_states { FC_start, FC_inc, FC_dec, FC_power, FC_buffer };

int festivechooserSMTick(int state) {
    unsigned char button = (~PINA) & 0x03;

    switch(state) {
        case FC_start:
            if(go == 0) {
                transmit_data(0);
            }
            if(button == 0x03) {
                state = FC_power;
            } else if(button == 0x02) {
                state = FC_inc;
            } else if(button == 0x01) {
                state = FC_dec;
            } else {
                state = FC_start;
            }
            break;
        case FC_power: 
            if(go == 0) { 
                go = 1;
            } else {
                go = 0;
                transmit_data(0);
            } state = FC_buffer; break;
        case FC_inc: 
            if(go == 3) { 
                go = 1; 
            } else if(go != 0) {
                ++go;
            }
            if(button == 0x03) {
                state = FC_power;
            } else {
                state = FC_buffer;
            }
            break;
        case FC_dec: 
            if(go == 1) {
                go = 3;
            } else if(go != 0) { 
                --go; 
            }
            if(button == 0x03) {
                state = FC_power;
            } else {
                state = FC_buffer;
            }
            break;
        case FC_buffer: 
            if(button != 0x01 && button != 0x02) { 
                state = FC_start; 
            } break;
        default: state = FC_start; break;
    }

    return state;
}


enum festive1_states { festive1_state1, festive1_state2, festive1_state3, festive1_state4, festive1_state5 };

int festive1SMTick(int state) {
    switch(state) {
        case festive1_state1: if(go == 1) { transmit_data(0x81); } 
                              state = festive1_state2; break;
        case festive1_state2: if(go == 1) { transmit_data(0x42); } 
                              state = festive1_state3; break;
        case festive1_state3: if(go == 1) { transmit_data(0x24); } 
                              state = festive1_state4; break;
        case festive1_state4: if(go == 1) { transmit_data(0x18); }
                              state = festive1_state5; break;
        case festive1_state5: if(go == 1) { transmit_data(0x00); }
                              state = festive1_state1; break;
        default: state = festive1_state1; break;
    }
    return state;
}


enum festive2_states { festive2_state1, festive2_state2, festive2_state3, festive2_state4, festive2_state5 };

int festive2SMTick(int state) {
    switch(state) {
        case festive2_state1: if(go == 2) { transmit_data(0xFF); } 
                              state = festive2_state2; break;
        case festive2_state2: if(go == 2) { transmit_data(0xAA); } 
                              state = festive2_state3; break;
        case festive2_state3: if(go == 2) { transmit_data(0x88); } 
                              state = festive2_state4; break;
        case festive2_state4: if(go == 2) { transmit_data(0x80); } 
                              state = festive2_state5; break;
        case festive2_state5: if(go == 2) { transmit_data(0x00); } 
                              state = festive2_state1; break;
        default: state = festive1_state1; break;
    }
    return state;
}


enum festive3_states { festive3_state1, festive3_state2, festive3_state3, festive3_state4, festive3_state5 };

int festive3SMTick(int state) {
    switch(state) {
        case festive3_state1: if(go == 3) { transmit_data(0xC0); } 
                              state = festive3_state2; break;
        case festive3_state2: if(go == 3) { transmit_data(0x18); } 
                              state = festive3_state3; break;
        case festive3_state3: if(go == 3) { transmit_data(0x03); } 
                              state = festive3_state4; break;
        case festive3_state4: if(go == 3) { transmit_data(0x06); } 
                              state = festive3_state5; break;
        case festive3_state5: if(go == 3) { transmit_data(0x60); } 
                              state = festive3_state1; break;
        default: state = festive1_state1; break;
    }
    return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRC = 0xFF; PORTC = 0x00;
    DDRA = 0x00; PORTA = 0xFF;
    /* Insert your solution below */
    static task task1, task2, task3, task4;
    task *tasks[] = { &task1, &task2, &task3, &task4 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(*tasks);
    const char start = -1;
    // choose festive lights sequence
    task1.state = start;
    task1.period = 30;
    task1.elapsedTime = task1.period;
    task1.TickFct = &festivechooserSMTick;
    // festive lights sequence 1
    task2.state = start;
    task2.period = 100;
    task2.elapsedTime = task2.period;
    task2.TickFct = &festive1SMTick;
    // festive lights sequence 2
    task3.state = start;
    task3.period = 100;
    task3.elapsedTime = task3.period;
    task3.TickFct = &festive2SMTick;
    // festive lights sequence 3
    task4.state = start;
    task4.period = 100;
    task4.elapsedTime = task4.period;
    task4.TickFct = &festive3SMTick;

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
