 /* Author: Hulbert Zeng
 * Partner(s) Name (if applicable):  
 * Lab Section: 021
 * Assignment: Lab #11  Exercise #5
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 *
 *  Demo Link: https://youtu.be/xVzF1uftF5E
 */ 
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

#include "timer.h"
#include "scheduler.h"

void transmit_data(unsigned short data) {
    int i;
    for(i = 0; i < 16; ++i) {
        PORTC = 0x08;
        PORTC |= ((data >> i) & 0x01);
        PORTC |= 0x02;
    }
    PORTC |= 0x04;
    PORTC = 0x00;
}

enum player_states { player_wait, player_left, player_right, player_leftjump, player_rightjump, player_buffer };
unsigned short player = 0x8000;

int playerSMTick(int state) {
    unsigned char left = (~PINA) & 0x04;
    unsigned char jump = (~PINA) & 0x02;
    unsigned char right = (~PINA) & 0x01;

    switch(state) {
        case player_wait: 
            if(left && !jump) {
                state = player_left;
            } else if(left && jump) {
                state = player_leftjump;
            } else if(right && !jump) {
                state = player_right;
            } else if(right && jump) {
                state = player_rightjump;
            } else {
                state = player_wait;
            }
            break;
        case player_left: 
            if(player != 0x8000) {
                player = player << 1;
            }
            state = player_buffer;
            break;
        case player_right: 
            if(player != 0x0001) {
                player = player >> 1;
            }
            state = player_buffer;
            break;
        case player_leftjump: 
            if(player <= 0x0800) {
                player = player << 4;
            } else {
                player = 0x8000;
            }
            state = player_buffer;
            break;
        case player_rightjump: 
            if(player >= 0x0010) {
                player = player >> 4;
            } else {
                player = 0x0001;
            }
            state = player_buffer;
            break;
        case player_buffer:
            if(!left && !jump && !right) {
                state = player_wait;
            } else {
                state = player_buffer;
            }
        default: state = player_wait; break;
    }

    return state;
}


enum enemy_states { enemy_left, enemy_right };
unsigned short enemy = 0x0001;

int enemySMTick(int state) {
    switch(state) {
        case enemy_left: 
            if(enemy != 0x8000) {
                enemy = enemy << 1;
                state = enemy_left;
            } else {
                state = enemy_right;
            }
            break;
        case enemy_right: 
            if(enemy != 0x0001) {
                enemy = enemy >> 1;
                state = enemy_right;
            } else {
                state = enemy_left;
            }
            break;
        default: state = enemy_left; break;
    }

    return state;
}


enum game_states { game_gameplay, game_collide };
unsigned short gameover = 0xFFFF;
unsigned char i = 0;

int gameSMTick(int state) {
    switch(state) {
        case game_gameplay: 
            if(player != enemy) {
                unsigned short temp = player | enemy;
                transmit_data(temp);
                state = game_gameplay;
            } else {
                state = game_collide;
            }
            break;
        case game_collide: 
            if(i < 32) {
                transmit_data(gameover);
                gameover = ~gameover;
                ++i;
            } else {
                player = 0x8000;
                enemy = 0x0001;
                i = 0;
                state = game_gameplay;
            }
            break;
        default: state = game_gameplay; break;
    }

    return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRC = 0xFF; PORTC = 0x00;
    DDRA = 0x00; PORTA = 0xFF;
    /* Insert your solution below */
    static task task1, task2, task3;
    task *tasks[] = { &task1, &task2, &task3 };
    const unsigned short numTasks = sizeof(tasks)/sizeof(*tasks);
    const char start = -1;
    // player
    task1.state = start;
    task1.period = 70;
    task1.elapsedTime = task1.period;
    task1.TickFct = &playerSMTick;
    // enemy
    task2.state = start;
    task2.period = 500;
    task2.elapsedTime = task2.period;
    task2.TickFct = &enemySMTick;
    // game
    task3.state = start;
    task3.period = 50;
    task3.elapsedTime = task3.period;
    task3.TickFct = &gameSMTick;

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
