// RIOS for Gameboy
// main.c
// Cameron Smith
// March 6 2020

// This is a port of RIOS (Riverside-Irvine Operating System) to the Nintendo Game Boy

#include <gb/gb.h> //gameboy library found online
#include <stdio.h>
#include "../graphics/taskSprites.c" // graphics
#include "../graphics/taskSmiles.c" // graphics
#include "gbTimer.h" // functions to set gb timer attributes

typedef struct task {
   unsigned int period;      // Rate at which the task should tick
   unsigned int elapsedTime; // Time since task's last tick
   void (*TickFct)(void);     // Function to call for task's tick

   unsigned long running;
   unsigned int state;
} task;

task tasks[3];

const unsigned char tasksNum = 3;
const unsigned int periodT1   = 4080;
const unsigned int periodT2 = 8160;
const unsigned int periodT3 = 12240;
unsigned int gcdEvents = 4080;

unsigned long interruptLimit = 16;
unsigned long currInterrupts = 0;

void TimerISR(void);
int TickFct_T1(int state);
int TickFct_T2(int state);
int TickFct_T3(int state);

unsigned char runningTasks[3] = {255}; //Track running tasks-[0] always idleTaskconst 
unsigned long idleTask = 255;       // 0 highest priority, 255 lowest 
unsigned long currentTask = 0;     // Index of highest priority task in runningTasks


void setInterruptLimit(int limit){
    interruptLimit = limit;
}

void TimerISR(void) {
    unsigned char i;    
    
    currInterrupts++;

    //has there been enough interrupts to envoke the ISR?
    if (currInterrupts == interruptLimit){

        // the library gb.h takes care of saving and restoring registers
        // when ISR is called
        // http://gbdk.sourceforge.net/doc/gbdk-doc.pdf page 14

        currInterrupts = 0;
        
        for (i=0; i < tasksNum; ++i) { 
            
            // Heart of scheduler code       
            if (  ((unsigned int)tasks[i].elapsedTime >= (unsigned int)tasks[i].period)  // Task ready           
                && (runningTasks[currentTask] > i)    // Task priority > current task priority         
                && (!tasks[i].running)         // Task not already running (no self-preemption)         
            ) { 
                
                disable_interrupts(); // Critical section 
                tasks[i].elapsedTime = 0;  // Reset time since last tick          
                tasks[i].running = 1;          // Mark as running          
                currentTask++;          
                runningTasks[currentTask] = i;      // Add to runningTasks
                enable_interrupts(); // End critical section    

                tasks[i].state = (int)tasks[i].TickFct(tasks[i].state);      // Execute tick    
                
                disable_interrupts();; // Critical section          
                tasks[i].running = 0;   
                runningTasks[currentTask] = idleTask; // Remove from runningTasks          
                currentTask -= 1;          
                enable_interrupts(); // End critical section  
                
            }
            
            // (255 ticks in until an interrupt) * (How many interrupts until ISR dispatched)
            tasks[i].elapsedTime += 255 * (unsigned int)interruptLimit;

           
        }    
        
    }
    

}

int main(void) {

    unsigned char i=0;  

    // set up the sprites on the gameboy's display
    set_sprite_data(0x1A,6,taskSmiles);
    set_sprite_tile(0, 0x1A);
    set_sprite_tile(1, 0x1B);
    set_sprite_tile(2, 0x1C);
    set_sprite_tile(3, 0x1D);
    set_sprite_tile(4, 0x1E);
    set_sprite_tile(5, 0x1F);
    set_sprite_tile(6, 0x00);

    move_sprite(0, 78, 78);
    move_sprite(1, 88, 78);
    
    SHOW_SPRITES;

    // set up the tasks
    tasks[i].period = periodT1;    
    tasks[i].elapsedTime = tasks[i].period;    
    tasks[i].TickFct = TickFct_T1;   
    ++i;    
    tasks[i].period = periodT2;    
    tasks[i].elapsedTime = tasks[i].period;    
    tasks[i].TickFct = TickFct_T2;   
    ++i;
    tasks[i].period = periodT3;    
    tasks[i].elapsedTime = tasks[i].period;    
    tasks[i].TickFct = TickFct_T3;   
    
    // set up the timers and begin
    printf("Welcome to RIOS \nfor Gameboy\n\n");
    // set TimerISR to be the ISR when there's a TIMA overflow
    add_TIM(TimerISR);
    // choose frequency to run gameboy at
    TimerSet(0); 
    // choose how many interrupts until ISR is serviced
    setInterruptLimit(gcdEvents/255);
    TimerOn();
    while(1) {  } 

   
}

int TickFct_T1(int state){
    // state transitions
    if (state == 0){
        state = 1;
    } else if (state == 1) {
        state = 2;
    } else if (state == 2){
        state = 3;
    } else {
        state = 1;
    }

    // state actions
    if (state == 1) {
        set_sprite_tile(0, 26);
    } else if (state == 2){
        set_sprite_tile(0, 27);
    } else {
        set_sprite_tile(0, 28);
    }
    
    return state;
    
}

int TickFct_T2(int state){
    
    // state transitions
    if (state == 0){
        state = 1;
    } else if (state == 1) {
        state = 2;
    } else if (state == 2){
        state = 3;
    } else {
        state = 1;
    }

    // state actions
    if (state == 1) {
        set_sprite_tile(1, 29);
    } else if (state == 2){
        set_sprite_tile(1, 30);
    } else {
        set_sprite_tile(1, 31);
    }

    return state;
    
}

int TickFct_T3(int state){
    // state transitions
    if (state == 0){
        state = 1;
    } else if (state == 1) {
        state = 2;
    } else if (state == 2){
        state = 3;
    } else {
        state = 1;
    }

    // state actions
    if (state == 1) {
        printf("state 1 tick\n\n");
    } else if (state == 2){
        printf("state 2 tick\n\n");
    } else {
        printf("state 3 tick\n\n");
    }

    return state;
    
}