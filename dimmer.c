/*
 * File:   dimmer.c
 * Author: Josefe
 * 
 * Created on 17 April 2017, 10:46
 */

/*
 * Digital phase-controlled dimmer with mains frequency measurement. 
 * Behavioral description file. All functions needed ton control it are included here.
 * For using it, just initialize it, set ISR functions, enable them and pass dimming value.
 * It works with 256 discrete time slots, where, according to the AC RMS value required, the output will be fired if the AC phase is bigger than the required RMS phase value.
 * When the AC phase goes to 0, outputs are turned off, AC frequeny updated and reseted for a new firing cycle that will begin when Zero crossing ends.
 * In case of AC fail, firing goes off.
 */

#include <xc.h>
#include "build_config.h"
#include "dimmer_hal.h"
#include "dimmer.h"

#ifdef no_inline
    #define inline
#endif

//Dimmer states
typedef enum 
{
    DIMMER_IDLE,                    //Dimmer is waiting for first frequency measurement      
    DIMMER_START_FREQ_MEASURING,    //Dimmer is measuring frequency previously to start firing
    DIMMER_FREQ_LOADED,             //AC waveform goes to 0 and outputs are turned off
    DIMMER_ZC,                      //Same but after a firing process
    DIMMER_FIRING                   //Dimmer is firing channels and remeasuring frequency
}states;

unsigned char slot_counter;                          //AC firing slot counter
unsigned char fire_tresholds_buffer[NUM_CHANNELS];   //Channel's slots where it has to be fired
states dimmer_status;                       //State variable

/*
 * Updates channel's dimming values.
 * It receives the dimming values and its size
 * It sets the firing slots according to a linear firing rom (AC RMS value is LINEAR to dimming value)
 */
void set_fire_tresholds_buffer(unsigned char* data, unsigned char data_length)
{
    char i;
    for (i=0; i<data_length && i<NUM_CHANNELS; ++i )
    {
        fire_tresholds_buffer[i]=(firing_map[data[i]]>>1); //128 dimming steps
    }
}

/*
 * Provides a pointer to the shaped firing buffer
 */

unsigned char* get_fire_tresholds_buffer(void)
{
    return fire_tresholds_buffer;
}

/*
 * Initializes dimmer HW with data
 * It receives the first dimming values and its size
 * 
 */
void dimmer_init (unsigned char* data, unsigned char data_length)
{
    set_fire_tresholds_buffer(data, data_length); 
    slot_counter=RESET_SLOT;        //Reset values
    dimmer_status=DIMMER_IDLE;      
    channels_init();                //Port init
    zc_init();                      //Zero crossing init
    firing_timer_init();            //Firing slot timer init
    freq_measuring_timer_init();    

}

/*
 * Interrupt service routine for zero crossing process
 */
inline void zc_isr(void) 
{
    if (zc_check_flag())
    {      
        zc_clear_flag();
        switch (dimmer_status)
        {
            case DIMMER_START_FREQ_MEASURING: //First falling edge has happened
                //Turn all on could be used
                zc_set_edge_direction(ZC_rising_edge); //Waits for rising edge
                firing_timer_disable(); //No firing
                freq_measuring_timer_restart();   //Starts first frequency measurement             
                dimmer_status=DIMMER_FREQ_LOADED; //Next rising is a valid zero crossing
#ifdef debug
                debug_pin=0;
#endif
                break;
            case DIMMER_ZC:          //Valid zero crossing
            case DIMMER_FREQ_LOADED: //First measurement rising edge has happened
                turn_all_off(); //Turn all triac gates off
                zc_set_edge_direction(ZC_falling_edge);   //Waits for falling edge (and re-start firing) 
                firing_timer_disable(); //Stops firing timer
                firing_timer_update_period(freq_measuring_timer_freeze()); //Updates measured freq
                slot_counter=RESET_SLOT; //Resets slot counter too                
                dimmer_status=DIMMER_FIRING; //Next falling is firing!!
#ifdef debug
                debug_pin=1;
#endif                
                break;
            case DIMMER_FIRING: //Falling has happened, firing begins
                zc_set_edge_direction(ZC_rising_edge); //Waits for rising edge and zero crossing                  
                firing_timer_enable(); //Resets and starts firing timer
                //firing_timer_reset();  
                freq_measuring_timer_restart(); //Measure again
                dimmer_status=DIMMER_ZC; //waits for ZC
#ifdef debug
                debug_pin=0;
#endif                
                break;            
            default:
            case DIMMER_IDLE: //A rising edge has happened before first measuring
                turn_all_off(); //Output is off
                zc_set_edge_direction(ZC_falling_edge); //Wait for falling
                freq_measuring_timer_freeze(); //No measuring freq
                firing_timer_disable();        //No firing
                dimmer_status=DIMMER_START_FREQ_MEASURING; //Next falling starts measuring for the first time
#ifdef debug
                debug_pin=1;
#endif
                break;
        }      
    }
}

/*
 * ISR for firing channels
 */

inline void firing_timer_isr(void)
{
    if (firing_timer_check_flag())
    {
        firing_timer_clear_flag();
        if (slot_counter<LAST_SLOT) //If not all the time slots have passed
        {
            firing_timer_reset();  //Resets slot increase timer
            fire_all(slot_counter, fire_tresholds_buffer); //Check if any channel  has to be fired
            ++slot_counter; //Increses slot            
        } 
    }
}

/*
 * ISR for the frequency measuring timer overflow
 * If it happens, AC has gone off and firing must be stopped to prevent any damage
 */
inline void freq_measuring_timer_overflow_isr(void)
{
    if(freq_measuring_timer_check_flag())
    {
        freq_measuring_timer_clear_flag();
        turn_all_off(); //Output is turned off
        zc_set_edge_direction(ZC_rising_edge); //Wait for a rising edge
        freq_measuring_timer_freeze(); //No measuring freq
        firing_timer_disable();        //No firing
        //firing_timer_reset_period();   //Max period
        dimmer_status=DIMMER_IDLE; //waits for a new valid measurement
    }
}
