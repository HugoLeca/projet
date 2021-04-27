#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "play_melody.h"
#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <usbcfg.h>
#include <main.h>
#include <motors.h>
#include <camera/po8030.h>
#include <sensors/VL53L0X/VL53L0X.h>
#include <chprintf.h>

#include <pi_regulator.h>
#include <process_image.h>
#include <process_distance.h>
#include <test_audio.h>

void SendUint8ToComputer(uint8_t* data, uint16_t size) 
{
	chSequentialStreamWrite((BaseSequentialStream *)&SDU1, (uint8_t*)"START", 5);
	chSequentialStreamWrite((BaseSequentialStream *)&SDU1, (uint8_t*)&size, sizeof(uint16_t));
	chSequentialStreamWrite((BaseSequentialStream *)&SDU1, (uint8_t*)data, size);
}

static void serial_start(void)
{
	static SerialConfig ser_cfg = {
	    115200,
	    0,
	    0,
	    0,
	};

	sdStart(&SD3, &ser_cfg); // UART3.
}


int main(void)
{

    halInit();
    chSysInit();
    mpu_init();

    //starts the serial communication
    serial_start();
    //starts the USB communication
    usb_start();
    //starts the camera
    dcmi_start();
	po8030_start();
	//inits the motors
	//motors_init();

	//starts the threads for the pi regulator
	//pi_regulator_start();
	process_image_start();

	playMelodyStart();

    test_audio_external();

	
	
	//starts the thread for the ToF 
	VL53L0X_start();

	process_distance_start();

	//VL53L0X_stop();



    /* Infinite loop. */
    while (1) {

    	//waits 1 second
        chThdSleepMilliseconds(1000);

    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}