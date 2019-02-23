/*
 * File:   dimmer_hal.c
 * Author: Josefe
 *
 * Created on 17 April 2017, 10:49
 */


#include "build_config.h"
#include <xc.h>
#include "dimmer_hal.h"

#ifdef no_inline
    #define inline
#endif

/*ZERO CROSSING FUNCTIONS*/

/*
 * Initializes Zero crossing external interrupt pin
 */
void zc_init(void)
{
    trisZC |= (1<<bitZC); //set as input pin
    interrupt_enableZC = ZC_enabled; //Enables interrupts (always high priority)
    interrupt_edgeZC = ZC_rising_edge; //Sets first valid edge as a rising edge
}

/*
 * Checks Zero crossing interrupt flag
 */
inline char zc_check_flag(void) 
{
    return (flagZC && interrupt_enableZC);
}

/*
 * Clears Zero crossing interrupt flag
 */
inline void zc_clear_flag(void)
{
    flagZC=0;
}

/*
 * Changes Zero crossing edge direction and clears flag
 * Needs the desired new edge
 */
inline void zc_set_edge_direction(char direction)
{
    interrupt_edgeZC=direction;
    flagZC=0;
}

/*SLOT COUNTER TIMER FUNCTIONS*/

/*
 * Initializes slot counter timer to produce an interrupt each time the slot time has passed.
 * Uses CCP as output comparator and timer 3 as timebase.
 */
void firing_timer_init(void)
{
    TMR3H=0;                //Timer count reset
    TMR3L=0;
    T3CONbits.RD16=1;       //One single 16 bit write    
    T3CONbits.T3CKPS=0b11;  //:8 prescaler
    T3CONbits.T3CCP2=1;     //Drives both ccps (we use only CCP1). Other option affects to other used timers
    
    CCPR1H=0xFF;            //Inits CCP on max compare
    CCPR1L=0xFF;
    CCP1CONbits.CCP1M=0b1010; //Generate interrupts on match
    PIR1bits.CCP1IF=0;        //Clear interrupt flag    
    
    IPR1bits.CCP1IP=1;      //High priority for ccp match
}

/*
 * Starts the slot timer counter
 */
inline void firing_timer_enable(void)
{
    TMR3H=0;            //Count reset
    TMR3L=0;
    PIE1bits.CCP1IE=1;  //Interrupts on
    PIR1bits.CCP1IF=0;  //Clear interrupt flag
    T3CONbits.TMR3ON=1; //Timer on
    
}

/*
 * Stops firing timer
 */
inline void firing_timer_disable(void)
{
    PIE1bits.CCP1IE=0;  //Interrupts off
    T3CONbits.TMR3ON=0; //Timer off
}

/*
 * Resets firing timer
 */
inline void firing_timer_reset(void)
{
    TMR3H=0;          // Count reset
    TMR3L=0;
}

/*
 * Checks firing timer flag
 */
inline char firing_timer_check_flag(void) 
{
    return (PIE1bits.CCP1IE && PIR1bits.CCP1IF);
}

/*
 * Clears firing timer flag
 */
inline void firing_timer_clear_flag(void)
{
    PIR1bits.CCP1IF=0;
}

/*
 * Updates firing timer slot period
 * Each time the period has passed, one slot is incremented
 * Period is shifted straight away to the low register
 */
inline void firing_timer_update_period(char period)
{
    CCPR1H=0;       //First High, second low to allow a proper reset
    CCPR1L=period;
}

/*
 * Sets the firing timer slot period to its max value
 */
inline void firing_timer_reset_period(void)
{
    CCPR1H=0xFF; //Inits CCP on max compare
    CCPR1L=0xFF;
}

/*FREQUENCY MEASUREMENT TIMER FUNCTIONS*/

/*
 * Initializes the frequency measurement timer
 * the measured value will be used to calculate the comparison treshold to produce the slot increment.
 * This allows a dynamic frecuency measurement.
 * If it overflows means that zero crossing hasn't appeard,indicating that mains has gone away.
 * It is important to allow a full measurement range for the lowest frequency value without overflowing.
 */
inline void freq_measuring_timer_init(void)
{
    T1CONbits.T1CKPS=0b10; //:4 prescaler    
    IPR1bits.TMR1IP=1;     //High priority    
}

/*
 * Restarts frequency measuring timer
 */
inline void freq_measuring_timer_restart (void)
{    
    PIE1bits.TMR1IE=1;  //Enable interrupts
    PIR1bits.TMR1IF=0;  
    TMR1H=0;            //Clear count
    TMR1L=0;
    T1CONbits.TMR1ON=1; //start timer
}

/*
 * Stops the frequency measurement timer, returning the measured value
 */
inline unsigned char freq_measuring_timer_freeze(void)
{
    PIE1bits.TMR1IE=0;  //Interrupts off
    T1CONbits.TMR1ON=0; //Timer off
    return TMR1H;
}

/*
 * Checks measuring timer overflow flag
 */
inline char freq_measuring_timer_check_flag(void)
{
    return (    PIE1bits.TMR1IE &&  PIR1bits.TMR1IF);
}

/*
 * Clears measuring timer overflow flag
 */ 
inline void freq_measuring_timer_clear_flag(void)
{
    PIR1bits.TMR1IF=0; 
}


/*OUTPUT FIRNG PORTS (I/O PORTS) FUNCTIONS*/

/*
 * Initializes firing ports, setting up tris as output ports and clearing the output value
 * It uses a table where each firing port latch, tris and mask are arranged according to the output channel numbers
 */
void channels_init (void)
{
    {
    int i;
    for (i=0; i<NUM_CHANNELS; ++i)
    {
        *(output_channels_tris[i])&=~(1<<output_channels_bits[i]);
        *(output_channels_addresses[i])&=~(1<<output_channels_bits[i]);
    }
}
}

/*
 * Turns all firing ports off
 */
inline void turn_all_off(void)
{
    int i;
    for (i=0; i<NUM_CHANNELS; ++i)
    {
        *(output_channels_addresses[i])&=~(1<<output_channels_bits[i]);
    }
}

/*
 * Checks and fires all the ports if the current time slot is bigger than the setted one
 * It requires the current slot value and the firing tresholds table pointer
 */
inline void fire_all(unsigned char count, unsigned char* fire_tresholds)
{
    int i;
    for (i=0; i<NUM_CHANNELS; ++i)
    {
        if (count>=fire_tresholds[i])
        {
            *(output_channels_addresses[i])|=(1<<output_channels_bits[i]);
        }
    }
}
