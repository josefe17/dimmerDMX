/*
 * File:   dimmer.h
 * Author: Rober
 *
 * Created on 17 April 2017, 10:46
 */

#ifndef DIMMER_H
#define	DIMMER_H

#include <xc.h> // include processor files - each processor file is guarded. 
#include "dimmer_hal.h"

#ifdef no_inline
    #define inline
#endif

//Slots counter reset and max values
#define LAST_SLOT 255
#define RESET_SLOT 0

//Linear firing rom
//Casts form dimming value to the slot where to fire the channel so as to get that dimming
//unsigned char firing_map[] = {255, 255, 227, 223, 220, 217, 215, 212, 210, 209, 207, 205, 204, 202, 201, 200, 198, 197, 196, 195, 194, 193, 192, 191, 190, 189, 188, 187, 186, 185, 184, 183, 182, 181, 181, 180, 179, 178, 178, 177, 176, 175, 175, 174, 173, 172, 172, 171, 170, 170, 169, 168, 168, 167, 166, 166, 165, 164, 164, 163, 163, 162, 161, 161, 160, 160, 159, 158, 158, 157, 157, 156, 156, 155, 154, 154, 153, 153, 152, 152, 151, 151, 150, 149, 149, 148, 148, 147, 147, 146, 146, 145, 145, 144, 144, 143, 143, 142, 142, 141, 140, 140, 139, 139, 138, 138, 137, 137, 136, 136, 135, 135, 134, 134, 133, 133, 132, 132, 131, 131, 130, 130, 129, 129, 128, 128, 127, 127, 126, 126, 125, 125, 124, 124, 123, 123, 122, 122, 121, 121, 120, 120, 119, 119, 118, 118, 117, 117, 116, 116, 115, 115, 114, 114, 113, 113, 112, 112, 111, 111, 110, 110, 109, 109, 108, 107, 107, 106, 106, 105, 105, 104, 104, 103, 103, 102, 102, 101, 100, 100, 99, 99, 98, 98, 97, 97, 96, 95, 95, 94, 94, 93, 92, 92, 91, 91, 90, 89, 89, 88, 87, 87, 86, 86, 85, 84, 84, 83, 82, 81, 81, 80, 79, 79, 78, 77, 76, 76, 75, 74, 73, 73, 72, 71, 70, 69, 68, 67, 67, 66, 65, 64, 63, 62, 61, 60, 59, 57, 56, 55, 54, 52, 51, 50, 48, 47, 45, 43, 41, 39, 37, 34, 31, 27, 0, 0};
//unsigned char firing_map[] = {255, 219, 214, 210, 207, 204, 202, 200, 198, 196, 195, 193, 192, 191, 189, 188, 187, 186, 185, 184, 183, 182, 181, 180, 179, 178, 177, 176, 175, 174, 174, 173, 172, 171, 170, 170, 169, 168, 167, 167, 166, 165, 165, 164, 163, 163, 162, 161, 161, 160, 159, 159, 158, 158, 157, 156, 156, 155, 155, 154, 153, 153, 152, 152, 151, 151, 150, 150, 149, 148, 148, 147, 147, 146, 146, 145, 145, 144, 144, 143, 143, 142, 142, 141, 141, 140, 140, 139, 139, 138, 138, 137, 137, 136, 136, 135, 135, 134, 134, 133, 133, 132, 132, 131, 131, 130, 130, 129, 129, 128, 128, 127, 127, 126, 126, 126, 125, 125, 124, 124, 123, 123, 122, 122, 121, 121, 120, 120, 119, 119, 118, 118, 118, 117, 117, 116, 116, 115, 115, 114, 114, 113, 113, 112, 112, 111, 111, 110, 110, 109, 109, 109, 108, 108, 107, 107, 106, 106, 105, 105, 104, 104, 103, 103, 102, 102, 101, 101, 100, 100, 99, 99, 98, 98, 97, 97, 96, 96, 95, 95, 94, 94, 93, 92, 92, 91, 91, 90, 90, 89, 89, 88, 88, 87, 86, 86, 85, 85, 84, 83, 83, 82, 82, 81, 80, 80, 79, 79, 78, 77, 77, 76, 75, 75, 74, 73, 73, 72, 71, 70, 70, 69, 68, 67, 67, 66, 65, 64, 63, 62, 61, 61, 60, 59, 58, 57, 56, 55, 54, 52, 51, 50, 49, 47, 46, 44, 43, 41, 39, 37, 35, 32, 29, 26, 20, 0 };
const unsigned char firing_map[] =   {255, 238, 232, 227, 224, 221, 218, 216, 214, 212, 210, 208, 207, 205, 204, 202, 201, 200, 198, 197, 196, 195, 194, 193, 192, 191, 190, 189, 188, 187, 186, 185, 184, 183, 182, 181, 180, 180, 179, 178, 177, 176, 176, 175, 174, 173, 173, 172, 171, 170, 170, 169, 168, 168, 167, 166, 166, 165, 164, 164, 163, 162, 162, 161, 160, 160, 159, 158, 158, 157, 157, 156, 155, 155, 154, 154, 153, 152, 152, 151, 151, 150, 149, 149, 148, 148, 147, 147, 146, 145, 145, 144, 144, 143, 143, 142, 141, 141, 140, 140, 139, 139, 138, 138, 137, 136, 136, 135, 135, 134, 134, 133, 133, 132, 132, 131, 131, 130, 129, 129, 128, 128, 127, 127, 126, 126, 125, 125, 124, 124, 123, 123, 122, 121, 121, 120, 120, 119, 119, 118, 118, 117, 117, 116, 116, 115, 114, 114, 113, 113, 112, 112, 111, 111, 110, 110, 109, 108, 108, 107, 107, 106, 106, 105, 105, 104, 103, 103, 102, 102, 101, 101, 100, 99, 99, 98, 98, 97, 96, 96, 95, 95, 94, 93, 93, 92, 92, 91, 90, 90, 89, 89, 88, 87, 87, 86, 85, 85, 84, 83, 83, 82, 81, 81, 80, 79, 78, 78, 77, 76, 75, 75, 74, 73, 72, 72, 71, 70, 69, 68, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 50, 49, 48, 46, 45, 44, 42, 40, 39, 37, 35, 33, 31, 28, 25, 22, 17, 11, 0};

//User function's prototypes
void set_fire_tresholds_buffer(unsigned char* data, unsigned char data_length);
unsigned char* get_fire_tresholds_buffer(void);
void dimmer_init (unsigned char* data, unsigned char data_length);

//ISR function's prototypes
inline void zc_isr(void);
inline void firing_timer_isr(void);
inline void freq_measuring_timer_overflow_isr(void);


#endif	/* DIMMER_H */
