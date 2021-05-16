#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>


#include <main.h>
#include <p_regulator.h>

//simple P regulator implementation
int16_t pi_regulator(float distance, float goal){

	float error = 0;
	float speed = 0;

	static float sum_error = 0;

	error = distance - goal;

	//disables the P regulator if the error is to small
	//this avoids to always move as we cannot exactly be where we want
	if(fabs(error) < ERROR_THRESHOLD){
		return 0;
	}

	speed = KP * error;

    return (int16_t)speed;
}

