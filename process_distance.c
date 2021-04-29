#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <sensors/VL53L0X/VL53L0X.h>

#include <process_distance.h>


// fonction pour que le robot se place devant le bon pattern du début 
// void good_start_position(void){


// 	while(GOOD_START == false) {
// 		if(VL53L0X_get_dist_mm > DISTANCE_THRESHOLD) {
// 			GOOD_START = false;
// 		}
// 		else {
// 			if
// 		}

// 	}
// }


// ToF deja configuré => si on veut changer le mode (continous/single/continuous timed) ou l'accuracy
// (default mode/high accuracy/long range/high spped) aller dans VL53L0X.c

// VL53L0X_start(); //si le ToF est déjà configuré : ne fait rien - sinon intialise la communication et configure le ToF dans une thread
// VL53L0X_stop(); // stop la thread
// VL53L0X_get_dist_mm(); // retourne la distance en mm

static THD_WORKING_AREA(waProcessDistanceThd, 512);
static THD_FUNCTION(ProcessDistanceThd, arg) {

	chRegSetThreadName("ProcessDistance Thd");
	(void)arg;

	uint16_t distance_feuille = 0;

    /* Reader thread loop.*/
    while (1) {

    	//distance_feuille = VL53L0X_get_dist_mm();
		//chprintf((BaseSequentialStream *)&SDU1, "distance_mm␣=␣%d␣\n", distance_feuille);

		chThdSleepMilliseconds(100);
    }
}



void process_distance_start(void){
	chThdCreateStatic(waProcessDistanceThd, sizeof(waProcessDistanceThd), NORMALPRIO, ProcessDistanceThd, NULL);
}





