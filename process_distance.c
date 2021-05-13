#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <sensors/VL53L0X/VL53L0X.h>
#include <sensors/proximity.h>
#include <motors.h>

#include <process_distance.h>



// ToF deja configuré => si on veut changer le mode (continous/single/continuous timed) ou l'accuracy
// (default mode/high accuracy/long range/high spped) aller dans VL53L0X.c

// VL53L0X_start(); //si le ToF est déjà configuré : ne fait rien - sinon intialise la communication et configure le ToF dans une thread
// VL53L0X_stop(); // stop la thread
// VL53L0X_get_dist_mm(); // retourne la distance en mm

//proximity sensors : get_calibrated_prox => à la lumiere ambiante prox = 0; plus on approche un objet plus la valeur augmente (jusqu'à > 1000)
static uint8_t turn_right = 0;
static uint8_t turn_left = 0;
static uint8_t good_position = 0;

void good_rotation(void){

	if(get_calibrated_prox(0) >= get_calibrated_prox(4) && get_calibrated_prox(2) >= get_calibrated_prox(5)) {
		turn_left = 1;
		turn_right = 0;
		good_position = 0; 
		if((get_calibrated_prox(5) == get_calibrated_prox(7) == 0) && (get_calibrated_prox(3) > get_calibrated_prox(7)))
			good_position = 1;
	} else if (get_calibrated_prox(0) <= get_calibrated_prox(4) && get_calibrated_prox(2) <= get_calibrated_prox(5)) {
		turn_left = 0;
		turn_right = 1;
		good_position = 0; 
	}

	if (good_position) {
		   left_motor_set_speed(0);
		   right_motor_set_speed(0);
	} else if(turn_right) {
			left_motor_set_speed(MOTOR_SPEED);
	    	right_motor_set_speed(-MOTOR_SPEED);
	} else if(turn_left) {
			left_motor_set_speed(-MOTOR_SPEED);
	    	right_motor_set_speed(MOTOR_SPEED);
	}
}

static THD_WORKING_AREA(waRotationThd, 512);
static THD_FUNCTION(RotationThd, arg) {

	chRegSetThreadName("Rotation Thd");
	(void)arg;

    /* Reader thread loop.*/
    while (1) {

		good_rotation();
		//chprintf((BaseSequentialStream *)&SDU1, "prox(0)␣=␣%d␣prox(2)␣=␣%d␣prox(3)␣=␣%d␣prox(4)␣=␣%dprox(5)␣=␣%dprox(7)␣=␣%d␣turn_right␣=␣%d␣turn_left␣=␣%d\n\r", 
			//get_calibrated_prox(0), get_calibrated_prox(2), get_calibrated_prox(3), get_calibrated_prox(4), get_calibrated_prox(5), get_calibrated_prox(7), turn_right, turn_left);
		chThdSleepMilliseconds(300);
    }
}


static THD_WORKING_AREA(waProcessDistanceThd, 512);
static THD_FUNCTION(ProcessDistanceThd, arg) {

	chRegSetThreadName("ProcessDistance Thd");
	(void)arg;

    /* Reader thread loop.*/
    while (1) {

		chprintf((BaseSequentialStream *)&SDU1, "prox[2]␣=␣%d␣\n prox[5]␣=␣%d␣\n ", get_calibrated_prox(2), get_calibrated_prox(5));
		chThdSleepMilliseconds(300);
    }
}

void rotation_start(void){
	chThdCreateStatic(waRotationThd, sizeof(waRotationThd), NORMALPRIO, RotationThd, NULL);
}

void process_distance_start(void){
	chThdCreateStatic(waProcessDistanceThd, sizeof(waProcessDistanceThd), NORMALPRIO, ProcessDistanceThd, NULL);
}






