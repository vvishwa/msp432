// TExaSdisplayMain.c
// Runs on MSP432
// Main program for the TExaSdisplay demo that initializes
// ports and toggles Profile pins to be seen on both the
// TExaSdisplay interface and on an actual logic analyzer
// connected to the Profile pins.
// Daniel Valvano
// September 27, 2016

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2016

   Program 5.12, section 5.7

 Copyright 2016 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

// oscilloscope or logic analyzer to profile the system
// J3.27/P4.5 Profile 0
// J3.28/P4.7 Profile 1
// Virtual logic analyzer Bit0
// Virtual scope P4.4/A9

#include <stdint.h>
#include "../inc/CortexM.h"
#include "../inc/Profile.h"
#include "TExaS.h"
#include "../inc/clock.h"
// **************SysTick_Init*********************
// Initialize SysTick periodic interrupts
// Input: interrupt period
//        Units of period are 20.83ns (assuming 48 MHz clock)
//        Maximum is 2^24-1
//        Minimum is determined by length of ISR
// Output: none
volatile uint32_t Counts = 0;
void SysTick_Init(uint32_t period){ long sr;
  sr = StartCritical();
  Profile_Init();
  Counts = 0;
  STCTRL = 0;                      // 1) disable SysTick during setup
  STRELOAD = period - 1;           // 2) reload value sets period
  STCURRENT = 0;                   // 3) any write to current clears it
  SYSPRI3 = (SYSPRI3&0x00FFFFFF)|0x40000000; // priority 2
  STCTRL = 0x00000007;             // 4) enable SysTick with core clock and interrupts
  EndCritical(sr);
}
// Interrupt service routine
// Executed every 20.83ns*(period)
void SysTick_Handler(void){
  Profile_Toggle0();          // toggle real logic analyzer bit
  Profile_Toggle0();          // toggle real logic analyzer bit
  TExaS_Task0();              // toggle TExaS logic analyzer
  Counts = Counts + 1;
  Profile_Toggle0();          // toggle real logic analyzer bit
}

int main(void){               // bus clock at 48 MHz
uint32_t count=0;
  Clock_InitFastest();        // subsystem clock at 12 MHz
//  TExaS_Init(SCOPE, 12000000);
  TExaS_Init(LOGICANALYZER, 12000000);
  SysTick_Init(480000);       // initialize SysTick timer
  EnableInterrupts();
  while(1){                   // interrupts every 10ms
    Profile_Toggle1();        // toggle bit
    count++;
    if(count == 10000){
      count = 0;
      TExaS_Task1();          // toggle every 10000 times 
    }
  }
}
