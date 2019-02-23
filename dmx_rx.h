

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef DMX_RX_H
#define	DMX_RX_H

#include <xc.h> // include processor files - each processor file is guarded.
#include "build_config.h"

#define TOTALCHANNELS   512 
#define LOG2_512        9
#define ADDRESS_OFFSET  0

#ifndef NUM_CHANNELS
    #define NUM_CHANNELS 8
#endif

/*1 sec period
 256*256-12.000.000/PRESCALER[256]*/
#define USART_TIMEOUT_H 72
#define USART_TIMEOUT_L 229

volatile unsigned char TramaDMX[TOTALCHANNELS]={0}; //Rx buffer
volatile unsigned int address;                              //Address variable

volatile enum
{
    DATA_RX_INVALID,
    DATA_RX_VALID
}rx_valid;

void usart_timeout_timer_init(char low, char high);
inline void usart_timeout_reset(char low, char high);
inline void usart_timeout_isr(void);
void usart_config(void);
inline void  usart_isr(void);

void address_init(void);
unsigned int read_address(void);

#endif	/* DMX_RX_H */

