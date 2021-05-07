#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <usbcfg.h>
#include <main.h>
#include <motors.h>
#include <camera/po8030.h>
#include <sensors/VL53L0X/VL53L0X.h>
#include <sensors/proximity.h>
#include <audio/audio_thread.h>
#include <audio/play_melody.h>
#include <msgbus/messagebus.h>
#include <chprintf.h>

#include <pi_regulator.h>
#include <process_image.h>
#include <robot_management.h>
#include <test_audio.h>

messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

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
    messagebus_init(&bus, &bus_lock, &bus_condvar);

    //starts the serial communication
    serial_start();
    //starts the USB communication
    usb_start();
    //starts the camera
    dcmi_start();
	po8030_start();
	//inits the motors
	motors_init();
	//starts the audio
	//dac_start();
	//starts the threads for the proximity sensors 
	proximity_start();
	calibrate_ir();

	//starts the thread for the ToF sensor
	VL53L0X_start();

	//starts the thread for the pi regulator
	//pi_regulator_start();
	//starts the thread for the robot
	robot_management_start();

	//process_image_start();

	
	//starts the thread for playing melodies, internals or externals
    //playMelodyStart();
    //test_audio_external();

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
