/*
 * File:   main.c
 * Author: josefe
 *
 * Created on 21 de abril de 2017, 15:21
 */



#include "build_config.h"
#include <xc.h>
//#define _XTAL_FREQ 8000000 //Int 8 MHz
#define _XTAL_FREQ 48000000

#include "config.h"
#include "dimmer.h"
#include "dimmer_hal.h"
#include "dmx_rx.h"

void process_channels(void);
void adc_init(void);
void start_conversions(unsigned char* output_buffer, unsigned char first_channel, unsigned char last_channel);
void adc_mux_set(void);
void adc_mux_reset(void);

//8 channels buffer
unsigned char channels_data[NUM_CHANNELS]={0, 0, 0, 0, 0, 0, 0, 0};
unsigned char adc_buffer[NUM_CHANNELS]={0, 0, 0, 0, 0, 0, 0, 0};

void main(void) {
    OSCCON = 0b01111100;   //HS PLL oscillator @48Mhz
    //OSCCON = 0b01111110;   //Int 8 MHz
    
#ifdef debug
    TRISDbits.TRISD0=0;
    PORTDbits.RD0=0;
#endif
    

    dimmer_init (channels_data, NUM_CHANNELS);
    adc_init();
    usart_config();
    usart_timeout_timer_init(USART_TIMEOUT_L, USART_TIMEOUT_H);
    usart_timeout_reset(USART_TIMEOUT_L, USART_TIMEOUT_H);
    address_init();
    address=0;
    //Enable interrupts  
    RCONbits.IPEN=1;
    INTCONbits.GIEL=1;
    INTCONbits.GIEH=1;
    //Go!
    while(1)
    {
#ifdef debug
        /*
        TRISBbits.TRISB0=1;
        debug_pin=LATBbits.LATB0; //Attach interrupt line to output
         */
        
        //PORTDbits.RD0=0;
        //__delay_ms(5);
        PORTDbits.RD0=1;
        //__delay_ms(5);
         
#endif
        //TODO
        address=read_address();
        start_conversions(adc_buffer, 0, NUM_CHANNELS);
        process_channels();
        set_fire_tresholds_buffer(channels_data, NUM_CHANNELS);
    }
}
     
void __interrupt(high_priority) isr_high(void)
{
    zc_isr();
    firing_timer_isr();
    freq_measuring_timer_overflow_isr();
}

void __interrupt(low_priority)   isr_low(void)
{   
    usart_isr();
    usart_timeout_isr();  
}

void process_channels(void)
{
	unsigned char i;
	
	for (i=0; i<NUM_CHANNELS; i++)	// For each channel					
	{
		if (rx_valid==DATA_RX_VALID)// && address<(TOTALCHANNELS-i)) // If data or address aren't valid
            {
            if (address<(TOTALCHANNELS-i))
            
                {
                    if (TramaDMX[i]>adc_buffer[i])
                    {
                        channels_data[i]=TramaDMX[i];			// Passes value to output straight away
                    }
                    else
                    {
                        channels_data[i]=adc_buffer[i]; // Output is set to 0
                        //channels_data[i]=0;
                    } 
                }
		}
		else
		{
			channels_data[i]=adc_buffer[i]; // Output is set to 0
            //channels_data[i]=0;
		}   
	}
}

void adc_init()
{
    ADCON0bits.GO_DONE=0;           // Converter is idle
    ADCON1bits.VCFG=0b00;           // No reference inputs
    ADCON1bits.PCFG=0b1001;         // All channels analog inputs
    ADCON2=0b00000001;              // Left justified, 0 Tad adquisition, Fosc/8
    TRISA |= 0b00101111;            // Set TRIS for ADC pins
    TRISE |= 0b00000111;   
    ADON=1;                         // ADC on
}

void start_conversions(unsigned char* output_buffer, unsigned char first_channel, unsigned char last_channel)
{
    unsigned char adc_index=first_channel;

    ADCON0bits.CHS=adc_index;       // Select channel
    while (adc_index<last_channel)
    {
        __delay_us(10);                 // Wait for adquisition time
        ADCON0bits.GO_DONE = 1;         // Start conversion
        __delay_us(5);                  // Wait 5 us before changing mux to allow C to be disconnected
        ADCON0bits.CHS=++adc_index;     // Increment channel index
        while (ADCON0bits.GO_DONE){}    // Wait for conversion
        output_buffer[adc_index-1] = ADRESH; // Save converted value
        ADRESH = 0;
    }
    
}

void adc_mux_set(void)
{
#ifdef _18F2550
    return;
#else
#ifndef six_ch    
    LATEbits.LATE1=1;   //Mux enabled    
#else 
    return;
#endif //end of six_ch
#endif //end of _18F2550
}

void adc_mux_reset(void)
{    
#ifdef _18F2550
    return;
#else
#ifndef six_ch    
    LATEbits.LATE1=0;   //Mux disaabled 
#else 
    return;
#endif //end of six_ch
#endif //end of _18F2550
}

